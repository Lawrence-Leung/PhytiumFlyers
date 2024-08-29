# crossroadsDetector.py
# 基于OpenCV机器视觉的斑马线检测与路口识别程序
# by Lawrence Leung 2024

import os
import cv2
import numpy as np
import random
# import matplotlib

# matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from scipy.interpolate import UnivariateSpline

from classes.stairsDetector import LineDetector
from classes import yolov5v8


# 输入相对于当前脚本的位置，输出绝对位置
# 输入：filepath 相对目录位置字符串
# 输出：relative_path 绝对目录位置字符串
def toAbsolutePath(current_script_path, filepath):
    current_dir = os.path.dirname(os.path.abspath(current_script_path))  # 获取当前脚本的绝对路径
    relative_path = os.path.join(current_dir, filepath)
    return relative_path


# 斑马线检测工具类
# 新的ZebraLineDetector，由于与stairsDetector.py的LineDetector在用法上有类似之处，因此直接继承处理
class ZebraLineDetector(LineDetector):
    # 新的构造函数
    def __init__(self):
        super(ZebraLineDetector)
        super().__init__()

    # ----------------------------------------- 工具函数
    # step 1 专用函数：将某个目标框内的特定的线条过滤出来
    def filterLinesWithinSingleBox(self, original_lines, singlefinalbox):
        '''
        将某个目标框内的特定的线条过滤出来
        :param original_lines: 原始的未过滤的线条
        :param singlefinalbox: 单个目标检测框
        :return: 一个list，如果存在线条，那么list中有线条；如果不存在，那么list为空
        '''
        box_x1, box_y1, box_x2, box_y2 = singlefinalbox
        inside_box_lines = []
        if original_lines is None:
            return [], []
        if len(original_lines) <= 0:
            return [], []

        for line in original_lines:
            for x1, y1, x2, y2 in line:
                # 检查线条的所有端点是否在框内（不是任意端点在框内）
                if (box_x1 <= x1 <= box_x2 and box_y1 <= y1 <= box_y2) and (
                        box_x1 <= x2 <= box_x2 and box_y1 <= y2 <= box_y2):
                    inside_box_lines.append(line)
                    break
        if inside_box_lines is not None:
            return inside_box_lines
        else:
            return []

    # step 2 专用函数：分析框内的长度分布，并将属于最长长度的线段输出出来
    def analysisFilterLengthOfLines(self, lines, isPlot=False):
        # 计算每个线段的长度
        # 注意：lines需要这样提取：[[x1, y1, x2, y2]]！别忘了！
        lengths = [np.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2) for [[x1, y1, x2, y2]] in lines]
        # 得到长度的频率分布直方图
        n, bins, patches = plt.hist(lengths, bins=8, density=True, visible=isPlot)
        # 注意，bins=12是不错的选择！# todo: 12这个参数是可行的!
        # 使用spline进行拟合
        bin_centers = 0.5 * (bins[:-1] + bins[1:])
        spline = UnivariateSpline(bin_centers, n, s=0, k=3)  # 保证它是三次样条曲线，而不报错

        # 绘制曲线
        if isPlot is True:
            plt.xlabel('Length')
            plt.ylabel('Frequency')
            plt.title('Frequency Distribution of Line Lengths')
            xs = np.linspace(min(lengths), max(lengths), 1000)
            plt.plot(xs, spline(xs), 'r-', label='Spline Fit')
            plt.legend()
            plt.show()

        # 找到波谷和波峰
        # n 是直方图的值，bin_centers 是直方图中每个桶的中心值
        n_derivative = np.diff(n)  # 计算一阶导数
        n_derivative2 = np.diff(n_derivative)  # 计算二阶导数
        # 寻找波峰和波谷
        peaks = []  # 波峰：一阶导数符号由正转负，二阶导数为负
        troughs = []  # 波谷：一阶导数符号由负转正，二阶导数为正
        for i in range(1, len(n_derivative)):
            if n_derivative[i - 1] > 0 > n_derivative[i] and n_derivative2[i - 1] < 0:
                peaks.append(bin_centers[i])
            elif n_derivative[i - 1] < 0 < n_derivative[i] and n_derivative2[i - 1] > 0:
                troughs.append(bin_centers[i])
        # 确定波峰
        if peaks:
            peak = max(peaks)
        else:
            # print("No peaks found.")
            peak = None
        # 确定波谷
        if not troughs:
            left_trough, right_trough = min(bin_centers), max(bin_centers)
        elif len(troughs) == 1:
            if peak and peak > troughs[0]:
                left_trough, right_trough = troughs[0], max(bin_centers)
            else:
                left_trough, right_trough = min(bin_centers), troughs[0]
        else:
            if peak:
                left_trough = max([trough for trough in troughs if trough < peak], default=min(bin_centers))
                right_trough = min([trough for trough in troughs if trough > peak], default=max(bin_centers))
            else:
                left_trough, right_trough = min(troughs), max(troughs)
        # 筛选符合条件的线段
        filtered_lines = [line for line in lines if
                          left_trough <= np.sqrt(
                              (line[0][2] - line[0][0]) ** 2 + (line[0][3] - line[0][1]) ** 2) <= right_trough]
        return filtered_lines

    # step 3 专用函数：去除过于密集的线
    def removeDenseLines(self, lines, threshold=55):
        """
        移除过于密集的线段。

        :param lines: 原始线段列表，每个线段的格式为[[x1, y1, x2, y2]]
        :param threshold: 距离阈值，用于判断线段是否过于密集 # TODO：调整了
        :return: 稀疏化后的线段列表
        """
        midpoints = [((line[0][0] + line[0][2]) / 2, (line[0][1] + line[0][3]) / 2) for line in lines]
        keep = [True] * len(lines)  # 初始化所有线段的保留状态为True

        for i in range(len(midpoints)):
            for j in range(i + 1, len(midpoints)):
                if keep[i] and keep[j]:
                    # 计算两中点之间的距离
                    dist = np.sqrt((midpoints[i][0] - midpoints[j][0]) ** 2 + (midpoints[i][1] - midpoints[j][1]) ** 2)
                    if dist < threshold:
                        # 如果两线段过于靠近，则不保留后一条线段
                        keep[j] = False

        # 只保留未被标记为过于密集的线段
        return [line for k, line in zip(keep, lines) if k]

    # step 4 专用函数：计算斜率，取斜率的平均值，然后取这个平均值的垂直线
    # 返回值：direction，是(x5, y5, x6, y6)，这是一条新的线段，该线段在目标框内并且通过目标框的中点，指示这个平均值的垂直线
    # degree：这个名为“direction”的新线段的斜率相对于图像垂直线的角度。这个角度在-90到90度之间，顺时针为正，逆时针为负。
    def analysisAveOfSlopeAndReturnByVerticalLine(self, densed_lines, singlefinalbox):
        # 计算所有线段的斜率
        # 注意：line中需要中间嵌套一层
        slopes = [
            (line[0][3] - line[0][1]) / (line[0][2] - line[0][0]) if (line[0][2] - line[0][0]) != 0 else float('inf')
            for line in
            densed_lines]
        # 计算斜率的平均值

        avg_slope = np.mean([slope for slope in slopes if slope != float('inf')])
        # 计算垂直斜率 k1
        if avg_slope == 0:
            k1 = float('inf')  # 防止除以0
        else:
            k1 = -1 / avg_slope
        # k1 不是绝对的垂直，而是和斑马线框的比例系数有关系
        x_min, y_min, x_max, y_max = singlefinalbox

        # if abs(k1) > 3:
        k1 = k1 * ((y_max - y_min) / (x_max - x_min))  # 进一步修正   todo

        # 线段中点
        # mid_point = [(singlefinalbox[0] + singlefinalbox[2]) / 2, (singlefinalbox[1] + singlefinalbox[3]) / 2]
        midpoints = [((line[0][0] + line[0][2]) / 2, (line[0][1] + line[0][3]) / 2) for line in densed_lines]
        if len(midpoints) > 0:
            avg_x = sum(point[0] for point in midpoints) / len(midpoints)
            avg_y = sum(point[1] for point in midpoints) / len(midpoints)
        else:
            avg_x = np.int32(320)
            avg_y = np.int32(320)
        mid_point = [np.int32(avg_x), np.int32(avg_y)]

        # 根据斜率和中点计算direction线段的两个端点
        # 由于线段需要在目标框内，我们可以将x设置为x3和x4，然后计算对应的y
        if k1 != float('inf'):
            x5, x6 = singlefinalbox[0], singlefinalbox[2]
            y6 = k1 * (x6 - mid_point[0]) + mid_point[1]  # 不要调换了！
            y5 = k1 * (x5 - mid_point[0]) + mid_point[1]  # 不要调换了！
        else:
            # 垂直线段的情况
            y5, y6 = singlefinalbox[1], singlefinalbox[3]
            x5 = x6 = mid_point[0]

        # 检查并调整线段的端点，确保它们在目标框内
        def adjust_point(x, y, k1, x_min, x_max, y_min, y_max):
            # 如果线段垂直
            if k1 == float('inf'):
                return x, max(min(y, y_max), y_min)
            # 如果线段水平
            elif k1 == 0:
                return max(min(x, x_max), x_min), y
            else:
                # 检查并调整y5
                if y < y_min:
                    x = x + ((y_min - y) / k1)  # 这一行和下面一行不要调换顺序！
                    y = y_min
                elif y > y_max:
                    x = x - ((y - y_max) / k1)  # 这一行和下面一行不要调换顺序！
                    y = y_max

                # 确保x在边界内
                # x = max(min(x, x_max), x_min)
                return x, y

        x5_new, y5_new = adjust_point(x5, y5, k1, x_min, x_max, y_min, y_max)
        x6_new, y6_new = adjust_point(x6, y6, k1, x_min, x_max, y_min, y_max)
        direction = [np.int32(x5_new), np.int32(y5_new), np.int32(x6_new), np.int32(y6_new)]
        # 计算角度值
        # 对于垂直于x轴的线，我们定义角度为90或-90
        if k1 == 0:
            degree = 0
        elif k1 == float('inf'):
            degree = 90
        else:
            degree = np.arctan(1 / -k1) * 180 / np.pi  # -k1不要搞错哦
        # # 调整角度范围到-90到90
        if degree > 90:
            degree -= 180
        elif degree < -90:
            degree += 180
        return direction, degree, k1

    # ----------------------------------------- 流程函数
    # 为斑马线设计的输出流程，适用于单个斑马线box，而不是所有的斑马线boxes
    def zebraProcessLinesWithinBox(self, original_lines, singlefinalbox):
        # step 1 检测框内线条
        inside_box_lines = self.filterLinesWithinSingleBox(original_lines, singlefinalbox)
        # step 2 分析框内的长度分布，然后取最长的那部分线条出来，通常能够代表斑马线
        # 最长的那部分线，位于densed_lines中。
        densed_lines = self.analysisFilterLengthOfLines(inside_box_lines)
        if len(densed_lines) <= 0:
            return None, None, None, None
        # step 3 去除过于密集的线
        densed_lines_new = self.removeDenseLines(densed_lines)
        if len(densed_lines) <= 0:
            return None, None, None, None
        # step 4 对最长的那几条线，计算它们的斜率，然后取这些斜率的平均值，最后取这个平均值的垂直线，得到对应的斜率
        direction, degree, k1 = self.analysisAveOfSlopeAndReturnByVerticalLine(densed_lines_new, singlefinalbox)

        return direction, degree, densed_lines_new, k1

