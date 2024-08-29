import time
import cv2
import numpy as np
import random
import matplotlib.pyplot as plt
from scipy.interpolate import UnivariateSpline
from scipy.signal import find_peaks
import os
from classes import yolov5v8
from PySide6.QtGui import QImage
from PySide6.QtCore import QObject, Signal
# 输入相对于当前脚本的位置，输出绝对位置
# 输入：filepath 相对目录位置字符串
# 输出：relative_path 绝对目录位置字符串
def toAbsolutePath(current_script_path, filepath):
    current_dir = os.path.dirname(os.path.abspath(current_script_path))  # 获取当前脚本的绝对路径
    relative_path = os.path.join(current_dir, filepath)
    return relative_path
class Notifier(QObject):
    stairs_changed = Signal(str)
    stair_status_msg = Signal(str)
# 连线检测器
class LineDetector:
    def __init__(self):
        # 固定随机数种子，以保证每次运行结果一致
        random.seed(42)

    # 完整的检测流程
    # 输入：1. image，for debug only
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

    # 完整的单个楼梯检测流程（包括标签框内）
    # 输入：1. original_lines：所有检测到的边缘
    # 2. singlefinalbox：单个标签框，不是一组标签框，包括[x1, y1, x2, y2]
    def processLinesWithinBox(self, original_lines, singlefinalbox):
        # step 1 检测框内线条
        box_x1, box_y1, box_x2, box_y2 = singlefinalbox
        inside_box_lines = []
        if original_lines is None:
            return [], []
        if len(original_lines) <= 0:
            return [], []

        for line in original_lines:
            for x1, y1, x2, y2 in line:
                # 检查线条的任一端点是否在框内
                if (box_x1 <= x1 <= box_x2 and box_y1 <= y1 <= box_y2) or (
                        box_x1 <= x2 <= box_x2 and box_y1 <= y2 <= box_y2):
                    inside_box_lines.append(line)
                    break

        if inside_box_lines is not None:
            # step 2 计算线条方程
            line_equations = self.calculateLineEquations(inside_box_lines)
            # step 3 分析斜率k 分布
            self.filtered_lines, max_peak_x = self.analysisFilterLinesAroundPeak(line_equations)
            # step 4 分析截距b 分布
            if self.filtered_lines is None: # 如果存在这种峰值分布 todo: 后续可能需要更新
                return [], []   # 需要返回两个空list，保持返回形式一致
            else:
                self.rough_lines = self.analysisInterceptDistribution(self.filtered_lines, max_peak_x)
            # 返回：粗处理后的线条
            return self.rough_lines, inside_box_lines
        else:
            return [], []   # 需要返回两个空list，保持返回形式一致

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
        # count, bins = np.histogram(slopes, bins='auto', density=False)
        count, bins = np.histogram(finite_slopes, bins='auto', density=False)
        # Use the middle of each bin for the x values
        x = (bins[:-1] + bins[1:]) / 2
        # Find the peaks in the histogram
        peaks, _ = find_peaks(count)
        # Find the highest peak
        if len(count[peaks]) == 0:
            return None, 0
        else:
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
        y_peaks, _ = find_peaks(y_smooth, distance=25, height=0.0050)    # todo：需要进一步调节最小距离、阈值高度
        # 将拟合好的线条提取出来
        rough_lines = []
        for y_peak in y_peaks:
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
class StairsDetector:

    # 构造函数
    def __init__(self):
        self.original_image = None   # 原始图像
        self.output_image = None
        self.linedetector = LineDetector()  # 线条检测器
        random.seed(42)
        self.stair_update_notifier=Notifier()

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

    # 绘制粗处理后的线条，image为图片，rough_lines是粗处理后的线条，由大量的(斜率, 截距)tuple组成
    # singlefinalbox为单独的一个标签框
    # 返回值：该标签框内的线条数量
    def extractRoughLinesOnImage(self, image, rough_lines, singlefinalbox):
        x1, y1, x2, y2 = int(singlefinalbox[0]), int(singlefinalbox[1]), int(singlefinalbox[2]), int(singlefinalbox[3])
        # 绘制标签框，并在图像上显示直线方程的数量
        cv2.rectangle(image, (x1, y1), (x2, y2), (255, 0, 0), 2)  # 目标框显示
        # 将识别结果、识别分数、识别距离显示在目标框旁
        num_lines_text = f"Stair Steps: {len(rough_lines)}"
        print(num_lines_text)   #for debug only
        cv2.putText(image, '{0}'.format(num_lines_text),
                    (x2 - 300, y2 - 20),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    1.2, (255, 255, 120), 3)
        #cv2.putText(image, num_lines_text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
        return len(rough_lines) # 如果有需要将识别到的阶梯级数显示出来


    # 将上述所使用的工具函数组合得到的完整检测流程
    # 输入：image：原始OpenCV图像，640*640
    # bounding_boxes：原始目标检测网络（通用/楼梯专用）的检测集合，需要是YOLOV5V8类生成的
    # target_class：
    def TotalDetection(self, image, bounding_boxes=None, target_class = 0):
        
        self.original_image = image
        # step 1, 保留特定的楼梯标签框
        finalboxes, finalscores = processDetectionResults(bounding_boxes, target_class)
        # finalboxes：被保留的楼梯标签框
        # finalscores：每个框的得分
        if not finalboxes or not finalscores:
            return image

        self.edged_image = self.detectEdges(self.original_image)    # step 2, 检测边缘
        # edged_image 是灰度图，带有边缘的灰度图
        self.original_lines = self.detectLines(self.edged_image)    # step 3，检测线条

        for box in finalboxes:  # 遍历所有的finalboxes
            # step 4, 检测可能为楼梯线条的粗处理后的线条
            self.rough_lines, self.insidebox_lines = self.linedetector.processLinesWithinBox(self.original_lines, box)

            # 注释：由于检测楼梯线条的算法不成熟，后续还需要借由目标检测、图像分割等算法辅助完成。

            if len(self.rough_lines) > 0:   # 要检测到有楼梯级数
                self.output_image = self.drawLines(self.original_image, self.insidebox_lines) # step 5, 将原始识别出的线条绘制在图像上
                self.extractRoughLinesOnImage(self.output_image, self.rough_lines, box)     # step 6, 将粗处理后线条提取出来
        if len(self.rough_lines) > 0:
            return self.output_image
        else:
            return image
    def detect_stairs(self,image,stairs_boxes):
    # 纯楼梯检测
        frame_time = time.time()
        self.original_image = image
        image1 = self.TotalDetection(self.original_image,stairs_boxes, target_class=0)
        end_time = time.time()  # 记录帧处理时间
        fps = 1 / (end_time - frame_time)  # 计算FPS
        
        
        # 将 OpenCV 图像转换为 Qt 图像
        height, width, channel = image1.shape
        bytes_per_line = 3 * width
        qt_img = QImage(cv2.cvtColor(image1, cv2.COLOR_BGR2RGB), width, height, bytes_per_line,QImage.Format_RGB888)

        
        # print(len(self.rough_lines))
        # print("Stairs number return:", len(self.rough_lines)) 
        self.stair_update_notifier.stairs_changed.emit(f"{len(self.rough_lines)}")
        if len(self.rough_lines) !=0:
            self.stair_update_notifier.stair_status_msg.emit(f"Watching out {len(self.rough_lines)} stairs!")
        
        return qt_img,len(self.rough_lines),fps
