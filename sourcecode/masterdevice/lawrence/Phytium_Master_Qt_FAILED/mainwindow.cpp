/*  Test 20240305 for Phutium
 *  by Lawrence Leung
*/

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QTimer>
#include <QDebug>

const QString videopath = "/home/lawrence/projects/Phytium2024-Local/masterdev_lawrence/sample.mp4";

MainWindow::MainWindow(QWidget *parent) //Class Construction Function
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //System internal components init
    timer = new QTimer(this);
    data_lock = new QMutex();

    //Graphical UI init
    initUI();

}

MainWindow::~MainWindow()   //Class Deconstruct Function
{
    delete ui;
}

/**
 * @brief MainWindow::initUI Graphical UI Initialization
 */
void MainWindow::initUI()
{
    //1. Connect items on main menu to functions
    qDebug() << "Graphical UI Inited.";

    /********************************************/
    //1. Start video inference (as an demo)
    capturer = new camera(videopath, data_lock);    //A new model of capturer

    connect(capturer, SIGNAL(frameCaptured(cv::Mat*)), this, SLOT(updateFrame(cv::Mat*)));

    lanedetector = new UltraFastLaneDetector(
        NULL,
        "/home/lawrence/projects/Phytium2024-Local/masterdev_lawrence/Phytium_Master_Qt/models/lane.onnx",
        CULANE,
        "/home/lawrence/projects/Phytium2024-Local/other_repos/onnx-Ultra-Fast-Lane-Detection-Inference/input.jpg"
        );

    cv::Mat visualization = lanedetector->drawLanes(lanedetector->image,
                            lanedetector->result,
                            true);

    QImage qImage;
        // 对于3通道的颜色图像(BGR格式)
        cv::cvtColor(visualization, visualization, cv::COLOR_BGR2RGB);
        qImage = QImage((const unsigned char*)(visualization.data), visualization.cols, visualization.rows, visualization.step, QImage::Format_RGB888);

    // 将QImage转换为QPixmap
    QPixmap pixmap = QPixmap::fromImage(qImage);


    // 在QLabel中显示QPixmap
    ui->label->setPixmap(pixmap);

    //capturer->start();  //Due to the successive relation, executing "start" function is equivalent to executing "run" function.



}

/**
 * @brief MainWindow::calculateFPS
 */
void MainWindow::calculateFPS()
{
    fps=cap.get(cv::CAP_PROP_FPS);
    qDebug() << "Visual Source FPS:" << fps;
}

/**
 * @brief MainWindow::updateFrame
 * @param mat
 */
void MainWindow::updateFrame(cv::Mat *mat)
{
    data_lock->lock();
    currentFrame = *mat;
    data_lock->unlock();

    // 确保图像是以 RGB 格式显示，而不是 BGR
    //cv::cvtColor(currentFrame, currentFrame, cv::COLOR_);

    QImage frame(
        currentFrame.data,
        currentFrame.cols,
        currentFrame.rows,
        currentFrame.step,
        QImage::Format_RGB888);
    QPixmap image = QPixmap::fromImage(frame);

    ui->label->setPixmap(image);
    // 设置 QLabel 来自动缩放内容
    //ui->label->setScaledContents(true);
    // 显示图像
    //ui->label->setPixmap(image.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::FastTransformation));

}
