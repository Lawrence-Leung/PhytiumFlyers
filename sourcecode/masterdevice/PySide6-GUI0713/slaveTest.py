import cv2
import time
import os
import sys
import numpy as np
from PySide6.QtCore import QCoreApplication, QThread, Signal, Slot ,QObject
from videoThread import VideoDetectionSystem
import cv2
import time
import sys
import os

class SlaveTest(QObject):
    gpstest=Signal(tuple)
    def __init__(self, parent=None):
        super().__init__(parent)
        self.video_detection_system = VideoDetectionSystem()
        self.video_detection_system.broadcast_signal.connect(self.handle_broadcast)
        print("Test Started")
    @Slot(str)
    def handle_broadcast(self,content):
        print("接收播报:",content)
        gps=(113.32405139420317, 23.096203484758096)
        self.gpstest.emit(gps)

