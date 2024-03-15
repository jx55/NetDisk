#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit> // 信息显示框
#include <QListWidget> // 信息列表
#include <QLineEdit> // 信息输入
#include <QPushButton> // 按钮
#include <QVBoxLayout> // 垂直布局
#include <QHBoxLayout> // 水平布局

#include "online.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnlineUser(PDU *pdu);
    void updateFriendList(PDU *pdu);
    QString m_strSearchName;

    void updateGroupMsg(PDU *pdu); // 更新群聊信息

    QListWidget *getFriendList(); // 返回好友列表

signals:

public slots:
    void showOnline();
    void SearchUser();
    void flushFriend();
    void delFriend();
    void privateChat(); // 私聊
    void groupChat(); // 群聊

private:
    QTextEdit *m_pShowMsgTE; // 显示信息
    QListWidget *m_pFriendListWidget; // 显示好友列表
    QLineEdit *m_pInputMsgLE; // 信息输入框

    // 按钮
    QPushButton *m_pDelFriendPB; // 删除好友
    QPushButton *m_pFlushFriendPB; // 刷新在线好友列表
    QPushButton *m_pShowOnlineUserPB; // 查看在线用户
    QPushButton *m_pSearchUserPB; // 查找用户
    QPushButton *m_pMsgSendPB; //发送消息
    QPushButton *m_pPrivateChatPB; // 私聊

    Online *m_pOnline;

};

#endif // FRIEND_H








