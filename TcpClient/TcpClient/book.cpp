#include "book.h"
#include "tcpclient.h"

#include <QInputDialog> // 输入框
#include <QMessageBox>
#include <QFileDialog>
#include "sharefile.h"

// 实现异步操作
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_strEnterDir.clear(); // 刚开始时清空当前目录

    m_pTimer = new QTimer(); // 创建定时器
    m_bDownload = false;

    m_pBookListw = new QListWidget;
    m_pReturnPB = new QPushButton("返回"); //
    m_pCreateDirPB = new QPushButton("创建文件夹"); //
    m_pDelDirPB = new QPushButton("删除文件夹"); //
    m_pRenamePB = new QPushButton("重命名文件夹"); //
    m_pFlushFilePB = new QPushButton("刷新文件夹"); //

    // 创建一个垂直布局，将对文件夹的操作放入垂直布局中
    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    m_pUploadPB = new QPushButton("上传文件"); //
    m_pDownLoadPB = new QPushButton("下载文件"); //
    m_pDelFilePB = new QPushButton("删除文件"); //
    m_pShareFilePB = new QPushButton("分享文件"); //
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);

    // 将对文件的操作也放入一个垂直布局中
    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookListw);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    // 关联信号槽
    // 创建文件夹
    connect(m_pCreateDirPB,SIGNAL(clicked(bool)),this,SLOT(createDir()));
    // 关联刷新文件 按钮与其槽函数
    connect(m_pFlushFilePB,SIGNAL(clicked(bool)),this,SLOT(flushFile()));
    // 关联 删除文件夹 按钮与其槽函数
    connect(m_pDelDirPB,SIGNAL(clicked(bool)),this,SLOT(delDir()));
    // 关联 重命名文件 按钮与其槽函数
    connect(m_pRenamePB,SIGNAL(clicked(bool)),this,SLOT(renameFile()));
    // 关联 双击文件 按钮与其槽函数
    connect(m_pBookListw,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(enterDir(QModelIndex)));
    // 关联 返回上一级目录 按钮与其槽函数
    connect(m_pReturnPB,SIGNAL(clicked(bool)),this,SLOT(returnPre()));
    // 关联 删除文件 按钮与其槽函数
    connect(m_pDelFilePB,SIGNAL(clicked(bool)),this,SLOT(delRegFile()));
    // 关联 上传文件 按钮与其槽函数
    connect(m_pUploadPB,SIGNAL(clicked(bool)),this,SLOT(uploadFile()));
    // 关联 定时器 与其槽函数（timeout()时间结束了，就会进入uploadFileData()函数中
    connect(m_pTimer,SIGNAL(timeout()),this,SLOT(uploadFileData()));
    // 关联 下载文件 与其槽函数
    connect(m_pDownLoadPB,SIGNAL(clicked(bool)),this,SLOT(downloadFile()));
    // 关联 共享文件 按钮与其槽函数
    connect(m_pShareFilePB,SIGNAL(clicked(bool)),this,SLOT(shareFile()));
    // 关联 移动文件 按钮与其信号槽
    connect(m_pMoveFilePB,SIGNAL(clicked(bool)),this,SLOT(moveFile()));
    // 关联 目标目录 按钮与其信号槽
    connect(m_pSelectDirPB,SIGNAL(clicked(bool)),this,SLOT(selectDestDir()));
}

// 更新文件列表
void Book::updateFileList(const PDU *pdu)
{
    // 更新文件列表之前先清空列表
    m_pBookListw->clear();

    if(NULL == pdu)
    {
        // 形参无效，直接返回
        return;
    }
    FileInfo *pFileInfo = NULL;
    int iCount = pdu->uiMsgLen/sizeof(FileInfo); // 文件个数
    for(int i=0;i<iCount;i++)
    {
        pFileInfo = (FileInfo*)(pdu->caMsg)+i;

        QListWidgetItem *pItem = new QListWidgetItem;
        // 根据文件类型 加载资源中我们添加的图标
        if(0 == pFileInfo->iFileType)
        {
            pItem->setIcon(QIcon(QPixmap(":/dir.jpg")));
        }
        else if(1 == pFileInfo->iFileType)
        {
            pItem->setIcon(QIcon(QPixmap(":/file.jpg")));
        }
        pItem->setText(pFileInfo->caName);

        m_pBookListw->addItem(pItem);
    }
}

