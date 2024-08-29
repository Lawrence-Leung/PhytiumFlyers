# -*- coding: utf-8 -*-
# 基于YOLOv8算法目标框，实现单目测距转换
# MNN推理框架,0713更新

import os
import cv2
import numpy as np
import MNN
import time
from classes import coordination


# Haizhu数据集，共有50个 label classes
ClassesHAIZHU = ['Car', 'Wheel', 'Person', 'Motorcycle', 'Bicycle', 'Houseplant',
                 'zebraline', 'pillar', 'bollard', 'Plant', 'stairs', 'Tree', 'Clothing',
                 'Footwear', 'barrier', 'Building', 'Man', 'Vehicle registration plate',
                 'Woman', 'Bicycle wheel', 'Bus', 'cabinet', 'Land vehicle', 'Fire hydrant',
                 'Truck', 'Jeans', 'bus station', 'Umbrella', 'Bench', 'Van', 'Stairs',
                 'Traffic light', 'Traffic sign', 'Table', 'Chair', 'Human face', 'Poster',
                 'slope', 'Fast food', 'Plastic bag', 'Helmet', 'Bicycle helmet', 'Fruit',
                 'Handbag', 'Glasses', 'Camel', 'Dog', 'Door', 'Window', 'Flower']

# 通用类，可以同时支持输入YOLOv5和YOLOv8程序
class YOLOV5V8():
    # 构造函数
    # 输入：mnnpath，字符串，带有.mnn格式YOLOv8文件
    def __init__(self, mnnpath: object, isType='YOLOV8') -> object:
        self.interpreter = MNN.Interpreter(mnnpath)
        self.session = self.interpreter.createSession()
        self.input_tensor = self.interpreter.getSessionInput(self.session)
        self.coord_conversion = coordination.CoordConvertion()  # 内置距离推理模块
        self.map = coordination.LittleMap()     # 内置小地图
        self.conf_threshold = 0.5   # 置信度阈值
        self.iou_threshold = 0.5    # IOU阈值
        if isType == 'HAIZHU':
            self.CLASSES = ClassesHAIZHU    # 使用自行训练的数据集模型

    def getInputFeed(self, img_tensor: object) -> object:
        tmp_input = MNN.Tensor(img_tensor.shape, MNN.Halide_Type_Float, img_tensor, MNN.Tensor_DimensionType_Caffe)
        self.input_tensor.copyFrom(tmp_input)
        return self.input_tensor

    # 核心推理函数
    # 输入：frame，OpenCV2格式，输入图像帧
    # 输出：
    #      预测结果：一个tuple，(boxes, scores, classes)
    #      or_img：；拉伸到(640, 640)的原始图像帧
    def inference(self, frame: object) -> object:
        # 1. 图像预处理，这里YOLOv5和YOLOv8是一致的
        or_img = cv2.resize(frame, (640, 640))  # 拉伸图像
        img = or_img[:, :, ::-1].transpose(2, 0, 1)  # BGR2RGB和HWC2CHW
        img = img.astype(dtype=np.float32)
        img /= 255.0
        img = np.expand_dims(img, axis=0)

        # 2. 开始推理
        input_feed = self.getInputFeed(img)
        self.interpreter.runSession(self.session)
        output_tensor = self.interpreter.getSessionOutput(self.session)
        pred = output_tensor.getNumpyData()

        # 需要注意：
        # pred变量，对于YOLOv8而言，返回的是：1 * 605 * 8400，而YOLOv5返回的是：1 * 25200 * 85

        # 3. 返回数据：
        boxes, scores, classes = self.normalpred(pred)
        c = classes * 640
        nb = boxes + c[:, np.newaxis]
        id = self.numpy_nms(nb, scores, self.iou_threshold)

        return (boxes[id], scores[id], classes[id]), or_img

    # 将inference()得到的pred变量，直接转换为boxes, scores, classes使用
    # 同时具有处理YOLOv8和YOLOv5两种目标检测模型的功能
    def normalpred(self, pred: object) -> object:
        if pred.shape[1] < pred.shape[2]:   # YOLOv8适用
            pred = np.squeeze(pred).T #1 * 605 * 8400 -> 8400 * 605
            scores = np.max(pred[:, 4:], axis=1)
            classes = np.argmax(pred[:, 4:], axis=1)
            mask = scores > self.conf_threshold #置信度过滤
            boxes = self.xywh_to_x1y1x2y2(pred[mask])
            scores = scores[mask]
            classes = classes[mask]
            return boxes, scores, classes

        else:   # YOLOv5适用
            pred = np.squeeze(pred)
            scores = pred[:, 4]
            classes = np.argmax(pred[:, 5:], axis=1)
            mask = scores > self.conf_threshold  # 置信度过滤
            boxes = self.xywh_to_x1y1x2y2(pred[mask])
            scores = scores[mask]
            classes = classes[mask]
            return boxes, scores, classes

    # 将XYWH格式转换为X1Y1X2Y2格式
    def xywh_to_x1y1x2y2(self, boxes):
        # 提取中心点坐标和宽高
        x_center, y_center, width, height = boxes[:, 0], boxes[:, 1], boxes[:, 2], boxes[:, 3]
        # 计算左上角和右下角坐标
        x1 = x_center - width / 2
        y1 = y_center - height / 2
        x2 = x_center + width / 2
        y2 = y_center + height / 2
        # 将计算结果组合成新的数组
        xyxy_boxes = np.stack((x1, y1, x2, y2), axis=1)
        return xyxy_boxes

    # 非极大抑制
    def numpy_nms(self, boxes, scores, iou_threshold):
        idxs = scores.argsort()  # 按分数 降序排列的索引 [N]
        keep = []
        while idxs.size > 0:  # 统计数组中元素的个数
            max_score_index = idxs[-1]
            max_score_box = boxes[max_score_index][None, :]
            keep.append(max_score_index)
            if idxs.size == 1:
                break
            idxs = idxs[:-1]  # 将得分最大框 从索引中删除； 剩余索引对应的框 和 得分最大框 计算IoU；
            other_boxes = boxes[idxs]  # [?, 4]
            ious = self.box_iou(max_score_box, other_boxes)  # 一个框和其余框比较 1XM
            idxs = idxs[ious[0] <= iou_threshold]

        return keep

    # 功能函数
    def box_area(self, boxes):
        return (boxes[:, 2] - boxes[:, 0]) * (boxes[:, 3] - boxes[:, 1])

    # 功能函数
    def box_iou(self, box1, box2):
        area1 = self.box_area(box1)  # N
        area2 = self.box_area(box2)  # M
        # broadcasting, 两个数组各维度大小 从后往前对比一致， 或者 有一维度值为1；
        lt = np.maximum(box1[:, np.newaxis, :2], box2[:, :2])
        rb = np.minimum(box1[:, np.newaxis, 2:], box2[:, 2:])
        wh = rb - lt
        wh = np.maximum(0, wh)  # [N, M, 2]
        inter = wh[:, :, 0] * wh[:, :, 1]
        iou = inter / (area1[:, np.newaxis] + area2 - inter)
        return iou

    # 画图函数，不带小地图
    # 输入：
    # image：原始图片（640*640），OpenCV2 image格式
    # box_data：(boxes, scores, classes)
    def draw(self, image: object, box_data: object) -> object:
        for box, score, cl in zip(box_data[0], box_data[1], box_data[2]): # 将boxes, scores, classes数据提取出来
            top, left, right, bottom = int(box[0]), int(box[1]), int(box[2]), int(box[3])
            dist, height, width, anglehorizon, angleheight = self.coord_conversion.box2Measurement(box, self.CLASSES[cl])   #SSES[cl])
            # 收集单个目标
            cv2.rectangle(image, (top, left), (right, bottom), (255, 0, 0), 2)  # 目标框显示
            # 将识别结果、识别分数、识别距离显示在目标框旁
            cv2.putText(image, '{0} {1:.2f}'.format(self.CLASSES[cl], score),
                        (top, bottom),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.6, (0, 0, 255), 2)
            # 距离、水平角、垂直角显示在目标框旁
            cv2.putText(image, '{0:.2f}m {1:.2f}hor {2:.2f}hei'.format(
                dist, anglehorizon, angleheight),
                (top, left),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.5, (100, 100, 255), 2)

    # 画图函数，带小地图
    # 输入：
    # image：原始图片（640*640），OpenCV2 image格式
    # box_data：(boxes, scores, classes)
    def drawWithMap(self, image: object, box_data: object) -> object:
        self.map.clearMap()  # 清空地图
        for box, score, cl in zip(box_data[0], box_data[1], box_data[2]): # 将boxes, scores, classes数据提取出来
            top, left, right, bottom = int(box[0]), int(box[1]), int(box[2]), int(box[3])
            #print('class: {}, score: {}'.format(CLASSES[cl], score))
            #print('box coordinate left,top,right,down: [{}, {}, {}, {}]'.format(top, left, right, bottom))


            dist, height, width, anglehorizon, angleheight = self.coord_conversion.box2Measurement(box, self.CLASSES[cl])   #SSES[cl])
            #print('actual dist, height, width: [{}, {}, {}]'.format(dist, height, width))

            # 收集单个目标
            self.map.collectSingleObject(self.CLASSES[cl], dist, anglehorizon)
            cv2.rectangle(image, (top, left), (right, bottom), (255, 0, 0), 2)  # 目标框显示
            # 将识别结果、识别分数、识别距离显示在目标框旁
            cv2.putText(image, '{0} {1:.2f}'.format(self.CLASSES[cl], score),
                        (top, bottom),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        1.0, (0, 0, 255), 3)
            # 距离、水平角、垂直角显示在目标框旁
            cv2.putText(image, '{0:.2f}m {1:.2f}hor {2:.2f}hei'.format(
                dist, anglehorizon, angleheight),
                (top, left),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.8, (100, 100, 255), 2)
        object=self.map.showObjects(0) # 显示地图中的所有目标
        return object

     # 画图函数，带小地图，带输出，0504更新
    # 输入：
    # image：原始图片（640*640），OpenCV2 image格式
    # box_data：(boxes, scores, classes)
    # 输出：output_target_list，需要输出的目标检测结果
    def drawWithMapOutput(self, image: object, box_data: object) -> object:
        self.map.clearMap()  # 清空地图
        output_target_list = [] # 需要输出的目标检测结果
        for box, score, cl in zip(box_data[0], box_data[1], box_data[2]): # 将boxes, scores, classes数据提取出来
            top, left, right, bottom = int(box[0]), int(box[1]), int(box[2]), int(box[3])
            #print('class: {}, score: {}'.format(CLASSES[cl], score))
            #print('box coordinate left,top,right,down: [{}, {}, {}, {}]'.format(top, left, right, bottom))

            dist, height, width, anglehorizon, angleheight = self.coord_conversion.box2Measurement(box, self.CLASSES[cl])
            #print('actual dist, height, width: [{}, {}, {}]'.format(dist, height, width))

            # 2024.5.4 更新
            if cl not in [12, 13, 17, 25, 36, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49]:  # 除掉一些不重要的东西
                output_target_list.append((int((box[1] + box[2])/2), int((box[0] + box[3])/2), cl, int(dist*10), int(angleheight), int(anglehorizon*10)))
            # 结果：x, y, class, dist, angleheight, anglehorizon，注意都打包到一个tuple中！

            # 收集单个目标
            self.map.collectSingleObject(self.CLASSES[cl], dist, anglehorizon)
            cv2.rectangle(image, (top, left), (right, bottom), (255, 0, 0), 2)  # 目标框显示
            # 将识别结果、识别分数、识别距离显示在目标框旁
            cv2.putText(image, '{0} {1:.2f}'.format(self.CLASSES[cl], score),
                        (top, bottom),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.6, (0, 0, 255), 2)
            # 距离、水平角、垂直角显示在目标框旁
            cv2.putText(image, '{0:.2f}m {1:.2f}hor {2:.2f}hei'.format(
                dist, anglehorizon, angleheight),
                (top, left),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.5, (100, 100, 255), 2)
        object=self.map.showObjects(0)
        #self.map.showObjects(0) # 显示地图中的所有目标
        #self.map.showMapOnWindow("Map") # 在窗口上显示地图

        return output_target_list[:78],object  # 取前78个

