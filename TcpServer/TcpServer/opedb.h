#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
// 连接数据库
#include <QSqlDatabase>
// 查询数据库
#include <QSqlQuery>
#include <QStringList>

class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    // 静态成员函数
    static OpeDB& getInstance();

    // 将数据库的连接写成一个初始化函数
    void init();

    ~OpeDB();

    bool handleRegist(const char *name, const char *pwd); // 处理注册
    bool handleLogin(const char *name, const char *pwd); // 处理登陆
    void handleOffline(const char *name);

    // 查询所有在线用户，返回一个列表
    QStringList handleAllOnline();
    // 处理搜索好友
    int handleSearchUser(const char* name);
    // 查询A是否是B的好友，有3种情况，所以用int类型
    // 已经是好友、不是好友但不在线、不是好友在线
    int handleAddFriend(const char *pername, const char *name);
    // 同意添加好友
    bool handleAgreeAddFriend(const char *pername,const char *name);
    // 根据用户名字查询其好友
    QStringList handleFlushFriend(const char *name);
    // 删除好友
    bool handleDelFriend(const char *name,const char *friendName);

signals:

public slots:
private:
    QSqlDatabase m_db; // 连接数据库
};

#endif // OPEDB_H







