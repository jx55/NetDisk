#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include "protocol.h"

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = nullptr);
    ~PrivateChat();

    // 为了方便使用聊天的窗口，将其设置为单例模式
    static PrivateChat &getInstance();

    // 设置聊天的对象
    void setChatName(QString strName);
    // 更新聊天界面
    void updateMsg(const PDU *pdu);


private slots:
    void on_sendMsg_pb_clicked();  // 发送按钮的槽函数

private:
    Ui::PrivateChat *ui;
    QString m_strChatName;
    QString m_strLoginName;
};

#endif // PRIVATECHAT_H










