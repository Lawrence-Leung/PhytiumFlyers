from PySide6.QtWidgets import QApplication, QMainWindow, QFileDialog, QLabel,QListWidget,QMenu,QMessageBox,QDialog, QVBoxLayout,QComboBox, QPushButton
from PySide6.QtCore import QPoint, QTimer, QThread, Signal, Qt
from PySide6.QtGui import QImage, QPixmap, QColor
from ui.test_ui import Ui_MainWindow
from ui.CustomMessageBox import MessageBox
from utils.capnums import Camera
from classes.stairsDetector import StairsDetector
from classes.stairDetecor1 import StairsDetector
from UIFunctions import *
import sys
import cv2
import os
import time
import json
from classes import yolov5
from classes.coordination import LittleMap
from classes import yolov5v8
# from classes import gpsdecode
from scipy.interpolate import UnivariateSpline
from scipy.signal import find_peaks
import matplotlib.pyplot as plt
from classes.stairDetecor1 import LineDetector
import numpy as np
# from drivers.rpmsg import RPMsg
# from drivers.rpmsg import *
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
#                 # formatted_result=tuple(f'{value:.2f}' for value in result)
#                 formatted_result=tuple(round(value,2) for value in result)
#                 print("formatted data: ", formatted_result)
#                 self.gps_update.emit(str(formatted_result))
#                 formatted_address=gpsdecode.decode_address(result[0],result[1])
#                 print("地理信息：",formatted_address)

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
    stair_detected = Signal(np.ndarray)  # Signal for emitting stair detection results
    stair_status_msg = Signal(str)
    stairs_changed = Signal(str)
    fps1_update = Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.running = False
        self.linedetector = LineDetector()
        self.stairs_detector = StairsDetector() 

    def run(self):
        self.running = True
        current_script_path = os.path.abspath(__file__)
        models_dir = os.path.join(os.path.dirname(current_script_path), "models")#从子目录读取
        stairsmodel_path = os.path.join(models_dir, "stairs_yolov8n.onnx")
        stairsobject_detector = yolov5v8.YOLOV5V8(stairsmodel_path, isType='TEST')
        stair_dir=os.path.join(os.path.dirname(current_script_path), "video")
        video_path = os.path.join(stair_dir, "stair2.mp4")
        self.staircap=cv2.VideoCapture(video_path)
        while self.running:
            
            ret, init_stair = self.staircap.read()
            if not ret:
                self.stop()
                return

            stairs_boxes, _ = stairsobject_detector.inference(init_stair)
            stair_counted,stair_num ,fps1= self.stairs_detector.detect_stairs(init_stair, stairs_boxes)
            
            if stair_num !=0:
                self.stair_status_msg.emit(f"Watching out {stair_num} stairs!")

            self.stair_detected.emit(stair_counted)
            self.stairs_changed.emit(f"{stair_num}")
            self.fps1_update.emit(f"{fps1:.2f}")

            # Adjust sleep time as needed
            self.sleep(0.1)

    def toAbsolutePath(self,current_script_path, filepath):
        current_dir = os.path.dirname(os.path.abspath(current_script_path))  # 获取当前脚本的绝对路径
        relative_path = os.path.join(current_dir, filepath)
        return relative_path

    def stop(self):
        self.running = False



