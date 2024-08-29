
from PySide6.QtWidgets import QApplication, QMainWindow, QFileDialog, QMenu,QLabel
from PySide6.QtGui import QImage, QPixmap, QColor
from PySide6.QtCore import QTimer, QThread, Signal, QObject, QPoint, Qt

from UIFunctions import UIFuncitons
from ui.CustomMessageBox import MessageBox
from ui.test_ui import Ui_MainWindow

from collections import defaultdict
from pathlib import Path
from utils.capnums import Camera
from utils.rtsp_win import Window
import numpy as np
import time
import json
#import torch
import sys
import cv2
import os
from videoDetection import VideoDetection
from classes.stairsDetector import StairsDetector
from classes import stairsDetector



class MainWindow(Ui_MainWindow,QMainWindow):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setupUi(self)
        self.setAttribute(Qt.WA_TranslucentBackground)  # rounded transparent
        self.setWindowFlags(Qt.FramelessWindowHint)  # Set window flag: hide window borders
        self.ToggleBotton.clicked.connect(lambda: UIFuncitons.toggleMenu(self, True))   # left navigation button
        self.settings_button.clicked.connect(lambda: UIFuncitons.settingBox(self, True))   # top right settings button

        # 其他初始化代码
        lanemodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/lane_int8.onnx"
        detectmodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/yolov5n.onnx"
        imagePath = "/home/jimkwokying/projectTest/masterdevice/import_img/stair1.jpg"
        image = cv2.imread(imagePath)
        image = stairsDetector.ResizeImage(image, 640, 640)
        self.stair_detection = StairsDetector(image)
        self.stair_detection.detect_stairs(image)
        
        self.video_detection = VideoDetection(lanemodel_path, detectmodel_path)
        self.video_detection.initialize_video("/home/jimkwokying/Videos/walking.mp4")
        self.video_detection.initialize_models()
        self.video_detection.start_detection()
        # self.video_detection.stair_detection()
        self.video_detection.frame_processed.connect(self.update_pre_video)
        self.video_detection.fps_update.connect(lambda x: self.fps_label.setText(x))
        # self.video_detection.stair_update.connect(lambda x: self.stair_label.setText(x))
        self.video_detection.stair_detected.connect(self.update_res_video)
        self.video_detection.stair.stair_update_notifier.stairs_changed.connect(lambda x: self.stair_label.setText(x))
        self.video_detection.gps_update.connect(lambda x: self.gps_label.setText(x))
        self.video_detection.stair.stair_update_notifier.stair_status_msg.connect(lambda x: self.status_bar.setText(x))
        
        # self.video_detection.object_detector.map.background_changed.connect(self.update_res_video)
        # self.video_detection.map.map_update_notifier.background_changed.connect(self.update_res_video)
    def test(self):
        print("signal ok")
    def convert_cv_to_qt(self, cv_img):
        height, width, channel = cv_img.shape
        bytes_per_line = 3 * width
        # qt_img = QImage(cv_img.data, width, height, bytes_per_line, QImage.Format_RGB888)
        qt_img = QImage(cv2.cvtColor(cv_img, cv2.COLOR_BGR2RGB), width, height, bytes_per_line, QImage.Format_RGB888)
        return qt_img   
    def update_res_video(self, stair_counted):
        # 将 OpenCV 图像转换为 Qt 图像
        # map_img = self.convert_cv_to_qt(background_img)

        # 将图像显示在 QLabel 控件上
        self.res_video.setPixmap(QPixmap.fromImage(stair_counted))
    def update_pre_video(self, qt_img):
        self.pre_video.setPixmap(QPixmap.fromImage(qt_img))

if __name__ == "__main__":
    app = QApplication(sys.argv)
    Home = MainWindow()
    Home.show()
    sys.exit(app.exec()) 
