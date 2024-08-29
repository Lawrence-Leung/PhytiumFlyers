/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionRead_Camera;
    QAction *actionOpen_Camera;
    QAction *actionExit;
    QAction *actionCalculate_FPS;
    QAction *actionStop;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QGridLayout *gridLayout_2;
    QLabel *label;
    QCheckBox *checkBox;
    QPushButton *stop;
    QMenuBar *menubar;
    QMenu *menumonitor;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(960, 676);
        actionRead_Camera = new QAction(MainWindow);
        actionRead_Camera->setObjectName(QString::fromUtf8("actionRead_Camera"));
        actionOpen_Camera = new QAction(MainWindow);
        actionOpen_Camera->setObjectName(QString::fromUtf8("actionOpen_Camera"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionCalculate_FPS = new QAction(MainWindow);
        actionCalculate_FPS->setObjectName(QString::fromUtf8("actionCalculate_FPS"));
        actionStop = new QAction(MainWindow);
        actionStop->setObjectName(QString::fromUtf8("actionStop"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        scrollArea = new QScrollArea(centralwidget);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 940, 552));
        gridLayout_2 = new QGridLayout(scrollAreaWidgetContents);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label = new QLabel(scrollAreaWidgetContents);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        scrollArea->setWidget(scrollAreaWidgetContents);

        gridLayout->addWidget(scrollArea, 0, 0, 1, 1);

        checkBox = new QCheckBox(centralwidget);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));

        gridLayout->addWidget(checkBox, 2, 0, 1, 1);

        stop = new QPushButton(centralwidget);
        stop->setObjectName(QString::fromUtf8("stop"));
        stop->setMaximumSize(QSize(45, 45));

        gridLayout->addWidget(stop, 3, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 960, 22));
        menubar->setDefaultUp(false);
        menumonitor = new QMenu(menubar);
        menumonitor->setObjectName(QString::fromUtf8("menumonitor"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menumonitor->menuAction());
        menumonitor->addSeparator();
        menumonitor->addSeparator();
        menumonitor->addAction(actionRead_Camera);
        menumonitor->addAction(actionOpen_Camera);
        menumonitor->addAction(actionCalculate_FPS);
        menumonitor->addAction(actionExit);
        menumonitor->addAction(actionStop);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        actionRead_Camera->setText(QApplication::translate("MainWindow", "Camera Infomation", nullptr));
        actionOpen_Camera->setText(QApplication::translate("MainWindow", "Open Camera", nullptr));
        actionExit->setText(QApplication::translate("MainWindow", "Exit", nullptr));
        actionCalculate_FPS->setText(QApplication::translate("MainWindow", "Calculate FPS", nullptr));
        actionStop->setText(QApplication::translate("MainWindow", "Stop", nullptr));
        label->setText(QApplication::translate("MainWindow", "\346\221\204\345\203\217\345\244\264/\350\247\206\351\242\221", nullptr));
        checkBox->setText(QApplication::translate("MainWindow", "Monitor ON/OFF", nullptr));
        stop->setText(QApplication::translate("MainWindow", "Stop", nullptr));
        menumonitor->setTitle(QApplication::translate("MainWindow", "Menu", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
