#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget> // 列表
#include <QPushButton> // 按钮
#include <QHBoxLayout> // 水平布局
#include <QVBoxLayout> // 垂直布局

#include "protocol.h"
#include <QTimer> // 定时器

// 即文件操作的界面
class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(const PDU *pdu); // 更新文件列表
    void clearEnterDir(); // 清除当前进入的文件夹名字
    QString enterDir(); // 获得进入的文件夹名字
    void setDownloadStatus(bool status); // 设置下载状态

    // 下载文件
    qint64 m_iTotal; // 总的大小
    qint64 m_iRecved; // 已经收到的大小
    bool getDownloadStatus(); // 获得下载文件的状态
    QString getSaveFilePath(); // 获得保存文件的路径

    QString getShareFileName(); // 获得要分享的文件名

signals:

public slots:
    void createDir();
    void flushFile(); // 刷新文件
    void delDir(); // 删除文件夹
    void renameFile(); // 重命名文件
    // 这里的形参要与双击文件的函数doubleClicked()的参数相一致
    void enterDir(const QModelIndex &index); // 进入文件夹
    void returnPre(); // 返回上一级目录
    void delRegFile(); // 删除常规文件
    void uploadFile(); // 上传文件
    void uploadFileData(); // 定时器时间
    void downloadFile(); // 下载文件
    void shareFile(); // 分享文件
    void moveFile(); // 移动文件
    void selectDestDir(); // 目标目录

private:
    QListWidget *m_pBookListw; // 显示文件名字的列表
    // 操作文件的按钮
    QPushButton *m_pReturnPB; // 返回
    QPushButton *m_pCreateDirPB; // 创建文件夹
    QPushButton *m_pDelDirPB; // 删除文件夹
    QPushButton *m_pRenamePB; // 重命名文件
    QPushButton *m_pFlushFilePB; // 刷新文件夹
    QPushButton *m_pUploadPB; // 上传文件
    QPushButton *m_pDownLoadPB; // 下载文件
    QPushButton *m_pDelFilePB; // 删除文件
    QPushButton *m_pShareFilePB; // 分享文件
    QPushButton *m_pMoveFilePB; // 移动文件
    QPushButton *m_pSelectDirPB; // 目标目录

    QString m_strEnterDir; // 保存当前进入的名字
    QString m_strUploadFilePath; // 上传文件的路径
    QTimer *m_pTimer; // 定时器

    QString m_strSaveFilePath;  // 保存文件的路径
    bool m_bDownload; // 下载文件的状态

    QString m_strShareFileName; // 分享的文件名字

    QString m_strMoveFileName; // 保存要移动的文件名字
    QString m_strMoveFilePath; // 要移动的文件路径
    QString m_strMoveFileDestPath; // 目的路径
};

#endif // BOOK_H














