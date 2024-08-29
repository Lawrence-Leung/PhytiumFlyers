# stairsDetector.py
# 基于OpenCV机器视觉的楼梯检测程序
# by Lawrence Leung 2024

import time
import cv2
import numpy as np
import random
import matplotlib.pyplot as plt
from scipy.interpolate import UnivariateSpline
from scipy.signal import find_peaks
from classes import yolov5
from PySide6.QtCore import QObject, Signal, QTimer
from PySide6.QtGui import QImage

class Notifier(QObject):
    stairs_changed = Signal(str)
    stair_status_msg = Signal(str)
# 连线检测器
class LineDetector:
    def __init__(self):
        # 固定随机数种子，以保证每次运行结果一致
        random.seed(42)

    # 完整的检测流程，输入image，for debug only
    def detectLines(self, image):
        # step 1 检测线条
        lines = cv2.HoughLinesP(image, 1, np.pi / 180, threshold=50, minLineLength=50, maxLineGap=10)
        if lines is not None:
            # step 2 计算线条方程
            line_equations = self.calculateLineEquations(lines)
            # step 3 分析斜率k 分布
            self.filtered_lines, max_peak_x = self.analysisFilterLinesAroundPeak(line_equations)
            # step 4 分析截距b 分布
            self.rough_lines = self.analysisInterceptDistribution(self.filtered_lines, max_peak_x)
            # 返回：粗处理后的线条
            return self.rough_lines, lines
        else:
            return None

    # 计算线条方程
    # 输出一个List，由大量的(斜率, 截距)tuple 组成。
    def calculateLineEquations(self, lines):
        line_equations = []
        for line in lines:
            for x1, y1, x2, y2 in line:
                if x2 != x1:
                    # 计算斜率和截距
                    m = (y2 - y1) / (x2 - x1)
                    b = y1 - m * x1
                    line_equations.append((m, b))
                else:
                    # 处理垂直线段的情况
                    line_equations.append((float('inf'), x1))  # 使用 'inf' 表示斜率无穷大，存储x坐标作为截距
        return line_equations

    # 分析斜率k 分布
    # 输入：
    # lines，一个list，由大量(斜率, 截距)tuple组成；
    # isplot, 是否绘制图像
    # 返回值：
    # filtered_lines，一个list，由大量(斜率, 截距)tuple组成；
    # max_peak_x，峰值的x坐标
    def analysisFilterLinesAroundPeak(self, lines, isplot = False):
        slopes = [slope for slope, _ in lines]
        slopes = np.array(slopes, dtype=float)
        finite_slopes = slopes[np.isfinite(slopes)]
        # Calculate the histogram of the slopes
        count, bins = np.histogram(finite_slopes, bins='auto', density=False)
        # Use the middle of each bin for the x values
        x = (bins[:-1] + bins[1:]) / 2
        # Find the peaks in the histogram
        peaks, _ = find_peaks(count)
        # Find the highest peak
        max_peak_idx = np.argmax(count[peaks])
        max_peak_x = x[peaks][max_peak_idx]
        # Define the range around the peak
        range_min, range_max = max_peak_x - 0.25, max_peak_x + 0.25 # todo：range是经验值
        # Filter the lines based on the slope being within the range of the main peak +- 3
        filtered_lines = [line for line in lines if range_min <= line[0] <= range_max]

        # 当且仅当绘制图像时，为true
        if isplot is True:
            # Create a spline to fit the histogram
            spline = UnivariateSpline(x, count, s=0)
            # Generate more points to create a smooth line
            x_smooth = np.linspace(x.min(), x.max(), 1000)
            y_smooth = spline(x_smooth)
            # Plot the results
            plt.figure(figsize=(8, 3))
            plt.hist(slopes, bins='auto', density=True, alpha=0.5, label='Histogram of slopes')
            plt.plot(x_smooth, y_smooth, label='Spline of distribution')
            plt.xlabel('Slope')
            plt.ylabel('Density')
            plt.title('Probability Distribution of Slopes')
            plt.legend()
            plt.show()

        return filtered_lines, max_peak_x

    # 分析截距b 分布
    # 输入：
    # lines，一个list，由大量(斜率, 截距)tuple组成；
    # max_peak_x, 峰值的x坐标
    # isplot, 是否绘制图像
    # 输出：
    # rough_lines, 所有得到的楼梯的粗处理（不是精处理）线条，一个list，由大量(斜率, 截距)tuple组成；
    def analysisInterceptDistribution(self, lines, max_peak_x, isplot = False):
        # Extract the intercepts from the list of tuples
        intercepts = [intercept for _, intercept in lines]
        # Calculate the histogram of the intercepts
        count, bins = np.histogram(intercepts, bins=60, density=True)
        # Use the middle of each bin for the x values
        x = (bins[:-1] + bins[1:]) / 2
        # Create a spline to fit the histogram
        spline = UnivariateSpline(x, count, s=0)
        # Generate more points to create a smooth line
        x_smooth = np.linspace(x.min(), x.max(), 1000)
        y_smooth = spline(x_smooth)
        # 从截距曲线中寻找波峰
        # distance：波层之间的最小距离
        # height：波峰的阈值高度
        y_peaks, _ = find_peaks(y_smooth, distance=20, height=0.0022)    # todo：需要进一步调节最小距离、阈值高度
        # 将拟合好的线条提取出来
        rough_lines = []
        for y_peak in y_peaks:
            if y_peak > 250:    # todo: 进一步的判断算法有待改进
                rough_lines.append((max_peak_x, y_peak))   # 得到了新的初步分析的楼梯的线条，但仍需进一步筛选，以确认属于楼梯

        # 当且仅当绘制图像时，设置为True
        if isplot is True:
            plt.figure(figsize=(8, 3))
            plt.hist(intercepts, bins='auto', density=True, alpha=0.5, label='Histogram of intercepts')
            plt.plot(x_smooth, y_smooth, label='Spline of distribution')
            plt.xlabel('Intercept')
            plt.ylabel('Density')
            plt.title('Probability Distribution of Intercepts')
            plt.legend()
            plt.show()

        return rough_lines


