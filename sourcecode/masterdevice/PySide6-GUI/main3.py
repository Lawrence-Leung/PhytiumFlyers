from ultralytics.yolo.engine.predictor import BasePredictor
from ultralytics.yolo.engine.results import Results
from ultralytics.yolo.utils import DEFAULT_CFG, LOGGER, SETTINGS, callbacks, ops
from ultralytics.yolo.utils.plotting import Annotator, colors, save_one_box
from ultralytics.yolo.utils.torch_utils import smart_inference_mode
from ultralytics.yolo.utils.files import increment_path
from ultralytics.yolo.utils.checks import check_imshow
from ultralytics.yolo.cfg import get_cfg
from PySide6.QtWidgets import QApplication, QMainWindow, QFileDialog, QMenu,QLabel
from PySide6.QtGui import QImage, QPixmap, QColor
from PySide6.QtCore import QTimer, QThread, Signal, QObject, QPoint, Qt
from ui.CustomMessageBox import MessageBox
from ui.test_ui import Ui_MainWindow
from UIFunctions import *
from collections import defaultdict
from pathlib import Path
from utils.capnums import Camera
from utils.rtsp_win import Window
import numpy as np
import time
import json
import torch
import sys
import cv2
import os
from videoDetectbacked import VideoDetection
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
        UIFuncitons.uiDefinitions(self)
        # Show module shadows
        UIFuncitons.shadow_style(self, self.Class_QF, QColor(94,96,92))
        UIFuncitons.shadow_style(self, self.Target_QF, QColor(186,189,182))
        UIFuncitons.shadow_style(self, self.Fps_QF, QColor(211,215,207))
        UIFuncitons.shadow_style(self, self.Model_QF, QColor(136,138,133))
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
        self.video_detection.stair_detected.connect(self.update_res_video)
        self.video_detection.stair.stair_update_notifier.stairs_changed.connect(lambda x: self.stair_label.setText(x))
        self.video_detection.stair.stair_update_notifier.stair_status_msg.connect(lambda x: self.status_bar.setText(x))
        #select source from file
        self.src_file_button.clicked.connect(self.open_src_file)

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

    def open_src_file(self):
        config_file = 'config/fold.json'    
        config = json.load(open(config_file, 'r', encoding='utf-8'))
        open_fold = config['open_fold']     
        if not os.path.exists(open_fold):
            open_fold = os.getcwd()
        name, _ = QFileDialog.getOpenFileName(self, 'Video/image', open_fold, "Pic File(*.mp4 *.mkv *.avi *.flv *.jpg *.png)")
        if name:
            self.yolo_predict.source = name
            self.show_status('Load File：{}'.format(os.path.basename(name))) 
            config['open_fold'] = os.path.dirname(name)
            config_json = json.dumps(config, ensure_ascii=False, indent=2)  
            with open(config_file, 'w', encoding='utf-8') as f:
                f.write(config_json)
            self.stop() 
if __name__ == "__main__":
    app = QApplication(sys.argv)
    Home = MainWindow()
    Home.show()
    sys.exit(app.exec()) 