# 交通灯识别工具类
class TrafficLightIdentifier:
    def __init__(self):
        pass

    # 检测单个交通灯目标框内的颜色，判定是红灯还是绿灯
    def trafficLightColor(self, image, singlebox, isPlot=False):
        color = "invalid"
        if singlebox is None:
            return color, 0

        x1, y1, x2, y2 = singlebox

        width = x2 - x1
        height = y2 - y1
        size = width * height   # 红绿灯的面积大小，可供参考

        # 调整目标框尺寸
        if width >= 4 and width % 4 != 0:
            width -= width % 4
        if height >= 8 and height % 8 != 0:
            height -= height % 8

        # 如果目标框太小，不分网格
        # 函数需要输出的量：grid_rgb_max
        if width <= 0 or height <= 0:
            return color, size
        if width < 6 or height < 8:
            color_values = np.mean(image[y1:y2, x1:x2], axis=(0, 1))
            grid_rgb_val = np.tile(color_values.reshape(3, 1, 1), (1, 6, 8))
        else:   # 分为4*8的网络
            grid_width = width // 6
            grid_height = height // 8
            grid_rgb_val = np.zeros((3, 6, 8), dtype=np.uint8)

            for i in range(6):
                for j in range(8):
                    grid_x1 = x1 + i * grid_width
                    grid_y1 = y1 + j * grid_height
                    grid_x2 = grid_x1 + grid_width
                    grid_y2 = grid_y1 + grid_height

                    # 提取并保存每个网格中R，G，B通道的最大值
                    grid_rgb_val[:, i, j] = np.mean(image[grid_y1:grid_y2, grid_x1:grid_x2], axis=(0, 1))
                    # 注意：通道1为B/G/R，不是R/G/B。通道1，[0]为B，[1]为G，[2]为R。
                    # 通道2是x方向，通道3是y方向。

        # 检测亮灯情况：
        # 如果为红灯，那么必有R>200且G,B至少一项小于145。
        # 如果为绿灯，那么必有G>200且R小于G-50。
        # 接着，记录红灯或绿灯的块数。红灯优先。如果红灯/绿灯块数>1，那么直接判断是红灯或绿灯，否则为unidentified。
        redcount = 0
        greencount = 0
        for i in range(6):
            for j in range(8):
                # OpenCV中R通道是第2个维度的最后一个，即[2]
                if grid_rgb_val[2, i, j] > 200 and (grid_rgb_val[0, i, j] < 145 or grid_rgb_val[1, i, j] < 145):
                    redcount += 1
                # OpenCV中G通道是第2个维度的中间一个，即[1]
                if grid_rgb_val[1, i, j] > 200 and grid_rgb_val[2, i, j] < grid_rgb_val[1, i, j] - 50:
                    greencount += 1
        if redcount <= 0 and greencount <= 0:
            color = "unidentified"
        elif redcount >= greencount - 1:    # 测试结果
            color = "red"
        else:
            color = "green"

        # for debug only
        if isPlot is True:
            fig, axs = plt.subplots(1, 3, figsize=(15, 5))
            for i, color in enumerate(['Reds', 'Greens', 'Blues']):
                axs[i].imshow(grid_rgb_val[i], cmap=color, interpolation='nearest')
                axs[i].set_title(f"{['R', 'G', 'B'][i]} channel")
                for j in range(4):
                    for k in range(8):
                        axs[i].text(j, k, f'{grid_rgb_val[i, j, k]}', ha='center', va='center', color='w')

            plt.title(f"Rect {x1}, {y1} to {x2}, {y2}")
            plt.tight_layout()
            plt.show()

        # 返回值：
        # color：颜色，它是一个string，对应为"red", "green", "unidentified"。
        # size：红绿灯的面积大小，像素值。
        return color, size



