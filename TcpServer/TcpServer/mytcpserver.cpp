#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer() {
    // 怎么接收客户端的连接呢？首先要进行监听
    // 可以将MyTcpServer作为一个对象来进行监听

}

// getInstance函数定义
MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    // 将这个对象作为引用返回
    // 后面凡是需要用到MyTcpServer的地方，可以直接通过 类名.getInstance()获得静态的局部对象来进行操作
    // 因为instance对象是静态的，所以无论调用多少次，都只有一个对象
    return instance;
}

// 服务器接收客户端连接的时候，会自动进入incomingConnection()函数中，自动会生成一个TCP Socket，然后把这个Tcp Socket描述符返回回来
// 这个时候我们只需要产生一个socket，把这个描述符保存在socket里面就可以收发数据了
void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    // qDebug()<<"new client connected";
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);

    // 关联删除socket的信号
    connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*)),this,SLOT(deleteSocket(MyTcpSocket*)));
}

// 转发函数，通过用户的name找到其socket，然后才能转发给它
void MyTcpServer::resend(const char *pername, PDU *pdu)
{
    if(NULL == pername || NULL == pdu)
    {
        return;
    }
    QString strName = pername;
    // 遍历链表找到name对应的socket
    for(int i=0;i<m_tcpSocketList.size();i++)
    {
        if(strName == m_tcpSocketList.at(i)->getName())
        {
            // 将pdu发送给对方
            m_tcpSocketList.at(i)->write((char*)pdu,pdu->uiPDULen);
            break;
        }
    }
}

void MyTcpServer::deleteSocket(MyTcpSocket *mysocket)
{
    // 在该槽函数中遍历列表，进行匹配，匹配到了进行删除
    QList<MyTcpSocket*>::iterator iter=m_tcpSocketList.begin();
    for(; iter != m_tcpSocketList.end(); iter++)
    {
        if(mysocket == *iter) // 匹配成功
        { // 这里删除了两个东西，第一个是 指针指向的对象，第二个是 列表中存放的指针
            // delete *iter; //使用这个会有问题
            (*iter)->deleteLater(); // 删除它指向的对象
            *iter=NULL;

            // 通过迭代器将存在list中的socket指针也删除掉
            m_tcpSocketList.erase(iter);
            break;
        }
    }

    // 打印当前列表（即删除指定元素之后的列表
    for(int i=0;i<m_tcpSocketList.size();i++)
    {
        // qDebug() << m_tcpSocketList.at(i)->getName();
    }
}























