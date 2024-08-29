# -*- coding: utf-8 -*-
# vieoDetection_AddedDataOutput_0504.py
# 新视频检测方法：使用multiprocessing
# 加入：斑马线检测方法
# by Lawrence Leung 2024
# 更新：2024.4.6

import cv2
import time
import os
import sys
import numpy as np
from PySide6.QtCore import QCoreApplication, QThread, Signal, Slot  # QCoreApplication用于命令行Qt程序，用不上
from classes import yolov5v8, stairsDetector, crossroadsDetector
from drivers import detectResultPackUp
from drivers.speakout import *
from classes.roadDeviationDetector import *

# 新实验
from multiprocessing import Process, Queue

# 输入相对于当前脚本的位置，输出绝对位置
# 输入：filepath 相对目录位置字符串
# 输出：relative_path 绝对目录位置字符串
def toAbsolutePath(current_script_path, filepath):
    current_dir = os.path.dirname(os.path.abspath(current_script_path))  # 获取当前脚本的绝对路径
    relative_path = os.path.join(current_dir, filepath)
    return relative_path

# 将OpenCV图像经过Canny算子返回边缘检测后的图像
def imageToEdges(image):
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    edges = cv2.Canny(gray, 100, 200)
    return edges

######################################### 真并行操作

# 对照组，获得一帧图片之后，先发给实验组，然后自己做边沿检测、输出。
def capture_and_detect_image(image_queue,
                             bounding_box_queue,
                             ):
    # 读取楼梯检测器
    stairsnumber_detector = stairsDetector.StairsDetector()
    crossroad_guider = crossroadsDetector.CrossRoadGuider()
    objdetect_result = None
    detected_image = None

    # 用于语音播报环节
    out_stairs_list = []
    out_zebralines_list = []
    out_trafficlights_list = []
    out_detect_list = []
    string_packer = detectResultPackUp.PackUpResultClass(8)
    test_road_lists = [
        (113.32742024872587, 23.096267632545597, 113.32302142594145, 23.0962133536494),  # 新港中路（西）
        (113.32746852848814, 23.09624789476786, 113.32980741474913, 23.096272566989867),  # 新港中路（东）
        (113.32738269779966, 23.096351518069685, 113.32739342663572, 23.09855226074607),  # 赤岗北路
        (113.32739879105375, 23.09629230476447, 113.32743097756193, 23.09346977360184)  # 赤岗路（南）
    ]   # 位于广州海珠区附近的道路
    speaker = Speaker() # 解包、播报信息
    deviation_detector = DeviationDetector() # 是否沿道路行走

    # 正式运行代码
    cap = cv2.VideoCapture("/home/lawrence/projects/Phytium2024-Local/other_repos/haizhu_dataset/backup/VID_20240328_170131.mp4")
    # cap = cv2.VideoCapture(0)

    while cap.isOpened():
        frame_start_time = time.time()  # 开始记录FPS
        ret, frame = cap.read()
        if ret: # 成功读取到信息
            ###################################
            ###################################
            # time.sleep()里面需要手动调参，以保证最优效果
            time.sleep(0.0625)

            # 边缘检测
            frame = cv2.resize(frame, (640, 640))   # 将图片拉伸至640*640
            framecopy = frame.copy()    # 将图片复制一份
            while not image_queue.empty():  # 清空队列，不清空否则会堆栈溢出
                try:
                    image_queue.get_nowait()
                except Exception as e:
                    print(f"{e} Error by image_queue of def 'capture_and_detect_edges'")
                    break
            image_queue.put(frame)  # 将处理后的帧放入队列

            if not bounding_box_queue.empty():
                (objdetect_result, detected_image, out_detect_list) = bounding_box_queue.get()   # 获取最新的一个检测结果

            ###################################
            # 正式开始根据检测结果进行推理
            if objdetect_result is not None:
                # 楼梯数检测，这一步已经加上了输出数据    todo: 此处更新，日期0504
                # 注意：由于函数bug，这个函数的输出必须只有一个，不能有另一个。否则出问题！
                framecopy = stairsnumber_detector.TotalDetectionWithOutputData(framecopy,objdetect_result, 10, out_stairs_list)
                # 斑马线检测，这一步已经加上了输出数据    todo: 此处更新，0504
                out_zebralines_list, out_trafficlights_list, framecopy = crossroad_guider.TotalDetectionAsyncWithDataOutput(detected_image, framecopy, objdetect_result)  # debug only

            # FPS
            frame_end_time = time.time()
            fps = 1 / (frame_end_time - frame_start_time)
            cv2.putText(framecopy, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

            # debug 将数据打印出来
            # print("楼梯数据", out_stairs_list, "\r\n", "斑马线数据", out_zebralines_list, "\r\n", "交通灯数据", out_trafficlights_list, "其他数据", out_detect_list, len(out_detect_list))
            # 全部转化为数据序列
            string_packer.ComplexFromListsToString(out_stairs_list, out_zebralines_list, out_trafficlights_list, out_detect_list)
            # todo: 从这里开始，后续是需要将这个bytearray传到slave端，而目前需要为了演示，因此暂时在master端执行。

            ####################################
            # 解包，这个过程暂时先用Python实现（master端，后续放到slave端）
            array = string_packer.string
            if isinstance(array, bytearray) and len(array) > 0:
                # 从字符串中提取各种信息
                speaker.object_number, speaker.zebraline_number, speaker.traffic_light_number, speaker.stairs_number = SpkUnpackHead(array)
                speaker.objects = UnpackObject(array, speaker.object_number)
                speaker.zebralines = UnpackZebraline(array, speaker.zebraline_number)
                speaker.traffic_lights = UnpackTrafficLight(array, speaker.traffic_light_number)
                speaker.stairs = UnpackStairs(array, speaker.stairs_number)

                # 开始播报障碍物了，{}里面的内容是函数所返回的字符串，这些字符串使用的是上面所提取出的信息
                print(f'[播报1]: {SpeakObstacles(speaker.objects)}')   # todo：需要传入slave端：001 障碍物识别结果
                zebrastring, trafficstring = SpeakZebraTraffic(speaker.zebralines, speaker.traffic_lights) # todo：需要传入slave端：002 斑马线识别结果、003 交通灯识别结果
                print(f'[播报2,3]: {zebrastring} {trafficstring}')
                print(f'[播报4]" {SpeakStairs(speaker.stairs)}') # todo：需要传入slave端：004 楼梯识别结果

            # 将GPS数据放到这里来
            current_location = (113.32405139420317, 23.096203484758096) # todo：这里要把它替换成由GPS传感器传过来的
            isnearroad, isnearcrossing, isalongroad, nearest_road = deviation_detector.CompleteRoadDeviationProcess(test_road_lists, current_location)  # 分析行人是否沿着道路行走
            # 开始播报是否沿着道路行走了，{}里面的内容是字符串
            print(f'[播报5]: {SpeakDeviationRoad(isalongroad, isnearroad, isnearcrossing)} \r\n')   # todo：需要传入slave端：005 是否沿着道路行走

            # 收尾
            out_stairs_list.clear()
            out_zebralines_list.clear()
            out_trafficlights_list.clear()

            ####################################
            cv2.imshow('Control Group', framecopy)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                cv2.destroyAllWindows()
                break


# 实验组
def process_images(image_queue,  # multiprocess的image队列
                   bounding_box_queue,  # multiprocess的bounding box队列
                   i,  # 计数
                   ):
    current_script_path = os.path.abspath(__file__) # 当前python脚本目录，后续迁移可以不需要改动
    detectmodel_path = toAbsolutePath(current_script_path, "models/haizhuv8nint8.onnx") # 相对当前脚本位置
    # 目标检测网络
    object_detector = yolov5v8.YOLOV5V8(detectmodel_path, isType='HAIZHU')

    frame_start_time = time.time()  # 事件最初始化

    out_detect_list = []

    while True:
        i += 1
        # print("响应：", i)
        time.sleep(0.02)
        image = image_queue.get()  # 从队列中获取处理后的帧
        if image is None: break
        # 这里可以添加图像处理逻辑
        ##################################
        ##################################
        # 注意这个参数：i % xxx：手动调参到最优
        if i % 4 == 0:
            i = 0
            objectsboundingboxes, output_img2 = object_detector.inference(image)    # ONNX 推理
            out_detect_list = object_detector.drawWithMapOutput(output_img2, objectsboundingboxes)  # 把推理结果放到小地图上

            # 将推理结果放入队列
            while not bounding_box_queue.empty():
                try:
                    bounding_box_queue.get_nowait()
                except Exception as e:
                    print(f"{e} Error by bounding_box_queue of def 'capture_and_detect_edges'")
            bounding_box_queue.put((objectsboundingboxes, image, out_detect_list))   # 为了检测红绿灯的临时操作。

            # FPS计算
            frame_end_time = time.time()
            fps = 1 / (frame_end_time - frame_start_time)

            cv2.putText(output_img2, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.imshow('Experiment Group', output_img2)

            frame_start_time = time.time()

        if cv2.waitKey(1) == ord('q'):
            cv2.destroyAllWindows()
            break

if __name__ == '__main__':

    print("start")

    i = 0   # 计数

    image_queue = Queue()           # 新队列：图像队列
    bounding_box_queue = Queue()

    p1 = Process(target=capture_and_detect_image, args=(image_queue, bounding_box_queue))
    p2 = Process(target=process_images, args=(image_queue,
                                              bounding_box_queue,
                                              i,  #计数工具
                                              ))
    p1.start()
    p2.start()
    p1.join()
    p2.join()
