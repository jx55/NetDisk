#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

OpeDB::OpeDB(QObject *parent)
    : QObject{parent}
{
    // 连接数据库需要到QT中的sql模块，需要在pro文件中添加

    // 告诉QSqlDatabase要操作的是哪个数据库
    m_db=QSqlDatabase::addDatabase("QSQLITE");

    // 可以在这里调用init()函数，也可以在后面调用
}

OpeDB &OpeDB::getInstance()
{
    // 静态成员函数中定义静态成员变量
    static OpeDB instance;
    // 返回静态对象的引用
    return instance;
}

void OpeDB::init()
{
    // 连接数据库
    // setHostName设置IP地址，本地的话就写localhost
    m_db.setHostName("localhost");
    // 要操作的数据库的名字（可以先将数据库文件作为资源文件添加到项目中）
    m_db.setDatabaseName("D:\\A\\project\\_c\\TcpServer\\TcpServer\\cloud.db");

    // 打开数据库成功
    if(m_db.open())
    {
        // 成功的话，打印测试
        QSqlQuery query;
        query.exec("select * from userInfo");
        while(query.next()){
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
            // qDebug() << data;
        }
    }
    else // 打开数据库失败
    {
        QMessageBox::critical(NULL,"打开数据库","打开数据库失败");
    }
}

OpeDB::~OpeDB()
{
    // 在析构函数中关闭数据库
    m_db.close();
}

// 处理注册
bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    // 考虑异常情况
    if (NULL == name || NULL == pwd)
    {
        return false;
    }
    // 注意后面的数据库语句中的%1、%2是要有引号的
    QString data=QString("insert into userInfo(name,pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);

    QSqlQuery query;
    // 因为数据库要求名字是unique，所以如果有重复的话，就会返回false
    return query.exec(data);
}

// 处理登录
bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if (NULL == name || NULL == pwd) // 考虑异常情况
    {
        return false;
    }
    // 用户登陆成功需要满足3个条件，返回的是一个结果集，通过next()函数可以逐个遍历
    QString data=QString("select * from userInfo where name=\'%1\' and pwd=\'%2\' and online=0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(data);
    // 如果执行next()函数为真，证明有数据，因为name字段是唯一的，所以也只会有一条数据
    if (query.next()) // 登录成功应该修改数据库，online为1
    {
        data = QString("update userInfo set online=1 where name=\'%1\' and pwd=\'%2\'").arg(name).arg(pwd);
        // qDebug() << data;
        QSqlQuery query;
        query.exec(data);

        return true;
    }
    else
    {
        return false;
    }
}

void OpeDB::handleOffline(const char *name)
{
    if(NULL == name)
    {
        // qDebug() << "name is NULL";
        return;
    }
    // qDebug() <<"handleOffline"<< name;

    QString data=QString("update userInfo set online=0 where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handleAllOnline()
{
    // 查找所有在线的用户，所以找online=1的即可
    QString data = QString("select name from userInfo where online=1");
    QSqlQuery query;
    query.exec(data);

    // 这里同样使用next()函数来访问结果集
    QStringList result;
    result.clear();
    // 获取每一条结果值
    while(query.next())
    {
        // query.value(0)得到的类型是QVariant，转换为string类型
        result.append(query.value(0).toString());
    }
    return result; // 返回结果集
}

// 处理搜索，如果不存在该用户返回-1；存在用户在线返回1，不在线返回0；
int OpeDB::handleSearchUser(const char *name)
{
    if(NULL == name){
        return -1;
    }
    QString data = QString("select online from userInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        int ret = query.value(0).toInt();
        if(1 == ret)
        {
            return 1;
        }
        else if(0 == ret)
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

// 判断是否是好友
int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if(NULL == pername || NULL == name){
        return -1;
    }
    QString data = QString("select * from friend where (id=(select id from userInfo where name=\'%1\') "
                           "and friendId=(select id from userInfo where name=\'%2\')) "
                           "or (friendId=(select id from userInfo where name=\'%3\') "
                           "and id=(select id from userInfo where name=\'%4\'));").arg(pername).arg(name).arg(name).arg(pername);
    // qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if(query.next()) //可以执行next()函数即query中有数据，故已经是好友关系
    {
        // qDebug() << "opedb.cpp handleAddFriend already friend";
        return 0; // 表明双方已经是好友了
    }
    else // 不是好友，判断对方是否在线
    {
        data=QString("select online from userInfo where name=\'%1\'").arg(pername);
        // qDebug() << data;
        QSqlQuery query;
        query.exec(data);
        if(query.next()) //
        {
            int ret = query.value(0).toInt();
            if(1 == ret){ // 用户在线
                return 1;
            }
            else if(0 == ret){ // 用户不在线
                return 2;
            }
        }
        else // 即不存在该用户
        {
            return 3;
        }
    }
}

// 同意添加好友的处理
bool OpeDB::handleAgreeAddFriend(const char *pername,const char *name)
{

}

// 刷新好友列表(返回在线好友) 的请求
QStringList OpeDB::handleFlushFriend(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();
    if(NULL == name) // 如果name为空，则直接返回空列表
    {
        return strFriendList;
    }

    // 好友有两种情况，id|friendId,friendId|id，对这两种情况分别进行查询
    QString data=QString("select name from userInfo where online=1 and "
                           "id in (select id from friend where friendId=(select id from userInfo "
                           "where name=\'%1\'))").arg(name);
    // qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    while(query.next())
    {
        strFriendList.append(query.value(0).toString());
        qDebug() <<query.value(0).toString();
    }

    data=QString("select name from userInfo where online=1 and "
                   "id in (select friendId from friend where id=(select id from userInfo "
                   "where name=\'%1\'))").arg(name);
    // qDebug() << data;
    query.exec(data);
    while(query.next())
    {
        strFriendList.append(query.value(0).toString());
        // qDebug() <<query.value(0).toString();
    }
    return strFriendList;
}

// 处理删除好友
bool OpeDB::handleDelFriend(const char *name, const char *friendName)
{
    // 对形参有效性进行判断
    if(NULL == name || NULL == friendName)
    {
        return false;
    }

    // 例如删除编号为1和2的好友，要考虑1|2、2|1两种情况
    QString data = QString("delete from friend where id=(select id from userInfo where name=\'%1\') and "
                           "friendId=(select id from userInfo where name=\'%2\')").arg(name).arg(friendName);

    QSqlQuery query;
    query.exec(data);
    // qDebug() <<"opedb.cpp1: "<< data;

    data = QString("delete from friend where friendId=(select id from userInfo where name=\'%1\') and "
                           "id=(select id from userInfo where name=\'%2\')").arg(name).arg(friendName);
    query.exec(data);
    // qDebug() << "opedb.cpp2: "<<data;
    return true;
}











