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
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionCalculate_FPS;
    QAction *actionDevice;
    QAction *actionExit;
    QAction *actionOpen_Camera;
    QAction *actionRead_Camera;
    QWidget *centralwidget;
    QLabel *label;
    QMenuBar *menubar;
    QMenu *menumonitor;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1920, 1080);
        actionCalculate_FPS = new QAction(MainWindow);
        actionCalculate_FPS->setObjectName(QString::fromUtf8("actionCalculate_FPS"));
        actionDevice = new QAction(MainWindow);
        actionDevice->setObjectName(QString::fromUtf8("actionDevice"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionOpen_Camera = new QAction(MainWindow);
        actionOpen_Camera->setObjectName(QString::fromUtf8("actionOpen_Camera"));
        actionRead_Camera = new QAction(MainWindow);
        actionRead_Camera->setObjectName(QString::fromUtf8("actionRead_Camera"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(0, 0, 1920, 1080));
        label->setFrameShape(QFrame::Box);
        label->setLineWidth(3);
        label->setAlignment(Qt::AlignCenter);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1920, 27));
        menumonitor = new QMenu(menubar);
        menumonitor->setObjectName(QString::fromUtf8("menumonitor"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menumonitor->menuAction());
        menumonitor->addAction(actionExit);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        actionCalculate_FPS->setText(QApplication::translate("MainWindow", "Calculate FPS", nullptr));
        actionDevice->setText(QApplication::translate("MainWindow", "Device", nullptr));
        actionExit->setText(QApplication::translate("MainWindow", "Exit", nullptr));
        actionOpen_Camera->setText(QApplication::translate("MainWindow", "Open Camera", nullptr));
        actionRead_Camera->setText(QApplication::translate("MainWindow", "Read Camera", nullptr));
        label->setText(QApplication::translate("MainWindow", "800 * 450 \n"
" 16:9 Monitor", nullptr));
        menumonitor->setTitle(QApplication::translate("MainWindow", "Menu", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
