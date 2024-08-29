#include "camera.h"
#include <QDebug>
#include <QTime>
#include <fstream>
#include <opencv2/opencv.hpp>

//Structs, constants are defined here.
const std::vector<cv::Scalar> colors = {cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0)};
const float INPUT_WIDTH = 640.0;
const float INPUT_HEIGHT = 640.0;
const float SCORE_THRESHOLD = 0.2;
const float NMS_THRESHOLD = 0.4;
const float CONFIDENCE_THRESHOLD = 0.04;
const QString yolopath = "/home/lawrence/projects/Phytium2024-Local/masterdev_lawrence/Phytium_Master_Qt/yolov5n.onnx";
const QString classespath = "/home/lawrence/projects/Phytium2024-Local/masterdev_lawrence/Phytium_Master_Qt/classes.txt";

struct Detection
{
    int class_id;
    float confidence;
    cv::Rect box;
};

camera::camera(QString videoPath, QMutex *lock) :
    running(false), data_lock(lock), cameraID(-1), videoPath(videoPath)
{
    fps_calculating = false;
    fps = 0.0;

    frame_width = frame_height = 0;
    video_saving_status = STOPPED;
    video_writer = nullptr;

    motion_detecting_status = false;

}
camera::~camera(){};

/**
 * @brief camera::run Start running a camera class object
 */
void camera::run(){

    qDebug() << "Capturer start running.";
    //0. Preparation for image processing
    running = true;     //Set running bit
    cv::Mat tmp_frame0;  //用于暂时存储从摄像机或视频文件读取的每一帧
    cv::Mat tmp_frame;
    int frame_count = 0;
    auto start = std::chrono::high_resolution_clock::now(); //high resolution clock

    //0.1 Preparation for class detection
    std::string classPath = classespath.toStdString();
    std::vector<std::string> class_list = load_class_list(classPath);

    //0.2 Open a video file(demo)
    std::string stdvideoPath = videoPath.toStdString();
    std::string yoloPath = yolopath.toStdString();
    qDebug() << videoPath;
    // std::cerr << cv::getBuildInformation(); //For debug only
    cv::VideoCapture cap(stdvideoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error opening video file\n";
    }

    //0.3 Load a YOLO net
    cv::dnn::Net net;
    this->load_net(net, yoloPath);

    //1. Continuous circulation
    while(running) {
        cap.read(tmp_frame0);    //Read frames from video file
        if (tmp_frame0.empty()) {
            qDebug() <<"End of stream\n";
            break;}
        cvtColor(tmp_frame0, tmp_frame, cv::COLOR_BGR2RGB); //将帧的颜色空间转换为 RGB

        this->detect(tmp_frame, net, output, class_list);
        frame_count++;
        detections = output.size();

        for (int i = 0; i < detections; ++i)    //Display all bounding boxes to CV
        {
            auto detection = output[i];
            auto box = detection.box;
            auto classId = detection.class_id;
            //std::cout << "classId: " << classId << std::endl; //EXTRACTION
            const auto color = colors[classId % colors.size()];
            cv::rectangle(tmp_frame, box, color, 3);
            cv::rectangle(tmp_frame, cv::Point(box.x, box.y - 20), cv::Point(box.x + box.width, box.y), color, cv::FILLED);
            cv::putText(tmp_frame, class_list[classId].c_str(), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
            //qDebug() << "num:"<< i << Qt::endl;       //EXTRACTION
        }

        if (frame_count >= 30)
        {
            auto end = std::chrono::high_resolution_clock::now();
            fps = frame_count * 1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            frame_count = 0;
            start = std::chrono::high_resolution_clock::now();
        }

        if (fps > 0)
        {

            std::ostringstream fps_label;
            fps_label << std::fixed << std::setprecision(2);
            fps_label << "FPS: " << fps;
            std::string fps_label_str = fps_label.str();

            cv::putText(tmp_frame, fps_label_str.c_str(), cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
        }

        data_lock->lock();
        frame = tmp_frame.clone();
        emit frameCaptured(&frame); //Captured a frame for further process.
        data_lock -> unlock();
    }
}

/**
 * @brief camera::load_class_list
 * @return std::vector<std::string>
 */
std::vector<std::string> camera::load_class_list(std::string classfile)
{
    std::vector<std::string> class_list;
    std::ifstream ifs(classfile);
    if (!ifs) {
        std::cerr << "Error opening file: " << classfile << std::endl;
        // 处理错误，例如返回空的class_list或抛出异常
    }
    std::string line;
    while (getline(ifs, line))
    {
        class_list.push_back(line);
    }
    return class_list;
}

/**
 * @brief camera::load_net
 * @param net An OpenCV DNN neural network file
 */
void camera::load_net(cv::dnn::Net &net, std::string yolopath)
{
    auto result = cv::dnn::readNetFromONNX(yolopath);
    std::cout << "[Target] Running on CPU\n";
    result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    net = result;
}

/**
 * @brief camera::format_yolov5
 * @param source
 * @return cv::Mat
 */
cv::Mat camera::format_yolov5(const cv::Mat &source)
{
    int col = source.cols;
    int row = source.rows;
    int _max = MAX(col, row);
    cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
    source.copyTo(result(cv::Rect(0, 0, col, row)));
    return result;
}

/**
 * @brief camera::detect Core Detection
 * @param image OpenCV image source
 * @param net DNN network
 * @param output Detection results
 * @param className Classes to be detected
 */
void camera::detect(cv::Mat &image, cv::dnn::Net &net, std::vector<Detection> &output, const std::vector<std::string> &className)
{
    //qDebug() << "Yes~";
    cv::Mat blob;
    output.clear();     //Erase all elements
    auto input_image = format_yolov5(image);

    cv::dnn::blobFromImage(input_image, blob, 1./255., cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);
    net.setInput(blob);

    //std::cout << "Blob size: " << blob.size[0] << " x " << blob.size[1] << " x " << blob.size[2] << " x " << blob.size[3] << std::endl;

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

        //YOLOV8 inference
        float *classes_scores = data+4;

        cv::Mat scores(1, className.size(), CV_32FC1, classes_scores);
        cv::Point class_id;
        double maxClassScore;

        minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

        if (maxClassScore > SCORE_THRESHOLD)
        {
            confidences.push_back(maxClassScore);
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

# if 0  //YOLOV5 inference
        float confidence = data[4];
        if (confidence >= CONFIDENCE_THRESHOLD) {
        //qDebug() << confidence << "confidence";
            float * classes_scores = data + 5;
            cv::Mat scores(1, className.size(), CV_32FC1, classes_scores);
            cv::Point class_id;
            double max_class_score;
            minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
            //qDebug() << max_class_score << "max_class_score";
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
#endif
        data += dimensions;

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
