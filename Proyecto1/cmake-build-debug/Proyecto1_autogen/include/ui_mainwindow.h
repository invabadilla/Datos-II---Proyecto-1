/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QColumnView>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_IDE
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QLabel *label;
    QLabel *label_2;
    QTextEdit *textEdit;
    QColumnView *columnView;
    QLabel *label_3;
    QTextEdit *textEdit_2;
    QLabel *label_4;
    QTextEdit *textEdit_3;
    QMenuBar *menubar;
    QMenu *menuIDE_C;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *IDE)
    {
        if (IDE->objectName().isEmpty())
            IDE->setObjectName(QString::fromUtf8("IDE"));
        IDE->setEnabled(true);
        IDE->resize(814, 797);
        IDE->setMinimumSize(QSize(814, 797));
        IDE->setMaximumSize(QSize(814, 797));
        centralwidget = new QWidget(IDE);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 1, 1, 1);

        textEdit = new QTextEdit(centralwidget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setEnabled(true);

        gridLayout->addWidget(textEdit, 1, 0, 1, 1);

        columnView = new QColumnView(centralwidget);
        columnView->setObjectName(QString::fromUtf8("columnView"));
        columnView->setEnabled(false);
        columnView->setLineWidth(4);

        gridLayout->addWidget(columnView, 1, 1, 1, 1);

        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        textEdit_2 = new QTextEdit(centralwidget);
        textEdit_2->setObjectName(QString::fromUtf8("textEdit_2"));
        textEdit_2->setEnabled(false);

        gridLayout->addWidget(textEdit_2, 3, 0, 1, 2);

        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 4, 0, 1, 1);

        textEdit_3 = new QTextEdit(centralwidget);
        textEdit_3->setObjectName(QString::fromUtf8("textEdit_3"));
        textEdit_3->setEnabled(false);

        gridLayout->addWidget(textEdit_3, 5, 0, 1, 2);

        IDE->setCentralWidget(centralwidget);
        menubar = new QMenuBar(IDE);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 814, 22));
        menuIDE_C = new QMenu(menubar);
        menuIDE_C->setObjectName(QString::fromUtf8("menuIDE_C"));
        IDE->setMenuBar(menubar);
        statusbar = new QStatusBar(IDE);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        IDE->setStatusBar(statusbar);

        menubar->addAction(menuIDE_C->menuAction());

        retranslateUi(IDE);

        QMetaObject::connectSlotsByName(IDE);
    } // setupUi

    void retranslateUi(QMainWindow *IDE)
    {
        IDE->setWindowTitle(QApplication::translate("IDE", "IDE C!", nullptr));
#ifndef QT_NO_TOOLTIP
        IDE->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        label->setText(QApplication::translate("IDE", "RUN", nullptr));
        label_2->setText(QApplication::translate("IDE", "RAM", nullptr));
        label_3->setText(QApplication::translate("IDE", "Std Out", nullptr));
        label_4->setText(QApplication::translate("IDE", "Log", nullptr));
        menuIDE_C->setTitle(QApplication::translate("IDE", "IDE C!", nullptr));
    } // retranslateUi

};

namespace Ui {
    class IDE: public Ui_IDE {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
