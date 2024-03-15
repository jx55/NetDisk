#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include <QListWidget>

#include "friend.h"
#include "book.h"
// 因为图书、好友窗口每次只能显示一个，所以使用堆栈窗口来解决这个问题
#include <QStackedWidget>
class OpeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OpeWidget(QWidget *parent = nullptr);
    static OpeWidget &getInstance();
    Friend *getFriend(); // 获得好友的界面
    Book *getBook(); // 获得文件界面

signals:

public slots:

private:
    QListWidget *m_pListw;

    Friend *m_pFriend;
    Book *m_pBook;

    QStackedWidget *m_pSW;
};

#endif // OPEWIDGET_H