class VideoDetectionThread(QThread):
    frame_processed = Signal(QImage)
    fps_update = Signal(str)
    map_processed = Signal(QImage)
    
    def __init__(self, parent=None):
        super().__init__(parent)
        # self.args = get_cfg(cfg, overrides)
        # project = self.args.project or Path(SETTINGS['runs_dir']) / self.args.task
        # name = f'{self.args.mode}'
        # self.save_dir = increment_path(Path(project) / name, exist_ok=self.args.exist_ok)
        #文件路径
        current_script_path = os.path.abspath(__file__)
        models_dir = os.path.join(os.path.dirname(current_script_path), "models")#从子目录读取
        detectmodel_path = os.path.join(models_dir,"yolov5n_lite.onnx") #"haizhuv8nint8.onnx")
        self.detectmodel_path = detectmodel_path
        self.running = False
        self.map=LittleMap()
        #input source
        self.source = '' 
        self.stop_dtc = False            # Termination detection
        self.continue_dtc = True         # pause   
        self.save_res = False            # Save test results
        self.save_txt = False            # save label(txt) file

    def run(self):
        self.running = True
        # 初始化视频读取
        # current_script_path = os.path.abspath(__file__)
        # stair_dir=os.path.join(os.path.dirname(current_script_path), "video")
        # video_path = os.path.join(stair_dir, "walking.mp4")
        print(type(self.source))
        cap = cv2.VideoCapture(self.source )#从本地文件读取
        if not cap.isOpened():
            print("Error: Unable to open video file.")
            return

        # 初始化模型
        object_detector = yolov5.YOLOV5(self.detectmodel_path)

        # 记录上次检测的时间戳
        last_detection_time = time.time()
        print("Starting detection")
        while self.running:
            frame_time = time.time()

            try:
                # 从视频中读取帧
                ret, output_img = cap.read()
            except:
                continue

            if ret:
                # 判断是否到达检测时间
                if frame_time - last_detection_time >= 1:
                    
                    objoutput, output_img = object_detector.inference(output_img)  # 进行目标检测
                    objbox = yolov5.filterBox(objoutput, 0.5, 0.5)
                    objects=object_detector.draw(output_img, objbox)
                    self.map.emit_background_changed_signal(objects)
                   
                    
                    # 更新上次检测的时间戳
                    last_detection_time = frame_time
                # objoutput, output_img = object_detector.inference(output_img)  # 进行目标检测
                # objbox = yolov5.filterBox(objoutput, 0.5, 0.5)
                # objects=object_detector.draw(output_img, objbox)
                # self.map.emit_background_changed_signal(objects)
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
    main2detect_begin_sgl = Signal()#signal to control thread of detection
    status_msg=Signal(str)
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
        self.stair_thread = StairDetectionThread()
        
        self.stair_thread.stair_detected.connect(self.update_stair_video)
        self.stair_thread.stair_status_msg.connect(lambda x: self.status_bar.setText(x))
        self.stair_thread.stairs_changed.connect(lambda x: self.stair_label.setText(x))
        self.stair_thread.fps1_update.connect(lambda x: self.Target_num_3.setText(x))
        self.stair_thread.stairs_changed.connect(lambda x: self.stair_label_3.setText(x))
        self.stair_thread.fps1_update.connect(lambda x: self.Target_num.setText(x))
        self.object_thread = VideoDetectionThread()
        self.status_msg.connect(lambda x: self.status_bar.setText(x))
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
        if self.object_thread.source == '':
            self.status_msg.emit('Please select a video source before starting detection...')
            self.run_button.setChecked(False)
        else:
            self.stair_thread.start()
            self.object_thread.start()
            # self.slave_thread.start()

        


    def stop_detection(self):
        self.stair_thread.stop()
        self.object_thread.stop()
        # self.slave_thread.stop()
        self.run_button.setChecked(False)
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
#open local file
    def open_src_file(self):
        self.stop_detection()  
        config_file = 'config/fold.json'    
        config = json.load(open(config_file, 'r', encoding='utf-8'))
        open_fold = config['open_fold']     
        if not os.path.exists(open_fold):
            open_fold = os.getcwd()
        name, _ = QFileDialog.getOpenFileName(self, 'Video/image', open_fold, "Pic File(*.mp4 *.mkv *.avi *.flv *.jpg *.png)")
        if name:
            self.object_thread.source = name
            print("source:", self.object_thread.source)
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
                self.object_thread.source = int(selected_camera)
                self.status_msg.emit('Loading camera: {}'.format(selected_camera))
            print("sourse:",self.object_thread.source)

        except Exception as e:
            self.status_msg.emit(str(e))

    

if __name__ == "__main__":
    app = QApplication(sys.argv)
    Home = MainWindow()
    Home.show()
    sys.exit(app.exec())
