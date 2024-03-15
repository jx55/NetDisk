/********************************************************************************
** Form generated from reading UI file 'tcpclient.ui'
**
** Created by: Qt User Interface Compiler version 6.6.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCPCLIENT_H
#define UI_TCPCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TcpClient
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *name_lab;
    QLineEdit *name_le;
    QHBoxLayout *horizontalLayout_2;
    QLabel *pwd_lab;
    QLineEdit *pwd_le;
    QPushButton *login_pb;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *regist_pb;
    QPushButton *cancel_pb;

    void setupUi(QWidget *TcpClient)
    {
        if (TcpClient->objectName().isEmpty())
            TcpClient->setObjectName("TcpClient");
        TcpClient->resize(286, 181);
        QFont font;
        font.setPointSize(7);
        TcpClient->setFont(font);
        layoutWidget = new QWidget(TcpClient);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(10, 10, 266, 161));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        name_lab = new QLabel(layoutWidget);
        name_lab->setObjectName("name_lab");
        QFont font1;
        font1.setPointSize(16);
        name_lab->setFont(font1);

        horizontalLayout->addWidget(name_lab);

        name_le = new QLineEdit(layoutWidget);
        name_le->setObjectName("name_le");
        name_le->setFont(font1);

        horizontalLayout->addWidget(name_le);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        pwd_lab = new QLabel(layoutWidget);
        pwd_lab->setObjectName("pwd_lab");
        pwd_lab->setFont(font1);

        horizontalLayout_2->addWidget(pwd_lab);

        pwd_le = new QLineEdit(layoutWidget);
        pwd_le->setObjectName("pwd_le");
        pwd_le->setFont(font1);
        pwd_le->setEchoMode(QLineEdit::Password);

        horizontalLayout_2->addWidget(pwd_le);


        verticalLayout->addLayout(horizontalLayout_2);

        login_pb = new QPushButton(layoutWidget);
        login_pb->setObjectName("login_pb");
        QFont font2;
        font2.setPointSize(18);
        login_pb->setFont(font2);

        verticalLayout->addWidget(login_pb);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        regist_pb = new QPushButton(layoutWidget);
        regist_pb->setObjectName("regist_pb");
        QFont font3;
        font3.setPointSize(11);
        regist_pb->setFont(font3);

        horizontalLayout_3->addWidget(regist_pb);

        cancel_pb = new QPushButton(layoutWidget);
        cancel_pb->setObjectName("cancel_pb");
        cancel_pb->setFont(font3);

        horizontalLayout_3->addWidget(cancel_pb);


        verticalLayout->addLayout(horizontalLayout_3);


        retranslateUi(TcpClient);

        QMetaObject::connectSlotsByName(TcpClient);
    } // setupUi

    void retranslateUi(QWidget *TcpClient)
    {
        TcpClient->setWindowTitle(QCoreApplication::translate("TcpClient", "TcpClient", nullptr));
        name_lab->setText(QCoreApplication::translate("TcpClient", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        pwd_lab->setText(QCoreApplication::translate("TcpClient", "\345\257\206   \347\240\201\357\274\232", nullptr));
        login_pb->setText(QCoreApplication::translate("TcpClient", "\347\231\273\351\231\206", nullptr));
        regist_pb->setText(QCoreApplication::translate("TcpClient", "\346\263\250\345\206\214", nullptr));
        cancel_pb->setText(QCoreApplication::translate("TcpClient", "\346\263\250\351\224\200", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TcpClient: public Ui_TcpClient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCPCLIENT_H
