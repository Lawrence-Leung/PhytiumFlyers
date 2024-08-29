from videoDetection import VideoDetection
from classes import stairsDetector
from classes.stairsDetector import StairsDetector
import cv2
if __name__ == "__main__":
    lanemodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/lane_int8.onnx"
    detectmodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/yolov5n_lite.onnx"
    video_detector = VideoDetection(lanemodel_path, detectmodel_path)
    cap=video_detector.initialize_video("/home/jimkwokying/Videos/sample.mp4")
    img = cv2.imread('/home/jimkwokying/projectTest/masterdevice/import_img/long1.jpg')
    video_detector.initialize_models()
    video_detector.start_detection()
    # video_detector.stair_detection()
    stairs_detector=StairsDetector(img)
    stairs_detector.detect_stairs(img)