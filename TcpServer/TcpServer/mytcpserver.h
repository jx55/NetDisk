#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    // 为了支持信号槽，需要继承Q_OBJECT类，并且要有Q_OBJECT宏
    // QTcpServer已经继承了Q_OBJECT了，所以这里需要加上Q_OBJECT宏，即满足条件
    Q_OBJECT

public:
    MyTcpServer();

    // 单例模式
    static MyTcpServer &getInstance();

    // 该函数原型是虚函数，需要重写该函数
    void incomingConnection(qintptr socketDescriptor);

    // 转发函数
    void resend(const char *pername,PDU *pdu);

public slots:
    void deleteSocket(MyTcpSocket *mysocket);

private:
    // 定义一个链表，保存所有的socket
    QList<MyTcpSocket*> m_tcpSocketList;

};

#endif // MYTCPSERVER_H
















