#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea> // 浏览的区域，例如一个页面放不下的情况
#include <QCheckBox> // 实现打勾进行选择
#include <QListWidget>

class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);
    void test(); // 向界面中添加好友进行测试
    static ShareFile &getInstance(); // 构造单例
    void updateFriend(QListWidget *pFriendList); // 更新好友

signals:

public slots:
    void selectAll(); // 全选
    void cancelSelect(); // 取消选择
    void okPb(); // 确定
    void cancelPb(); // 取消

private:
    QPushButton *m_pSelectAllPB; // 全选按钮
    QPushButton *m_pCancelSelectPB; // 取消选择
    QPushButton *m_pOKPB; // 确定按钮
    QPushButton *m_pCancelPB; // 取消按钮

    QScrollArea *m_pSA; // 滑动区域
    QWidget *m_pFriendW; // 好友区域，将交给QScrollArea即可实现滚动条的效果
    QVBoxLayout *m_pFriendWVBL; // 垂直布局
    QButtonGroup *m_pButtonGroup; // 管理所有的好友
};

#endif // SHAREFILE_H
