#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include "protocol.h"
#include <QTcpSocket>

#include <QDir> // 用于操作文件夹
#include <QFile>
#include <QTimer>

/*
 * 我们要自己封装一个tcp socket，如果直接new一个socket，然后保存描述符，那到时候发送数据的时候就不知道是谁发的
 * 为什么要封装tcp Socket,因为有多个客户端连接服务器的话，都会产生socket，很多socket混在一起，无法判断readyRead信号是哪个socket发出来的。
*/

class MyTcpSocket : public QTcpSocket
{
    // 与之前一样，有Q_OBJECT宏，继承了QOBJECT类，满足这两个条件即支持信号槽
    Q_OBJECT

public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getName(); // 用来获得私有成员变量 m_strName
    void copyDir(QString strSourceDir,QString strDestinationDir); // 文件夹的拷贝

signals:
    // 列表中的socket就是MyTcpSocket类型的，所以可以通过地址进行匹配
    void offline(MyTcpSocket *mysocket);

public slots:
    // 当socket有数据发送过来的时候，它就会发出readyRead信号，就可以使用recvMsg来接收
    void recvMsg();
    void clientOffline(); // 创建一个槽函数，用来处理客户端下线的信号
    void sendFileToClient(); // 发送文件数据给客户端

private:
    // 因为在查找的时候，需要根据用户名来查找，所以添加一个成员变量
    QString m_strName;

    QFile m_file;
    qint64 m_iTotal; // 文件的总大小
    qint64  m_iRecved; // 已经接收了的大小
    bool m_bUpload; // 是否处于上传状态

    QTimer *m_pTimer; // 定时器
};

#endif // MYTCPSOCKET_H












