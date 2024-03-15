#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QInputDialog> // 输入框
#include <QDebug>
#include <QMessageBox>
#include "privatechat.h"


Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE = new QTextEdit; // 显示信息
    m_pFriendListWidget = new QListWidget; // 显示好友列表
    m_pInputMsgLE = new QLineEdit; // 信息输入框

    m_pDelFriendPB = new QPushButton("删除好友"); // 删除好友
    m_pFlushFriendPB= new QPushButton("刷新好友"); // 刷新在线好友列表
    m_pShowOnlineUserPB = new QPushButton("显示在线好友"); // 查看在线用户
    m_pSearchUserPB = new QPushButton("查找用户"); // 查找用户
    m_pMsgSendPB = new QPushButton("发送消息"); // 发送消息
    m_pPrivateChatPB = new QPushButton("私聊"); // 私聊

    // 设置布局，按钮设置为垂直布局
    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUserPB);
    pRightPBVBL->addWidget(m_pSearchUserPB);
    // pRightPBVBL->addWidget(m_pMsgSendPB); // 这个不需要，这个按钮和消息输入框一起构成布局
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    // 好友列表、消息显示、以及所有的按钮构成一个水平布局
    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);

    // 输入框单独弄一个水平布局
    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    // 在线用户的弹出窗口，本来是不显示的，点击在线用户的时候才显示
    m_pOnline = new Online;

    // 上下两个水平布局再构成一个垂直布局
    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();

    setLayout(pMain);

    // 关联显示在线用户的信号槽
    connect(m_pShowOnlineUserPB,SIGNAL(clicked(bool)),this,SLOT(showOnline()));
    // 关联搜索用户的信号槽(connect()函数的参数：sender、signal、receiver、处理信号的函数)
    connect(m_pSearchUserPB,SIGNAL(clicked(bool)),this,SLOT(SearchUser()));
    // 关联刷新好友列表的信号槽
    connect(m_pFlushFriendPB,SIGNAL(clicked(bool)),this,SLOT(flushFriend()));
    // 关联删除按钮与其槽函数
    connect(m_pDelFriendPB,SIGNAL(clicked(bool)),this,SLOT(delFriend()));
    // 关联私聊按钮与其槽函数
    connect(m_pPrivateChatPB,SIGNAL(clicked(bool)),this,SLOT(privateChat()));
    // 关联群聊与其槽函数
    connect(m_pMsgSendPB,SIGNAL(clicked(bool)),this,SLOT(groupChat()));
}

// 显示所有在线的用户
void Friend::showAllOnlineUser(PDU *pdu)
{
    if(NULL == pdu) // pdu为空的情况
    {
        return;
    }
    m_pOnline->showUser(pdu);
}

// 更新好友列表
void Friend::updateFriendList(PDU *pdu)
{
    if(NULL == pdu){
        return;
    }
    uint uiSize = pdu->uiMsgLen/32; // 计算有多少个人
    char caName[32] = {'\0'};
    m_pFriendListWidget->clear();
    for(uint i=0;i<uiSize;i++)
    {
        memcpy(caName,(char*)(pdu->caMsg)+i*32,32);
        m_pFriendListWidget->addItem(caName);
    }
    qDebug() << "friend.cpp: update friend list finish.";
}

// 更新群聊信息
void Friend::updateGroupMsg(PDU *pdu)
{
    QString strMsg = QString("%1 says: %2").arg(pdu->caData).arg((char*)pdu->caMsg);
    m_pShowMsgTE->append(strMsg);
}

// 返回好友列表，用于在shareFile文件中添加好友列表
QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline()
{
    if (m_pOnline->isHidden()) // 如果online窗口是隐藏的，则显示出来
    {
        m_pOnline->show();

        // 产生协议数据单元
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_REQUEST; // 请求类型
        // 发送请求信息
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
    else // 已经显示了，则隐藏
    {
        m_pOnline->hide();
    }
}

// 搜索用户：这里简化一点，直接通过名字来搜索
void Friend::SearchUser()
{
    // QInputDialog::getText()参数：父窗口parent、标题title、标签label
    m_strSearchName = QInputDialog::getText(this,"搜索","用户名：");
    if(!m_strSearchName.isEmpty()) // 如果名字非空
    {
        qDebug() << m_strSearchName;
        PDU *pdu = mkPDU(0); // 不需要额外的信息，名字放在data中即可
        // 将数据拷贝进pdu中
        memcpy(pdu->caData,m_strSearchName.toStdString().c_str(),m_strSearchName.size());
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_REQUEST; // 请求类型

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
}

// 刷新好友列表
void Friend::flushFriend()
{
    qDebug() << "friend.cpp flushFriend function.";
    // 将自己的用户名发送给服务器，服务器才能进行搜索
    QString strName = TcpClient::getInstance().loginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST; // 设置消息类型
    // qDebug() << strName;
    memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());

    // 获得socket并发送数据
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

// 删除好友
void Friend::delFriend()
{
    // 判断是否选中了一个好友
    if(NULL != m_pFriendListWidget->currentItem())
    {
        QString strFriendName = m_pFriendListWidget->currentItem()->text(); // 要删除的好友的名字
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        QString strSelfName = TcpClient::getInstance().loginName(); // 自己的名字

        // 将两个名字拷贝到数据中
        memcpy(pdu->caData,strSelfName.toStdString().c_str(),strSelfName.size());
        memcpy(pdu->caData+32,strFriendName.toStdString().c_str(),strFriendName.size());

        // 打印自己名字、要删除好友的名字
        qDebug() << strSelfName << ' ' << strFriendName;

        // 发送数据
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);

        free(pdu);
        pdu = NULL;
    }
    else{
        QMessageBox::information(this,"删除好友","请选中一个好友！");
    }

}

// 私聊
void Friend::privateChat()
{
    // 点击私聊按钮，如果私聊界面是隐藏的，就显示出来
    if(NULL != m_pFriendListWidget->currentItem())
    {
        QString strChatName = m_pFriendListWidget->currentItem()->text(); // 私聊的好友的名字
        PrivateChat::getInstance().setChatName(strChatName); // 这个函数可以同时设置自己的名字、好友的名字

        if(PrivateChat::getInstance().isHidden()) // 如果聊天界面是隐藏的
        {
            PrivateChat::getInstance().show(); // 显示私聊的界面
        }
    }
    else
    {
        QMessageBox::warning(this,"私聊","请选择私聊的对象");
    }
}

// 群聊
void Friend::groupChat()
{
    // 包括发送方的用户名，以及聊天信息
    QString strMsg = m_pInputMsgLE->text(); // 获得输入框中的内容
    m_pInputMsgLE->clear(); // 获得内容后清空输入框
    if(!strMsg.isEmpty()) // 非空
    {
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        QString strName = TcpClient::getInstance().loginName();
        // memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());
        // memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
        strncpy(pdu->caData,strName.toStdString().c_str(),strName.size());
        strncpy((char*)(pdu->caMsg),strMsg.toStdString().c_str(),strMsg.size());

        // 发送者、消息都有了，将其发送给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);

    }
    else
    {
        QMessageBox::warning(this,"群聊","消息不能为空！");
    }
}


















