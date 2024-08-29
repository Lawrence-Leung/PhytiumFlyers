from PySide6.QtWidgets import QApplication, QMainWindow, QFileDialog
from PySide6.QtCore import Signal, Slot,QCoreApplication,Qt,QThread
from PySide6.QtGui import QImage, QPixmap, QColor
from ui.test_ui import Ui_MainWindow
from UIFunctions import UIFuncitons
from cameraSource import CameraSelectionDialog
from videoThread0 import VideoDetectionSystem
from slaveThread1 import SlaveDeviceThread
from utils.capnums import Camera
import cv2
import sys
import os
import threading
import json
import psutil

def toAbsolutePath(current_script_path, filepath):
    current_dir = os.path.dirname(os.path.abspath(current_script_path))  
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
  
class MainWindow(Ui_MainWindow, QMainWindow):
    main2detect_begin_sgl = Signal()#signal to control thread of detection
    status_msg=Signal(str)
    cpusig=Signal(str)
    cputotal=Signal(str)
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
        
################################################################以上内容每次移植都不作修改,以下设置线程/进程类初始化以及槽函数连接
        self.video_detection_system = VideoDetectionSystem()
        if QThread.currentThread() is not QCoreApplication.instance().thread():
            raise RuntimeError("Attempted to create VideoDetectionSystem outside of the main thread.")
        self.video_detection_system.moveToThread(QCoreApplication.instance().thread())
        self.video_detection_system.signal_pre1.connect(self.update_pre1_image)
        self.video_detection_system.signal_pre2.connect(self.update_pre2_image)
        self.video_detection_system.signal_pre3.connect(self.update_map)
        self.video_detection_system.sysT.connect(lambda x:self.time0_label.setText(f"{x}s"))

        self.run_button.clicked.connect(self.start_detection)
        self.status_msg.connect(lambda x: self.status_bar.setText(x))
        self.stop_button.clicked.connect(self.stop_detection)
        
        self.cputotal.connect(lambda x:self.cputotal_label.setText(f"整体CPU占用率：{x}"))
        self.cpusig.connect(lambda x:self.cpu_label.setText(x))
        ################################################################
        self.slave_thread=SlaveDeviceThread()
        self.slave_thread.moveToThread(QCoreApplication.instance().thread())
        self.slave_thread.gps_update.connect(lambda x: self.gps_label.setText(x))
        self.slave_thread.loc_detected.connect(lambda x: self.label_3.setText(x))
        self.slave_thread.down_update.connect(lambda x: self.status_label.setText(x))
        self.slave_thread.cam_update.connect(lambda x: self.cam_label.setText(x))
        self.slave_thread.dht_update.connect(lambda x: self.hum_label.setText(x))
        self.slave_thread.mtosT.connect(lambda x:self.time1_label.setText(f"{x}ms"))
        self.video_detection_system.sysT.connect(self.source_using)
        self.video_detection_system.broadcast_signal.connect(self.slave_thread.handle_broadcast)

    @Slot(QImage)
    def update_pre1_image(self, image):
        label_width = self.res_video.width()
        label_height = self.res_video.height()

    # 调整图像大小以适应 QLabel
        scaled_img = image.scaled(label_width, label_height)
        self.res_video_2.setPixmap(QPixmap.fromImage(scaled_img))
        
    @Slot(QImage)
    def update_pre2_image(self, image):
        label_width = self.pre_video.width()
        label_height = self.pre_video.height()
        scaled_img = image.scaled(label_width, label_height)
        self.pre_video.setPixmap(QPixmap.fromImage(scaled_img))
        self.pre_video_2.setPixmap(QPixmap.fromImage(scaled_img))

    def update_map(self, image):
        label_width = self.pre_video.width()
        label_height = self.pre_video.height()
        scaled_img = image.scaled(label_width, label_height)
        self.res_video.setPixmap(QPixmap.fromImage(scaled_img))
    
    def open_src_file(self):
        #为防止切换源后内存冲突的操作，清空上一次视频源的结果
        try:
            while True:
                self.video_detection_system.image_queue.get_nowait()
                self.video_detection_system.bounding_box_queue.get_nowait()
                self.video_detection_system.output_img_queue.get_nowait()
                self.video_detection_system.framecopy_queue.get_nowait()
                self.video_detection_system.map_queue.get_nowait()
        except Exception as e:
            pass 
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
        #为防止切换源后内存冲突的操作，清空上一次视频源的结果
        try:
            while True:
                self.video_detection_system.image_queue.get_nowait()
                self.video_detection_system.bounding_box_queue.get_nowait()
                self.video_detection_system.output_img_queue.get_nowait()
                self.video_detection_system.framecopy_queue.get_nowait()
                self.video_detection_system.map_queue.get_nowait()
        except Exception as e:
            pass
        
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

        

    def start_detection(self):
        if self.video_detection_system.source == '':
            self.status_msg.emit('Please select a video source before starting detection...')
            self.run_button.setChecked(False)
        else:
            self.video_detection_system.start_system()
            self.slave_thread.start_device()

    def stop_detection(self):
        if self.video_detection_system.p1 is not None:
            self.video_detection_system.stop_system()
            self.run_button.setChecked(False)
   
    def mousePressEvent(self, event):
        p = event.globalPosition()
        globalPos = p.toPoint()
        self.dragPos = globalPos
        
    def closeEvent(self,event):
        # 处理程序退出时的逻辑
        if self.video_detection_system.p1 is not None:
            self.slave_thread.stop_device()
            self.video_detection_system.stop_system()

    def source_using(self):
        # 获取每个CPU的使用率
        cpu_usage = psutil.cpu_percent(interval=1, percpu=True)
        cpu_str="("+"%,".join(map(str,cpu_usage))+"%)"
        self.cpusig.emit(cpu_str)
        # 获取系统整体CPU使用率
        total_cpu_usage = psutil.cpu_percent(interval=1)
        self.cputotal.emit(f"{total_cpu_usage}%")

if __name__ == '__main__':
    app = QApplication(sys.argv)
    mainWindow = MainWindow()
    mainWindow.show()
    sys.exit(app.exec())
