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
from PySide6.QtCore import QCoreApplication, QThread, Signal, Slot, QObject
from PySide6.QtGui import QImage, QPixmap, QColor
from PySide6.QtWidgets import QApplication, QMainWindow, QFileDialog, QLabel,QMessageBox,QDialog, QVBoxLayout,QComboBox, QPushButton
from classes import yolov5v8, stairsDetector, crossroadsDetector
from drivers import detectResultPackUp
from drivers.rpmsg import RPMsg
from drivers.rpmsg import *
from drivers.speakout import *
from classes.roadDeviationDetector import *
from ui.test_ui import Ui_MainWindow
from UIFunctions import *
from utils.capnums import Camera
import cv2
import time
import sys
import os
import threading
import json
import psutil

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

def convert_cv_to_qt(cv_img):
        height, width, channel = cv_img.shape
        bytes_per_line = 3 * width
        qt_img = QImage(cv2.cvtColor(cv_img, cv2.COLOR_BGR2RGB), width, height, bytes_per_line, QImage.Format_RGB888)
        return qt_img

class CameraSelectionDialog(QDialog):
    def __init__(self, cams, parent=None):
        super().__init__(parent)
        self.setWindowTitle('Select Camera')
        layout = QVBoxLayout(self)
        
        self.label = QLabel('Select camera:', self)
        layout.addWidget(self.label)

        self.comboBox = QComboBox(self)
        layout.addWidget(self.comboBox)

        for cam in cams:
            self.comboBox.addItem(str(cam))  # 将设备编号作为下拉列表项添加
        self.okButton = QPushButton('OK', self)
        self.okButton.clicked.connect(self.accept)
        layout.addWidget(self.okButton)

        self.setLayout(layout)
        self.setWindowFlag(Qt.WindowContextHelpButtonHint, False)

    def selected_camera(self):
        return self.comboBox.currentText()

