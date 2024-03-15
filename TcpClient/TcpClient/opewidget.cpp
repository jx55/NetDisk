#include "opewidget.h"

OpeWidget::OpeWidget(QWidget *parent)
    : QWidget{parent}
{
    m_pListw = new QListWidget(this);
    m_pListw->addItem("好友");
    m_pListw->addItem("文件");

    // 产生好友、图书对象
    m_pFriend = new Friend;
    m_pBook =new Book;

    // 来处理好友、图书窗口每次只能显示一个的问题
    m_pSW = new QStackedWidget;
    // 将上面两个窗口加进来,如果没有设置显示哪一个，默认就显示第一个窗口
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pListw);
    pMain->addWidget(m_pSW);

    setLayout(pMain);

    // 关联切换图书的信号槽
    connect(m_pListw,SIGNAL(currentRowChanged(int)),m_pSW,SLOT(setCurrentIndex(int)));
}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance;
    return instance;
}

Friend *OpeWidget::getFriend()
{
    return m_pFriend;
}

Book *OpeWidget::getBook()
{
    return m_pBook;
}
















