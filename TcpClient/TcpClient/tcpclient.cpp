#include "tcpclient.h"
#include "ui_tcpclient.h"

#include <QMessageBox>
#include <QByteArray>
#include <QDebug>
#include <QHostAddress>

#include "protocol.h"
#include "privatechat.h"
#include <QFile>

#include <QPalette>
#include <QBrush>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent), ui(new Ui::TcpClient)

{
    ui->setupUi(this);
    // 设置界面大小
    resize(300,200);
    this->setWindowTitle("登录");

    // QWidget *window = new QWidget;
    this->setStyleSheet("QWidget { background-image: url(:/image/back.png); }");


    // 加载配置文件
    loadConfig();

    // 使用connect()函数来进行关联
    // 第一个参数是信号的发送者，第二个参数是信号的类型,connected()即连接服务器成功后就会自动发出的
    // 第三个参数是信号的接收方，第四个参数是用哪个函数来处理
    connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));
    connect(&m_tcpSocket,SIGNAL(readyRead()),this,SLOT(recvMsg()));

    //连接服务器
    // 产生一个QHostAddress的对象，将服务器的IP作为参数
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);
}

// 析构函数
TcpClient::~TcpClient()
{
    delete ui;
}

// 加载配置信息
void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    // 指定打开方式，返回一个布尔值
    if(file.open(QIODevice::ReadOnly)){
        // 成功打开,使用readAll()方法读取所有数据
        // 返回的是一个QByteArray对象，需要加头文件
        QByteArray baData=file.readAll();

        //转换类型，不过不能直接转换为String，只能转换为char*
        QString strData=baData.toStdString().c_str();

        // 调用后才会有输出，所以要在前面调用
        qDebug()<<strData;

        // 输出结果如下，要对结果进行解析
        // "127.0.0.1\r\n8888\r\n"
        strData.replace("\r\n"," ");
        qDebug()<<strData;

        // 切分字符串
        QStringList strList=strData.split(" ");
        // for(int i=0;i<strList.size();i++){
        //     qDebug()<<"-->"<<strList[i];
        // }

        m_strIP=strList[0];
        m_usPort=strList[1].toUShort();
        qDebug()<<m_strIP<<":"<<m_usPort;

        // 文件用完了关闭掉
        file.close();
    }
    else{
        // 打开失败
        QMessageBox::critical(this,"open config","open config failed");
    }
}

// 单例模式
TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

// 无论哪里需要用到这个TcpSocket，只需要调用这个函数就可以拿到返回的tcpSocket
QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::loginName()
{
    return m_strLoginName;
}

// 返回 当前的路径
QString TcpClient::curPath()
{
    return m_strCurPath;
}

// 设置当前目录
void TcpClient::setCurPath(QString strCurPath)
{
    m_strCurPath = strCurPath;
}

// 需要关联 信号 和 信号的处理函数，要不然他们之间不会产生联系
void TcpClient::showConnect()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}

#if 0
// 点击发送就会跳到这里来
void TcpClient::on_sendpb_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        // 发送的数据要封装到协议数据单元中
        PDU *pdu = mkPDU(strMsg.size());
        pdu->uiMsgType=8888; // 随便指定一个消息类型
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
    else
    {
        QMessageBox::warning(this,"信息发送","发送的消息不能为空");
    }
}
#endif

// 点击登录按钮
void TcpClient::on_login_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();
    // 用户名、密码都不为空
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        // 保存登陆时的用户名，后面加好友的时候会用到自己的名字
        m_strLoginName = strName;

        // 正确输入了用户名和密码，就将其写入到pdu中
        // 用户名和密码各占32位，pdu结构体中预留的是64，所以是满足的
        // 注册时实际消息的部分为0，所以分配的空间为0即可
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;

        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        // 从后面32位开始放密码
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);

        // 通过socket发送出去
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
    else // 即有一个为空
    {
        QMessageBox::critical(this,"登陆","登陆失败：用户名或密码为空");
    }
}

