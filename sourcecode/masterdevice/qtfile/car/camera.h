#ifndef CAMERA_H
#define CAMERA_H
#include <QMutex>
#include <QString>
#include <QFile>
#include <QThread>
#include <fstream>
#include <opencv2/opencv.hpp>

class camera : public QThread
{
    Q_OBJECT


    struct Detection
    {
        int class_id;
        float confidence;
        cv::Rect box;
    };

public:
    camera(QString videoPath, QMutex *lock);
    camera(int camID, QMutex *lock);
    ~camera();
    enum VideoSavingStatus {
                            STARTING,
                            STARTED,
                            STOPPING,
                            STOPPED
    };
    void setMotionDetectingStatus(bool status) {
            motion_detecting_status = status;
            motion_detected = false;
            if(video_saving_status != STOPPED) video_saving_status = STOPPING;
        };
    void setRunning(bool run) {running = run; };
private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    QTime *timer;
    cv::Mat frame;
    std::vector<std::string> class_list;
    cv::dnn::Net net;

    std::vector<Detection> output;
    int detections;

    bool fps_calculating;
    float fps;

    int frame_width, frame_height;
    VideoSavingStatus video_saving_status;
    QString saved_video_name;
    cv::VideoWriter *video_writer;

    bool motion_detecting_status;
    bool motion_detected;
protected:
    void run() override;
signals:
    void newFrame();
    void fpsChanged(float fps);
    void frameCaptured(cv::Mat *data);

public slots:
    void updateFrame();
    void calculateFPS(cv::VideoCapture &cap);
private:
    std::vector<std::string> load_class_list();
    void load_net(cv::dnn::Net &net);
    cv::Mat format_yolov5(const cv::Mat &source);
    void detect(cv::Mat &image, cv::dnn::Net &net, std::vector<Detection> &output, const std::vector<std::string> &className);

};

#endif // CAMERA_H
