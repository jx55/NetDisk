#include "mytcpsocket.h"
#include <QDebug>
#include "opedb.h"
#include <stdio.h>
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>
#include  <QFile>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    // 关联信号
    connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg()));
    // 关联退出登录的信号(参数：发出信号的、信号源、接收信号的、处理信号的函数)
    connect(this,SIGNAL(disconnected()),this,SLOT(clientOffline()));

    m_bUpload = false; // 先将上传文件的状态置为 false
    m_pTimer = new QTimer;
    // 关联 定时器 与其信号槽
    connect(m_pTimer,SIGNAL(timeout()),this,SLOT(sendFileToClient()));

}

QString MyTcpSocket::getName()
{
    return m_strName;
}

// 文件夹的拷贝，源文件夹、目的文件夹
void MyTcpSocket::copyDir(QString strSourceDir, QString strDestinationDir)
{
    QDir dir;
    dir.mkdir(strDestinationDir); // 产生一个目录

    dir.setPath(strSourceDir);
    QFileInfoList fileInfoList = dir.entryInfoList();

    QString srcTmp,destTmp;
    for(int i=0; i<fileInfoList.size(); i++)
    {
        // 判断该目录下的每一个文件是常规文件还是目录，如果是常规文件直接拷贝，如果是目录的话进行递归拷贝
        if(fileInfoList[i].isFile())
        {
            srcTmp = strSourceDir + '/' + fileInfoList[i].fileName(); // 源地址
            destTmp = strDestinationDir + '/' + fileInfoList[i].fileName(); // 目的地址
            QFile::copy(srcTmp,destTmp);
        }
        else if(fileInfoList[i].isDir())
        {
            // 这里对文件夹的复制的递归的，并没有建立文件夹的操作，可能是在使用copy()函数复制文件的时候，使用的是路径，而如果其中涉及的文件夹不存在，则会自动建立
            if(QString(".") == fileInfoList[i].fileName() || QString("..") == fileInfoList[i].fileName())
            {
                continue;
            }
            srcTmp = strSourceDir + '/' + fileInfoList[i].fileName(); // 源地址
            destTmp = strDestinationDir + '/' + fileInfoList[i].fileName(); // 目的地址
            copyDir(srcTmp,destTmp);
        }
    }
}

