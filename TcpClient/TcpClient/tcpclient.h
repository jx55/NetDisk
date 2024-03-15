#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>

#include "opewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    // 将登录界面写成单例模式
    static TcpClient &getInstance();
    // 因为TcpClient中有socket的，我们可以将所有数据的发送都交给TcpClient
    // 可以在TcpClient中封装一个函数，这个函数用来获得tcpSocket，然后就可以用这个TcpSocket来发送信息
    // 也可以封装一个专门发送信息的函数，谁需要发送信息就把信息传给这个函数，就可以进行发送
    // 这里采用第一种方式，获得tcpSocket的引用
    QTcpSocket &getTcpSocket();

    // 获得私有变量m_strLoginName
    QString loginName();
    QString curPath();

    // 记录当前目录
    void setCurPath(QString strCurPath);

// 添加一个槽函数，作为信号的处理函数，和普通函数进行区分
public slots:
    // 显示连接的一个动态
    void showConnect();
    void recvMsg(); // 客户端接收数据

private slots:
    // void on_sendpb_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_cancel_pb_clicked();

private:
    Ui::TcpClient *ui;

    // 将读取到的信息添加到全局变量中
    QString m_strIP;
    // 端口用无符号的16位整型来表示
    quint16 m_usPort;

    // 产生一个socket对象，连接服务器和服务器进行数据交互
    QTcpSocket m_tcpSocket;
    // 登陆时使用的用户名
    QString m_strLoginName;

    // 当前路径
    QString m_strCurPath;

    QFile m_qfile;

};
#endif // TCPCLIENT_H