class SlaveDeviceThread(QObject):
    dht_update=Signal(str)#温湿度
    down_update=Signal(str)#摔倒
    cam_update=Signal(str)#相机位置
    gps_update=Signal(str)#GPS
    loc_detected=Signal(str)#地理位置
    mtosT=Signal(str)#响应时间
    def __init__(self, parent=None):
        super().__init__(parent)
        self.running = False
        self.dht_queue=Queue()
        self.gps_queue=Queue()
        self.cam_queue=Queue()
        self.t_queue=Queue()
        self.test_road_lists = [
            (113.32742024872587, 23.096267632545597, 113.32302142594145, 23.0962133536494),  # 新港中路（西）
            (113.32746852848814, 23.09624789476786, 113.32980741474913, 23.096272566989867),  # 新港中路（东）
            (113.32738269779966, 23.096351518069685, 113.32739342663572, 23.09855226074607),  # 赤岗北路
            (113.32739879105375, 23.09629230476447, 113.32743097756193, 23.09346977360184)  # 赤岗路（南）
        ]   # 位于广州海珠区附近的道路
        speaker = Speaker() # 解包、播报信息
        self.deviation_detector = DeviationDetector() # 是否沿道路行走

        self.api_key = "e0b93d4f1620b219cc59584de3bb01ee"
        self.rpmsg0 = RPMsg("/dev/rpmsg_ctrl_lawrence_10",
                   "/dev/rpmsg_lawrence_10",
                   "hello", 11, 0)
        self.rpmsg0.openCtrlDevice()
        self.rpmsg1 = RPMsg("/dev/rpmsg_ctrl_lawrence_20",
                  "/dev/rpmsg_lawrence_20",
                  "world", 22, 0)
        self.rpmsg1.openCtrlDevice()
        self.rpmsg2 = RPMsg("/dev/rpmsg_ctrl_lawrence_30",
                   "/dev/rpmsg_lawrence_30",
                   "hello", 33, 0)
        self.rpmsg2.openCtrlDevice()
    # 创建第四个端点
        self.rpmsg3 = RPMsg("/dev/rpmsg_ctrl_lawrence_40",
                   "/dev/rpmsg_lawrence_40",
                   "world", 44, 0)
        self.rpmsg3.openCtrlDevice()
        self.rpmsg0.createEndpoint()
        self.rpmsg1.createEndpoint()
        self.rpmsg2.createEndpoint()
        self.rpmsg3.createEndpoint()
        self.rpmsg0.openEptDevice()
        self.rpmsg1.openEptDevice()
        self.rpmsg2.openEptDevice()
        self.rpmsg3.openEptDevice()
        self.count = 15
        self.time1=0

        self.jy61pdata = np.zeros((3, 3))	# JY61P接收缓存
        data_history = []
        self.var=None
        self.timetosend()
    def slavetask(self):
        self.running=True
        print("Slave running")
        while self.running:
            # try:
            #DHT11
                # self.msleep(100)   # 睡眠1秒
                start_time=time.time()
                self.rpmsg0.writeEptDevice() # 将已有的数据写入到端点里面去
                end_ime=time.time()
                self.time1=end_ime-start_time
                
                time_formatted=round(self.time1*1000,2)
                print("time1:",time_formatted)
                self.t_queue.put(str(time_formatted))
                print("[OPENAMP] WAIT data from DHT11")
                self.rpmsg0.pollEptDeviceWithReadEvent()

                time.sleep(1)   # 睡眠1秒
                print("[OPENAMP] read data from DHT11")

                self.rpmsg0data = self.rpmsg0.readEptDevice()
                
                if self.rpmsg0data is not None:
                    result = slaveDHT11Analysis(self.rpmsg0data)
                else:
                    continue
                if result:
                    result_str="({}%,{}°C)".format(result[0],result[1])
                    print("[OPENAMP] DHT11 received data: ", result_str)
                    self.dht_queue.put(str(result_str))
                    #formatted_result=tuple(round(value,2) for value in result)
                    #print("formatted data: ", formatted_result) 
            #GPS
                self.rpmsg1.writeEptDevice()
                print("[OPENAMP] WAIT data from GPS")
                self.rpmsg1.pollEptDeviceWithReadEvent()
                time.sleep(1)
                print("[OPENAMP] read data from GPS")
                self.rpmsg1data = self.rpmsg1.readEptDevice()
                if type(self.rpmsg1data) is not type(self.var):
                    result = slaveGPSAnalysis(self.rpmsg1data)
                    if result and result!=(0.0,0.0):
                        print("[OPENAMP] GPS received data: ", result)
                        current_location = result
                        isnearroad, isnearcrossing, isalongroad, nearest_road = self.deviation_detector.CompleteRoadDeviationProcess(self.test_road_lists, current_location)  # 分析行人是否沿着道路行走
                        # 开始播报是否沿着道路行走了，{}里面的内容是字符串
                        print(f'[播报5]: {SpeakDeviationRoad(isalongroad, isnearroad, isnearcrossing)} \r\n')   # todo：需要传入slave端：005 是否沿着道路行走
                        # formatted_result=tuple(f'{value:.2f}' for value in result)
                        formatted_result=tuple(round(value,2) for value in result)
                        print("formatted data: ", formatted_result)
                        formatted_address,citycode=gpsdecode.decode_address(result[0],result[1],self.api_key)
                        weather_data=gpsdecode.get_weather(self.api_key,citycode)
                        current_weather = weather_data['forecasts'][0]['casts'][0]
                        weather_result=current_weather['dayweather']

                        print("地理信息：",formatted_address,"当前天气：",weather_result)
                        self.gps_queue.put((str(formatted_result),str(formatted_address),str(weather_result)))
                        time.sleep(1)

            # #JY61P
                time.sleep(1)    # 睡眠1秒
                self.rpmsg3.writeEptDevice() # 将已有的数据写入到端点里面去
                print("[OPENAMP] WAIT data from JY61P")
                self.rpmsg3.pollEptDeviceWithReadEvent()

                # time.sleep(1)   # 睡眠1秒
                print("[OPENAMP] read data from JY61P")
            # 以下是数据解析
                data = self.rpmsg3.readEptDevice()
                rpmsg3_result=slaveJY61PAnalysis(self.jy61pdata, data)
                #print("len:",len(rpmsg3_result))
                print(type(rpmsg3_result))
                if rpmsg3_result is not None:
                    for i in range(len(rpmsg3_result[0])):
                        if rpmsg3_result[0][i]<=3:
                            status="未摔倒"
                        else:
                            status="摔倒"

                    for i in range(len(rpmsg3_result[2])):
                        if rpmsg3_result[2][i]<=6:
                            cam="相机视角正常"
                        else:
                            cam="相机视角异常"
                    self.cam_queue.put((status,cam))
                else:
                    continue
                time.sleep(1)
            #SYN6288
                if not self.cam_queue.empty():
                    string=self.cam_queue.get()
                    string1="{},{}".format(string[0],string[1])
                    print("播报：",string1)
                    # self.rpmsg2.writeSpeaker(string1) # 将已有的数据写入到端点里面去  
                else:
                    # self.rpmsg2.writeEptDevice()
                    continue


            # except Exception as e:
            #     print(f"Example operation failed. {e}")

        # self.rpmsg0.closeEptDevice()
        # self.rpmsg0.closeCtrlDevice()
        # self.rpmsg1.closeEptDevice()
        # self.rpmsg1.closeCtrlDevice()
        # self.rpmsg2.closeEptDevice()
        # #print("rpmsg2 closed")
        # self.rpmsg2.closeCtrlDevice()
        # self.rpmsg3.closeEptDevice()
        # self.rpmsg3.closeCtrlDevice()

    def start_device(self):
        self.slave_process=Process(target=self.slavetask)
        self.slave_process.start()

    def stop_device(self):
        self.running = False
        self.rpmsg0.closeEptDevice()
        self.rpmsg0.closeCtrlDevice()
        self.rpmsg1.closeEptDevice()
        self.rpmsg1.closeCtrlDevice()
        self.rpmsg2.closeEptDevice()
        self.rpmsg2.closeCtrlDevice()
        self.rpmsg3.closeEptDevice()
        self.rpmsg3.closeCtrlDevice()
        self.slave_process.terminate()

    def timetosend(self):
        self.timerS = QTimer()
        self.timerS.timeout.connect(self.emit_signal_to_UI)
        self.timerS.start(1)  # 每毫秒发射一次信号

    def emit_signal_to_UI(self):
        if not self.dht_queue.empty():
            dht=self.dht_queue.get()
            print("dht:",dht)
            self.dht_update.emit(dht)
        if not self.gps_queue.empty():
            tuple_result=self.gps_queue.get()
            gps,address,wethr=tuple_result
            self.gps_update.emit(gps)
            address1="位置：{}， 当前天气：{}".format(address,wethr)
            self.loc_detected.emit(address1)
        if not self.cam_queue.empty():
            tuple_result2=self.cam_queue.get()
            print("cam_queue:",tuple_result2)
            status,cam=tuple_result2
            self.down_update.emit(status)
            self.cam_update.emit(cam)
            self.rpmsg2.writeSpeaker(f"{status}, {cam}")
        if not self.t_queue.empty():
            time=self.t_queue.get()
            
            self.mtosT.emit(time)

    @Slot(str)
    def handle_broadcast(self,content):
        print("接收播报:",content)
        self.rpmsg2.writeSpeaker(content)



