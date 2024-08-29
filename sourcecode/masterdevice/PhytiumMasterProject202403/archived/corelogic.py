# corelogic.py
# 核心逻辑
# by Lawrence Leung 2024

# ---------------------------------------------------------------- 包管理
import os
import multiprocessing
import cv2
from PyQt5.QtGui import QImage, QPixmap
from PyQt5.QtWidgets import QLabel, QVBoxLayout, QWidget
import time  # for debug only
from classes import corelogicdef    # 自己的核心逻辑框架类

# debug 使用的窗口
class VideoWidget(QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("OpenCV Video Stream")
        self.layout = QVBoxLayout()
        self.setLayout(self.layout)

        self.label = QLabel()
        self.layout.addWidget(self.label)

        self.queue = multiprocessing.Queue()    # 创建队列用于传递图像数据

        # 创建定时器，用于图像处理
        #self.timer = QTimer(self)
        #self.timer.timeout.connect(self.updateFrame)
        #self.timer.start(1000 // 20)    # 每秒20帧

    # 更新一帧图像到Qt上
    def updateFrame(self, frame):
        #frame = self.queue.get()
        # OpenCV图像是BGR格式，需要转换为RGB格式
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        # 将OpenCV图像转换为Qt图像
        h, w, ch = rgb_frame.shape
        bytes_per_line = ch * w
        qt_image = QImage(rgb_frame.data, w, h, bytes_per_line, QImage.Format_RGB888)
        pixmap = QPixmap.fromImage(qt_image)
        # 更新标签上的图像
        self.label.setPixmap(pixmap)
        print("[VIDEOWIDGET] 创建图像中")


# ------------------------------------------------------------------ 主进程
# 主程序逻辑 todo: 需要将这个主程序移植到Qt框架显示界面中去
if __name__ == '__main__':
    print('\033[1;35m' + '[CORELOGIC] --- 2024 飞腾风驰队 Phytium Flyers ---' + '\033[1;0m')
    # step 0 初始化所需要的框架，以供后续使用
    # 0.1 模型路径与属性引入
    current_script_path = os.path.abspath(__file__)
    detectmodel_path = corelogicdef.toAbsolutePath(current_script_path, '../models/yolov8n-oiv7-int8.onnx')


    # 0.4 创建窗口 todo: 创建OpenCV2窗口
    #qtapp = QApplication(sys.argv)
    #window = VideoWidget()
    #window.show()

    # step 1 创建子进程控制器
    corecontroller = corelogicdef.CoreProcessController()

    # step 2 创建管道，注意这个Pipe返回两个参数，第一个是管道的起点，第二个是管道的终点，两条管道都可以双向沟通
    pipe1start, pipe1end = multiprocessing.Pipe()
    pipe2start, pipe2end = multiprocessing.Pipe()

    # step 3 创建所有子进程的实例，并将管道端传递给它们
    taskprocess1 = corelogicdef.WorkerProcess("OTHERS",
                                      corelogicdef.importDataFunc("OTHERS-TEST"),
                                      corelogicdef.returnDataFunc,
                                      pipe1start,
                                      pipe2end)
    taskprocess2 = corelogicdef.WorkerProcess("OTHERS",
                                      corelogicdef.importDataFunc("OTHERS-TEST"),
                                      corelogicdef.returnDataFunc,
                                      pipe2start,
                                      pipe1end)

    # step 4 将所有子进程实例添加到进程列表中，然后开始所有进程
    processlist = [taskprocess1, taskprocess2]
    corecontroller.startAll(processlist)

    # step 5 模拟一段时间后停止所有进程
    time.sleep(5)   # 模拟让这两个进程运行5秒
    corecontroller.stopAll()    # 停止所有进程
    print('\033[1;35m' + '[CORELOGIC] --- 2024 飞腾风驰队 Phytium Flyers ---' + '\033[1;0m')

    #sys.exit(qtapp.exec_())