// 清空当前进入的文件夹名字
void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

// 返回进入的文件夹 名字
QString Book::enterDir()
{
    return m_strEnterDir;
}

// 创建文件夹
void Book::createDir()
{
    QString strNewDirName = QInputDialog::getText(this,"新建文件夹","请输入文件夹名字:");
    if (!strNewDirName.isEmpty())
    {
        if(strNewDirName.size()>32) // 新文件夹的名字长度大于32
        {
            QMessageBox::warning(this,"新建文件夹","文件夹名字过长!");
        }
        else
        {
            QString strName = TcpClient::getInstance().loginName();
            QString strCurPath = TcpClient::getInstance().curPath();
            // 用户名放在caData部分，当前目录放在消息部分
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;

            // 将用户名、新建文件夹名字放在caData中
            strncpy(pdu->caData,strName.toStdString().c_str(),strName.size());
            strncpy(pdu->caData+32,strNewDirName.toStdString().c_str(),strNewDirName.size());

            // 将当前目录放在caMsg部分
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());

            // 发送给服务器
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
    }
    else
    {
        QMessageBox::warning(this,"新建文件夹","文件夹名字不能为空!");
    }
}

// 刷新文件
/*
 * 用户点击刷新文件后，先到该函数，该函数中发送请求给服务器，
 * 服务器处理后返回信息给客户端，客户端使用updateFileList()函数来更新列表
*/
void Book::flushFile()
{
    // 获取当前目录
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(strCurPath.size()+1); // 将当前目录 放在caMsg部分
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    // strncpy需要char*类型的，所以对pdu->caMsg需要类型转换
    strncpy((char*)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.size());

    // 如果是刷新文件的话，应该将m_strEnterDir置为空，否则会连续添加当前目录
    m_strEnterDir = NULL;

    // 发送给服务器
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

// 删除文件夹
void Book::delDir()
{
    // 获取当前目录
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListw->currentItem(); // 获得当前的选项
    if(NULL == pItem) // 选项为空
    {
        QMessageBox::warning(this,"删除文件","请选择一个文件！");
    }
    else
    {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;

        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        strncpy((char*)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.size());

        // 发送给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

// 重命名文件
void Book::renameFile()
{
    // 获取当前目录
    QString strCurPath = TcpClient::getInstance().curPath();
    // 获得用户选中的要修改的用户名
    QListWidgetItem *pItem = m_pBookListw->currentItem(); // 获得当前的选项
    if(NULL == pItem) // 选项为空
    {
        QMessageBox::warning(this,"重命名文件","请选择一个文件！");
    }
    else
    {
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this,"重命名文件","请输入新的文件名");
        if(!strNewName.isEmpty()) // 新的文件名给不为空
        {
            // 将新文件名放在caData中，当前目录放在caMsg中
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData,strNewName.toStdString().c_str(),strNewName.size());
            strncpy(pdu->caData+32,strOldName.toStdString().c_str(),strOldName.size());
            memcpy((char*)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.size());

            // 发送给服务器
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
        else // 为空
        {
            QMessageBox::warning(this,"重命名文件","文件名不能为空!");
        }
    }
}

// 进入文件夹
void Book::enterDir(const QModelIndex &index)
{
    // index.data()返回的是QVariant类型，需要转化为String类型
    QString strDirName = index.data().toString();
    m_strEnterDir = strDirName; // 保存当前进入的路径的名字
    // qDebug() << strDirName;
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData,strDirName.toStdString().c_str(),strDirName.size());
    memcpy((char*)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

// 返回上一级目录
void Book::returnPre()
{
    // 记录当前目录
    QString strCurPath = TcpClient::getInstance().curPath();
    // 记录顶级目录，即./+用户名
    QString strRootPath = "./" + TcpClient::getInstance().loginName();
    // 如果当前目录是根目录，则无法再返回
    if(strCurPath == strRootPath)
    {
        QMessageBox::warning(this,"返回","返回失败：已经在顶层目录");
    }
    else
    {
        // 返回最后一个/的下标
        int index = strCurPath.lastIndexOf('/');
        // remove()函数参数：从哪开始删除、删除个数
        strCurPath.remove(index,strCurPath.size()-index);
        // 更新当前目录，因为很多操作都是基于当前目录来操作的，所以必须及时更新当前目录
        qDebug() << strCurPath;
        TcpClient::getInstance().setCurPath(strCurPath);

        // 因为flushFile()函数处理后，会给客户端一个反馈，客户端更新当前目录
        // 而返回上一级目录，我们在上面已经设置了新的当前目录，不需要让客户端重复处理，将m_strCurPath置空，就不会被客户端重复处理
        clearEnterDir();

        // 给服务器发送一个刷新的请求
        flushFile();
    }
    PDU *pdu = mkPDU(0);
}

// 删除常规文件
void Book::delRegFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListw->currentItem(); // 获取当前选中的内容
    if(NULL == pItem)
    {
        QMessageBox::warning(this,"删除文件","请选中一个文件！");
    }
    else
    {
        // 判断是否是常规文件
        QString strDelFileName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_REG_FILE_REQUEST;

        strncpy(pdu->caData,strDelFileName.toStdString().c_str(),strDelFileName.size());
        strncpy((char*)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.size());

        // 发送给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

// 上传文件
void Book::uploadFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    // 返回你选中的文件路径
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    // qDebug() << m_strUploadFilePath;

    if(!m_strUploadFilePath.isEmpty())
    {
        int index = m_strUploadFilePath.lastIndexOf('/');
        // 提取最后一个/之后的内容，例如./aa/bb/cc，只提取'cc'
        QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);
        // qDebug() << strFileName;

        // 新建一个文件对象
        QFile file(m_strUploadFilePath);
        qint64 fileSize = file.size();

        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        // strncpy(pdu->caData,strFileName.toStdString().c_str(),strFileName.size());
        memcpy((char*)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.size());
        sprintf(pdu->caData,"%s %lld",strFileName.toStdString().c_str(),fileSize);

        // 发送给服务器
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

        // 客户端发送完上传文件请求后，不能立刻上传文件，否则可能会出现粘包的情况，无法区分请求与上传的文件
        // 所以需要用到定时器，发送完请求后启动定时器
        m_pTimer->start(1000);
    }
    else // 文件名字为空
    {
        QMessageBox::warning(this,"上传文件","上传文件的名字不能为空！");
    }
}

// 设置定时器时间（时间到了才能开始上传文件
void Book::uploadFileData()
{
    // 先关闭定时器，否则会重新计时
    m_pTimer->stop();
    // qDebug() << "uploadFileData()";
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly)) // 以只读的方式打开
    {
        QMessageBox::warning(this,"上传文件","打开文件失败");
        return;
    }

    // 有人做过测试，每次传输数据大小为4096时，效率比较高
    char *pBuffer = new char[4096];
    qint64 ret=0;
    while(true) // 循环读数据
    {
        // 这里的ret不一定等于4096，如果文件中只有100个字节，那它的大小即为100
        ret = file.read(pBuffer,4096);
        if(ret>0 && ret<=4096) // 即读到了数据
        {
            // 发送给服务器
            TcpClient::getInstance().getTcpSocket().write(pBuffer,ret);
        }
        else if(0 == ret) // 即读到了文件末尾
        {
            // qDebug() << "Read file finish!";
            break; // 结束循环
        }
        else
        {
            QMessageBox::warning(this,"上传文件","上传文件失败：读文件失败！");
        }
    }

    file.close(); // 关闭文件
    delete []pBuffer;
    pBuffer = NULL;
}

