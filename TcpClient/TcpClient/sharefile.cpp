#include "sharefile.h"
#include "tcpclient.h"
#include "opewidget.h"

ShareFile::ShareFile(QWidget *parent)
    : QWidget{parent}
{
    m_pSelectAllPB = new QPushButton("全选"); // 全选按钮
    m_pCancelSelectPB = new QPushButton("取消选择"); // 取消选择
    m_pOKPB = new QPushButton("确认"); // 确定按钮
    m_pCancelPB = new QPushButton("取消"); // 取消按钮

    m_pSA = new QScrollArea; // 滑动区域
    m_pFriendW = new QWidget; // 好友区域，将交给QScrollArea即可实现滚动条的效果
    m_pFriendWVBL = new QVBoxLayout(m_pFriendW); // 其作用对象是Friend Widget
    m_pButtonGroup = new QButtonGroup(m_pFriendW); // 管理所有的好友
    m_pButtonGroup->setExclusive(false); // 设置可以多选，设置为true则只能单选

    // 全选、取消选择 构成一个水平布局
    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch(); // 加一个弹簧，让两个按钮在最左边

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);

    setLayout(pMainVBL);

    // 调用构造函数
    // test();

    // 关联 全选 按钮
    connect(m_pSelectAllPB,SIGNAL(clicked(bool)),this,SLOT(selectAll()));
    // 关联 取消选择 按钮与其信号槽
    connect(m_pCancelSelectPB,SIGNAL(clicked(bool)),this,SLOT(cancelSelect()));
    // 关联 确认 按钮
    connect(m_pOKPB,SIGNAL(clicked(bool)),this,SLOT(okPb()));
    // 关联 取消 按钮
    connect(m_pCancelPB,SIGNAL(clicked(bool)),this,SLOT(cancelPb()));
}

// 测试函数
void ShareFile::test()
{
    QVBoxLayout *p = new QVBoxLayout(m_pFriendW);
    QCheckBox *pCB = NULL;
    for(int i=0;i<15;i++)
    {
        pCB = new QCheckBox("jack");
        p->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

// ShareFile单例模式，这样不管调用多少次，访问的始终是同一个对象的引用
ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

// 更新点击分享文件后 弹出的 好友列表
void ShareFile::updateFriend(QListWidget *pFriendList)
{
    if(NULL == pFriendList)
    {
        return;
    }

    QAbstractButton *tmp = NULL;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons();
    // 清空之前的好友
    for(int i=0; i<preFriendList.size(); i++)
    {
        qDebug() << "sharefile.cpp: clear pre friend.";
        tmp = preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp); // 在布局中移除掉
        m_pButtonGroup->removeButton(tmp); // 在ButtonGroup中移除掉
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }

    QCheckBox *pCB = NULL;
    for(int i=0;i<pFriendList->count();i++)
    {
        pCB = new QCheckBox(pFriendList->item(i)->text()); // 获得当前项的名字
        qDebug() << "sharefile.cpp: " << pFriendList->item(i)->text();
        m_pFriendWVBL->addWidget(pCB); // 添加到布局中
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

// 全选
void ShareFile::selectAll()
{
    // buttons()获得所有可选择的列表
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    // 循环判断该选项是否被选
    for(int i=0;i<cbList.size();i++)
    {
        if(!cbList[i]->isChecked())
        {
            cbList[i]->setChecked(true);
        }
    }
}

// 取消选择
void ShareFile::cancelSelect()
{
    // buttons()获得所有可选择的列表
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    // 循环判断该选项是否被选
    for(int i=0;i<cbList.size();i++)
    {
        if(cbList[i]->isChecked())
        {
            cbList[i]->setChecked(false);
        }
    }
}

// 确认
void ShareFile::okPb()
{
    // 点击确认后，即开始分享
    // 需要获得分享者、当前路径；将分享者存入caData中，分享人数存caData中
    // 所有的被分享者依次存入caMsg中，然后当前路径与文件名拼接后也存入caMsg中

    QString strName = TcpClient::getInstance().loginName(); // 分享者
    QString strCurPath = TcpClient::getInstance().curPath(); // 当前路径
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName();
    // 拼接路径
    QString strPath = strCurPath + "/" + strShareFileName;

    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    int num=0;
    // 循环判断该选项是否被选
    for(int i=0;i<cbList.size();i++)
    {
        if(cbList[i]->isChecked())
        {
            num++;
        }
    }

    // pdu的大小即caMsg部分的大小
    PDU *pdu = mkPDU(32*num+strPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",strName.toStdString().c_str(),num);
    int j=0;
    for(int i=0;i<cbList.size();i++)
    {
        if(cbList[i]->isChecked())
        {
            memcpy((char*)(pdu->caMsg)+j*32,cbList[i]->text().toStdString().c_str(),cbList[i]->text().size()    );
        }
    }
    memcpy((char*)(pdu->caMsg)+num*32,strPath.toStdString().c_str(),32);

    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

// 取消
void ShareFile::cancelPb()
{
    hide(); // 即可隐藏当前窗口
}










