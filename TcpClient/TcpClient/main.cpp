#include "tcpclient.h"

#include <QApplication>

#include "opewidget.h"
#include "online.h"
#include "friend.h"
#include "sharefile.h"
#include "book.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFont font("Times",18,QFont::Bold);
    a.setFont(font);

    // 测试共享文件的界面
    // ShareFile w;
    // w.test();
    // w.show();

    // 因为客户端的登录界面重写为了单例模式，所以不能这样写了
    // TcpClient w;
    // w.show();

    TcpClient::getInstance().show();

    // Friend w;
    // w.show();

    // Online w;
    // w.show();

    // OpeWidget w;
    // w.show();

    // Book w;
    // w.show();

    return a.exec();
}
