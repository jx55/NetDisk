/********************************************************************************
** Form generated from reading UI file 'privatechat.ui'
**
** Created by: Qt User Interface Compiler version 6.6.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRIVATECHAT_H
#define UI_PRIVATECHAT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PrivateChat
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QTextEdit *showMessage_te;
    QHBoxLayout *horizontalLayout;
    QLineEdit *inputMsg_le;
    QPushButton *sendMsg_pb;

    void setupUi(QWidget *PrivateChat)
    {
        if (PrivateChat->objectName().isEmpty())
            PrivateChat->setObjectName("PrivateChat");
        PrivateChat->resize(400, 300);
        verticalLayout_2 = new QVBoxLayout(PrivateChat);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        showMessage_te = new QTextEdit(PrivateChat);
        showMessage_te->setObjectName("showMessage_te");
        QFont font;
        font.setPointSize(18);
        showMessage_te->setFont(font);

        verticalLayout->addWidget(showMessage_te);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        inputMsg_le = new QLineEdit(PrivateChat);
        inputMsg_le->setObjectName("inputMsg_le");
        inputMsg_le->setFont(font);

        horizontalLayout->addWidget(inputMsg_le);

        sendMsg_pb = new QPushButton(PrivateChat);
        sendMsg_pb->setObjectName("sendMsg_pb");
        sendMsg_pb->setFont(font);

        horizontalLayout->addWidget(sendMsg_pb);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(PrivateChat);

        QMetaObject::connectSlotsByName(PrivateChat);
    } // setupUi

    void retranslateUi(QWidget *PrivateChat)
    {
        PrivateChat->setWindowTitle(QCoreApplication::translate("PrivateChat", "Form", nullptr));
        sendMsg_pb->setText(QCoreApplication::translate("PrivateChat", "\345\217\221\351\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PrivateChat: public Ui_PrivateChat {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRIVATECHAT_H