// 点击注册按钮
void TcpClient::on_regist_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();
    // 用户名、密码都不为空
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        // 正确输入了用户名和密码，就将其写入到pdu中
        // 用户名和密码各占32位，pdu结构体中预留的是64，所以是满足的
        // 注册时实际消息的部分为0，所以分配的空间为0即可
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;

        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        // 从后面32位开始放密码
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);

        // 通过socket发送出去
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
    else // 即有一个为空
    {
        QMessageBox::critical(this,"注册","注册失败：用户名或密码为空");
    }
}

// 客户端接收消息
void TcpClient::recvMsg()
{
    //  如果不是下载文件的过程，就用PDU的方式来处理
    if(!OpeWidget::getInstance().getBook()->getDownloadStatus())
    {
    //当前可读的数据有多少,但是这个可能会发生混乱，因为如果有两个数据一起过来的话，就可能有这种问题
    // qDebug()<<m_tcpSocket.bytesAvailable();
    // 读数据的时候先读PDU的大小，再读剩余的大小
    uint uiPDULen=0;
    m_tcpSocket.read((char*)&uiPDULen,sizeof(uint));
    // PDU总的大小 - PDU单元的大小，即为实际消息的大小
    uint uiMsgLen=uiPDULen-sizeof(PDU);
    PDU *pdu=mkPDU(uiMsgLen);
    m_tcpSocket.read((char*)pdu+sizeof(uint),uiPDULen);
    // qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);

    // 收到数据之后，判断一下它的消息类型
    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_RESPOND: // 消息类型是 注册
    {
        // 看一下回复的消息是成功还是失败
        if(0 == strcmp(pdu->caData,REGIST_OK)) // 成功
        {
            QMessageBox::information(this,"注册",REGIST_OK);
        }
        else if(0 == strcmp(pdu->caData,REGIST_FAILED))
        {
            QMessageBox::warning(this,"注册",REGIST_FAILED);
        }
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_RESPOND: // 消息类型是 登陆
    {
        // 看一下回复的消息是成功还是失败
        if(0 == strcmp(pdu->caData,LOGIN_OK)) // 成功
        {
            // 登录成功后，就以根目录作为当前目录
            m_strCurPath = QString("./%1").arg(m_strLoginName);

            // qDebug() << LOGIN_OK;
            QMessageBox::information(this,"登录",LOGIN_OK);
            // 登录成功就显示好友、图书界面
            OpeWidget::getInstance().show();
            // 显示好友图书界面的同时应该隐藏登录的界面
            this->hide();
        }
        else if(0 == strcmp(pdu->caData,LOGIN_FAILED))
        {
            qDebug() << LOGIN_FAILED;
            QMessageBox::warning(this,"登录",LOGIN_FAILED);
        }
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND: // 消息类型是 查看在线用户
    {
        // getInstance()是操作界面，getFriend()是获得好友界面，通过好友界面的showAllOnlineUser()将pdu传过去
        OpeWidget::getInstance().getFriend()->showAllOnlineUser(pdu);
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USER_RESPOND: // 消息类型是 搜索用户
    {
        // 这里的0指的是比较的结果，对应的并不是用户不存在、用户在线、用户不在线
        if(0 == strcmp(SEARCH_USER_NO,pdu->caData))
        {
            QMessageBox::information(this,"搜索",QString("%1: not exist").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
        }
        else if(0 == strcmp(SEARCH_USER_ONLINE,pdu->caData))
        {
            // qDebug()  << "tcpclient.cpp search user online";
            QMessageBox::information(this,"搜索",QString("%1: online").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
        }
        else if(0 == strcmp(SEARCH_USER_OFFLINE,pdu->caData))
        {
            // qDebug() << "tcpclient.cpp search user offline";
            QMessageBox::information(this,"搜索",QString("%1: offline").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: // 消息类型是 添加好友请求；服务器的转发
    {
        char caName[32]={'\0'};
        strncpy(caName,pdu->caData+32,32);
        int ret = QMessageBox::information(this,"添加好友",
                                           QString("%1 want to add you as friend?").arg(caName),
                                           QMessageBox::Yes,QMessageBox::No);

        PDU *respdu = mkPDU(0);
        // 根据返回值 Yes/No 来处理
        if(QMessageBox::Yes == ret) // 同意添加好友
        {
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;
        }
        else if(QMessageBox::No == ret) // 不同意添加好友
        {
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
        }
        // 发送消息
        m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: // 消息类型是 添加好友回复；服务器的回复
    {
        QMessageBox::information(this,"添加好友",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE: // 同意添加好友
    {
        char caPerName[32]={'\0'};
        strncpy(caPerName,pdu->caData,32);
        QMessageBox::information(this,"添加好友",""); // 这里未写完
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: // 拒绝添加好友
    {
        char caPerName[32]={'\0'};
        strncpy(caPerName,pdu->caData,32);
        QMessageBox::information(this,"添加好友",""); // 这里未写完
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND: // 刷新好友的回复
    {
        OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: // 删除好友请求
    {
        // 谁 把好友关系 删除掉了
        char caName[32] = {'\0'};
        memcpy(caName,pdu->caData,32);
        QMessageBox::information(this,"删除好友",QString("%1 删除你作为它的好友").arg(caName));
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND: // 删除好友回复
    {
        // 直接打印删除好友成功
        QMessageBox::information(this,"删除好友","删除好友成功");
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: // 私聊请求的处理
    {
        // 客户端收到从服务器转发来的信息，要将其展示在聊天窗口
        if(PrivateChat::getInstance().isHidden()) // 如果是隐藏的就将其显示出来，再刷新其信息
        {
            PrivateChat::getInstance().show();
        }
        // 隐藏的则先显示出来，再做下面的操作
        char caSendName[32] = {'\0'};
        memcpy(caSendName,pdu->caData,32); // 获得发送方的名字
        QString strSendName = caSendName;
        PrivateChat::getInstance().setChatName(strSendName);

        PrivateChat::getInstance().updateMsg(pdu);

        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: // 群聊请求的处理
    {
        // 客户端收到服务器转发的群聊，将其显示在界面即可（与私聊不是同一个界面
        // 因为 friend不是单例模式，所以需要通过OpeWidget对象来获得
        // OpeWidget::getInstance().getFriend()获得friend窗口，然后调用它里面的函数
        OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);

        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_RESPOND: // 创建文件夹 的回复
    {
        // 服务器封装了返回信息在pdu->caData中，这里直接展示其中的内容即可
        QMessageBox::information(this,"创建文件",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND: // 刷新文件 的回复
    {
        OpeWidget::getInstance().getBook()->updateFileList(pdu);

        // 修改进入的文件夹名字
        QString strEnterDir = OpeWidget::getInstance().getBook()->enterDir();
        if(!strEnterDir.isEmpty())
        {
            m_strCurPath = m_strCurPath + "/" + strEnterDir;
            qDebug() << m_strCurPath;
        }

        break;
    }
    case ENUM_MSG_TYPE_DEL_DIR_RESPOND: // 删除文件夹 的回复
    {
        // 将服务器发来的数据 直接作为提示内容 即可
        QMessageBox::information(this,"删除文件夹",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_RENAME_FILE_RESPOND: // 重命名 回复
    {
        QMessageBox::information(this,"重命名文件",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_ENTER_DIR_RESPOND: // 进入文件夹 回复
    {
        OpeWidget::getInstance().getBook()->clearEnterDir();
        QMessageBox::information(this,"进入文件夹",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_DEL_REG_FILE_RESPOND: // 删除文件 回复
    {
        // 将服务器发来的数据 直接作为提示内容 即可
        QMessageBox::information(this,"删除文件夹",pdu->caData);

        // 这里与删除文件夹的处理有点不同，这里用户确认后直接更新文件列表
        OpeWidget::getInstance().getBook()->clearEnterDir();
        OpeWidget::getInstance().getBook()->flushFile();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND: // 上传文件 回复
    {
        QMessageBox::information(this,"上传文件",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND: // 下载文件 回复
    {
        // qDebug() << pdu->caData;
        char caFileName[32] = {'\0'};
        sscanf(pdu->caData,"%s %lld",caFileName,&(OpeWidget::getInstance().getBook()->m_iTotal));
        // qDebug() << "tcpclient.cpp" << caFileName << "--" << OpeWidget::getInstance().getBook()->m_iTotal;
        // 设置下载文件状态为 true
        if(strlen(caFileName)>0 && OpeWidget::getInstance().getBook()->m_iTotal>0)
        {
            OpeWidget::getInstance().getBook()->m_iRecved=0;
            OpeWidget::getInstance().getBook()->setDownloadStatus(true);
            m_qfile.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
            if(!m_qfile.open(QIODevice::WriteOnly)) // 打开失败
            {
                QMessageBox::warning(this,"下载文件","获得保存文件的路径失败！");
            }
        }

        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_RESPOND: // 共享文件 回复
    {
        QMessageBox::information(this,"共享文件",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_NOTE: // 共享文件 通知
    {
        char *pPath = new char[pdu->uiMsgLen]; // 存放发送过来的文件路径
        memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
        // strrchr()函数的作用是 在一个字符串中查找指定字符最后一次出现的位置，并返回该位置的指针。
        char *pos = strrchr(pPath,'/'); // 找最后一个/

        qDebug() << "tcpclient.cpp share file note: " << pos << ' ' << pPath;

        if(NULL != pos)
        {
            pos++;
            QString strNote = QString("%1 share file->%2 \n Do you accept?").arg(pdu->caData).arg(pos);
            int ret = QMessageBox::question(this,"共享文件",strNote);
            if(QMessageBox::Yes == ret) // 如果接收
            {
                PDU *respdu = mkPDU(pdu->uiMsgLen); // pdu的大小为文件的地址
                respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                memcpy((char*)respdu->caMsg,pdu->caMsg,pdu->uiMsgLen);
                QString strName = TcpClient::getInstance().loginName(); // 获得接收者自己的名字
                strncpy(respdu->caData,strName.toStdString().c_str(),strName.size());
                m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
            }
            // 不接收则不进行处理
        }
        break;
    }
    case ENUM_MSG_TYPE_MOVE_FILE_RESPOND: // 移动文件 回复
    {
        QMessageBox::information(this,"移动文件",pdu->caData);
        break;
    }
    default:
        break;
    }

    free(pdu);
    pdu=NULL;
    }
    else // 否则以二进制文件的形式接收文件
    {
        // qDebug() << "tcpclient.cpp recv file.";
        QByteArray buffer = m_tcpSocket.readAll();
        // 将收到的数据保存到文件中
        m_qfile.write(buffer);

        Book *pBook = OpeWidget::getInstance().getBook();
        pBook->m_iRecved += buffer.size();
        if(pBook->m_iTotal == pBook->m_iRecved) // 接收结束
        {
            m_qfile.close();
            pBook->m_iRecved=0;
            pBook->m_iTotal=0;
            pBook->setDownloadStatus(false);

            QMessageBox::information(this,"下载文件","下载文件完成！");
        }
        else if(pBook->m_iTotal < pBook->m_iRecved) // 下载文件失败
        {
            m_qfile.close();
            pBook->m_iRecved=0;
            pBook->m_iTotal=0;
            pBook->setDownloadStatus(false);

            QMessageBox::critical(this,"下载文件","下载文件失败");
        }
        // 其它情况 表示还没接收完，继续循环接收即可
    }
}

void TcpClient::on_cancel_pb_clicked()
{

}








