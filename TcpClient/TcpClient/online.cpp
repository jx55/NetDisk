#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"

Online::Online(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUser(PDU *pdu)
{
    if(NULL == pdu) // 对形参的有效性进行判断
    {
        return;
    }
    uint uiSize = pdu->uiMsgLen/32; // 个数
    char caTmp[32];
    // 将用户插入列表前，先清空列表
    // ui->online_lw->clear();

    // 将返回的结果插入到列表中
    for (uint i=0;i<uiSize;i++)
    {
        memcpy(caTmp,(char*)(pdu->caMsg)+i*32,32);
        ui->online_lw->addItem(caTmp);
    }
}

// 添加好友按钮 的处理
void Online::on_addFriend_pb_clicked()
{
    // 封装一个PDU，将加好友信息发送给服务器
    // 根据ui设计，我们需要在左边选择一个用户;
    // currentItem()返回一个QListWidgetItem对象；currentRow()返回当前行号
    QListWidgetItem *pItem = ui->online_lw->currentItem();
    QString strPerUserName = pItem->text(); // 获得所点击行的内容，即对方的用户名
    QString strLoginName = TcpClient::getInstance().loginName(); // 获得当前用户的名字
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    // 将两个名字拷贝到pdu的caData部分
    memcpy(pdu->caData,strPerUserName.toStdString().c_str(),strPerUserName.size());
    memcpy(pdu->caData+32,strLoginName.toStdString().c_str(),strLoginName.size());

    // 获得tcpSocket，发送消息
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    // 释放pdu
    free(pdu);
    pdu = NULL;
}














