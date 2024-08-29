/* Team: Phytium Flyers
 * by Lawrence Leung 2024
 *
 * ultrafastlane.h
 * For detection of multiple drive lanes on detected images, maximum 4 lanes,
 * based on ONNX and OpenCV.
 * this code is migrated from Python scripts, based on repository
 * "onnx-Ultra-Fast-Lane-Detection-Inference" by user ibaiGorordo on GitHub.
*/

#ifndef ULTRAFASTLANE_H
#define ULTRAFASTLANE_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <chrono>

/**
 * @brief Selecting pretrained ONNX model type, from GitHub.
 */
enum ModelType {
    TUSIMPLE,
    CULANE,
};

/**
 * @brief Configuration of selected pretrained ONNX model type,
 * specifically used on initializing the ONNX model before the program
 * is ready to use.
 */
class ModelConfig : public QObject
{
    Q_OBJECT
public:
    int img_w;
    int img_h;
    std::vector<int> row_anchor;
    int griding_num;
    int cls_num_per_lane;

    std::vector<cv::Scalar> lane_colors = {
        cv::Scalar(0, 0, 255),
        cv::Scalar(0, 255, 0),
        cv::Scalar(255, 0, 0),
        cv::Scalar(0, 255, 255)
    };

    std::vector<int> tusimple_row_anchor = {
        64,  68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112,
        116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156, 160, 164,
        168, 172, 176, 180, 184, 188, 192, 196, 200, 204, 208, 212, 216,
        220, 224, 228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268,
        272, 276, 280, 284
    };

    std::vector<int> culane_row_anchor = {
        121, 131, 141, 150, 160, 170, 180, 189, 199, 209, 219, 228, 238,
        248, 258, 267, 277, 287
    };

    explicit ModelConfig(QObject *parent = nullptr, ModelType model_type = CULANE) {
        if (model_type == ModelType::TUSIMPLE) {
            init_tusimple_config();
        } else {
            init_culane_config();
        }
    }
signals:
    /* No signals really used here. Just keep aligned with the format of
     * succeeding class QObject. */

private:
    /**
     * @brief init_tusimple_config
     * Configuration of TUSIMPLE type of ONNX model from GitHub.
     */
    void init_tusimple_config() {
        img_w = 1280;
        img_h = 720;
        row_anchor = tusimple_row_anchor;
        griding_num = 100;
        cls_num_per_lane = 56;
    }

    /**
     * @brief init_tusimple_config
     * Configuration of CULANE type of ONNX model from GitHub.
     */
    void init_culane_config() {
        img_w = 1640;
        img_h = 590;
        row_anchor = culane_row_anchor;
        griding_num = 200;
        cls_num_per_lane = 18;
    }
};

/**
 * @brief The UltraFastLaneDetector class
 * Main class for all operations and features, like inferencing
 * and post processing the output drive lane data for visualization, etc.
 */
class UltraFastLaneDetector : public QObject
{
    Q_OBJECT
private:
    double fps;
    std::chrono::time_point<std::chrono::steady_clock> timeLastPrediction;
    int frameCounter;

    cv::dnn::Net net;

    const int channels = 288;
    const int inputHeight = 288;
    const int inputWidth = 800;
    const int numPoints = 201;
    const int numAnchors = 18;

    /* Other member variables to be added here. */

public:
    std::pair<std::vector<std::vector<cv::Point>>, std::vector<bool>> result;
    ModelConfig *cfg;
    cv::Mat image;

    /**
     * @brief UltraFastLaneDetector
     * Construction function of this class
     * @param parent succeeded from class QObject, no use but
     * just for fitting the format of succession.
     * @param model_path the path of ONNX formatted pretrained model file.
     * @param model_type pretrained model type, either CULANE or TUSIMPLE.
     */
    explicit UltraFastLaneDetector(
        QObject *parent = nullptr,
        const std::string model_path = "/home/lawrence/projects/Phytium2024-Local/masterdev_lawrence/Phytium_Master_Qt/models/lane.onnx",
        ModelType model_type = CULANE,
        std::string image_path = "/home/lawrence/projects/Phytium2024-Local/other_repos/onnx-Ultra-Fast-Lane-Detection-Inference/input2.jpg"
        ) {

        fps = 0;
        timeLastPrediction = std::chrono::steady_clock::now();
        frameCounter = 0;

        /* Load model configuration settings */
        cfg = new ModelConfig(this, model_type);

        loadNet(model_path);

        /* 0 for debug test. */
        image = cv::imread(image_path, cv::IMREAD_COLOR);
        cv::Mat input_tensor = prepareInput(image, inputWidth, inputHeight);

        cv::Mat output = inference(net, input_tensor);
        result = processOutput(output, *cfg);
        //cv::Mat visualization = drawLanes(image, result, true);

        //cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);
        //cv::imshow("Display window", visualization);
        //cv::waitKey(0);
        //cv::destroyAllWindows();
    }