// 下载文件
void Book::downloadFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListw->currentItem(); // 获取当前选中的内容
    if(NULL == pItem)
    {
        QMessageBox::warning(this,"下载文件","请选中一个文件！");
    }
    else
    {
        // 设置保存文件的位置,getSaveFileName()会弹出一个窗口，让用户选择一个路径
        QString strSaveFile = QFileDialog::getSaveFileName();
        // qDebug() << strSaveFile;
        if(strSaveFile.isEmpty()) // 指定的路径为空
        {
            QMessageBox::warning(this,"下载文件","请指定下载位置！");
            m_strSaveFilePath.clear(); // 接收端发现为空，则不进行处理
        }
        else
        {
            m_strSaveFilePath = strSaveFile;
            // qDebug() << m_strSaveFilePath << "--" << strSaveFile;
        }

        PDU *pdu = mkPDU(strCurPath.size()+1);
        QString strFileName = pItem->text();
        strncpy(pdu->caData,strFileName.toStdString().c_str(),strFileName.size()); // 保存选中的文件名
        memcpy((char*)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.size()); // 保存当前路径
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;

        // 发送请求
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}


// 分享文件（这样实现需要先在好友界面 刷新好友，然后点击分享文件才能看到好友）
void Book::shareFile()
{
    // 获得用户选择的文件
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListw->currentItem(); // 获取当前选中的内容
    if(NULL == pItem)
    {
        QMessageBox::warning(this,"分享文件","请选择一个文件！");
        return;
    }
    else // 则已经选择了要分享的文件
    {
        m_strShareFileName = pItem->text();
    }

    // qDebug() << "book.cpp shareFile";
    // 得到好友界面的指针
    Friend *pFriend = OpeWidget::getInstance().getFriend(); // 获得好友界面的指针
    // 通过好友界面的指针获得好友对象
    QListWidget *pFriendList = pFriend->getFriendList();

    for(int i=0;i<pFriendList->count();i++)
    {
        // qDebug() <<"Friend List: "<< pFriendList->item(i)->text();
    }

    ShareFile::getInstance().updateFriend(pFriendList);
    // 如果该窗口是隐藏的，就显示出来
    if(ShareFile::getInstance().isHidden())
    {
        ShareFile::getInstance().show();
    }
}

