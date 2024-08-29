from videoDetection import VideoDetection
if __name__ == "__main__":
    lanemodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/lane_int8.onnx"
    detectmodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/yolov5n_lite.onnx"
    video_detector = VideoDetection(lanemodel_path, detectmodel_path)
    video_detector.initialize_video("/home/jimkwokying/Videos/sample.mp4")
    video_detector.initialize_models()
    video_detector.run_detection()