void MyTcpSocket::recvMsg()
{
    // 如果上传文件的状态，就以PDU的方式进行；
    if(!m_bUpload)
    {
    //当前可读的数据有多少,但是这个可能会发生混乱，因为如果有两个数据一起过来的话，就可能有这种问题
    qDebug()<<this->bytesAvailable();
    //  读数据的时候先读PDU的大小，再读剩余的大小
    uint uiPDULen=0;
    this->read((char*)&uiPDULen,sizeof(uint));
    // PDU总的大小 - PDU单元的大小，即为实际消息的大小
    uint uiMsgLen=uiPDULen-sizeof(PDU);
    PDU *pdu=mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint),uiPDULen);
    // qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);

    // 收到数据之后，判断一下它的消息类型
    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_REQUEST: // 消息类型是注册
    {
        // 将收到的数据拷贝到数组中
        char caName[32]={'\0'};
        char caPwd[32]={'\0'};
        strncpy(caName,pdu->caData,32);
        // strncpy()参数：目的数组、源数组、复制的最大字符数
        strncpy(caPwd,pdu->caData+32,32);

        // 交给数据库处理请求，并接收返回值
        bool reg = OpeDB::getInstance().handleRegist(caName,caPwd);
        // 注册成功的返回信息
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType=ENUM_MSG_TYPE_REGIST_RESPOND; // 消息类型是注册回复
        if(reg) // 返回值为真，即注册成功
        {
            strcpy(respdu->caData,REGIST_OK);

            // 用户注册成功的时候为其创建文件夹
            QDir dir;
            // 以用户名作为根目录名字
            dir.mkdir(QString("./%1").arg(caName));
            // qDebug() << caName << "Current directory:" << QDir::currentPath(); // 打印创建的文件夹的绝对路径
        }
        else // 注册失败
        {
            strcpy(respdu->caData,REGIST_FAILED);           
        }

        // 通过socket发送出去，这里不需要m_tcpSocket，因为已经在tcpSocket里面了
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;

        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST: // 消息类型是登录
    {
        // 获得用户名、密码
        char caName[32]={'\0'};
        char caPwd[32]={'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);

        // 服务器接收到客户端发送的登陆请求，需要在数据库中进行核验，相关的函数在opedb.cpp中实现
        bool ret = OpeDB::getInstance().handleLogin(caName,caPwd);

        PDU *respdu = mkPDU(0);
        respdu->uiMsgType=ENUM_MSG_TYPE_LOGIN_RESPOND; // 消息类型是登陆回复
        if(ret) // 返回值为真，即登陆成功
        {
            strcpy(respdu->caData,LOGIN_OK);
            // 登录成功的时候记录用户名
            m_strName=caName;
        }
        else // 登陆失败
        {
            strcpy(respdu->caData,LOGIN_FAILED);
        }

        // 通过socket发送出去，这里不需要m_tcpSocket，因为已经在tcpSocket里面了
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;

        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST: // 查看所有在线用户的请求
    {
        // 接收到数据库查询的结果
        QStringList ret=OpeDB::getInstance().handleAllOnline();
        // 用一个多大的PDU来存这些名字呢？
        uint uiMsgLen = ret.size()*32; // 即消息部分的长度
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;

        // 将接收到的用户名循环拷贝到respdu中
        for(int i=0; i<ret.size(); i++)
        {
            memcpy((char*)(respdu->caMsg)+i*32, // 目的地址
                   ret.at(i).toStdString().c_str(), // 要拷贝数据的首地址
                   ret.at(i).size()); // 要拷贝数据的大小
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USER_REQUEST:// 搜索用户
    {
        int ret = OpeDB::getInstance().handleSearchUser(pdu->caData);
        qDebug() <<"ret: "<<ret<<"pdu->caData: "<< pdu->caData;
        PDU *respdu = mkPDU(0); // 产生pdu，只需要指定
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_RESPOND; // 设置消息类型
        // 根据数据库的结果，对respdu->caData进行赋值
        if(-1 == ret){
            strcpy(respdu->caData,SEARCH_USER_NO);
        }
        else if(1 == ret){
            qDebug() << "mytcpsocket.cpp SEARCH_USER_ONLINE";
            strcpy(respdu->caData,SEARCH_USER_ONLINE);
        }
        else if(0 == ret){
            qDebug() << "mytcpsocket.cpp SEARCH_USER_OFFLINE";
            strcpy(respdu->caData,SEARCH_USER_OFFLINE);
        }

        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: // 添加好友请求的处理
    {
        // 将收到的数据拷贝到数组中
        char caPerName[32]={'\0'};
        char caName[32]={'\0'};
        strncpy(caPerName,pdu->caData,32);
        // strncpy()参数：目的数组、源数组、复制的最大字符数
        strncpy(caName,pdu->caData+32,32);

        int ret = OpeDB::getInstance().handleAddFriend(caPerName,caName);
        PDU *respdu = NULL;
        // 分情况进行处理
        if(-1 == ret){ // 未知错误
            respdu=mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,UNKNOW_ERROR);

            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
        }
        else if(0 == ret){ // 用户已经是好友
            respdu=mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,EXISTED_FRIEND);

            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
        }
        else if(1 == ret){ // 用户在线，即需要转发给对方
            qDebug() << "add friend successful";
            MyTcpServer::getInstance().resend(caPerName,pdu);
        }
        else if(2 == ret){ // 不在线
            respdu=mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,ADD_FRIEND_OFFLINE);

            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
        }
        else if(3 == ret){ // 用户不存在
            qDebug() << "mytcpsocket.cpp ret==3 user not exist" << ret;
            respdu=mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,ADD_FRIEND_NO_EXIST);

            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE: // 同意添加好友
    {
        // 将收到的数据拷贝到数组中
        char caPerName[32]={'\0'};
        char caName[32]={'\0'};
        strncpy(caPerName,pdu->caData,32);
        // strncpy()参数：目的数组、源数组、复制的最大字符数
        strncpy(caName,pdu->caData+32,32);
        OpeDB::getInstance().handleAgreeAddFriend(caPerName,caName);

        MyTcpServer::getInstance().resend(caName,pdu);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: // 拒绝添加好友
    {
        char caName[32]={'\0'};
        strncpy(caName,pdu->caData+32,32);
        MyTcpServer::getInstance().resend(caName,pdu);
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST: // 处理刷新好友的请求
    {
        char caName[32]={'\0'};
        strncpy(caName,pdu->caData,32);
        // qDebug() <<"mytcpsocket.cpp ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST: "<<caName;
        QStringList ret = OpeDB::getInstance().handleFlushFriend(caName);

        uint uiMsgLen = ret.size()*32;
        PDU *respdu = mkPDU(uiMsgLen); // 消息部分的大小为uiMsgLen
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
        // 将所有的名字拷贝到实际消息部分
        for(int i=0;i<ret.size();i++)
        {
            memcpy((char*)(respdu->caMsg)+i*32,ret.at(i).toStdString().c_str(),ret.at(i).size());
        }

        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: // 处理删除好友的请求
    {
        qDebug() << "mytcpsocket.cpp delete friend request.";
        char caSelfName[32]={'\0'};
        char caFriendName[32]={'\0'};
        strncpy(caSelfName,pdu->caData,32);
        strncpy(caFriendName,pdu->caData+32,32);
        // getInstance()来获得对象，通过对象来调用
        OpeDB::getInstance().handleDelFriend(caSelfName,caFriendName);

        // 先给A回复（给删除好友请求的客户端 回复）
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
        // 将提示信息放在caData中
        strcpy(respdu->caData,DEL_FRIEND_OK);

        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        // 回复给被删除的好友
        MyTcpServer::getInstance().resend(caFriendName,pdu);
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
        // 将提示信息放在caData中
        strcpy(respdu->caData,DEL_FRIEND_OK);

        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: // 处理私聊请求
    {
        // 找到对方的Socket，再进行转发信息
        char caPerName[32] = {'\0'};
        memcpy(caPerName,pdu->caData+32,32);
        MyTcpServer::getInstance().resend(caPerName,pdu); // 转发给另一个客户端

        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: // 群聊请求的处理
    {
        // 获得 发送方 所有的在线好友
        char caName[32]={'\0'};
        strncpy(caName,pdu->caData,32);
        QStringList onlineFriend = OpeDB::getInstance().handleFlushFriend(caName);

        // 进行转发，遍历所有好友
        for(int i=0; i<onlineFriend.size(); i++)
        {

            QString tmp = onlineFriend.at(i);
            // qDebug() << tmp;
            // 转发是原样进行转发，所有内容都不变，包括消息类型
            MyTcpServer::getInstance().resend(tmp.toStdString().c_str(),pdu);
        }

        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_REQUEST: // 创建文件夹请求
    {
        QDir dir;
        QString strCurPath = QString("%1").arg((char*)(pdu->caMsg));

        bool ret = dir.exists(strCurPath);
        PDU *respdu = NULL;
        if (ret) // 当前目录存在
        {
            // 则检查当前目录下是否已经有了要创建的文件夹名，已存在则创建新文件夹失败
            // 取出要创建的 新文件夹名字
            char caNewDirName[32] = {'\0'};
            memcpy(caNewDirName,pdu->caData+32,32);
            QString strNewPath = strCurPath+'/'+caNewDirName;
            // qDebug() << strNewPath;

            // 检查要创建的文件夹是否存在
            ret = dir.exists(strNewPath);
            if (ret) // 则该文件夹已经存在
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND; // 消息类型
                strcpy(respdu->caData,FILE_NAME_EXIST); // 回复的内容
            }
            else // 创建的文件夹不存在，则可以继续创建
            {
                dir.mkdir(strNewPath);

                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND; // 消息类型
                strcpy(respdu->caData,CREATE_DIR_OK); // 回复的内容
            }
        }
        else // 当前目录不存在，则新文件夹创建失败
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND; // 消息类型
            strcpy(respdu->caData,DIR_NO_EXIST); // 回复的内容
        }

        // 将制作好的pdu发送给客户端
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST: // 刷新文件请求
    {
        // 获取当前所在路径
        // 为什么这里要用char*呢？为什么不用QString呢？
        // 因为QString在定义的时候是没有空间的
        // QString strCurPath =
        char *pCurPath = new char[pdu->uiMsgLen]; // 用到实际消息长度字段
        memcpy(pCurPath,pdu->caMsg,pdu->uiMsgLen); // 从pdu中拷贝当前地址

        QDir dir(pCurPath); // 这里的参数是当前地址，不给参数的话会以当前文件位置作为地址
        QFileInfoList fileInfoList = dir.entryInfoList();

        // 文件个数
        int iFileCount = fileInfoList.size();
        // pdu的大小为每个文件的大小*文件个数，
        // 这里-2是因为去掉了.和..的目录
        PDU *respdu = mkPDU(sizeof(FileInfo)*iFileCount);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;

        FileInfo *pFileInfo = NULL;
        QString strFileName; // 存放 文件名字
        for(int i=0;i<fileInfoList.size();i++)
        {
            // 目录为.和..直接跳过
            // if(QString(".") == fileInfoList[i].fileName()
            //     || QString("..") == fileInfoList[i].fileName())
            // {
            //     continue;
            // }

            // FileInfo类型+1，即加了一个FileInfo结构体的大小
            pFileInfo = (FileInfo*)(respdu->caMsg)+i;
            strFileName = fileInfoList[i].fileName();
            memcpy(pFileInfo->caName,strFileName.toStdString().c_str(),strFileName.size());
            if(fileInfoList[i].isDir())
            {
                pFileInfo->iFileType = 0; // 表示是一个文件夹
            }
            else if(fileInfoList[i].isFile())
            {
                pFileInfo->iFileType = 1; // 表示是一个常规文件
            }

            /*
             * 打印测试
            qDebug() << fileInfoList[i].fileName()
                     << fileInfoList[i].size()
                     << "文件夹：" << fileInfoList[i].isDir()  // 是否是文件夹
                     << "常规文件：" <<fileInfoList[i].isDir(); // 是否是常规文件
            */
        }

        // 发送给客户端
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        break;
    }
    case ENUM_MSG_TYPE_DEL_DIR_REQUEST: // 删除文件夹 请求
    {
        char caName[32] = {'\0'};
        strcpy(caName,pdu->caData);
        char *pPath = new char[pdu->uiMsgLen];
        memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

        // 拼接路径
        QString strPath = QString("%1/%2").arg(pPath).arg(caName);
        qDebug() << pPath << "--" << caName << "--" << strPath;

        // 删除文件夹之前先检查类型 是否是文件夹
        QFileInfo fileInfo(strPath);
        bool ret = false; // 记录是否删除成功
        if(fileInfo.isDir()) // 类型是 文件夹，则可以删除
        {
            QDir dir;
            dir.setPath(strPath);
            // removeRecursively()会删除该文件以及其下的所有文件
            ret = dir.removeRecursively();
        }
        else if(fileInfo.isFile()) // 类型是 常规文件，则不能用该文件删除
        {
            ret = false;
        }

        // 产生一个回复的PDU
        PDU *respdu = NULL;
        if(ret) // 删除成功
        {
            respdu = mkPDU(strlen(DEL_DIR_OK)+1);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
            memcpy(respdu->caData,DEL_DIR_OK,strlen(DEL_DIR_OK));
        }
        else // 删除失败
        {
            respdu = mkPDU(strlen(DEL_DIR_FAILURED)+1);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
            memcpy(respdu->caData,DEL_DIR_FAILURED,strlen(DEL_DIR_FAILURED));
        }

        // 将打包好的pdu发送给客户端
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        break;
    }
    case ENUM_MSG_TYPE_RENAME_FILE_REQUEST: // 重命名文件 请求
    {
        char caNewName[32] = {'\0'};
        char caOldName[32] = {'\0'};
        strncpy(caNewName,pdu->caData,32);
        strncpy(caOldName,pdu->caData+32,32);

        char *pPath = new char[pdu->uiMsgLen];
        memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

        // 拼接两条路径，分别是旧路径、新路径
        QString strOldName = QString("%1/%2").arg(pPath).arg(caOldName);
        QString strNewName = QString("%1/%2").arg(pPath).arg(caNewName);

        // 进行重命名
        QDir dir;
        // 用一个布尔值来接收其返回值
        bool ret = dir.rename(strOldName,strNewName);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
        if(ret)
        {
            strcpy(respdu->caData,RENAME_FILE_OK);
        }
        else
        {
            strcpy(respdu->caData,RENAME_FILE_FAILURED);
        }

        // 将打包好的pdu发送给客户端
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        break;
    }
    case ENUM_MSG_TYPE_ENTER_DIR_REQUEST: // 双击 进入文件夹 请求
    {
        // 获得名字
        char caEnterName[32] = {'\0'};
        strncpy(caEnterName,pdu->caData,32);

        // 获得当前目录
        char *pPath = new char[pdu->uiMsgLen];
        memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

        // 拼接路径
        QString strPath = QString("%1/%2").arg(pPath).arg(caEnterName);
        QFileInfo fileInfo(strPath);
        PDU *respdu = NULL;
        // 判断是否是文件夹
        if(fileInfo.isDir())
        {
            // 这里的代码与刷新文件的代码是一样的
            QDir dir(strPath); // 这里的参数是当前地址，不给参数的话会以当前文件位置作为地址
            QFileInfoList fileInfoList = dir.entryInfoList();

            // 文件个数
            int iFileCount = fileInfoList.size();
            // pdu的大小为每个文件的大小*文件个数
            PDU *respdu = mkPDU(sizeof(FileInfo)*iFileCount);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;

            FileInfo *pFileInfo = NULL;
            QString strFileName; // 存放 文件名字
            for(int i=0;i<fileInfoList.size();i++)
            {
                // FileInfo类型+1，即加了一个FileInfo结构体的大小
                pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                strFileName = fileInfoList[i].fileName();
                memcpy(pFileInfo->caName,strFileName.toStdString().c_str(),strFileName.size());
                if(fileInfoList[i].isDir())
                {
                    pFileInfo->iFileType = 0; // 表示是一个文件夹
                }
                else if(fileInfoList[i].isFile())
                {
                    pFileInfo->iFileType = 1; // 表示是一个常规文件
                }
            }

            // 发送给客户端
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(fileInfo.isFile())
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
            strcpy(respdu->caData,ENTER_DIR_FAILURED);

            // 将打包好的pdu发送给客户端
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }

        break;
    }
    case ENUM_MSG_TYPE_DEL_REG_FILE_REQUEST: // 删除文件 请求
    {
        char caName[32] = {'\0'};
        strcpy(caName,pdu->caData);
        char *pPath = new char[pdu->uiMsgLen];
        memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);

        // 拼接路径
        QString strPath = QString("%1/%2").arg(pPath).arg(caName);
        qDebug() << pPath << "--" << caName << "--" << strPath;

        // 删除文件之前先检查类型 是否是常规文件
        QFileInfo fileInfo(strPath);
        bool ret = false; // 记录是否删除成功
        if(fileInfo.isDir()) // 类型是 文件夹
        {
            ret = false;
        }
        else if(fileInfo.isFile()) // 类型是 常规文件
        {
            QFile qfile;
            ret = qfile.remove(strPath);
            qDebug() << ret;
        }

        // 产生一个回复的PDU
        PDU *respdu = NULL;
        if(ret) // 删除成功
        {
            respdu = mkPDU(strlen(DEL_FILE_OK)+1);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_REG_FILE_RESPOND;
            memcpy(respdu->caData,DEL_FILE_OK,strlen(DEL_FILE_OK));
        }
        else // 删除失败
        {
            respdu = mkPDU(strlen(DEL_FILE_FAILURED)+1);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_REG_FILE_RESPOND;
            memcpy(respdu->caData,DEL_FILE_FAILURED,strlen(DEL_FILE_FAILURED));
        }

        // 将打包好的pdu发送给客户端
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST: // 上传文件 请求
    {
        char caFileName[32] = {'\0'};
        qint64 fileSize = 0;
        sscanf(pdu->caData,"%s %lld",caFileName,&fileSize);
        // qDebug() << caFileName << "--" <<  fileSize;

        char *pPath = new char[pdu->uiMsgLen];
        memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
        // qDebug() << pPath;

        // 拼接路径
        QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
        qDebug() << pPath << "--" << caFileName << "--" << strPath;

        delete []pPath;
        pPath = NULL;

        m_file.setFileName(strPath);
        // 以只写的方式打开文件，若文件不存在，则会自动创建文件
        if(m_file.open(QIODevice::WriteOnly))
        {
            m_bUpload = true; // 是上传文件的状态
            m_iTotal = fileSize;
            m_iRecved = 0;
        }

        break;
    }
    case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST: // 下载文件 请求
    {
        char caFileName[32] = {'\0'};
        strcpy(caFileName,pdu->caData);
        char *pPath = new char[pdu->uiMsgLen];
        memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
        // 拼接路径
        QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
        // qDebug() << pPath << "--" << caFileName << "--" << strPath;

        delete []pPath;
        pPath = NULL;

        // 先发送一个文件的大小给客户端，否则客户端无法判断是否接收结束了
        QFileInfo fileInfo(strPath);
        qint64 fileSize = fileInfo.size();
        PDU *respdu = mkPDU(0);
        // 存放文件的名字及大小
        sprintf(respdu->caData,"%s %lld",caFileName,fileSize);
        respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;

        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        // 给了客户端回复后，打开文件，启动计时器
        m_file.setFileName(strPath);
        m_file.open(QIODevice::ReadOnly); // 以只读的方式打开

        m_pTimer->start(1000);

        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_REQUEST: // 分享文件 请求
    {
        // 给客户端显示谁发送的文件，接收端有两种选择，接收或者不接收
        // 接收则将文件拷贝到接收者的目录中
        char caSendName[32] = {'\0'}; // 发送者的名字
        int num = 0;
        sscanf(pdu->caData,"%s %d",caSendName,&num); // 一定记得有&
        int Size = num*32;

        PDU *respdu = mkPDU(pdu->uiMsgLen-Size);
        respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
        strcpy(respdu->caData,caSendName); // 发送者
        memcpy(respdu->caMsg,(char*)(pdu->caMsg)+Size,pdu->uiMsgLen-Size); // 所发送文件的地址

        // 打印服务器接收到客户端的请求信息
        // qDebug() << "mytcpsocket.cpp: " << caSendName << ' ' <<num;

        char caRecvName[32] = {'\0'};
        for(int i=0; i<num; i++) // 循环获得接收者的名字，然后依次转发给接收者
        {
            memcpy(caRecvName,(char*)(pdu->caMsg)+i*32,32);
            MyTcpServer::getInstance().resend(caRecvName,respdu);
        }
        free(respdu);
        respdu = NULL;

        // 再回复给发送者一个信息
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
        strcpy(respdu->caData,"share file ok!");
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND: // 接收文件 通知请求的处理
    {
        QString strRecvPath = QString("./%1").arg(pdu->caData); // 接收者的名字即为要存放该文件的路径
        QString strShareFilePath = QString("%1").arg((char*)(pdu->caMsg)); // 文件完整地址
        int index = strShareFilePath.lastIndexOf('/');
        QString strFileName = strShareFilePath.right(strShareFilePath.size()-index-1); // 获取文件名
        strRecvPath = strRecvPath + '/' + strFileName;

        qDebug() << "mytcpsocket.cpp share file note respond: " << strRecvPath << ' ' << strShareFilePath;

        QFileInfo fileInfo(strShareFilePath);
        if(fileInfo.isFile()) // 如果是常规文件
        {
            QFile::copy(strShareFilePath,strRecvPath);
        }
        else if(fileInfo.isDir())
        {
            copyDir(strShareFilePath,strRecvPath);
        }
        break;
    }
    case ENUM_MSG_TYPE_MOVE_FILE_REQUEST: // 移动文件 请求
    {
        char caFileName[32] = {'\0'};
        int srcLen=0,destLen=0;
        sscanf(pdu->caData,"%d %d %s",&srcLen,&destLen,caFileName);
        char *strSrcPath = new char[srcLen+1];
        char *strDestPath = new char[destLen+1+32]; // strDestPath只是一个文件夹，后面需要拼上文件名，得到完整路径
        memset(strSrcPath,'\0',srcLen+1); // 先清空数组
        memset(strDestPath,'\0',destLen+1);
        memcpy(strSrcPath,(char*)(pdu->caMsg),srcLen);
        memcpy(strDestPath,(char*)(pdu->caMsg)+srcLen+1,destLen);

        qDebug() << strSrcPath << ' ' << strDestPath;

        PDU *respdu = mkPDU(0);
        QFileInfo fileInfo(strDestPath);
        if(fileInfo.isFile()) // 如果目的地址是常规文件
        {
            strcpy(respdu->caData,MOVE_FILE_FAILURED);
        }
        else if(fileInfo.isDir())
        {
            strcat(strDestPath,"/");
            strcat(strDestPath,caFileName); // 拼接完整路径

            qDebug() << strSrcPath << ' ' << strDestPath;

            bool ret = QFile::rename(strSrcPath,strDestPath);
            if(ret) // 如果ret为真，则移动成功
            {
                strcpy(respdu->caData,MOVE_FILE_OK);
            }
            else // 则移动失败
            {
                strcpy(respdu->caData,COMMON_ERR);
            }
        }
        respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;

        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    default:
        break;
    }

    free(pdu);
    pdu = NULL;

    // qDebug()<< caName << caPwd << pdu->uiMsgType;
    }
    else // 如果是上传文件的状态，就以二进制的方式进行，不封装到PDU中
    {
        qDebug() << "mytcpsocket.cpp read byte file!";
        PDU *respdu = NULL;
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;

        QByteArray buff = readAll();
        m_file.write(buff);
        m_iRecved += buff.size();

        if(m_iTotal == m_iRecved)
        {
            m_file.close();
            m_bUpload = false; // 接收完将上传标志置为false
            strcpy(respdu->caData,UPLOAD_FILE_OK);

            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(m_iTotal < m_iRecved)
        {
            m_file.close();
            m_bUpload = false; // 接收完将上传标志置为false
            strcpy(respdu->caData,UPLOAD_FILE_FAILURED);

            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        // 其余情况（例如文件未传输完成）应该是继续传输的

    }
}

void MyTcpSocket::clientOffline()
{
    // 在线状态设置为非在线状态
    // 先转换为C++的string类型，再转为char*类型
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    qDebug() << "clientOffline" << m_strName;

    // 删除列表中的socket
    // 列表中socket和server如何产生关联呢？可以通过信号槽来产生关联
    // 在mytcpsocket.h中创建信号，然后在更改数据库状态之后，就将这个信号发送出去
    // emit用于发送信号，this保存的就是当前对象的地址；需要在tcpserver中定义一个槽函数与之进行关联
    emit offline(this);
}

// 发送文件 给客户端
void MyTcpSocket::sendFileToClient()
{
    // 关闭计时器，否则重新开始计时
    m_pTimer->stop();

    qDebug() << "sendFileToClient:" << m_file.isOpen();

    char *pData = new char[4096];
    qint64 ret = 0; // 记录已经读取的文件大小
    while(true) // 循环读取文件
    {
        ret = m_file.read(pData,4096);
        if(ret>0 && ret<=4096)
        {
            write(pData,ret); // 发送给客户端
        }
        else if(0 == ret) // 读写结束
        {
            m_file.close();
            break;
        }
        else if(ret<0)
        {
            qDebug() << "发送文件内容给客户端过程中失败！";
            m_file.close();
            break;
        }
    }
    // qDebug() << "send to client finish!";
    delete []pData;
    pData = NULL;
}


