# 路口引导方法类
class CrossRoadGuider:
    def __init__(self):
        self.original_image = None
        self.output_image = None
        self.linedetector = ZebraLineDetector()  # 创建一个全新的线条检测器
        self.traflightidentifier = TrafficLightIdentifier() # 创建一个全新的交通灯识别器
        # 固定随机数种子，以保证每次运行结果一致
        random.seed(52)

    # 工具函数：检测边缘
    def detectEdges(self, image):
        temp_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)  # 转换为灰度图
        temp_image = cv2.GaussianBlur(temp_image, (5, 5), 0)  # 应用高斯模糊减少噪声
        temp_image = cv2.Canny(temp_image, 50, 150)  # 使用Canny算法检测边缘
        return temp_image

    # 工具函数：检测线条
    def detectLines(self, image):
        lines = cv2.HoughLinesP(image,
                                1, np.pi / 180,
                                threshold=50, minLineLength=50, maxLineGap=10)
        return lines

    # 工具函数：将单个box，使用rectangle绘制在image上
    def drawSingleBoxOnImage(self, image, singlebox):
        [x1, y1, x2, y2] = singlebox
        cv2.rectangle(image, (x1, y1), (x2, y2), (0, 255, 0), 2)  # 绿色边框，厚度为2像素
        return image

    # 工具函数：将所有的Line，使用Line绘制在image上
    def drawAllLinesOnImage(self, image, all_lines):
        for line in all_lines:
            [[x1, y1, x2, y2]] = line
            cv2.line(image, (x1, y1), (x2, y2), (255, 0, 0), 2)
        return image

    # 工具函数：将单个Line绘制在image上
    def drawSingleLineOnImage(self, image, singleline):
        [x1, y1, x2, y2] = singleline
        cv2.line(image, (x1, y1), (x2, y2), (0, 0, 255), 2)
        return image

    # 仅斑马线检测
    def TotalDetectionZebraLineOnly(self, image, bounding_boxes=None, target_class=6):
        self.original_image = image

        self.output_image = image  # 即将被输出的图像, for debug only

        # step 1, 根据target_class，保留特定的斑马线标签框
        finalboxes, finalscores = processDetectionResults(bounding_boxes, target_class)
        # finalboxes：被保留的斑马线标签框; finalscores：每个框的得分
        if not finalboxes or not finalscores:
            return image

        # step 2, 检测边缘
        self.edged_image = self.detectEdges(self.original_image)
        # edged_image 是灰度图，带有边缘的灰度图

        # step 3, 检测线条
        self.original_lines = self.detectLines(self.edged_image)
        # 注意，original_lines 类似于[[[0 2 307 2]],, [[12 258 190 248]],, ...,, ]，中间有套了一层！

        for box in finalboxes:  # 遍历所有的finalboxes
            # 注意：每个finalbox包括一个list，内容为[x1 y1 x2 y2]这样。
            box = box.astype(int)
            box = np.clip(box, 0, 639)
            [x1, y1, x2, y2] = box
            # step 4, 对每个单独的box以及对应的线条进行直接处理
            direction, degree, densed_lines, k1 = self.linedetector.zebraProcessLinesWithinBox(self.original_lines, box)

            if None not in (direction, degree, densed_lines, k1):
                self.output_image = self.drawSingleBoxOnImage(self.output_image, box)  # 将斑马线的目标框绘制在image上
                self.output_image = self.drawSingleLineOnImage(self.output_image, direction)
                self.output_image = self.drawAllLinesOnImage(self.output_image, densed_lines)
                cv2.putText(self.output_image, f"{degree:.2f} deg  {k1:.2f} slope", (x1 + 10, y1 + 25),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

        return self.output_image

    # 包括斑马线检测和交通灯检测
    def TotalDetectionZebraLineAndTrafficLight(self, image, bounding_boxes=None, zebraline_class=6,
                                               trafficlight_class=31):
        self.original_image = image
        self.output_image = image  # 即将被输出的图像, for debug only

        # step 1, 根据target_class，保留特定的斑马线标签框，以及交通灯标签框
        zebraline_finalboxes, zebraline_finalscores = processDetectionResults(bounding_boxes, zebraline_class)
        traflight_finalboxes, traflight_finalscores = processDetectionResults(bounding_boxes, trafficlight_class)
        # finalboxes：被保留的斑马线标签框; finalscores：每个框的得分

        if None in (zebraline_finalboxes, zebraline_finalscores, traflight_finalboxes, traflight_finalscores):
            # 只要这四个量中，任意一个是None
            return image

        ############################################################## 首先对斑马线进行处理
        zebralinedirection = None   # 首先初始化，以防报错
        degree = None
        densed_lines = None
        k1 = None

        # step 2, 检测边缘
        self.edged_image = self.detectEdges(self.original_image)
        # edged_image 是灰度图，带有边缘的灰度图

        # step 3, 检测线条
        self.original_lines = self.detectLines(self.edged_image)
        # 注意，original_lines 类似于[[[0 2 307 2]],, [[12 258 190 248]],, ...,, ]，中间有套了一层！

        for box in zebraline_finalboxes:  # 遍历所有的finalboxes
            # 注意：每个finalbox包括一个list，内容为[x1 y1 x2 y2]这样。
            box = box.astype(int)
            box = np.clip(box, 0, 639)
            [x1, y1, x2, y2] = box
            # step 4, 对每个单独的box以及对应的线条进行直接处理
            zebralinedirection, degree, densed_lines, k1 = self.linedetector.zebraProcessLinesWithinBox(
                self.original_lines, box)

            if None not in (zebralinedirection, degree, densed_lines, k1):  # 只要tuple中所有的项目均不为None
                self.output_image = self.drawSingleBoxOnImage(self.output_image, box)  # 将斑马线的目标框绘制在image上
                self.output_image = self.drawSingleLineOnImage(self.output_image, zebralinedirection)
                self.output_image = self.drawAllLinesOnImage(self.output_image, densed_lines)
                cv2.putText(self.output_image, f"{degree:.2f} deg  {k1:.2f} slope", (x1 + 10, y1 + 25),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

        ############################################################## 接着对红绿灯进行处理
        traflifht_colorlist = []
        temp_tuple = ()
        # traflight_colorlist：一个列表，每个项目是一个tuple: (color, size, x, y)。
        # 其中，color为字符串，"red", "green", "unidentified", "invalid"。unidentified表示不亮灯，invalid表示识别错误
        # x, y均为np.int32，代表红绿灯坐标框的中心位置。
        # 示例：[("red", 120, 240), ("green", 100, 200), ... ]

        for box in traflight_finalboxes:
            box = box.astype(int)
            box = np.clip(box, 0, 639)
            [x1, y1, x2, y2] = box
            # 同样需要注意，每个finalbox包括一个list，内容为[x1 y1 x2 y2]这样。
            color, size = self.traflightidentifier.trafficLightColor(self.original_image, box)

            if color is not None:
                temp_tuple = (color, size, np.int32((x1 + y1) / 2), np.int32((x2 + y2) / 2))
                traflifht_colorlist.append(temp_tuple)

            self.output_image = self.drawSingleBoxOnImage(self.output_image, box)  # 将红绿灯的目标框绘制在image上
            cv2.putText(self.output_image, f"{temp_tuple}", (x1, y1 - 15),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 255, 255), 1)

        return self.output_image

    def TotalDetectionAsynchronousAll(self, image_to_be_detected, image_to_be_monitored, bounding_boxes=None, zebraline_class=6,
                                      trafficlight_class=31):
        self.original_image = image_to_be_detected
        self.output_image = image_to_be_monitored  # 即将被输出的图像, for debug only

        # step 1, 根据target_class，保留特定的斑马线标签框，以及交通灯标签框
        zebraline_finalboxes, zebraline_finalscores = processDetectionResults(bounding_boxes, zebraline_class)
        traflight_finalboxes, traflight_finalscores = processDetectionResults(bounding_boxes, trafficlight_class)
        # finalboxes：被保留的斑马线标签框; finalscores：每个框的得分

        if None in (zebraline_finalboxes, zebraline_finalscores, traflight_finalboxes, traflight_finalscores):
            # 只要这四个量中，任意一个是None
            return image_to_be_monitored

        ############################################################## 首先对斑马线进行处理
        zebralinedirection = None   # 首先初始化，以防报错
        degree = None
        densed_lines = None
        k1 = None

        # step 2, 检测边缘
        self.edged_image = self.detectEdges(self.output_image)
        # edged_image 是灰度图，带有边缘的灰度图

        # step 3, 检测线条
        self.original_lines = self.detectLines(self.edged_image)
        # 注意，original_lines 类似于[[[0 2 307 2]],, [[12 258 190 248]],, ...,, ]，中间有套了一层！

        for box in zebraline_finalboxes:  # 遍历所有的finalboxes
            # 注意：每个finalbox包括一个list，内容为[x1 y1 x2 y2]这样。
            box = box.astype(int)
            box = np.clip(box, 0, 639)
            [x1, y1, x2, y2] = box
            # step 4, 对每个单独的box以及对应的线条进行直接处理
            zebralinedirection, degree, densed_lines, k1 = self.linedetector.zebraProcessLinesWithinBox(
                self.original_lines, box)

            if None not in (zebralinedirection, degree, densed_lines, k1):  # 只要tuple中所有的项目均不为None
                self.output_image = self.drawSingleBoxOnImage(self.output_image, box)  # 将斑马线的目标框绘制在image上
                self.output_image = self.drawSingleLineOnImage(self.output_image, zebralinedirection)
                self.output_image = self.drawAllLinesOnImage(self.output_image, densed_lines)
                cv2.putText(self.output_image, f"{degree:.2f} deg  {k1:.2f} slope", (x1 + 10, y1 + 25),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

        ############################################################## 接着对红绿灯进行处理
        traflifht_colorlist = []
        temp_tuple = ()
        # traflight_colorlist：一个列表，每个项目是一个tuple: (color, size, x, y)。
        # 其中，color为字符串，"red", "green", "unidentified", "invalid"。unidentified表示不亮灯，invalid表示识别错误
        # x, y均为np.int32，代表红绿灯坐标框的中心位置。
        # 示例：[("red", 120, 240), ("green", 100, 200), ... ]

        for box in traflight_finalboxes:
            box = box.astype(int)
            box = np.clip(box, 0, 639)
            [x1, y1, x2, y2] = box
            # 同样需要注意，每个finalbox包括一个list，内容为[x1 y1 x2 y2]这样。
            color, size = self.traflightidentifier.trafficLightColor(self.original_image, box)

            if color is not None:
                temp_tuple = (color, size, np.int32((x1 + y1) / 2), np.int32((x2 + y2) / 2))
                traflifht_colorlist.append(temp_tuple)

            self.output_image = self.drawSingleBoxOnImage(self.output_image, box)  # 将红绿灯的目标框绘制在image上
            cv2.putText(self.output_image, f"{temp_tuple}", (x1, y1 - 15),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 255, 255), 1)

        return self.output_image

    def TotalDetectionAsyncWithDataOutput(self, image_to_be_detected, image_to_be_monitored, bounding_boxes=None, zebraline_class=6,
                                          trafficlight_class=31):
        self.original_image = image_to_be_detected
        self.output_image = image_to_be_monitored  # 即将被输出的图像, for debug only

        # step 1, 根据target_class，保留特定的斑马线标签框，以及交通灯标签框
        zebraline_finalboxes, zebraline_finalscores = processDetectionResults(bounding_boxes, zebraline_class)
        traflight_finalboxes, traflight_finalscores = processDetectionResults(bounding_boxes, trafficlight_class)
        # finalboxes：被保留的斑马线标签框; finalscores：每个框的得分

        if None in (zebraline_finalboxes, zebraline_finalscores, traflight_finalboxes, traflight_finalscores):
            # 只要这四个量中，任意一个是None
            return image_to_be_monitored

        ############################################################## 首先对斑马线进行处理
        zebralinedirection = None   # 首先初始化，以防报错
        degree = None
        densed_lines = None
        k1 = None

        # step 2, 检测边缘
        self.edged_image = self.detectEdges(self.output_image)
        # edged_image 是灰度图，带有边缘的灰度图

        # step 3, 检测线条
        self.original_lines = self.detectLines(self.edged_image)
        # 注意，original_lines 类似于[[[0 2 307 2]],, [[12 258 190 248]],, ...,, ]，中间有套了一层！

        output_zebraline_list = []

        for box in zebraline_finalboxes:  # 遍历所有的finalboxes
            # 注意：每个finalbox包括一个list，内容为[x1 y1 x2 y2]这样。
            box = box.astype(int)
            box = np.clip(box, 0, 639)
            [x1, y1, x2, y2] = box
            # step 4, 对每个单独的box以及对应的线条进行直接处理
            zebralinedirection, degree, densed_lines, k1 = self.linedetector.zebraProcessLinesWithinBox(
                self.original_lines, box)

            if None not in (zebralinedirection, degree, densed_lines, k1):  # 只要tuple中所有的项目均不为None
                self.output_image = self.drawSingleBoxOnImage(self.output_image, box)  # 将斑马线的目标框绘制在image上
                self.output_image = self.drawSingleLineOnImage(self.output_image, zebralinedirection)
                self.output_image = self.drawAllLinesOnImage(self.output_image, densed_lines)
                cv2.putText(self.output_image, f"{degree:.2f} deg  {k1:.2f} slope", (x1 + 10, y1 + 25),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
            if not isinstance(degree, (int, float)):
                degree = 0
            output_zebraline_list.append((int((x1 + y1) / 2), int((x2 + y2) / 2), int(degree * 10)))

        ############################################################## 接着对红绿灯进行处理
        traflight_colorlist = []
        temp_tuple = ()
        # traflight_colorlist：一个列表，每个项目是一个tuple: (color, size, x, y)。
        # 其中，color为字符串，"red", "green", "unidentified", "invalid"。unidentified表示不亮灯，invalid表示识别错误
        # x, y均为np.int32，代表红绿灯坐标框的中心位置。
        # 示例：[("red", 120, 240), ("green", 100, 200), ... ]

        for box in traflight_finalboxes:
            box = box.astype(int)
            box = np.clip(box, 0, 639)
            [x1, y1, x2, y2] = box
            # 同样需要注意，每个finalbox包括一个list，内容为[x1 y1 x2 y2]这样。
            color, size = self.traflightidentifier.trafficLightColor(self.original_image, box)

            if color is not None:
                temp_tuple = (color, size, np.int32((x1 + y1) / 2), np.int32((x2 + y2) / 2))
                if color == "unidentified":    # 全部转变为数字
                    color = 0
                elif color == "red":
                    color = 1
                elif color == "green":
                    color = 2
                else:
                    color = 99

                traflight_colorlist.append((int((x1 + y1) / 2), int((x2 + y2) / 2), color))

            self.output_image = self.drawSingleBoxOnImage(self.output_image, box)  # 将红绿灯的目标框绘制在image上
            cv2.putText(self.output_image, f"{temp_tuple}", (x1, y1 - 15),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 255, 255), 1)

        # 注意：红绿灯数量最多输出10个，斑马线数量最多输出5个；图像一定要放在最后！否则有bug！
        return output_zebraline_list[:5], traflight_colorlist[:10], self.output_image    # 最后输出了这个！
    
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


# 主程序，仅debug使用
if __name__ == "__main__":
    imagePath = "/home/lawrence/Desktop/stair1.jpg"
    image = cv2.imread(imagePath)
    image = ResizeImage(image, 640, 640)

    # 初始化
    current_script_path = os.path.abspath(__file__)  # 当前python脚本目录，后续迁移可以不需要改动
    detectmodel_path = toAbsolutePath(current_script_path, "../models/haizhuv8nint8.onnx")  # 相对当前脚本位置
    realdetector = yolov5v8.YOLOV5V8(detectmodel_path, isType='HAIZHU')  # 创建一个YOLOv8n对象
    crossroad_guider = CrossRoadGuider()  # 创建一个路口引导工具

    # 楼梯目标检测
    stairsboxes, output_img = realdetector.inference(image)

    # 纯楼梯检测
    # target_class字段：
    # 如果使用的是HAIZHU，遇到斑马线，则填6
    image1 = crossroad_guider.TotalDetectionZebraLineAndTrafficLight(image, stairsboxes,
                                                                     6,
                                                                     31)
    cv2.imshow("Crossroad Guider", image1)

    # 循环等待按键事件，直到按下 'q' 键退出
    while True:
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
    cv2.destroyAllWindows()