######################################### 真并行操作
class VideoDetectionSystem(QObject):
    signal_pre1 = Signal(QImage)
    signal_pre2 = Signal(QImage)
    signal_pre3=Signal(QImage)
    broadcast_signal = Signal(str)
    sysT=Signal(str)
    def __init__(self):
        super().__init__()
        self.image_queue = Queue()
        self.bounding_box_queue = Queue()
        self.output_img_queue = Queue()
        self.framecopy_queue = Queue()
        self.map_queue = Queue()
        self.time_queue=Queue()
        self.p1 = None
        self.p2 = None
        self.i=0
        self.source = ''
        self.current_location=None

        self.timer_pre1 = QTimer()
        self.timer_pre1.timeout.connect(self.emit_signal_pre1)
        self.timer_pre1.start(0.5)  # 每毫秒发射一次信号

        self.timer_pre2 = QTimer()
        self.timer_pre2.timeout.connect(self.emit_signal_pre2)
        self.timer_pre2.start(0.5)  # 每毫秒发射一次信号

        self.timer_pre3 = QTimer()
        self.timer_pre3.timeout.connect(self.emit_signal_pre3)
        self.timer_pre3.start(0.5)  # 每毫秒发射一次信号
    def emit_signal_pre1(self):
        # 在这里实现发送 signal_pre1 的逻辑
        # 例如，从队列中获取图像，然后发射信号
        if not self.output_img_queue.empty():
            frame = self.output_img_queue.get()
            qt_img1 = convert_cv_to_qt(frame)
            self.signal_pre1.emit(qt_img1)
            time=self.time_queue.get()
            self.sysT.emit(str(time))

    def emit_signal_pre2(self):
        # 在这里实现发送 signal_pre2 的逻辑
        # 例如，从队列中获取图像，然后发射信号
        if not self.framecopy_queue.empty():
            frame = self.framecopy_queue.get()
            qt_img2 = convert_cv_to_qt(frame)
            self.signal_pre2.emit(qt_img2)

    def emit_signal_pre3(self):
        # 在这里实现发送 signal_pre2 的逻辑
        # 例如，从队列中获取图像，然后发射信号
        if not self.map_queue.empty():
            frame = self.map_queue.get()
            qt_img3 = convert_cv_to_qt(frame)
            self.signal_pre3.emit(qt_img3)
    

