from PySide6.QtWidgets import QApplication, QMainWindow, QFileDialog, QLabel,QListWidget
from PySide6.QtGui import QImage, QPixmap, QColor
from PySide6.QtCore import QObject, QTimer, QThread, Signal, Qt
from ui.test_ui import Ui_MainWindow
from classes.stairsDetector import StairsDetector
# from classes.stairDetecor1 import StairsDetector
from UIFunctions import *
import sys
import cv2
import time
from classes import yolov5
from classes.coordination import LittleMap
from classes import yolov5v8
from scipy.interpolate import UnivariateSpline
from scipy.signal import find_peaks
import matplotlib.pyplot as plt
from classes.stairDetecor1 import LineDetector
# from drivers.rpmsg import RPMsg
# from drivers.rpmsg import *
class OutputWidget():
    def __init__(self, log_list):
        self.log_list = log_list

    def write(self, text):
        self.log_list.addItem(text.strip())
        # self.log_list.scrollToBottom()

    def flush(self):
        pass  # 在此方法中不做任何操作，仅用于满足 Python 对标准输出对象的期望
# class SlaveDeviceThread(QThread):
#     gps_update=Signal(str)
#     def __init__(self, parent=None):
#         super().__init__(parent)
#         self.running = False
#         self.rpmsg0=RPMsg("/dev/rpmsg_ctrl_lawrence_10",
#                    "/dev/rpmsg_lawrence_10",
#                    "hello", 11, 0)
#         self.rpmsg0.openCtrlDevice()
#         self.rpmsg1=RPMsg("/dev/rpmsg_ctrl_lawrence_20",
#                    "/dev/rpmsg_lawrence_20",
#                    "world", 22, 0)
#         self.rpmsg1.openCtrlDevice()
#         self.rpmsg0.createEndpoint()
#         self.rpmsg1.createEndpoint()
#         self.rpmsg0.openEptDevice()
#         self.rpmsg1.openEptDevice()
#         self.count = 15

#         self.rpmsg0data = None
#         self.rpmsg1data = None
#     def run(self):
#         self.running=True
#         while self.running:
#             # self.frame_map.emit(qt_img)
#             #GPS
#             self.rpmsg1.writeEptDevice()
#             print("[OPENAMP] WAIT data from GPS")
#             self.rpmsg1.pollEptDeviceWithReadEvent()
#             time.sleep(1)
#             print("[OPENAMP] read data from GPS")
#             self.rpmsg1data = self.rpmsg1.readEptDevice()
#             result = slaveGPSAnalysis(self.rpmsg1data)
#             if result:
#                 print("[OPENAMP] GPS received data: ", result)
#                 self.gps_update.emit(str(result))
#         self.rpmsg0.closeEptDevice()
#         self.rpmsg0.closeCtrlDevice()
#         self.rpmsg1.closeEptDevice()
#         self.rpmsg1.closeCtrlDevice()

#     def stop(self):
#         self.running = False
#         self.rpmsg0.closeEptDevice()
#         self.rpmsg0.closeCtrlDevice()
#         self.rpmsg1.closeEptDevice()
#         self.rpmsg1.closeCtrlDevice()


class StairDetectionThread(QThread):
    stair_detected = Signal(QImage)  # Signal for emitting stair detection results
    stair_status_msg=Signal(str)
    stairs_changed=Signal(str)
    fps1_update=Signal(str)
    def __init__(self,  parent=None):
        super().__init__(parent)
        self.running = False

    def run(self):
        self.running = True
        # Initialize stair detector
        self.staircap=cv2.VideoCapture("/home/jimkwokying/Videos/stair2.mp4")
        ret,init_stair = self.staircap.read()
        stair_detector = StairsDetector(init_stair)
        # stair_detector.load_model(self.lanemodel_path)

        while self.running:
            # Perform stair detection on current frame
            res,frame = self.staircap.read() 
            stair_counted,stair_num ,fps1= stair_detector.detect_stairs(frame)
            if stair_num !=0:
                self.stair_status_msg.emit(f"Watching out {stair_num} stairs!")

            # qt_img = convert_cv_to_qt(stair_counted)
            self.stair_detected.emit(stair_counted)
            self.stairs_changed.emit(f"{stair_num}")
            self.fps1_update.emit(f"{fps1:.2f}")
            time.sleep(0.1)  # Adjust this sleep time as needed

    def stop(self):
        self.running = False

