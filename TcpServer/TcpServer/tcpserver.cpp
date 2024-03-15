#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"

#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QHostAddress>
#include <QByteArray>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    // 设置界面大小
    resize(250,250);

    loadConfig();

    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);

    // 监听之后，怎么知道有客户端连接呢？ 使用QTcpServer中的incomingConnect()函数

}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    // 指定打开方式，返回一个布尔值
    if(file.open(QIODevice::ReadOnly)){
        // 成功打开,使用readAll()方法读取所有数据
        // 返回的是一个QByteArray对象，需要加头文件
        QByteArray baData=file.readAll();

        //转换类型，不过不能直接转换为String，只能转换为char*
        QString strData=baData.toStdString().c_str();

        // 调用后才会有输出，所以要在前面调用
        // qDebug()<<strData;

        // 输出结果如下，要对结果进行解析
        // "127.0.0.1\r\n8888\r\n"
        strData.replace("\r\n"," ");
        // qDebug()<<strData;

        // 切分字符串
        QStringList strList=strData.split(" ");
        // for(int i=0;i<strList.size();i++){
        //     qDebug()<<"-->"<<strList[i];
        // }

        m_strIP=strList[0];
        m_usPort=strList[1].toUShort();
        // qDebug()<<m_strIP<<":"<<m_usPort;

        // 文件用完了关闭掉
        file.close();
    }
    else{
        // 打开失败
        QMessageBox::critical(this,"open config","open config failed");
    }
}










