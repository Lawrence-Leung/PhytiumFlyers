from PySide6.QtCore import QObject, Signal, Slot
from PySide6.QtGui import QImage
from multiprocessing import Process, Queue
from classes import yolov5v8, stairsDetector, crossroadsDetector,test
from drivers import detectResultPackUp
from classes.roadDeviationDetector import *
from classes.coordination import LittleMap
from UIFunctions import *
from cameraSource import *
import cv2
import time
import os
from queue import Empty

def convert_cv_to_qt(cv_img):
        height, width, channel = cv_img.shape
        bytes_per_line = 3 * width
        qt_img = QImage(cv2.cvtColor(cv_img, cv2.COLOR_BGR2RGB), width, height, bytes_per_line, QImage.Format_RGB888)
        return qt_img

class VideoDetectionSystem(QObject):
    signal_pre1 = Signal(QImage)
    signal_pre2 = Signal(QImage)
    signal_pre3=Signal(QImage)
    broadcast_signal = Signal(str)
    sysT=Signal(str)
    def __init__(self):
        super().__init__()
        self.map=LittleMap()
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

        self.timer_pre1 = QTimer()
        self.timer_pre1.timeout.connect(self.emit_signal_pre1)
        self.timer_pre1.start(1)  # 每毫秒发射一次信号

        self.timer_pre2 = QTimer()
        self.timer_pre2.timeout.connect(self.emit_signal_pre2)
        self.timer_pre2.start(1)  # 每毫秒发射一次信号

        self.timer_pre3 = QTimer()
        self.timer_pre3.timeout.connect(self.emit_signal_pre3)
        self.timer_pre3.start(1)  # 每毫秒发射一次信号
    
    def emit_signal_pre1(self):
        if not self.output_img_queue.empty():
            frame = self.output_img_queue.get()
            qt_img1 = convert_cv_to_qt(frame)
            self.signal_pre1.emit(qt_img1)
            time=self.time_queue.get()
            self.sysT.emit(str(time))

    def emit_signal_pre2(self):
        if not self.framecopy_queue.empty():
            frame = self.framecopy_queue.get()
            qt_img2 = convert_cv_to_qt(frame)
            self.signal_pre2.emit(qt_img2)

    def emit_signal_pre3(self):
        if not self.map_queue.empty():
            frame = self.map_queue.get()
            qt_img3 = convert_cv_to_qt(frame)
            self.signal_pre3.emit(qt_img3)
       
    @Slot()
    def start_system(self):
        self.p1 = Process(target=self.capture_and_detect_image,args=(self.image_queue, self.bounding_box_queue))
        self.p2 = Process(target=self.process_images,args=(self.image_queue, self.bounding_box_queue, self.i))
        self.p1.start()
        self.p2.start()
    @Slot()
    def stop_system(self):
        if self.p1 is not None:
            self.p1.terminate()
            self.p2.terminate()


    def gpsrecieved(self,gps):
        self.current_location=gps
        print("GPS received:",self.current_location)

    def capture_and_detect_image(self,image_queue,
                             bounding_box_queue
                             ):
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
        cap = cv2.VideoCapture(self.source)#"/mnt/hgfs/Desktop/sample.mp4")#/home/jimkwokying/Desktop/masterdevice/PySide6-GUI/video/sample.mp4")
    # cap = cv2.VideoCapture(0)

        while cap.isOpened():
            frame_start_time = time.time()  # 开始记录FPS
            ret, frame = cap.read()
            if ret: 
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
                    (objdetect_result, detected_image,out_detect_list) = bounding_box_queue.get()   # 获取最新的一个检测结果

            ###################################
            # 正式开始根据检测结果进行推理
                if objdetect_result is not None:
                    stair_start=time.time()
                    framecopy = stairsnumber_detector.TotalDetectionWithOutputData(framecopy,objdetect_result, 10, out_stairs_list)
                    stair_end_time = time.time()
                    #print("stair time:",stair_end_time - stair_start)
                    cross_start=time.time()
                    out_zebralines_list, out_trafficlights_list, framecopy = crossroad_guider.TotalDetectionAsyncWithDataOutput(detected_image, framecopy, objdetect_result)  # debug only
                    cross_end_time = time.time()
                    #print("cross time:",cross_end_time - cross_start)

            # FPS
                frame_end_time = time.time()
            	
                fps = 1 / (frame_end_time - frame_start_time)
                self.time0=frame_end_time - frame_start_time
                self.time0=round(self.time0,2)
                print("time0:",self.time0)#循环1帧的时间
                self.time_queue.put(self.time0)
                print("FPS_show:",fps)
                cv2.putText(framecopy, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
                # 全部转化为数据序列
                string_packer.ComplexFromListsToString(out_stairs_list, out_zebralines_list, out_trafficlights_list, out_detect_list)
                array = string_packer.string
                array_string=array.hex()
                print("array_string:",array_string)
                self.broadcast_signal.emit(array_string)
                out_stairs_list.clear()
                out_zebralines_list.clear()
                out_trafficlights_list.clear()
                self.framecopy_queue.put(framecopy)
                # self.signal_pre1.emit(qt_img1)
                # cv2.imshow('Control Group', framecopy)
            
    
    def process_images(self,image_queue,  # multiprocess的image队列
                    bounding_box_queue,  # multiprocess的bounding box队列
                    i,  # 计数
                    ):
        # 原本的 process_images 函数代码...
        # 注意：在该方法中发送图像给 MainWindow 更新 pre2 控件时，使用 self.signal_pre2.emit(qt_img)
        current_script_path = os.path.abspath(__file__) # 当前python脚本目录，后续迁移可以不需要改动
        # detectmodel_path = toAbsolutePath(current_script_path, "models/haizhuv8nint8.onnx") # 相对当前脚本位置
        detectmodel_path = toAbsolutePath(current_script_path,"models/hzv8nfp16.mnn")#"models/haizhuv8nint8.onnx")# 
        # 目标检测网络
        object_detector = test.YOLOV5V8(detectmodel_path, isType='HAIZHU')

        frame_start_time = time.time()  # 事件最初始化

        while True:
            i += 1
            print("响应：", i)
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
                out_detect_list,objects=object_detector.drawWithMapOutput(output_img2, objectsboundingboxes)  # 把推理结果放到小地图上
                self.map_queue.put(objects)

                # 将推理结果放入队列
                while not bounding_box_queue.empty():
                    try:
                        bounding_box_queue.get_nowait()
                    except Empty:
                    	break
                    except Exception as e:
                        print(f"{e} Error by bounding_box_queue of def 'capture_and_detect_edges'")

                bounding_box_queue.put((objectsboundingboxes, image,out_detect_list))   # 为了检测红绿灯的临时操作。


                # FPS计算
                frame_end_time = time.time()
                #print("video time:",frame_end_time - frame_start_time)
                fps = 1 / (frame_end_time - frame_start_time)
                print("fps_det",fps)
                cv2.putText(output_img2, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                # qt_img2 = convert_cv_to_qt(output_img2)
                self.output_img_queue.put(output_img2)
                # self.signal_pre2.emit(qt_img2)
                # cv2.imshow('Experiment Group', output_img2)

                frame_start_time = time.time()
               

            if cv2.waitKey(1) == ord('q'):
                cv2.destroyAllWindows()
                break
