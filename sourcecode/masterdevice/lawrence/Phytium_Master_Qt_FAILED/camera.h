#ifndef CAMERA_H
#define CAMERA_H

#include <QMutex>
#include <QString>
#include <QFile>
#include <QThread>
#include <opencv2/opencv.hpp>

/**
 * @brief The camera class, which is used as a program thread.
 */
class camera: public QThread
{
    Q_OBJECT

    struct Detection {
        int class_id;
        float confidence;
        cv::Rect box;
    };

    // Functions & Enumerations
public:
    camera(QString videopath, QMutex *lock);    //Support video file input
    camera(int camID, QMutex *lock);            //Support real camera input
    ~camera();

    enum VideoSavingStatus {
        STARTING,
        STARTED,
        STOPPING,
        STOPPED
    };

    // Variables
private:
    //System components
    bool running;
    QMutex *data_lock;
    QTime *timer;

    //Video capture components
    int cameraID;
    QString videoPath;
    QString saved_video_name;
    VideoSavingStatus video_saving_status;
    cv::VideoWriter *video_writer;
    cv::Mat frame;

    float fps;
    bool fps_calculating;

    //Target detection neural network components
    cv::dnn::Net net;

    //Target detection results
    std::vector<Detection> output;
    std::vector<std::string> class_list;
    int detections;
    int frame_width, frame_height;
    bool motion_detecting_status;
    bool motion_detected;

    //Functions
protected:
    void run() override;

signals:
    //Video capture components
    void frameCaptured(cv::Mat *data);

    //Functions
private:
    //Target detection neural network components
    std::vector<std::string> load_class_list(std::string classfile);
    void load_net(cv::dnn::Net &net, std::string yolopath);
    cv::Mat format_yolov5(const cv::Mat &source);

    //Target detection results
    void detect(cv::Mat &image, cv::dnn::Net &net, std::vector<Detection> &output, const std::vector<std::string> &className);

};

#endif // CAMERA_H
