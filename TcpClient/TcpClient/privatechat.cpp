#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h" // 聊天的时候也需要用到协议
#include "tcpclient.h" // 获得自己的名字 需要用到该文件中的loginName()函数
#include <QMessageBox>

PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

// 获得聊天窗口
PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

// 获得对方的名字
void PrivateChat::setChatName(QString strName)
{
    // 将名字保存到成员变量中
    m_strChatName = strName;
    m_strLoginName = TcpClient::getInstance().loginName();
}

void PrivateChat::updateMsg(const PDU *pdu)
{
    if(NULL == pdu) // 对形参有效性进行判断
    {
        return;
    }
    char caSendName[32] = {'\0'};
    memcpy(caSendName,pdu->caData,32); // caData的前32位是发送者名字
    QString strMsg = QString("%1 says: %2").arg(caSendName).arg((char*)pdu->caMsg);

    // 将信息显示在界面上
    ui->showMessage_te->append(strMsg);

}

void PrivateChat::on_sendMsg_pb_clicked()
{
    // 获取输入框中的内容
    QString strMsg = ui->inputMsg_le->text();
    ui->inputMsg_le->clear(); // 发送之后就清空输入框
    if(!strMsg.isEmpty()) // 非空
    {
        // 根据消息的大小设置pdu的大小
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData,m_strLoginName.toStdString().c_str(),m_strLoginName.size());
        memcpy(pdu->caData+32,m_strChatName.toStdString().c_str(),m_strChatName.size());

        strcpy((char*)pdu->caMsg,strMsg.toStdString().c_str());

        // 将消息发送给服务器，通过Socket发送出去
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else{
        QMessageBox::warning(this,"私聊","发送的信息不能为空！");
    }
}




