// 移动文件
void Book::moveFile()
{
    QListWidgetItem *pCurItem = m_pBookListw->currentItem();

    if(NULL != pCurItem)
    {
        m_strMoveFileName = pCurItem->text(); // 文件名
        QString strCurPath = TcpClient::getInstance().curPath(); // 文件路径
        m_strMoveFilePath = strCurPath + '/' + m_strMoveFileName; // 拼接，得到完整路径

        qDebug() << "m_strMoveFilePath: " << m_strMoveFilePath;

        m_pSelectDirPB->setEnabled(true); // 选择了文件后才能点击 目录目录按钮
    }
    else // 未选中文件
    {
        QMessageBox::warning(this,"移动文件",CHOOSE_WARN);
    }
}

// 选择目标目录
void Book::selectDestDir()
{
    QListWidgetItem *pCurItem = m_pBookListw->currentItem();

    if(NULL != pCurItem)
    {
        m_strMoveFileDestPath = pCurItem->text(); // 所选中的名字
        QString strCurPath = TcpClient::getInstance().curPath(); // 文件路径
        m_strMoveFileDestPath = strCurPath + '/' + m_strMoveFileDestPath; // 拼接，得到完整路径

        int srcLen = m_strMoveFilePath.size();
        int destLen = m_strMoveFileDestPath.size();
        PDU *pdu = mkPDU(srcLen+destLen+2);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData,"%d %d %s",srcLen,destLen,m_strMoveFileName.toStdString().c_str()); // 将两部分的长度、文件名分别放在caData部分
        memcpy((char*)pdu->caMsg,m_strMoveFilePath.toStdString().c_str(),srcLen);
        memcpy((char*)pdu->caMsg+srcLen+1,m_strMoveFileDestPath.toStdString().c_str(),destLen);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else // 未选中文件
    {
        QMessageBox::warning(this,"移动文件",CHOOSE_WARN);
    }

    m_pSelectDirPB->setEnabled(false);
}



// 设置下载状态
void Book::setDownloadStatus(bool status)
{
    m_bDownload = status;
}

// 返回 下载文件的状态
bool Book::getDownloadStatus()
{
    return m_bDownload;
}

// 返回 保存文件的路径
QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

// 返回共享的文件名
QString Book::getShareFileName()
{
    return m_strShareFileName;
}