    void loadNet(std::string model_path) {
        net = cv::dnn::readNetFromONNX(model_path.c_str());
        std::cout << "[UltraFastLane] Running on CPU. \n";
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    cv::Mat detect_lanes(const cv::Mat &image, bool draw_points = true) {
        /* 1. Prepare the input tensor from the image */
        cv::Mat input_tensor = prepareInput(image, inputWidth, inputHeight);
        /* 2. Perform inference on the image */
        //std::vector<float> output = inference(input_tensor);
        /* 3. Process output data to get lanes points and detection status */
        //auto [lanes_points, lanes_detected] = processOutput(output, cfg);
        /* 4. Draw lanes on the image and return the resultant image */
        //cv::Mat visualization_img = drawLanes(image, lanes_points, lanes_detected, cfg, draw_points);
        //return visualization_img;

    }

    cv::Mat prepareInput(const cv::Mat& image, int inputWidth, int inputHeight) {
        // 将图像从BGR转换为RGB
        cv::Mat img;
        cv::cvtColor(image, img, cv::COLOR_BGR2RGB);

        // 调整图像大小
        cv::Mat resizedImg;
        cv::resize(img, resizedImg, cv::Size(inputWidth, inputHeight));

        // 将像素值缩放到-1到1之间
        resizedImg.convertTo(resizedImg, CV_32F, 1.0 / 255);  // 转换为浮点并归一化到[0,1]
        cv::Mat mean = (cv::Mat_<float>(1, 3) << 0.485, 0.456, 0.406);
        cv::Mat std = (cv::Mat_<float>(1, 3) << 0.229, 0.224, 0.225);

        // 将mean和std扩展到与图像同样的尺寸，以便进行逐元素操作
        cv::Mat meanResized, stdResized;
        cv::resize(mean, meanResized, resizedImg.size());
        cv::resize(std, stdResized, resizedImg.size());

        cv::Mat meanResized3ch, stdResized3ch;
        std::vector<cv::Mat> meanChannels(3, meanResized);
        std::vector<cv::Mat> stdChannels(3, stdResized);

        // 合并单通道到三通道
        cv::merge(meanChannels, meanResized3ch);
        cv::merge(stdChannels, stdResized3ch);

        // 现在 meanResized3ch 和 stdResized3ch 为三通道，可以与 resizedImg 进行运算
        cv::subtract(resizedImg, meanResized3ch, resizedImg);
        cv::divide(resizedImg, stdResized3ch, resizedImg);

        // for debug only.
        std::cout << "resizedImg size: " << resizedImg.size() << std::endl;
        std::cout << "meanResized size: " << meanResized3ch.size() << std::endl;
        std::cout << "stdResized size: " << stdResized3ch.size() << std::endl;
        std::cout << "resizedImg channels: " << resizedImg.channels() << std::endl;
        std::cout << "meanResized channels: " << meanResized3ch.channels() << std::endl;
        std::cout << "stdResized channels: " << stdResized3ch.channels() << std::endl;

        cv::subtract(resizedImg, meanResized3ch, resizedImg);
        cv::divide(resizedImg, stdResized3ch, resizedImg);

        // 转换通道顺序为CHW
        cv::Mat chwImg;
        cv::dnn::blobFromImage(resizedImg, chwImg);

        // 检查chwImg的尺寸
        if (chwImg.dims == 4 &&
            chwImg.size[0] == 1 &&
            chwImg.size[1] == 3 &&
            chwImg.size[2] == 288 &&
            chwImg.size[3] == 800) {
            std::cout << "chwImg has the correct dimensions: (1, 3, 288, 800)" << std::endl;
        } else {
            std::cout << "chwImg dimensions are incorrect." << std::endl;
        }

        // 检查元素类型
        if (CV_MAT_DEPTH(chwImg.type()) == CV_32F) {
            std::cout << "chwImg element type is float32." << std::endl;
        } else {
            std::cout << "chwImg element type is not float32." << std::endl;
        }

        // 为 int8 量化预留代码空间
        // TODO: 为将模型转换为 int8 量化准备代码逻辑

        return chwImg;
    }

    cv::Mat inference(cv::dnn::Net& net, const cv::Mat& inputTensor) {
        // 将输入数据设置到网络中
        net.setInput(inputTensor);

        // 进行前向推理
        cv::Mat output = net.forward();

        // 检查输出的形状和类型
        std::cout << "Output dimensions: " << output.size[0] << "x" << output.size[1] << "x"
                  << output.size[2] << "x" << output.size[3] << std::endl;
        std::cout << "Output type: " << output.type() << std::endl;

        // 检查输出类型是否为float (CV_32F)
        if (output.type() == CV_32F) {
            std::cout << "Output type is float32." << std::endl;
        } else {
            std::cout << "Output type is not float32." << std::endl;
        }

        return output;
    }

    std::pair<std::vector<std::vector<cv::Point>>, std::vector<bool>>
        processOutput(
        const cv::Mat output,
        const ModelConfig& cfg) {

        // 假设output是一个形状为(1, 201, 18, 4)的cv::Mat
        std::vector<int> newShape = {201, 18, 4};
        cv::Mat processedOutput = output.reshape(1, newShape); // 重塑为(201, 18, 4)
        std::cout << "processedOutput: " << processedOutput.size[0] << ", " << processedOutput.size[1] << ", " << processedOutput.size[2] << std::endl;

        // Softmax process
        std::vector<cv::Range> ranges = {cv::Range(0, 200), cv::Range::all(), cv::Range::all()};
        cv::Mat slicedOutput = processedOutput(ranges);
        std::cout << "slicedOutput: " << slicedOutput.size[0] << ", " << slicedOutput.size[1] << ", " << slicedOutput.size[2] << std::endl;
        cv::Mat prob = softmax(slicedOutput);
        std::cout << "prob: " << prob.size[0] << ", " << prob.size[1] << ", " << prob.size[2] << std::endl;
        //Myread3dMat(prob);

        static cv::Mat idx = cv::Mat::zeros(1, cfg.griding_num, CV_32F);
        std::cout << "cfg.griding_num: " << cfg.griding_num << std::endl;
        for (int i = 0; i < cfg.griding_num; ++i) {
            idx.at<float>(0, i) = static_cast<float>(i + 1);
        }

        cv::Mat loc = cv::Mat::zeros(18, 4, CV_32F);
        // 执行元素乘法和沿着特定维度求和
        for (int i = 0; i < prob.size[0]; ++i) {
            float idxValue = idx.at<float>(0, i);
            for (int j = 0; j < prob.size[1]; ++j) {
                for (int k = 0; k < prob.size[2]; ++k) {
                    loc.at<float>(j, k) += prob.at<float>(i, j * 4 + k) * idxValue;
                }
            }
        }
        Myread2dFloatMat(loc);
        Myread3dFloatMat(processedOutput);

        cv::Mat maxIndexMat(18, 4, CV_32F);
        for (int j = 0; j < processedOutput.size[1]; ++j) {
            for (int k = 0; k < processedOutput.size[2]; ++k) {
                float maxValue = -FLT_MAX;
                float maxIndex = 0.0f;  // 以浮点数存储索引
                for (int i = 0; i < processedOutput.size[0]; ++i) {
                    float value = processedOutput.at<float>(i, j, k);
                    if (value > maxValue) {
                        maxValue = value;
                        maxIndex = static_cast<float>(i);  // 以浮点数存储索引
                    }
                }
                maxIndexMat.at<float>(j, k) = maxIndex;  // 存储的是类似1.0, 2.0这样的浮点数
            }
        }
        Myread2dFloatMat(maxIndexMat);

        for (int i = 0; i < loc.rows; i++) {
            for (int j = 0; j < loc.cols; j++) {
                if (maxIndexMat.at<float>(i, j) == static_cast<float>(cfg.griding_num)) {
                    loc.at<float>(i, j) = 0.0f;
                }
            }
        }
        Myread2dFloatMat(loc);
        // 最后，将loc赋值给processedOutput
        processedOutput = loc.clone();

        std::vector<float> col_sample;
        float start = 0.0f;
        float end = 800.0f - 1;
        float step = (end - start) / (cfg.griding_num - 1);
        for (int i = 0; i < cfg.griding_num; i++) {
            col_sample.push_back(start + step * i);
        }
        float col_sample_w = col_sample[1] - col_sample[0];

        std::vector<std::vector<cv::Point>> lanes_points;
        std::vector<bool> lanes_detected;
        int max_lanes = processedOutput.size[1];  // 这里假设processedOutput已经是(18, 4)的形状

        // Lane Inference
        for (int lane_num = 0; lane_num < max_lanes; ++lane_num) {
            std::vector<cv::Point> lane_points;
            int count = 0;

            // 检查这个车道上是否有检测到的点
            for (int point_num = 0; point_num < processedOutput.rows; ++point_num) {
                if (processedOutput.at<float>(point_num, lane_num) > 0) {
                    count++;
                }
            }

            if (count > 2) {
                lanes_detected.push_back(true);
                // 处理每个车道上的点
                for (int point_num = 0; point_num < processedOutput.rows; ++point_num) {
                    if (processedOutput.at<float>(point_num, lane_num) > 0) {
                        int x = static_cast<int>(processedOutput.at<float>(point_num, lane_num) * col_sample_w * cfg.img_w / 800) - 1;
                        int y = static_cast<int>(cfg.img_h * (cfg.row_anchor[cfg.cls_num_per_lane - 1 - point_num] / 288.0)) - 1;
                        lane_points.emplace_back(x, y);
                    }
                }
            } else {
                lanes_detected.push_back(false);
            }
            lanes_points.push_back(lane_points);
        }

        return {lanes_points, lanes_detected};
    }

    cv::Mat softmax(const cv::Mat& input) {
        std::cout << "SOFTMAX INPUT dimensions: " << input.size[0] << ", " << input.size[1] << ", " << input.size[2] << std::endl;
        // 预期输入形状为 (200, 18, 4)
        cv::Mat output = cv::Mat::zeros(input.dims, input.size, CV_32F);

        // 手动实现沿着0轴的最大值计算和softmax计算
        for (int i = 0; i < input.size[1]; ++i) {
            for (int j = 0; j < input.size[2]; ++j) {
                float maxVal = -FLT_MAX;
                for (int k = 0; k < input.size[0]; ++k) {
                    float val = input.at<float>(k, i, j);
                    if (val > maxVal) {
                        maxVal = val;
                    }
                }

                float sumExp = 0.0;
                for (int k = 0; k < input.size[0]; ++k) {
                    float expVal = std::exp(input.at<float>(k, i, j) - maxVal);
                    sumExp += expVal;
                    output.at<float>(k, i, j) = expVal;
                }

                for (int k = 0; k < input.size[0]; ++k) {
                    output.at<float>(k, i, j) /= sumExp;
                }
            }
        }

        return output;
    }

    cv::Mat drawLanes(
        cv::Mat inputImg,
        std::pair<std::vector<std::vector<cv::Point>>,
        std::vector<bool>> lanesData,
        bool drawPoints = true) {
        // Resize the input image
        cv::Mat visualizationImg;
        cv::resize(inputImg, visualizationImg, cv::Size(cfg->img_w, cfg->img_h), 0, 0, cv::INTER_AREA);

        // Draw a mask for the current lane
        if (lanesData.second[1] && lanesData.second[2]) {
            cv::Mat laneSegmentImg = visualizationImg.clone();

            std::vector<cv::Point> points;
            points.insert(points.end(), lanesData.first[1].begin(), lanesData.first[1].end());
            std::reverse(lanesData.first[2].begin(), lanesData.first[2].end());
            points.insert(points.end(), lanesData.first[2].begin(), lanesData.first[2].end());

            std::vector<std::vector<cv::Point>> pts = {points};
            cv::fillPoly(laneSegmentImg, pts, cv::Scalar(255, 191, 0));
            cv::addWeighted(visualizationImg, 0.7, laneSegmentImg, 0.3, 0, visualizationImg);
        }


        // Draw the detected lane points
        if (drawPoints) {
            for (int laneNum = 0; laneNum < lanesData.first.size(); laneNum++) {
                for (auto& lanePoint : lanesData.first[laneNum]) {
                    cv::circle(visualizationImg, lanePoint, 3, cfg->lane_colors[laneNum % cfg->lane_colors.size()], -1);
                }
            }
        }

        return visualizationImg;
    }

    void Myread2dFloatMat(cv::Mat mat) {
        std::cout << "READING 2D MAT" << std::endl;
        std::cout << "SIZE: " << mat.rows << ", " << mat.cols << std::endl;
        for (int i = 0; i < mat.rows; ++i) {
            for (int j = 0; j < mat.cols; ++j) {
                std::cout << "( " << i << ", " << j << "): " << mat.at<float>(i, j) << std::endl;
            }
            std::cout << std::endl;
        }
    }

    void Myread3dFloatMat(cv::Mat mat) {
        std::cout << "READING 3D MAT" << std::endl;
        std::cout << "SIZE: " << mat.size[0] << ", " << mat.size[1] << ", " << mat.size[2] << std::endl;
        for (int i = 0; i < mat.size[0]; ++i) {
            for (int j = 0; j < mat.size[1]; ++j) {
                for (int k = 0; k < mat.size[2]; ++k) {
                    std::cout << "( " << i << ", " << j << ", " << k << "): " << mat.at<float>(i, j, k) << std::endl;
                    /*
                    std::cout << "[" << i << ", " << j << ", " << k << "]: ("
                              << pixel[0] << ", " << pixel[1] << ", " << pixel[2] << ")\n";
                    */
                }
            }
        }
    }


signals:
};


#endif // ULTRAFASTLANE_H
