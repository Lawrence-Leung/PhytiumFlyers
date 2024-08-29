# coordination.py
# 基于YOLOv5算法目标框，实现单目测距转换与小地图输出
# by Lawrence Leung 2024

import cv2
import numpy as np
import random

class CoordConvertion:
    def __init__(self):
        self.f = 486.54    # 焦距系数
        self.output_pixHEIGHT = 640 # 输出图像高度，单位为像素
        self.output_pixWIDTH = 640  # 输出图像宽度，单位为像素
        self.viewangle_singleside = 1.0882 # 单边视野角度范围，单位为弧度rad
        self.widthradbiasadd = -0.2618  # 水平角度偏移系数
        self.widthradbiasmultiply = 2.5  # 水平角度乘积系数
        self.heightradbiasadd = -0.1222  # 垂直角度偏移系数
        self.heightradbiasmultiply = 3  # 垂直角度乘积系数
        self.horizon = 0.670   # 水平线距离顶部占全高度的比例
        self.maximum_height_deg = 0.7854 # 界面的最高仰角，单位为弧度rad

        pass

    # 将目标检测得到的Bounding Box，转换为距离、高度与宽度
    # 此种方法假设某种被检测的物体，宽度（height）是固定的
    def box2Measurement(self, box: object, classes: object) -> object:
        # 首先假设物体在你的正前方

        # 注意：
        # x1, y1, x2, y2 = int(box[0]), int(box[1]), int(box[2]), int(box[3])
        # 分别对应：top, left, right, bottom
        top, left, right, bottom = box  # 这是在640*640状况下的像素点的x、y值
        left = int(left)
        top = int(top)
        right = int(right)
        bottom = int(bottom)

        pixheight = abs(top - bottom)
        pixwidth = abs(right - left)

        actualvalue, iswidth = self.actualValByClass(classes)

        if pixwidth == 0 or pixheight == 0: #以免发生报错
            pixheight = 1
            pixwidth = 1

        # 根据物体类型判断，是以高度还是以宽度为固定
        if iswidth == True:
            actualwidth = actualvalue
            actualheight = abs((actualwidth * pixheight) / pixwidth)
        else:
            actualheight = actualvalue
            actualwidth = abs((pixwidth * actualheight) / pixheight)


        if pixheight == 0:  # 修复下面除以0的错误
            pixheight = 1

        dist = (actualheight * self.f) / pixheight

        # 如果不是正前方，那么需要对dist进行变换
        bbox_center_x = abs((right + left) / 2)
        center_loss = bbox_center_x - (self.output_pixWIDTH / 2)
        anglewidthrad = (((center_loss / (self.output_pixWIDTH / 2)) * self.viewangle_singleside + self.widthradbiasadd)
                         * self.widthradbiasmultiply) #单位为弧度rad
        distref = dist / np.cos(abs(anglewidthrad))
        anglewidthrad = anglewidthrad + np.pi / 4
        anglewidthdeg = anglewidthrad * 180.0 / np.pi

        # 继续修正距离
        # if (distref > 5.0) & (distref < 10):
        #     distref = 1.0 * (distref) ** 2 + (-6) * distref + 10
        # elif distref >= 10:
        #     distref = distref + 50

        # 求目标高度角
        bbox_center_y = abs((top + bottom) / 2)
        if bbox_center_y >= self.output_pixHEIGHT * self.horizon:
            angleheightrad = 0.0
        else:
            angleheightrad = (self.maximum_height_deg * (self.output_pixHEIGHT * self.horizon - bbox_center_y) /
                              self.output_pixHEIGHT) * self.heightradbiasmultiply #+ self.heightradbiasadd

        if angleheightrad <= 0.0:   # todo：目前暂时不考虑向下看的情形，后续功能添加在考虑
            angleheightrad = 0.0
        if angleheightrad >= self.maximum_height_deg:   # 太高的角度摄像头无法识别
            angleheightrad = self.maximum_height_deg

        angleheightdeg = angleheightrad * 180.0 / np.pi

        # 继续修正距离
        distref = distref / np.cos(angleheightrad)

        return distref, actualheight, actualwidth, anglewidthdeg, angleheightdeg

    # 根据不同的label，确定是以高度为判断依据还是以宽度为判断依据
    def actualValByClass (self, classes):
        # 注意：True指的是以宽度为判断依据；False指的是以高度为判断依据
        # 返回第一个值是以m为单位的宽度或高度，返回第二个值是判断依据
        if classes == "Vehicle" or classes == "Car" or classes == "car":
            return 2.00, False
        elif classes == "Wheel" or classes == "Bicycle wheel":
            return 0.70, False
        elif classes == "Cabinet":
            return 2.00, False
        elif classes == "Houseplant":
            return 1.10, False
        elif classes == "zebraline":    # 特殊类型，不适用宽高判断方法
            return 0.00, False
        elif classes == "pillar":
            return 0.35, True
        elif classes == "bollard":
            return 0.80, False
        elif classes == "Plant" or classes == "barrier":
            return 1.20, False
        elif classes == "Building":
            return 3.00, False
        elif classes == "Tree":
            return 3.00, True
        elif classes == "Land vehicle":
            return 1.60, False
        elif classes == "Fire hydrant":
            return 0.70, False
        elif classes == "bus station":
            return 3.00, False
        elif classes == "Person" or classes == "Man" or classes == "Woman":
            return 0.70, True
        elif classes == "Motorcycle":
            return 1.20, False
        elif classes == "Bicycle":
            return 1.20, False
        elif classes == "Bus":
            return 4.00, False
        elif classes == "Van":
            return 2.00, False
        elif classes == "Train":
            return 4.50, False
        elif classes == "Truck":
            return 4.50, False
        elif classes == "Traffic light" or classes == "Traffic sign":
            return 0.50, True
        elif classes == "Bench":
            return 0.80, False
        elif classes == "Chair":
            return 0.80, True
        elif classes == "Stop sign":
            return 1.00, True
        elif classes == "Parking meter":
            return 0.30, True
        elif classes == "Stairs" or classes == "stairs":
            return 3.00, True
        elif classes == "Chair" or classes == "Bench" or classes == "Table":
            return 0.70, False
        else:
            return 0.00, False

