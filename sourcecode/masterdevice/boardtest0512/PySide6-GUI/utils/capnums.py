import cv2

class Camera:
    def __init__(self):
        pass

    def get_cam_num(self):
        devices = []
        try:
            for device_index in range(1):  # 尝试最多10个设备
                cap = cv2.VideoCapture(device_index)
                if cap.isOpened():
                    devices.append(device_index)
                    cap.release()
            return len(devices), devices
        except Exception as e:
            print(f"An error occurred while trying to retrieve camera devices: {e}")
            return 0, []

    def get_video_stream(self, source):
        try:
            cap = cv2.VideoCapture(source)
            if not cap.isOpened():
                print("Unable to open video stream.")
                return None
            return cap
        except Exception as e:
            print(f"An error occurred while trying to open video stream: {e}")
            return None

if __name__ == '__main__':
    cam = Camera()
    cam_num, devices = cam.get_cam_num()
    print(cam_num, devices)
    print(type(devices[0]))

    if cam_num > 0:
        
        cap = cam.get_video_stream(devices[0])
        if cap:
            while True:
                ret, frame = cap.read()
                if ret:
                    cv2.imshow('Frame', frame)
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        break
                else:
                    break
            cap.release()
            cv2.destroyAllWindows()
