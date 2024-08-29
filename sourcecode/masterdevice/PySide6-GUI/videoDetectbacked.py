import cv2
import time
from classes.coordination import LittleMap
from classes.ultrafastLaneDetector import ModelType
from classes import yolov5
from PySide6.QtCore import QThread, Signal, QTimer,Slot
from PySide6.QtGui import QImage, QPixmap
from PySide6.QtWidgets import QApplication, QLabel, QMainWindow, QSplitter
from classes.stairsDetector import StairsDetector
from classes import stairsDetector
# from drivers.rpmsg import RPMsg

class VideoDetection(QThread):
    frame_processed = Signal(QImage)
    fps_update= Signal(str)
    stair_update = Signal(str)
    stair_detected=Signal(QImage)
    gps_update= Signal(str)
    def __init__(self, lanemodel_path, detectmodel_path):
        super().__init__()
        # lanemodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/lane_int8.onnx"
        # detectmodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/yolov5n_lite.onnx"
        # imagePath = "/home/jimkwokying/projectTest/masterdevice/import_img/stair1.jpg"
        # image = cv2.imread(imagePath)
        # self.image = stairsDetector.ResizeImage(image, 640, 640)
        self.image=None
        self.stair=StairsDetector(self.image)
        self.lanemodel_path = lanemodel_path
        self.detectmodel_path = detectmodel_path
        self.model_type = ModelType.CULANE
        self.cap = None
        self.staircap=None
        self.object_detector = None
        self.map=LittleMap()
        

        # self.rpmsg0=RPMsg("/dev/rpmsg_ctrl_lawrence_10",
        #            "/dev/rpmsg_lawrence_10",
        #            "hello", 11, 0)
        # self.rpmsg0.openCtrlDevice()
        # self.rpmsg1=RPMsg("/dev/rpmsg_ctrl_lawrence_20",
        #            "/dev/rpmsg_lawrence_20",
        #            "world", 22, 0)
        # self.rpmsg1.openCtrlDevice()
        # self.rpmsg0.createEndpoint()
        # self.rpmsg1.createEndpoint()
        # self.rpmsg0.openEptDevice()
        # self.rpmsg1.openEptDevice()
        # self.count = 15

        # self.rpmsg0data = None
        # self.rpmsg1data = None

    def initialize_video(self, video_path):
        self.cap = cv2.VideoCapture(video_path)
        print("Initializing")
        # self.staircap=cv2.VideoCapture("/home/jimkwokying/Videos/stair2.mp4")
        return self.cap

    def initialize_models(self):
        # 初始化模型
        self.object_detector = yolov5.YOLOV5(self.detectmodel_path)

    def process_frame(self):
        print("Running")
        frame_time = time.time()
        ret, output_img = self.cap.read()
        print("ret:",ret)
        # ret2, output_stair = self.staircap.read()

        if ret:
            # 目标检测
            objoutput, output_img2 = self.object_detector.inference(output_img)
            # self.image=output_stair

            objbox = yolov5.filterBox(objoutput, 0.5, 0.5)
            self.object_detector.draw(output_img2, objbox)
            self.map.emit_background_changed_signal()
            # 计算FPS
            end_time = time.time()  # 记录帧处理时间
            fps = 1 / (end_time - frame_time)  # 计算FPS
            self.fps_update.emit(f"{fps:.2f}")
            
            # stair_counted=self.stair.detect_stairs(self.image)
            # self.stair_detected.emit(stair_counted)#发送楼梯检测结果
            
            cv2.putText(output_img2, f"FPS: {fps:.2f}", (output_img2.shape[1] - 150, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            
            #GPS
            # self.rpmsg1.writeEptDevice()
            # print("[OPENAMP] WAIT data from GPS")
            # self.rpmsg1.pollEptDeviceWithReadEvent()
            # time.sleep(1)
            # print("[OPENAMP] read data from GPS")
            # self.rpmsg1data = self.rpmsg1.readEptDevice()
            # result = RPMsg.slaveGPSAnalysis(self.rpmsg1data)
            # if result:
            #     print("[OPENAMP] GPS received data: ", result)
            # 将 OpenCV 图像转换为 Qt 图像
            qt_img = cv2.cvtColor(output_img2, cv2.COLOR_BGR2RGB)
            h, w, ch = qt_img.shape
            bytes_per_line = ch * w
            qt_img = QImage(qt_img.data, w, h, bytes_per_line, QImage.Format_RGB888)

            # 发送信号
            self.frame_processed.emit(qt_img)
            # self.frame_map.emit(qt_img)
        # self.rpmsg0.closeEptDevice()
        # self.rpmsg0.closeCtrlDevice()
        # self.rpmsg1.closeEptDevice()
        # self.rpmsg1.closeCtrlDevice()
    def stair_detection(self):
        image=self.image
        stair_output=self.stair.detect_stairs(image)
        
        # cv2.imshow("Stairs Detection",stair_output)
        # cv2.waitKey(10000)
        # 不使用test.py时则注释掉以上两句
    @Slot()
    def start_detection(self):
           # 设置定时器，每隔一段时间处理一帧
        self.timer.timeout.connect(self.process_frame)
        self.timer.start(1)  # 设置每 30 毫秒处理一帧
    def run(self) :
        self.running = True
        self.initialize_video("/home/jimkwokying/Videos/walking.mp4")
        self.initialize_models()
        while self.running :
            self.timer = QTimer()
            self.process_frame()
        
        

    def stop(self):
        self.running = False