# class StairDetectionThread(QThread):
#     stair_detected = Signal(np.ndarray)  # Signal for emitting stair detection results
#     stair_status_msg = Signal(str)
#     stairs_changed = Signal(str)
#     fps1_update = Signal(str)

#     def __init__(self, parent=None):
#         super().__init__(parent)
#         self.running = False
#         self.linedetector = LineDetector()
#         self.stairs_detector = StairsDetector() 

#     def run(self):
#         self.running = True
#         current_script_path = os.path.abspath(__file__)
#         # stairsmodel_path = self.toAbsolutePath(current_script_path, "../models/stairs_yolov8n.onnx")#从上一级目录读取
#         models_dir = os.path.join(os.path.dirname(current_script_path), "models")#从子目录读取
#         stairsmodel_path = os.path.join(models_dir, "stairs_yolov8n.onnx")
#         stairsobject_detector = yolov5v8.YOLOV5V8(stairsmodel_path, isType='TEST')
#         self.staircap=cv2.VideoCapture("/home/jimkwokying/Videos/stair2.mp4")
#         while self.running:
            
#             ret, init_stair = self.staircap.read()
#             if not ret:
#                 self.stop()
#                 return

#             stairs_boxes, _ = stairsobject_detector.inference(init_stair)
#             stair_counted,stair_num ,fps1= self.stairs_detector.detect_stairs(init_stair, stairs_boxes)
            
#             if stair_num !=0:
#                 self.stair_status_msg.emit(f"Watching out {stair_num} stairs!")

#             self.stair_detected.emit(stair_counted)
#             self.stairs_changed.emit(f"{stair_num}")
#             self.fps1_update.emit(f"{fps1:.2f}")

#             # Adjust sleep time as needed
#             self.sleep(0.1)

#     def toAbsolutePath(self,current_script_path, filepath):
#         current_dir = os.path.dirname(os.path.abspath(current_script_path))  # 获取当前脚本的绝对路径
#         relative_path = os.path.join(current_dir, filepath)
#         return relative_path

#     def stop(self):
#         self.running = False



class VideoDetectionThread(QThread):
    frame_processed = Signal(QImage)
    fps_update = Signal(str)
    map_processed = Signal(QImage)
    def __init__(self, detectmodel_path, parent=None):
        super().__init__(parent)
        self.detectmodel_path = detectmodel_path
        self.running = False
        self.map=LittleMap()

    def run(self):
        self.running = True
        # 初始化视频读取
        cap = cv2.VideoCapture("/home/jimkwokying/Videos/walking.mp4")
        if not cap.isOpened():
            print("Error: Unable to open video file.")
            return

        # 初始化模型
        object_detector = yolov5.YOLOV5(self.detectmodel_path)

        # 记录上次检测的时间戳
        last_detection_time = time.time()

        while self.running:
            frame_time = time.time()

            try:
                # 从视频中读取帧
                ret, output_img = cap.read()
            except:
                continue

            if ret:
                # 判断是否到达检测时间
                if frame_time - last_detection_time >= 3:
                    print("Starting detection")
                    objoutput, output_img = object_detector.inference(output_img)  # 进行目标检测
                    objbox = yolov5.filterBox(objoutput, 0.5, 0.5)
                    objects=object_detector.draw(output_img, objbox)
                    self.map.emit_background_changed_signal(objects)
                   
                    
                    # 更新上次检测的时间戳
                    last_detection_time = frame_time

                # 计算FPS
                end_time = time.time()
                fps = 1 / (end_time - frame_time)

                # 发送信号
                self.fps_update.emit(f"{fps:.2f}")

                # 将 OpenCV 图像转换为 Qt 图像
                qt_img = cv2.cvtColor(output_img, cv2.COLOR_BGR2RGB)
                h, w, ch = qt_img.shape
                bytes_per_line = ch * w
                qt_img = QImage(qt_img.data, w, h, bytes_per_line, QImage.Format_RGB888)

                # 发送信号
                self.frame_processed.emit(qt_img)

            else:
                break

        # 释放资源
        cap.release()

    def stop(self):
        self.running = False


