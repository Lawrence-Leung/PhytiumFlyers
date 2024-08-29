from PySide6.QtWidgets import QApplication, QMainWindow, QFileDialog, QLabel,QListWidget,QMenu,QMessageBox,QDialog, QVBoxLayout,QComboBox, QPushButton
from PySide6.QtCore import QObject, Signal, Slot,QMetaObject,QCoreApplication,QThread,QEvent
from PySide6.QtGui import QImage, QPixmap, QColor
from multiprocessing import Process, Queue
from classes import yolov5v8, stairsDetector05, crossroadsDetector,gpsdecode
from classes.coordination import LittleMap
from ui.test_ui import Ui_MainWindow
from UIFunctions import *
from utils.capnums import Camera
from drivers import detectResultPackUp
from drivers.speakout import *
from classes.roadDeviationDetector import *
# from drivers.rpmsg import RPMsg
# from drivers.rpmsg import *
import cv2
import time
import sys
import os
import threading
import json
import psutil

def toAbsolutePath(current_script_path, filepath):
    current_dir = os.path.dirname(os.path.abspath(current_script_path))  # 获取当前脚本的绝对路径
    relative_path = os.path.join(current_dir, filepath)
    return relative_path

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
    
class VideoDetectionSystem(QObject):
    signal_pre1 = Signal(QImage)
    signal_pre2 = Signal(QImage)
    signal_pre3=Signal(QImage)
    
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
        self.time0=0
        self.time1=0
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
        # print(self.framecopy_queue.empty())
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


    def capture_and_detect_image(self,image_queue,
                             bounding_box_queue
                             ):
            
            # 注意：在该方法中发送图像给 MainWindow 更新 pre1 控件时，使用 self.signal_pre1.emit(qt_img)
            # 读取楼梯检测器
            stairsnumber_detector = stairsDetector05.StairsDetector()
            crossroad_guider = crossroadsDetector.CrossRoadGuider()
            objdetect_result = None
            detected_image = None

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
                        (objdetect_result, detected_image) = bounding_box_queue.get()   # 获取最新的一个检测结果

                ###################################
                # 正式开始根据检测结果进行推理
                    if objdetect_result is not None:
                        stair_start=time.time()
                        framecopy = stairsnumber_detector.TotalDetection2(framecopy, objdetect_result, 10)
                        stair_end_time = time.time()
                        #print("stair time:",stair_end_time - stair_start)
                        cross_start=time.time()
                        framecopy = crossroad_guider.TotalDetectionAsynchronousAll(detected_image, framecopy, objdetect_result)   # debug only
                        cross_end_time = time.time()
                        #print("cross time:",cross_end_time - cross_start)

                # FPS
                    frame_end_time = time.time()
                
                    fps = 1 / (frame_end_time - frame_start_time)
                    print("FPS_show:",fps)
                    cv2.putText(framecopy, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
                    # qt_img1=convert_cv_to_qt(framecopy)
                    # self.output_img_queue.put(qt_img1)
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
        detectmodel_path = toAbsolutePath(current_script_path, "models/haizhuv8nint8.onnx") # 相对当前脚本位置
        # 目标检测网络
        object_detector = yolov5v8.YOLOV5V8(detectmodel_path, isType='HAIZHU')

        frame_start_time = time.time()  # 事件最初始化

        while True:
            i += 1
            print("响应：", i)
            time.sleep(0.02)
            if not image_queue.empty():
                image = image_queue.get()  # 从队列中获取处理后的帧
                if image is None: break
                # 这里可以添加图像处理逻辑
                ##################################
                ##################################
                # 注意这个参数：i % xxx：手动调参到最优
                if i % 4 == 0:
                    i = 0
                    objectsboundingboxes, output_img2 = object_detector.inference(image)    # ONNX 推理
                    objects=object_detector.drawWithMap(output_img2, objectsboundingboxes)  # 把推理结果放到小地图上
                    self.map_queue.put(objects)

                    # 将推理结果放入队列
                    while not bounding_box_queue.empty():
                        try:
                            bounding_box_queue.get_nowait()
                        except Exception as e:
                            print(f"{e} Error by bounding_box_queue of def 'capture_and_detect_edges'")

                    bounding_box_queue.put((objectsboundingboxes, image))   # 为了检测红绿灯的临时操作。


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
                    self.rpmsg2.writeSpeaker(string1) # 将已有的数据写入到端点里面去  
                else:
                    self.rpmsg2.writeEptDevice()


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
        if not self.t_queue.empty():
            time=self.t_queue.get()
            
            self.mtosT.emit(time)


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
        # 发送终端输出结果到log_list控件
        #self.output_widget = OutputWidget(self.log_list)
        #sys.stdout = self.output_widget
        # print("initial is running in thread:", threading.current_thread().ident)
################################################################以上内容每次移植都不作修改,以下设置线程/进程类初始化以及槽函数连接
        self.run_button.clicked.connect(self.start_detection)
        self.status_msg.connect(lambda x: self.status_bar.setText(x))
        self.stop_button.clicked.connect(self.stop_detection)
        self.cputotal.connect(lambda x:self.cputotal_label.setText(f"整体CPU占用率：{x}"))
        self.cpusig.connect(lambda x:self.cpu_label.setText(x))
        
        
        # self.slave_thread=SlaveDeviceThread()
        # self.slave_thread.moveToThread(QCoreApplication.instance().thread())
        # self.slave_thread.gps_update.connect(lambda x: self.gps_label.setText(x))
        # self.slave_thread.loc_detected.connect(lambda x: self.label_3.setText(x))
        # self.slave_thread.down_update.connect(lambda x: self.status_label.setText(x))
        # self.slave_thread.cam_update.connect(lambda x: self.cam_label.setText(x))
        # self.slave_thread.dht_update.connect(lambda x: self.hum_label.setText(x))
        # self.slave_thread.mtosT.connect(lambda x:self.time1_label.setText(f"{x}ms"))
        # self.slave_thread.dht_update.connect(self.source_using)

        self.video_detection_system = VideoDetectionSystem()
        if QThread.currentThread() is not QCoreApplication.instance().thread():
            raise RuntimeError("Attempted to create VideoDetectionSystem outside of the main thread.")
        self.video_detection_system.moveToThread(QCoreApplication.instance().thread())
        self.video_detection_system.signal_pre1.connect(self.update_pre1_image)
        self.video_detection_system.signal_pre2.connect(self.update_pre2_image)
        self.video_detection_system.signal_pre3.connect(self.update_map)
        self.video_detection_system.sysT.connect(lambda x:self.time0_label.setText(f"{x}s"))
        # self.video_detection_system.sysT.connect(self.source_using)



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


        
    @Slot(QImage)
    def update_pre1_image(self, image):
        print("pre1 image ok")
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

    def start_detection(self):
        if self.video_detection_system.source == '':
            self.status_msg.emit('Please select a video source before starting detection...')
            self.run_button.setChecked(False)
        else:
            # self.slave_thread.start_device()
            self.video_detection_system.start_system()
            

    def stop_detection(self):
        if self.video_detection_system.p1 is not None:
            # self.slave_thread.stop_device()
            self.video_detection_system.stop_system()
        
        self.run_button.setChecked(False)
   
    def mousePressEvent(self, event):
        p = event.globalPosition()
        globalPos = p.toPoint()
        self.dragPos = globalPos

if __name__ == '__main__':
    app = QApplication(sys.argv)
    mainWindow = MainWindow()
    # print("main is running in thread:", threading.current_thread().ident)    
    mainWindow.show()
    sys.exit(app.exec())