# 楼梯检测器
class StairsDetector(QObject):
    test_signal = Signal() 
    stairs_num=Signal(str)
    stair_detected=Signal(QImage)
    # 构造函数
    def __init__(self,
                 input_image    # 输入一个OpenCV图像
                 ):
        super().__init__()
        self.original_image = input_image   # 原始图像
        self.output_image = None
        self.rough_lines = None
        self.stair_update_notifier=Notifier()
        random.seed(42)

    # 工具函数：检测边缘
    def detectEdges(self, image):
        temp_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)  #转换为灰度图
        temp_image = cv2.GaussianBlur(temp_image, (5, 5), 0) #应用高斯模糊减少噪声
        temp_image = cv2.Canny(temp_image, 50, 150) #使用Canny算法检测边缘
        return temp_image

    # 工具函数：检测线条
    def detectLines(self, image):
        lines = cv2.HoughLinesP(image,
                                     1, np.pi / 180,
                                     threshold=50, minLineLength=50, maxLineGap=10)
        return lines

    # 工具函数：将每一条线条延伸到边缘
    def detectLinesToImageEdge(self, image):
        lines = cv2.HoughLinesP(image, 1, np.pi / 180, 50, minLineLength=50, maxLineGap=10)
        if lines is not None:
            extended_lines = []
            for line in lines:
                for x1, y1, x2, y2 in line:
                    # 计算线段的斜率和截距
                    if x2 - x1 == 0:  # 垂直线处理
                        extended_lines.append([x1, 0, x1, image.shape[0]])
                    else:
                        slope = (y2 - y1) / (x2 - x1)
                        intercept = y1 - slope * x1

                        # 计算与图像边缘的交点
                        y_at_x0 = int(intercept)
                        y_at_xmax = int(slope * image.shape[1] + intercept)
                        if slope != 0:
                            x_at_y0 = int(-intercept / slope)
                            x_at_ymax = int((image.shape[0] - intercept) / slope)
                        else:
                            x_at_y0 = 0
                            x_at_ymax = image.shape[1]

                        # 确定线段的新端点
                        new_x1, new_y1, new_x2, new_y2 = x1, y1, x2, y2
                        if 0 <= y_at_x0 <= image.shape[0]:  # y = 0 交点
                            new_x1, new_y1 = 0, y_at_x0
                        if 0 <= y_at_xmax <= image.shape[0]:  # y = ymax 交点
                            new_x2, new_y2 = image.shape[1], y_at_xmax
                        if 0 <= x_at_y0 <= image.shape[1]:  # x = 0 交点
                            new_x1, new_y1 = x_at_y0, 0
                        if 0 <= x_at_ymax <= image.shape[1]:  # x = xmax 交点
                            new_x2, new_y2 = x_at_ymax, image.shape[0]

                        extended_lines.append([new_x1, new_y1, new_x2, new_y2])
            return np.array(extended_lines)
        return lines

    # 工具函数：绘制线条（当且仅当元素为Numpy64时使用）
    def drawLinesNumpy64(self, image, lines):
        if lines is not None:
            for line in lines:
                cv2.line(image, (line[0], line[1]),
                         (line[2], line[3]), (0, 255, 0), 1)
        return image

    # 工具函数：绘制线条（当且仅当元素为Iterator时使用）
    def drawLines(self, image, lines):
        if lines is not None:
            for line in lines:
                for x1, y1, x2, y2 in line:
                    cv2.line(image, (x1, y1), (x2, y2), (0, 255, 0), 1)
        return image

    # 绘制粗处理后的线条，image为图片，rough_lines为粗线（list，大量(斜率, 截距)tuple组成）
    def drawRoughLinesOnImage(self, image, rough_lines):
        # 绘制直线
        for m, b in rough_lines:
            if m != float('inf'):
                # 对于非垂直线，计算直线的两个端点
                x1 = 0
                y1 = int(b)
                x2 = image.shape[1]
                y2 = int(m * x2 + b)
                # cv2.line(image, (x1, y1), (x2, y2), (0, 0, 255), 2)
            else:
                # 对于垂直线，直接使用 x 坐标绘制
                x = int(b)
                # cv2.line(image, (x, 0), (x, image.shape[0]), (0, 0, 255), 2)

        # 在图像上显示直线方程的数量
        num_lines_text = f"Stair Steps: {len(rough_lines)}"
        cv2.putText(image, num_lines_text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
        

    # 将上述所使用的工具函数组合得到的完整检测流程
    def TotalDetection(self):
        self.edged_image = self.detectEdges(self.original_image)    # step 1, 检测边缘
        # self.lines = self.detectLines(self.edged_image)   # 暂时无用

        self.linedetector = LineDetector()  # step 2, 检测可能为楼梯线条的粗处理后的线条
        self.rough_lines, self.lines = self.linedetector.detectLines(self.edged_image)

        # 注释：由于检测楼梯线条的算法不成熟，后续还需要借由目标检测、图像分割等算法辅助完成。
        # 目前需要限制这些线条位于OpenCV image 在y > 220的空间内。
        
        self.output_image = self.drawLines(self.original_image, self.lines) # step 3, 将原始识别出的线条绘制在图像上
        self.drawRoughLinesOnImage(self.output_image, self.rough_lines)     # step 4, 将粗处理后线条绘制在图像上
        return self.output_image
    def detect_stairs(self,image):
    # 纯楼梯检测
        frame_time = time.time()
        self.original_image = image
        image1 = self.TotalDetection()
        end_time = time.time()  # 记录帧处理时间
        fps = 1 / (end_time - frame_time)  # 计算FPS
        # 将 OpenCV 图像转换为 Qt 图像
        height, width, channel = image1.shape
        bytes_per_line = 3 * width
        qt_img = QImage(cv2.cvtColor(image1, cv2.COLOR_BGR2RGB), width, height, bytes_per_line,QImage.Format_RGB888)

        # 发射信号
        self.stair_detected.emit(qt_img)
        # print(len(self.rough_lines))
        self.stairs_num.emit(f"{len(self.rough_lines)}")
        # print("Stairs number emitted:", len(self.rough_lines)) 
        self.stair_update_notifier.stairs_changed.emit(f"{len(self.rough_lines)}")
        if len(self.rough_lines) !=0:
            self.stair_update_notifier.stair_status_msg.emit(f"Watching out {len(self.rough_lines)} stairs!")
        
        return qt_img,len(self.rough_lines),fps
# 拉伸cv2图像
def ResizeImage(image, target_width, target_height):
    # 获取图像的宽度和高度
    height, width = image.shape[:2]
    # 根据拉伸比例进行拉伸
    resized_image = cv2.resize(image, (target_width, target_height))
    return resized_image


    # cv2.imshow("Stairs Detection", image1)

    # 目标检测
    # object_detect_model_path = "/home/lawrence/projects/Phytium2024-Local/masterdevice/PhytiumMasterProject202403/models/yolov5n_lite.onnx"
    # object_detector = yolov5.YOLOV5(object_detect_model_path)
    # objoutput, image2 = object_detector.inference(image)
    # objbox = yolov5.filterBox(objoutput, 0.5, 0.5)
    # object_detector.draw(image2, objbox)
    # cv2.imshow("Object Detection", image2)

    # 循环等待按键事件，直到按下 'q' 键退出
    # while True:
    #     key = cv2.waitKey(1) & 0xFF
    #     if key == ord('q'):
    #         break
    # cv2.destroyAllWindows()
