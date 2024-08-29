#include "camera.h"
#include <QDebug>
#include <QTime>
#include <fstream>
#include <opencv2/opencv.hpp>

camera::camera(QString videoPath, QMutex *lock) : running(false), cameraID(-1), videoPath(videoPath), data_lock(lock)
{
    fps_calculating = false;
        fps = 0.0;

        frame_width = frame_height = 0;
        video_saving_status = STOPPED;
        saved_video_name = "";
        video_writer = nullptr;

        motion_detecting_status = false;

}
camera::~camera(){};

void camera::run(){
    running = true;
   //    cv::VideoCapture cap(cameraID);
       std::vector<std::string> class_list = load_class_list();
       cv::Mat tmp_frame;//用于暂时存储从摄像机或视频文件读取的每一帧

       cv::VideoCapture cap("/mnt/hgfs/share/yolo1.1/cpp/sample.mp4");
       if (!cap.isOpened())
       {
           std::cerr << "Error opening video file\n";
           /*return -1*/;
       }
       while(running) {
       //        cap >> tmp_frame;// 从摄像机或视频文件读取帧
               cap.read(tmp_frame);
               if (tmp_frame.empty()) {
       //            std::cout << "End of stream\n";
                   qDebug() <<"End of stream\n";
                   break;}

               cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);// 将帧的颜色空间转换为 RGB
                       data_lock->lock();
                       frame = tmp_frame;
                       data_lock->unlock();// 对图像加锁，将帧拷贝给全局变量 frame，然后解锁
                       emit frameCaptured(&frame);

}}

void camera::updateFrame()
{
    qDebug()<<"Start";
}

std::vector<std::string> camera::load_class_list()
{
    std::vector<std::string> class_list;
    std::ifstream ifs("classes.txt");
    std::string line;
    while (getline(ifs, line))
    {
        class_list.push_back(line);
    }
    return class_list;
}

void camera::load_net(cv::dnn::Net &net)
{
    auto result = cv::dnn::readNet("yolov5s.onnx");
    std::cout << "Running on CPU\n";
    result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    net = result;
}

const std::vector<cv::Scalar> colors = {cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0)};


cv::Mat camera::format_yolov5(const cv::Mat &source) {
    int col = source.cols;
    int row = source.rows;
    int _max = MAX(col, row);
    cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
    source.copyTo(result(cv::Rect(0, 0, col, row)));
    return result;
}
const float INPUT_WIDTH = 640.0;
const float INPUT_HEIGHT = 640.0;
const float SCORE_THRESHOLD = 0.2;
const float NMS_THRESHOLD = 0.4;
const float CONFIDENCE_THRESHOLD = 0.4;
struct Detection
{
    int class_id;
    float confidence;
    cv::Rect box;
};

void camera::calculateFPS(cv::VideoCapture &cap){
    const int count_to_read = 100;//读取的帧数
        cv::Mat tmp_frame;

        timer->start();
        for(int i = 0; i < count_to_read; i++) {//从视频捕获设备读取100帧图像，这是为了计算读取这些帧所需的时间
                cap >> tmp_frame;
        }
        int elapsed_ms = timer->elapsed();
        fps = count_to_read / (elapsed_ms / 1000.0);//转化单位，ms->s
        fps_calculating = false;
        emit fpsChanged(fps);
}
void camera::detect(cv::Mat &image, cv::dnn::Net &net, std::vector<Detection> &output, const std::vector<std::string> &className) {
    cv::Mat blob;

    auto input_image = format_yolov5(image);

    cv::dnn::blobFromImage(input_image, blob, 1./255., cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);
    net.setInput(blob);
    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    float x_factor = input_image.cols / INPUT_WIDTH;
    float y_factor = input_image.rows / INPUT_HEIGHT;

    float *data = (float *)outputs[0].data;

    const int dimensions = 85;
    const int rows = 25200;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < rows; ++i) {

        float confidence = data[4];
        if (confidence >= CONFIDENCE_THRESHOLD) {

            float * classes_scores = data + 5;
            cv::Mat scores(1, className.size(), CV_32FC1, classes_scores);
            cv::Point class_id;
            double max_class_score;
            minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
            if (max_class_score > SCORE_THRESHOLD) {

                confidences.push_back(confidence);

                class_ids.push_back(class_id.x);

                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];
                int left = int((x - 0.5 * w) * x_factor);
                int top = int((y - 0.5 * h) * y_factor);
                int width = int(w * x_factor);
                int height = int(h * y_factor);
                boxes.push_back(cv::Rect(left, top, width, height));
            }

        }

        data += 85;

    }

    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);
    for (int i = 0; i < nms_result.size(); i++) {
        int idx = nms_result[i];
        Detection result;
        result.class_id = class_ids[idx];
        result.confidence = confidences[idx];
        result.box = boxes[idx];
        output.push_back(result);
    }
}
