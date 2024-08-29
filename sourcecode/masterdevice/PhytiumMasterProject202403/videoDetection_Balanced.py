# -*- coding: utf-8 -*-
# scratch.py
# 新视频检测方法：使用multiprocessing
# by Lawrence Leung 2024
# 更新：2024.3.21

import cv2
import time
import os
import sys
import numpy as np
from PySide6.QtCore import QCoreApplication, QThread, Signal, Slot  # QCoreApplication用于命令行Qt程序，用不上
from classes import yolov5v8, stairsDetector

# 新实验
from multiprocessing import Process, Queue

# 输入相对于当前脚本的位置，输出绝对位置
# 输入：filepath 相对目录位置字符串
# 输出：relative_path 绝对目录位置字符串
def toAbsolutePath(current_script_path, filepath):
    current_dir = os.path.dirname(os.path.abspath(current_script_path))  # 获取当前脚本的绝对路径
    relative_path = os.path.join(current_dir, filepath)
    return relative_path

# 捕获图像（基于QThread实现，此处不适用）
class CaptureThread(QThread):
    image_signal = Signal(object)   # 图像信号

    def __init__(self):
        super(CaptureThread, self).__init__()
        self.is_running = True
        self.number = 0

    def run(self):
        self.cap = cv2.VideoCapture(0)
        if not self.cap.isOpened():
            print("Error: Unable to open video file.")
            return
        while self.is_running and self.cap.isOpened():
            frame_start_time = time.time()
            ret, image = self.cap.read()
            self.number += 1

            if ret:
                if self.number % 15 == 0:
                    self.image_signal.emit(image)   # 将图像发送出去
                gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
                edges = cv2.Canny(gray, 100, 200)

                # 计算FPS
                frame_end_time = time.time()
                fps = 1 / (frame_end_time - frame_start_time)
                cv2.putText(edges, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
                cv2.imshow("Edge Detection", edges)
                if cv2.waitKey(1) == ord('q'):
                    self.is_running = False


    def stop(self):
        self.is_running = False

# 检测图像（基于QThread实现，此处不适用）
class DetectionThread(QThread):
    def __init__(self,
                 object_detector,
                 stairsobject_detector,
                 stairsnumber_detector):
        super(DetectionThread, self).__init__()
        self.object_detector = object_detector
        self.stairsobject_detector = stairsobject_detector
        self.stairsnumber_detector = stairsnumber_detector
        self.image = None

    @Slot(object)
    def update_image(self, image):
        self.image = image
        if self.image is not None:
            self.run_detection()

    def run_detection(self):
        frame_start_time = time.time()

        objectsboundingboxes, output_img2 = self.stairsobject_detector.inference(self.image)
        output_img2 = self.stairsnumber_detector.TotalDetection2(output_img2, objectsboundingboxes, 10)
        self.object_detector.drawWithMap(output_img2, objectsboundingboxes)

        frame_end_time = time.time()
        fps = 1 / (frame_end_time - frame_start_time)
        cv2.putText(output_img2, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        cv2.imshow("Detection", output_img2)
        if cv2.waitKey(1) == ord('q'):
            cv2.destroyAllWindows()

######################################### 真并行操作
# 对照组，获得一帧图片之后，先发给实验组，然后自己做边沿检测、输出。
def capture_and_detect_edges(queue):
    cap = cv2.VideoCapture(0)
    while cap.isOpened():
        frame_start_time = time.time()  # 开始记录FPS
        ret, frame = cap.read()
        if ret: # 成功读取到信息
            ###################################
            ###################################
            # time.sleep()里面需要手动调参，以保证最优效果
            time.sleep(0.0625)

            # 边缘检测
            framecopy = frame.copy()
            # 清空队列，不清空否则会堆栈溢出
            while not queue.empty():
                try:
                    queue.get_nowait()
                except Exception as e:
                    print(e)
                    break

            queue.put(frame)  # 将处理后的帧放入队列

            gray = cv2.cvtColor(framecopy, cv2.COLOR_BGR2GRAY)
            edges = cv2.Canny(gray, 100, 200)
            # FPS
            frame_end_time = time.time()
            fps = 1 / (frame_end_time - frame_start_time)
            cv2.putText(edges, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

            cv2.imshow('Control Group', edges)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                cv2.destroyAllWindows()
                break

# 实验组
def process_images(queue, # multiprocess队列
                   i,   # 计数
                   ):
    current_script_path = os.path.abspath(__file__) # 当前python脚本目录，后续迁移可以不需要改动
    detectmodel_path = toAbsolutePath(current_script_path, "models/haizhuv8nint8.onnx") # 相对当前脚本位置
    # 2 读取网络检测器
    object_detector = yolov5v8.YOLOV5V8(detectmodel_path, isType='YOLOV8')
    # 3 读取楼梯检测器
    stairsnumber_detector = stairsDetector.StairsDetector()
    frame_start_time = time.time()  # 最初始化

    while True:
        i += 1
        print("响应：", i)
        time.sleep(0.02)
        image = queue.get()  # 从队列中获取处理后的帧
        if image is None: break
        # 这里可以添加图像处理逻辑
        ##################################
        ##################################
        # 注意这个参数：i % xxx：手动调参到最优
        if i % 4 == 0:
            i = 0
            objectsboundingboxes, output_img2 = object_detector.inference(image)
            output_img2 = stairsnumber_detector.TotalDetection2(output_img2, objectsboundingboxes, 10)
            object_detector.drawWithMap(output_img2, objectsboundingboxes)

            # FPS计算
            frame_end_time = time.time()
            fps = 1 / (frame_end_time - frame_start_time)

            cv2.putText(output_img2, f"FPS: {fps:.2f} [{i:2d}]", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.imshow('Experiment Group', output_img2)

            frame_start_time = time.time()

        if cv2.waitKey(1) == ord('q'):
            cv2.destroyAllWindows()
            break

if __name__ == '__main__':

    print("start")

    i = 0   # 计数

    queue = Queue() # 初始化新建一个新的队列
    p1 = Process(target=capture_and_detect_edges, args=(queue,))
    p2 = Process(target=process_images, args=(queue,
                                              i #计数工具
                                              ))
    p1.start()
    p2.start()
    p1.join()
    p2.join()