# 对照组，获得一帧图片之后，先发给实验组，然后自己做边沿检测、输出。
    def capture_and_detect_image(self,image_queue,
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
        cap = cv2.VideoCapture(self.source)#"/home/jimkwokying/Videos/sample.mp4")
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
                self.time0=frame_end_time - frame_start_time
                self.time0=round(self.time0,2)
                print("time0:",self.time0)
                self.time_queue.put(self.time0)
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
                    # print(f'[播报1]: {SpeakObstacles(speaker.objects)}')   # todo：需要传入slave端：001 障碍物识别结果
                    zebrastring, trafficstring = SpeakZebraTraffic(speaker.zebralines, speaker.traffic_lights) # todo：需要传入slave端：002 斑马线识别结果、003 交通灯识别结果
                    # print(f'[播报2,3]: {zebrastring} {trafficstring}')
                    # print(f'[播报4]" {SpeakStairs(speaker.stairs)}') # todo：需要传入slave端：004 楼梯识别结果
                    
                    # 将GPS数据放到这里来
                    # current_location = (113.32405139420317, 23.096203484758096) # todo：这里要把它替换成由GPS传感器传过来的
                    # if current_location is not None:
                    #     isnearroad, isnearcrossing, isalongroad, nearest_road = deviation_detector.CompleteRoadDeviationProcess(test_road_lists, current_location)  # 分析行人是否沿着道路行走
                        # 开始播报是否沿着道路行走了，{}里面的内容是字符串
                        # print(f'[播报5]: {SpeakDeviationRoad(isalongroad, isnearroad, isnearcrossing)} \r\n')   # todo：需要传入slave端：005 是否沿着道路行走
                    broadcast_content = f'{SpeakObstacles(speaker.objects)}' + \
                                    f' {zebrastring} {trafficstring}' + \
                                    f'{SpeakStairs(speaker.stairs)}\n' #+ \
                                    #f' {SpeakDeviationRoad(isalongroad, isnearroad, isnearcrossing)}\n'
                    self.broadcast_signal.emit(broadcast_content)
                # 收尾
                out_stairs_list.clear()
                out_zebralines_list.clear()
                out_trafficlights_list.clear()

                ####################################
                # cv2.imshow('Control Group', framecopy)
                self.framecopy_queue.put(framecopy)
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    cv2.destroyAllWindows()
                    break


    # 实验组
    def process_images(self,image_queue,  # multiprocess的image队列
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
            # print(image_queue.empty())
            image = image_queue.get()  # 从队列中获取处理后的帧
            if image is None: break
            # 这里可以添加图像处理逻辑
            ##################################
            ##################################
            # 注意这个参数：i % xxx：手动调参到最优
            if i % 4 == 0:
                i = 0
                objectsboundingboxes, output_img2 = object_detector.inference(image)    # ONNX 推理
                out_detect_list,object = object_detector.drawWithMapOutput(output_img2, objectsboundingboxes)  # 把推理结果放到小地图上
                self.map_queue.put(object)

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
                # cv2.imshow('Experiment Group', output_img2)
                self.output_img_queue.put(output_img2)

                frame_start_time = time.time()

            if cv2.waitKey(1) == ord('q'):
                cv2.destroyAllWindows()
                break

    def start_system(self):
            self.p1 = Process(target=self.capture_and_detect_image,args=(self.image_queue, self.bounding_box_queue))
            self.p2 = Process(target=self.process_images,args=(self.image_queue, self.bounding_box_queue, self.i))
            self.p1.start()
            self.p2.start()

    def stop_system(self):
            if self.p1 is not None:
                self.p1.terminate()
                self.p2.terminate()

    def gpsrecieved(self,gps):
        self.current_location=gps
class SlaveTest(QObject):
    gpstest=Signal(tuple)
    def __init__(self, parent=None):
        super().__init__(parent)
        self.test_road_lists = [
            (113.32742024872587, 23.096267632545597, 113.32302142594145, 23.0962133536494),  # 新港中路（西）
            (113.32746852848814, 23.09624789476786, 113.32980741474913, 23.096272566989867),  # 新港中路（东）
            (113.32738269779966, 23.096351518069685, 113.32739342663572, 23.09855226074607),  # 赤岗北路
            (113.32739879105375, 23.09629230476447, 113.32743097756193, 23.09346977360184)  # 赤岗路（南）
        ]   # 位于广州海珠区附近的道路
        speaker = Speaker() # 解包、播报信息
        self.deviation_detector = DeviationDetector() # 是否沿道路行走
        # self.video_detection_system = VideoDetectionSystem()
        # self.video_detection_system.broadcast_signal.connect(self.handle_broadcast)
        print("Test Started")
    def slavetask(self):
        # 将GPS数据放到这里来
        while True:
            print("Waiting for gps")
            time.sleep(0.5)
            current_location = (113.32405139420317, 23.096203484758096) # todo：这里要把它替换成由GPS传感器传过来的
            if current_location is not None:
                isnearroad, isnearcrossing, isalongroad, nearest_road = self.deviation_detector.CompleteRoadDeviationProcess(self.test_road_lists, current_location)  # 分析行人是否沿着道路行走
                # 开始播报是否沿着道路行走了，{}里面的内容是字符串
                print(f'[播报5]: {SpeakDeviationRoad(isalongroad, isnearroad, isnearcrossing)} \r\n')   # todo：需要传入slave端：005 是否沿着道路行走
    @Slot(str)
    def handle_broadcast(self,content):
        print("接收播报:",content)

    def start_device(self):
        self.slave_process=Process(target=self.slavetask)
        self.slave_process.start()

    
class MainWindow(Ui_MainWindow, QMainWindow):
    main2detect_begin_sgl = Signal()#signal to control thread of detection
    status_msg=Signal(str)
    cputotal=Signal(str)
    cpusig=Signal(str)
    def __init__(self, parent=None):
        
        super().__init__(parent)
        self.setupUi(self)
        self.setAttribute(Qt.WA_TranslucentBackground)  # rounded transparent
        self.setWindowFlags(Qt.FramelessWindowHint)  # Set window flag: hide window borders
        self.ToggleBotton.clicked.connect(lambda: UIFuncitons.toggleMenu(self, True))   # left navigation button
        self.settings_button.clicked.connect(lambda: UIFuncitons.settingBox(self, True))   # top right settings button
        UIFuncitons.uiDefinitions(self)
        # Show module shadows
        UIFuncitons.shadow_style(self, self.Class_QF, QColor(94,96,92))
        UIFuncitons.shadow_style(self, self.Target_QF, QColor(186,189,182))
        UIFuncitons.shadow_style(self, self.Fps_QF, QColor(211,215,207))
        UIFuncitons.shadow_style(self, self.Model_QF, QColor(136,138,133))
        #左菜单栏
        self.src_file_button.clicked.connect(self.open_src_file)  # select local file
        self.src_cam_button.clicked.connect(self.chose_cam)
        self.status_msg.connect(lambda x: self.status_bar.setText(x))
        self.run_button.clicked.connect(self.start_detection)
        self.stop_button.clicked.connect(self.stop_detection)
        self.cputotal.connect(lambda x:self.cputotal_label.setText(f"整体CPU占用率：{x}"))
        self.cpusig.connect(lambda x:self.cpu_label.setText(x))
        
        
        self.video_detection_system = VideoDetectionSystem()
        self.video_detection_system.moveToThread(QCoreApplication.instance().thread())
        self.video_detection_system.signal_pre1.connect(self.update_pre1_image)
        self.video_detection_system.signal_pre2.connect(self.update_pre2_image)
        self.video_detection_system.signal_pre3.connect(self.update_map)
        self.video_detection_system.sysT.connect(lambda x:self.time0_label.setText(f"{x}s"))

        self.slave_thread=SlaveDeviceThread()
        self.slave_thread.moveToThread(QCoreApplication.instance().thread())
        self.slave_thread.gps_update.connect(lambda x: self.gps_label.setText(x))
        self.slave_thread.loc_detected.connect(lambda x: self.label_3.setText(x))
        self.slave_thread.down_update.connect(lambda x: self.status_label.setText(x))
        self.slave_thread.cam_update.connect(lambda x: self.cam_label.setText(x))
        self.slave_thread.dht_update.connect(lambda x: self.hum_label.setText(x))
        self.slave_thread.mtosT.connect(lambda x:self.time1_label.setText(f"{x}ms"))
        self.video_detection_system.broadcast_signal.connect(self.slave_thread.handle_broadcast)
        self.slave_thread.dht_update.connect(self.source_using)

        # self.slavetest=SlaveTest()
        # self.video_detection_system.broadcast_signal.connect(self.slavetest.handle_broadcast)
        # self.slavetest.gpstest.connect(self.video_detection_system.gpsrecieved)

    def start_detection(self):
        if self.video_detection_system.source == '':
            self.status_msg.emit('Please select a video source before starting detection...')
            self.run_button.setChecked(False)
        else:
            self.video_detection_system.start_system()
            # self.slavetest.start_device()
            self.slave_thread.start_device()

    def stop_detection(self):
        if self.video_detection_system.p1 is not None:
            self.slave_thread.stop_device()
            self.video_detection_system.stop_system()
        
   
    def mousePressEvent(self, event):
        p = event.globalPosition()
        globalPos = p.toPoint()
        self.dragPos = globalPos

    def closeEvent(self,event):
        # 处理程序退出时的逻辑
        if self.video_detection_system.p1 is not None:
            self.slave_thread.stop_device()
            self.video_detection_system.stop_system()

    @Slot(QImage)
    def update_pre1_image(self, image):
        # print("pre1 image ok")
        label_width = self.res_video.width()
        label_height = self.res_video.height()

    # 调整图像大小以适应 QLabel
        scaled_img = image.scaled(label_width, label_height)
        self.res_video_2.setPixmap(QPixmap.fromImage(scaled_img))
        
    @Slot(QImage)
    def update_pre2_image(self, image):
        # print("update_pre2_image is running in thread:", threading.current_thread().ident)
        label_width = self.pre_video.width()
        label_height = self.pre_video.height()
        scaled_img = image.scaled(label_width, label_height)
        self.pre_video.setPixmap(QPixmap.fromImage(scaled_img))
        self.pre_video_2.setPixmap(QPixmap.fromImage(scaled_img))

    @Slot(QImage)
    def update_map(self, image):
        # 更新 UI，例如将 image 显示在 pre_video QLabel 中
        label_width = self.pre_video.width()
        label_height = self.pre_video.height()
        scaled_img = image.scaled(label_width, label_height)
        self.res_video.setPixmap(QPixmap.fromImage(scaled_img))
    
    def open_src_file(self):
        self.stop_detection()  
        current_script_path = os.path.abspath(__file__)
        config_dir = os.path.join(os.path.dirname(current_script_path), "config")#从子目录读取
        config_file = os.path.join(config_dir, "fold.json") 
        config = json.load(open(config_file, 'r', encoding='utf-8'))
        open_fold = config['open_fold']     
        if not os.path.exists(open_fold):
            open_fold = os.getcwd()
        name, _ = QFileDialog.getOpenFileName(self, 'Video/image', open_fold, "Pic File(*.mp4 *.mkv *.avi *.flv *.jpg *.png)")
        if name:
            self.video_detection_system.source = name
            print("source:", self.video_detection_system.source)
            self.status_msg.emit('Load File：{}'.format(os.path.basename(name))) 
            config['open_fold'] = os.path.dirname(name)
            config_json = json.dumps(config, ensure_ascii=False, indent=2)  
            with open(config_file, 'w', encoding='utf-8') as f:
                f.write(config_json)
            

    def chose_cam(self):
        try:
            self.stop_detection()
            # get the number of local cameras
            _,cams = Camera().get_cam_num()
            print("Cams:", cams) 
            dialog = CameraSelectionDialog(cams)
            if dialog.exec():
                selected_camera = dialog.selected_camera()
                self.video_detection_system.source = int(selected_camera)
                self.status_msg.emit('Loading camera: {}'.format(selected_camera))
            print("sourse:",self.video_detection_system.source)

        except Exception as e:
            self.status_msg.emit(str(e))

    def source_using(self):
        # 获取每个CPU的使用率

        cpu_usage = psutil.cpu_percent(interval=1, percpu=True)
        cpu_str="("+"%,".join(map(str,cpu_usage))+"%)"
        self.cpusig.emit(cpu_str)
        for i, usage in enumerate(cpu_usage):
            print(f"CPU {i+1} 使用率: {usage}%")

        # 获取系统整体CPU使用率
        total_cpu_usage = psutil.cpu_percent(interval=1)
        print("系统整体CPU使用率:", total_cpu_usage, "%")
        self.cputotal.emit(f"{total_cpu_usage}%")
        time.sleep(0.5)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    mainWindow = MainWindow()
    # print("main is running in thread:", threading.current_thread().ident)    
    mainWindow.show()
    sys.exit(app.exec())

