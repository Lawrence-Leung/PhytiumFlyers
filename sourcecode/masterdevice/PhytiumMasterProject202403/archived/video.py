import cv2
import time
from classes import yolov5
from archived.ultrafastLaneDetector import ModelType

if __name__ == "__main__":
    lanemodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/lane_int8.onnx"
    detectmodel_path = "/home/jimkwokying/projectTest/masterdevice/PhytiumMasterProject202403/models/yolov5n.onnx"
    model_type = ModelType.CULANE
    print("start")
    
    # 初始化视频读取
    cap = cv2.VideoCapture("/home/jimkwokying/Videos/walking.mp4")
    if not cap.isOpened():
        print("Error: Unable to open video file.")
        exit()

    # 初始化模型
    object_detector = yolov5.YOLOV5(detectmodel_path)

    # 创建窗口
    cv2.namedWindow("Detection", cv2.WINDOW_NORMAL)

    # 记录上次检测的时间戳
    last_detection_time = time.time()
    print("Last detection time:", last_detection_time)

    while cap.isOpened():
        frame_time = time.time()
        # print("frame_time:", frame_time)

        try:
            # 从视频中读取帧
            ret, output_img = cap.read()
            print("video is opened")
        except:
            print("video is not opened")
            continue
            
        if ret:
            # 判断是否到达检测时间
            if frame_time - last_detection_time >= 3:
                print("Starting detection")
                objoutput, output_img = object_detector.inference(output_img)  # 进行目标检测
                objbox = yolov5.filterBox(objoutput, 0.5, 0.5)
                object_detector.draw(output_img, objbox)
                
                # 更新上次检测的时间戳
                last_detection_time = frame_time
                # print("Last detection time updated:", last_detection_time)

            # 计算FPS
            end_time = time.time()
            fps = 1 / (end_time - frame_time)
            print("fps:", fps)

            # 显示FPS
            cv2.putText(output_img, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.imshow("Detection", output_img)
            cv2.waitKey(10)

        else:
            break

        # 检测按键，如果按下"q"，退出循环
        if cv2.waitKey(1) == ord('q'):
            break
