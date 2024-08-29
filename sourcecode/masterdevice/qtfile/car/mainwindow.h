#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStatusBar>
#include <QLabel>
#include <QListView>
#include <QCheckBox>
#include <QPushButton>
#include <QGraphicsPixmapItem>
#include <QMutex>
#include <QStandardItemModel>
#include<QGridLayout>
#include<fstream>
#ifdef CAR_USE_QT_CAMERA
#include <QCameraViewfinder>
#include <QCamera>
#endif
#include "camera.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    struct Detection {
        cv::Rect box;
        int class_id;  // 类别ID
    };
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void initUI();

private:
    Ui::MainWindow *ui;
    bool running;
    camera *c;
    camera *cam;
    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;
    QTimer *timer;
    double fps;
#ifdef GAZER_USE_QT_CAMERA
    QCamera *camera;
    QCameraViewfinder *viewfinder;
#endif
    QMutex *data_lock;
    camera *capturer;
    std::vector<Detection> detections;
    cv::Mat frame;
    cv::Mat currentFrame;
    cv::VideoCapture cap;

    QGraphicsScene *imageScene;
    QGraphicsScene *scene;
    QGraphicsView *view;


private slots:
//    void FPS();
    void showCameraInfo();
    void openCamera();
    void calculateFPS();
    void updateFPS(float fps);
    void updateFrame(cv::Mat *);
    void importFrame();
//    void startDeviceThread();


    void on_checkBox_stateChanged(int arg1);
    void on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint);
    void on_stop_clicked();

    void on_DeviceConnect_clicked();
};
#endif // MAINWINDOW_H