# 小地图
class LittleMap:
    def __init__(self):
        self.mapsize = 640
        # background是一个图像。
        self.background = np.full((self.mapsize, self.mapsize, 3), 255, dtype=np.uint8)
        self.mapcenter = (self.mapsize // 2, self.mapsize // 2)
        self.mapnumcircle = 4   # 一幅图有4条环状线
        self.circledistance = 5  # 每条环状线间距为5m
        self.mapnumlines = 8    # 一幅图有8条放射状线
        self.clearMap()     # 初始化并清空图像
        # objects是一个list，里面具有大量tuple。这里面的tuple由不同的单个的(classes, distance, anglehotizondeg)组成
        self.objects = []

    # 一共mapnumcircle条环状线，每条的距离为2.5m，总共可以判定盲人周边10米以内的距离。

    # 初始化并清空图像。这个图像是320*320大小，由辐射状和环状网格组成，背景纯白，就像雷达一样
    def clearMap(self):
        # 初始化背景
        self.objects = []    #别忘了清空objects list！
        self.background = np.full((self.mapsize, self.mapsize, 3), 255, dtype=np.uint8)
        # 画环状网格（圆）
        for i in range(1, self.mapnumcircle + 1):
            radius = i * (self.mapsize // 2) // self.mapnumcircle
            cv2.circle(self.background, self.mapcenter, radius, (0, 0, 0), 1)

        # 画辐射状网格（线）
        for i in range(self.mapnumlines):
            angle = 2 * np.pi * i / self.mapnumlines
            x = int(self.mapcenter[0] + (self.mapsize // 2) * np.cos(angle))
            y = int(self.mapcenter[1] + (self.mapsize // 2) * np.sin(angle))
            cv2.line(self.background, self.mapcenter, (x, y), (0, 0, 0), 1)

        self.drawMyCenterPosition()

    # 绘制一个“自己的坐标”，蓝色的三角形，位于图像中心。
    def drawMyCenterPosition(self):
        # 定义三角形的三个顶点
        triangle_size = 10  # 三角形大小
        triangle_points = np.array([
            [self.mapcenter[0], self.mapcenter[1] - triangle_size],  # 上顶点
            [self.mapcenter[0] - triangle_size * np.sin(np.pi / 3), self.mapcenter[1] + triangle_size / 2],  # 左下顶点
            [self.mapcenter[0] + triangle_size * np.sin(np.pi / 3), self.mapcenter[1] + triangle_size / 2],  # 右下顶点
        ], np.int32)

        triangle_points = triangle_points.reshape((-1, 1, 2))

        # 在图像上绘制蓝色的三角形
        cv2.fillPoly(self.background, [triangle_points], (255, 0, 0))

    # 收集单个被检测到的物体，输入它的类型、距离、水平角
    def collectSingleObject(self, classes = "", distance = 0, anglehorizondeg = 0):
        self.objects.append((classes, distance, anglehorizondeg))

    # 在上述的基础上绘制各种点
    def showObjects(self, angle_offset = 0):
        self.max_distance = self.circledistance * self.mapnumcircle #(min(self.mapcenter) // self.circledistance)  # 最远距离对应到网格距离
        for obj in self.objects:
            detection_result, distance, angle_deg = obj
            # 保证距离不超出最大值
            distance = min(distance, self.max_distance)

            # 角度调整: 顺时针增加，逆时针减少，垂直向上为0度
            angle_rad = np.deg2rad(-angle_deg + 90 + angle_offset)

            # 极坐标转笛卡尔坐标
            x = int(self.mapcenter[0] + (distance / self.max_distance) * (self.mapsize / 2) * np.cos(angle_rad))
            y = int(self.mapcenter[1] - (distance / self.max_distance) * (self.mapsize / 2) * np.sin(angle_rad))

            # 为每个detection_result设置随机颜色
            random.seed(detection_result)
            color = (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))

            # 绘制圆点和文本
            cv2.circle(self.background, (x, y), 3, color, -1)
            cv2.putText(self.background, detection_result, (x + 5, y - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.4, color, 1)
            return self.background

    # 绘制小地图到窗口中
    def showMapOnWindow(self, cv2windowtitle):
        # cv2.imshow(cv2windowtitle, self.background)
        pass