class MainWindow(Ui_MainWindow, QMainWindow):
    def __init__(self, parent=None):
        lanemodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/lane_int8.onnx"
        detectmodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/yolov5n_lite.onnx"
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
        # 发送终端输出结果到log_list控件
        # self.output_widget = OutputWidget(self.log_list)
        # sys.stdout = self.output_widget
        self.stair_thread = StairDetectionThread()
        self.stair_thread.stair_detected.connect(self.update_stair_video)
        self.stair_thread.stair_status_msg.connect(lambda x: self.status_bar.setText(x))
        self.stair_thread.stairs_changed.connect(lambda x: self.stair_label.setText(x))
        self.stair_thread.fps1_update.connect(lambda x: self.Target_num_3.setText(x))
        self.stair_thread.stairs_changed.connect(lambda x: self.stair_label_3.setText(x))
        self.stair_thread.fps1_update.connect(lambda x: self.Target_num.setText(x))
        self.object_thread = VideoDetectionThread(detectmodel_path)
        self.object_thread.moveToThread(self.object_thread)
        self.object_thread.frame_processed.connect(self.update_pre_video)
        self.object_thread.fps_update.connect(lambda x: self.fps_label.setText(x))
        self.object_thread.fps_update.connect(lambda x: self.fps_label_3.setText(x))
        self.object_thread.map.map_update_notifier.background_changed.connect(self.update_res_video)
        # slave线程
        # self.slave_thread=SlaveDeviceThread()
        # self.slave_thread.gps_update.connect(lambda x: self.gps_label.setText(x))
        #启动/暂停
        self.run_button.clicked.connect(self.start_detection)
        self.stop_button.clicked.connect(self.stop_detection)

    def start_detection(self):
        self.stair_thread.start()
        self.object_thread.start()
        # self.slave_thread.start()


    def stop_detection(self):
        self.stair_thread.stop()
        self.object_thread.stop()
        # self.slave_thread.stop()

    def update_res_video(self, qt_img):
        label_width = self.res_video.width()
        label_height = self.res_video.height()

    # 调整图像大小以适应 QLabel
        scaled_img = qt_img.scaled(label_width, label_height)
        self.res_video.setPixmap(QPixmap.fromImage(scaled_img))

    def update_pre_video(self, qt_img):
        self.pre_video.setPixmap(QPixmap.fromImage(qt_img))
    def update_stair_video(self, raw_img):
        label_width = self.res_video.width()
        label_height = self.res_video.height()

    # 调整图像大小以适应 QLabel
        scaled_img = raw_img.scaled(label_width, label_height)
        self.pre_video_3.setPixmap(QPixmap.fromImage(scaled_img))

    def mousePressEvent(self, event):
        p = event.globalPosition()
        globalPos = p.toPoint()
        self.dragPos = globalPos



    def convert_cv_to_qt(self,cv_img):
        height, width, channel = cv_img.shape
        bytes_per_line = 3 * width
        qt_img = QImage(cv2.cvtColor(cv_img, cv2.COLOR_BGR2RGB), width, height, bytes_per_line, QImage.Format_RGB888)
        return qt_img


if __name__ == "__main__":
    app = QApplication(sys.argv)
    Home = MainWindow()
    Home.show()
    sys.exit(app.exec())
