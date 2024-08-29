#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <opencv4/opencv2/opencv.hpp>
#include "camera.h"     // Header file containing video operations
#include "ultrafastlane.h"
//#include "ultrafastlane.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    struct Detection {  //Visual Application for Yolov5
        cv::Rect box;   //Detection Box
        int class_id;   //Detected Class ID
    };

private:
    /* Functions */
    void initUI();

    /* Objects */
    Ui::MainWindow *ui;     //Main Window
    UltraFastLaneDetector *lanedetector;

    // System internal components
    bool running;         //Check if system is running currently
    QTimer *timer;        //Mainwindow timer
    QMutex *data_lock;    //Mutex for reading CV matrices

    // OpenCV components
    cv::Mat frame;        //Source frame
    cv::Mat currentFrame; //Current frame
    cv::VideoCapture cap; //Capturer (Video)
    QGraphicsScene *imageScene;
    QGraphicsScene *scene;
    QGraphicsView *view;
    double fps;

    // Application components
    //1. Visual detection components
    camera *capturer;     //Self-defined camera class
    std::vector<Detection> detections;

private slots:
    void updateFrame(cv::Mat *);
    // void startDeviceThread();    //todo
    void calculateFPS();

};
#endif // MAINWINDOW_H
