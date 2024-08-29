#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "camera.h"
#include "form.h"
#include "QLabel"
#include "device_thread.h"
#include <QtMultimedia/QtMultimedia>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QPushButton>
#include <QDebug>
#include <QMessageBox>

#include <QApplication>
#include <QFileDialog>
#include <QPixmap>
#include <QKeyEvent>
#include <QCameraInfo>
#include <QGridLayout>
#include <QIcon>
#include <QStandardItem>
#include <QSize>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QToolBar>
#include <QtConcurrent>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), capturer(nullptr)
{
    ui->setupUi(this);
    timer =new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(importFrame()));
//    Form* form=new Form(this);//指定父窗口为this
//    form->show();

#if 0


    InfoDialog* w=new InfoDialog;
    w->show();
    InfoDialog* w=new InfoDialog(this);//必须指定了父对象才能自动回收内存
    w->show();
#else

    initUI();
    data_lock = new QMutex();

#endif



}

void MainWindow::initUI()
{

    connect(ui->actionExit,SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(ui->actionRead_Camera,SIGNAL(triggered(bool)), this, SLOT(showCameraInfo()));
    connect(ui->actionOpen_Camera, SIGNAL(triggered(bool)), this, SLOT(openCamera()));
    connect(ui->actionCalculate_FPS,SIGNAL(triggered(bool)),this,SLOT(calculateFPS()));



    //setup status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Monitor is Ready");
    qDebug()<<"Over";
}

void MainWindow::showCameraInfo()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QString info = QString("Available Cameras: \n");

    foreach (const QCameraInfo &cameraInfo, cameras) {
        info += "  - " + cameraInfo.deviceName() +  ": ";
        info += cameraInfo.description() + "\n";
    }
    QMessageBox::information(this, "Cameras", info);
}

#ifdef CAR_USE_QT_CAMERA
void MainWindow::openCamera()
{
    camera->settureMode(QCamera::CaptureVideo);
    camera->start();
}
#else
void MainWindow::openCamera()
{
    connect(capturer, &camera::fpsChanged, this, &MainWindow::updateFPS);

   /* if(capturer != nullptr) {
            // if a thread is already running, stop it
            capturer->setRunning(false);
            disconnect(capturer,&camera::frameCaptured,this,&MainWindow::updateFrame);

//            connect(capturer, &camera::finished, capturer, &camera::deleteLater);
        }
    int camID=0;
    capturer=new camera(camID, data_lock);
    connect(capturer,&camera::frameCaptured,this,&MainWindow::updateFrame);
    capturer->start();
    mainStatusLabel->setText(QString("Capturing Camera %1").arg(camID));//更新主窗口的状态标签
    ui->checkBox->setCheckState(Qt::Unchecked);//把monitor这个复选框初始化为空
*/
//    cv::VideoCapture cap;
            cap.open("/mnt/hgfs/share/yolo1.1/dark/darkTest.mp4");
//            if (!cap.isOpened()) {
//                qDebug() << "Error: Unable to open video file";
//                return;
//            }
//            fps=cap.get(cv::CAP_PROP_FPS);
//            qDebug()<<"FPS:"<<fps;
//            qDebug()<<"video open success";
//            // 创建定时器来更新帧
//            QTimer *timer = new QTimer(this);
//            connect(timer, &QTimer::timeout, this,[=,&cap]() {
//                cv::Mat frame;
timer->start(33);
//               cap>>frame; // 读取视频帧

//                if (!cap.read(frame)) {
//                    qDebug() << "Error: Failed to read frame";
//                    return;
//                }
//                qDebug() << "Frame read successfully";
//                  cap.read(frame);
//                // 将 OpenCV 的 Mat 转换为 QImage
//                QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
//                image = image.rgbSwapped();

                // 在 QGraphicsScene 中添加图像
//                scene->clear();
//                scene->addPixmap(QPixmap::fromImage(image));

//                // 重设视图大小
//                view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
//            });
//            timer->start(1000 / cap.get(cv::CAP_PROP_FPS)); //
qDebug()<<"WELL";
}
#endif

void MainWindow::importFrame(){
    cap >> frame;
    cvtColor(frame, frame, cv::COLOR_BGR2RGB);//only RGB of Qt
    QImage srcQImage = QImage((uchar*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(srcQImage));
    ui->label->resize(srcQImage.size());
    ui->label->show();
}
void MainWindow::calculateFPS()
{
//    cap->startCalcFPS();
    fps=cap.get(cv::CAP_PROP_FPS);
    qDebug()<<"OK,FPS:"<<fps;

    updateFPS(fps);
}

void MainWindow::updateFPS(float fps)
{
    mainStatusLabel->setText(QString("FPS of current camera is %1").arg(fps));
}

void MainWindow::updateFrame(cv::Mat *mat)
{
    data_lock->lock();
    currentFrame = *mat;
    data_lock->unlock();

    QImage frame(
        currentFrame.data,
        currentFrame.cols,
        currentFrame.rows,
        currentFrame.step,
        QImage::Format_RGB888);
    QPixmap image = QPixmap::fromImage(frame);

}

MainWindow::~MainWindow()
{

    delete ui;
}


void MainWindow::on_checkBox_stateChanged(int arg1)
{
    this->calculateFPS();
//    c=new camera;
//    c->updateFrame();
}

void MainWindow::on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint)
{

}

void MainWindow::on_stop_clicked()
{
    timer->stop();
    cap.release();
}

void MainWindow::on_DeviceConnect_clicked()
{
    const int numThreads = 5;
    QThread* threads[numThreads];
    device_thread* deviceThreads[numThreads];

    for (int i = 0; i < numThreads; ++i) {
    threads[i] = new QThread();
    deviceThreads[i] = new device_thread(i, nullptr);
    deviceThreads[i]->moveToThread(threads[i]);

    // 连接每个子线程的工作完成信号与退出子线程的槽函数
    QObject::connect(deviceThreads[i], &device_thread::workFinished, threads[i], &QThread::quit);

   // 连接应用程序的退出信号与每个子线程的退出槽函数
//    QObject::connect(&capturer, &QCoreApplication::aboutToQuit, threads[i], &QThread::quit);

    // 在每个子线程中执行工作
    QMetaObject::invokeMethod(deviceThreads[i], "doWork", Qt::QueuedConnection);

    threads[i]->start();
}
}
