

if __name__ == "__main__":
    onnx_path = 'yolov5n_lite.onnx'
    model = YOLOV5(onnx_path)

    cap = cv2.VideoCapture('sample.mp4')
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break

        output, or_img = model.inference(frame)

        outbox = filter_box(output, 0.5, 0.5)
        draw(or_img, outbox)
        cv2.imshow('res', or_img)

        if cv2.waitKey(1) & 0xFF == ord('q'):  # 按 'q' 退出
            break

    cap.release()  # 释放摄像头或视频文件
    cv2.destroyAllWindows()

