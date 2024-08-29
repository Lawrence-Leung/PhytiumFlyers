# -*- coding: utf-8 -*-
# videoDetection.py
# 视频检测类
# by Lawrence Leung 2024
# 更新：2024.3.21

import cv2
import time
import os
import sys
import numpy as np
from PySide6.QtCore import QCoreApplication, QThread, Signal, Slot
from classes import yolov5v8, stairsDetector

# 输入相对于当前脚本的位置，输出绝对位置
# 输入：filepath 相对目录位置字符串
# 输出：relative_path 绝对目录位置字符串
def toAbsolutePath(current_script_path, filepath):
    current_dir = os.path.dirname(os.path.abspath(current_script_path))  # 获取当前脚本的绝对路径
    relative_path = os.path.join(current_dir, filepath)
    return relative_path

# 主函数
def expired():
    current_script_path = os.path.abspath(__file__) # 当前python脚本目录，后续迁移可以不需要改动
    detectmodel_path = toAbsolutePath(current_script_path, "../models/yolov8n-oiv7-int8.onnx") # 相对当前脚本位置
    stairsmodel_path = toAbsolutePath(current_script_path, "../models/stairs_yolov8n.onnx") # 相对当前脚本位置

    print("start")
    # 1 读取视频流数据
    cap = cv2.VideoCapture("/home/lawrence/projects/Phytium2024-Local/masterdev_lawrence/stairs2.mp4")
    if not cap.isOpened():
            print("Error: Unable to open video file.")
            exit()

    # 2 读取网络检测器
    object_detector = yolov5v8.YOLOV5V8(detectmodel_path, isType='YOLOV8')
    stairsobject_detector = yolov5v8.YOLOV5V8(stairsmodel_path, isType='TEST')

    # 3 读取楼梯检测器
    stairsnumber_detector = stairsDetector.StairsDetector()

    # 4 打开窗口
    cv2.namedWindow("Detection", cv2.WINDOW_NORMAL)
    cv2.namedWindow("Map", cv2.WINDOW_NORMAL)

    while cap.isOpened():
        frame_time = time.time()

        try:
            # 4 尝试打开cap
            ret, image = cap.read()
            #print("Video is opened")
        except:
            print("Video is not opened")
            continue

        if ret:
            # 当读取到一帧画面时，执行
            #print("A frame of video cap is loaded")

            ##############################
            # 注意：使用了yolov5v8类
            # 1. 目标检测网络推理
            objectsboundingboxes, output_img2 = stairsobject_detector.inference(image) # 楼梯目标检测网络

            # 2. 将检测结果绘制在图片上
            # target_class字段：
            # 如果使用的是楼梯目标检测网络，那么填0
            # 如果使用的是yolov8n-oiv7网络，那么填489
            output_img2 = stairsnumber_detector.TotalDetection2(output_img2, objectsboundingboxes, 0)#489)
            object_detector.drawWithMap(output_img2, objectsboundingboxes)  # 注意：第二个字段是objoutput！

            # 3. 计算FPS
            end_time = time.time()  # 截取结束时间
            fps = 1 / (end_time - frame_time)  # 计算FPS

            # 4. 将FPS数据显示在图片上
            cv2.putText(output_img2, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            # 5. 将数据显示在窗口上
            cv2.imshow("Detection", output_img2)

        else:
            break

        # 当在窗口中按下“Q”时退出
        if cv2.waitKey(1) == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

# 捕获图像
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

# 检测图像
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
        output_img2 = self.stairsnumber_detector.TotalDetection2(output_img2, objectsboundingboxes, 0)
        self.object_detector.drawWithMap(output_img2, objectsboundingboxes)

        frame_end_time = time.time()
        fps = 1 / (frame_end_time - frame_start_time)
        cv2.putText(output_img2, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        cv2.imshow("Detection", output_img2)
        if cv2.waitKey(1) == ord('q'):
            cv2.destroyAllWindows()

if __name__ == "__main__":
    app = QCoreApplication(sys.argv)

    current_script_path = os.path.abspath(__file__) # 当前python脚本目录，后续迁移可以不需要改动
    detectmodel_path = toAbsolutePath(current_script_path, "../models/yolov8n-oiv7-int8.onnx") # 相对当前脚本位置
    stairsmodel_path = toAbsolutePath(current_script_path, "../models/stairs_yolov8n.onnx") # 相对当前脚本位置

    print("start")

    # 2 读取网络检测器
    object_detector = yolov5v8.YOLOV5V8(detectmodel_path, isType='YOLOV8')
    stairsobject_detector = yolov5v8.YOLOV5V8(stairsmodel_path, isType='TEST')

    # 3 读取楼梯检测器
    stairsnumber_detector = stairsDetector.StairsDetector()

    capture_thread = CaptureThread()
    detection_thread = DetectionThread(object_detector, stairsobject_detector, stairsnumber_detector)

    capture_thread.image_signal.connect(detection_thread.update_image)

    capture_thread.start()
    detection_thread.start()

    sys.exit(app.exec())
