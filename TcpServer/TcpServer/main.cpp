#include "tcpserver.h"
#include "opedb.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 调用数据库初始化函数
    OpeDB::getInstance().init();

    TcpServer w;
    w.show();
    return a.exec();
}
