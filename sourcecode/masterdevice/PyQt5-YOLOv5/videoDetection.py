import cv2
import time
from classes.ultrafastLaneDetector import ModelType
from classes import yolov5

class VideoDetection:
    def __init__(self, lanemodel_path, detectmodel_path):
        self.lanemodel_path = lanemodel_path
        self.detectmodel_path = detectmodel_path
        self.model_type = ModelType.CULANE
        self.cap = None
        self.object_detector = None

    def initialize_video(self, video_path):
        self.cap = cv2.VideoCapture(video_path)

    def initialize_models(self):
        # 初始化模型
        self.object_detector = yolov5.YOLOV5(self.detectmodel_path)

    def run_detection(self):
        # 创建窗口
        cv2.namedWindow("Detection", cv2.WINDOW_NORMAL)
        cv2.namedWindow("Map", cv2.WINDOW_NORMAL)

        while self.cap.isOpened():
            frame_time = time.time()

            try:
                # 从视频中读取帧
                ret, output_img = self.cap.read()
            except:
                continue

            if ret:
                # 目标检测
                objoutput, output_img2 = self.object_detector.inference(output_img)

                objbox = yolov5.filterBox(objoutput, 0.5, 0.5)
                self.object_detector.draw(output_img2, objbox)

                # 计算FPS
                end_time = time.time()  # 记录帧处理时间
                fps = 1 / (end_time - frame_time)  # 计算FPS

                # 将FPS显示出来
                cv2.putText(output_img2, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                # 显示图像窗口
                cv2.imshow("Detection", output_img2)

            else:
                break

            # 窗口退出请按"Q"
            if cv2.waitKey(1) == ord('q'):
                break

        self.cap.release()
        cv2.destroyAllWindows()
 