#################################################### 功能函数
# 拉伸cv2图像
def ResizeImage(image, target_width, target_height):
    # 获取图像的宽度和高度
    height, width = image.shape[:2]
    # 根据拉伸比例进行拉伸
    resized_image = cv2.resize(image, (target_width, target_height))
    return resized_image

def isOverLap(box1, box2):
    """
    判断两个bounding box是否重叠。
    box的格式为[x1, y1, x2, y2]，其中(x1, y1)是左上角的坐标，(x2, y2)是右下角的坐标。
    """
    if box1[2] < box2[0] or box1[0] > box2[2] or box1[3] < box2[1] or box1[1] > box2[3]:
        return False
    return True

def processDetectionResults(input_data, target_class):
    """
    处理检测结果，保留特定class编号的对象，对于重叠的bounding boxes只保留得分最高的。
    """
    boxes, scores, classes = input_data
    filtered_boxes = []
    filtered_scores = []

    # 根据class编号筛选
    for box, score, cls in zip(boxes, scores, classes):
        if cls == target_class:
            filtered_boxes.append(box)
            filtered_scores.append(score)

    # 检查并处理重叠的bounding boxes
    final_boxes = []
    final_scores = []
    for i in range(len(filtered_boxes)):
        overlap = False
        for j in range(len(filtered_boxes)):
            if i != j and isOverLap(filtered_boxes[i], filtered_boxes[j]):
                overlap = True
                # 保留得分更高的bounding box
                if filtered_scores[i] < filtered_scores[j]:
                    break
        else:
            # 如果没有发现重叠或者当前box得分最高，则加入最终结果
            if not overlap or filtered_scores[i] >= max(filtered_scores):
                final_boxes.append(filtered_boxes[i])
                final_scores.append(filtered_scores[i])

    return final_boxes, final_scores


# 主程序，for debug only
if __name__ == "__main__":
    imagePath = "/home/jimkwokying/projectTest/masterdevice/import_img/stair1.jpg"
    image = cv2.imread(imagePath)
    image = ResizeImage(image, 640, 640)

    # 初始化
    current_script_path = os.path.abspath(__file__)  # 当前python脚本目录，后续迁移可以不需要改动
    stairsmodel_path = toAbsolutePath(current_script_path, "../models/stairs_yolov8n.onnx")  # 相对当前脚本位置
    stairsobject_detector = yolov5v8.YOLOV5V8(stairsmodel_path, isType='TEST')  # 创建一个YOLOv8n对象
    stairs_detector = StairsDetector()  # 创建一个楼梯检测对象

    # 楼梯目标检测
    stairsboxes, output_img = stairsobject_detector.inference(image)

    # 纯楼梯检测
    # target_class字段：
    # 如果使用的是楼梯目标检测网络，那么填0
    # 如果使用的是yolov8n-oiv7网络，那么填489
    image1 = stairs_detector.TotalDetection(image, stairsboxes, 0)
    cv2.imshow("Stairs Detection", image1)

    # 循环等待按键事件，直到按下 'q' 键退出
    while True:
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
    cv2.destroyAllWindows()