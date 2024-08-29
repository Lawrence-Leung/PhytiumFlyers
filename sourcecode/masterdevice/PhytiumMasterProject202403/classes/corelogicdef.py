# corelogicdef.py
# 核心逻辑类定义
# by Lawrence Leung 2024

# ---------------------------------------------------------------- 包管理
import os
import multiprocessing
import threading
import numpy as np
import cv2
import time  # for debug only

# ---------------------------------------------------------------- 进程处理模块

# 子进程类，继承了multiprocessing的功能 （一般不需要改这个）
# 所有该类以及该类的继承，初始化时，需要输入参数：
#       name：字符串，进程实例里的名称
#       sendpipe, recvpipe：每次运行过程中，能与这个管道进行通信。当然管道只有两方，而不是多方。
#       importdata_func：函数指针，这个指针在初始化时指定一个函数，这个函数输入参数data，能够实现将外部的data在每次循环执行前传入到进程中
#       returndata_func：函数指针，这个指针在初始化时指定一个函数，这个函数输出参数data，能够实现将进程中的data在每次循环执行后传输到进程中
# 每次运行开始前传入参数：
#       import_data：每次运行开始时，从外部输入的数据
# 每次运行结束后返回参数：
#       return_data：每次运行结束后，所返回的数据
class WorkerProcess(multiprocessing.Process):
    # 构造函数
    def __init__(self, name, importdata_func, returndata_func, sendpipe, recvpipe):
        super().__init__()
        self.name = name  # 进程名称
        self.importdata_func = importdata_func  # 传入参数的函数指针
        self.returndata_func = returndata_func  # 传出参数的函数指针
        self.input_data = None  # 需要被execute使用的函数，将传入参数的函数指针传递的数据放到这里
        self.output_data = None # execute输出的函数，待被传出参数的函数指针使用

        # 检查 pipe 是否为 multiprocessing.Pipe 类的实例
        if recvpipe is not None and not isinstance(recvpipe, multiprocessing.connection.Connection):
            raise TypeError(f"[WORKERPROCESS] <{self.name}> recvpipe 必须是 multiprocessing.Pipe 的实例")
        self.recvpipe = recvpipe  # 管道
        if sendpipe is not None and not isinstance(sendpipe, multiprocessing.connection.Connection):
            raise TypeError(f"[WORKERPROCESS] <{self.name}> sendpipe 必须是 multiprocessing.Pipe 的实例")
        self.sendpipe = sendpipe  # 管道
        self.stop_event = multiprocessing.Event()  # 停止事件
        print(f"[WORKERPROCESS] <{self.name}> 初始化完毕")  # 当触发了停止事件

    # 启动
    def run(self):
        print(f"[WORKERPROCESS] <{self.name}> 启动进程中")  # 当开始被启动时
        while not self.stop_event.is_set():  # 如果没有触发停止事件，循环：
            # step 1 从外部传入数据
            #print(f"[WORKERPROCESS] <{self.name}> 正在传入数据")
            if self.importdata_func is not None:    # 如果存在这个函数
                self.input_data = self.importdata_func  # 调用传入参数的函数指针获取数据
            else:
                self.input_data = None

            # step 2 开始执行进程体
            self.output_data = self.execute(self.input_data)

            # step 3 执行后向外部传出数据
            #print(f"[WORKERPROCESS] <{self.name}> 正在传出数据")
            if self.returndata_func is not None:    # 如果存在这个函数
                self.returndata_func(self.output_data)


        print(f"[WORKERPROCESS] <{self.name}> 停止进程中")  # 当触发了停止事件

    # 执行，注意：子类需要继承这个方法
    # 传入参数：input_data
    def execute(self, input_data):
        # 以下是示例，后面被继承的类不会执行以下的代码，而是被继承的execute方法覆盖掉
        self.printP("execute获取到了 " + input_data)
        if self.sendpipe:
            self.sendpipe.send(input_data) # 向管道发送数据
            self.printP("向sendpipe管道发送了 " + input_data)
        output_data = None
        time.sleep(1)
        if self.recvpipe:
            if self.recvpipe.poll(1):   # 等待最多1秒
                output_data = self.recvpipe.recv() # 从管道接收数据
                self.printP("从管道recvpipe接收了 " + output_data)
            else:
                self.printP("超时，没有从管道recvpipe获取到数据")
        if output_data is not None:
            self.printP("execute即将返回 " + output_data)
        return output_data

    # 停止
    def stop(self):
        self.stop_event.set()  # 触发停止事件
        print(f"[WORKERPROCESS] <{self.name}> 触发停止事件中")  # 当触发了停止事件

    # 输出进程信息的格式
    # 输入：string 随意的字符串
    def printP(self, string):
        print('\033[3;32m' + f"<{self.name}> " + '\033[1;0m' + f"{string}")


# 典型的 importdata_func 函数，用于从外部传入数据，这个需要被WorkerProcess类调用
# 输入：data 从外部传入的数据
# 输出：data 传入到进程体中的数据
def importDataFunc(data=None):
    # 在这里实现从外部获取数据的逻辑
    print(f"[importDataFunc] 获取到的数据: {data}")
    return data


# 典型的 returndata_func 函数，用于向外部传出数据，这个需要被WorkerProcess类调用
# 输入：无
# 输出：self.output_data
def returnDataFunc(data=None):
    # 在这里实现向外部传出数据的逻辑
    print(f"[returnDataFunc] 传出的数据: {data}")
    return data


# 子进程控制器 （一般不需要改这个）
class CoreProcessController:
    # 构造函数
    def __init__(self):
        self.runningprocesses = []  # 一个列表，装有所有运行的进程

    # 开始所有进程
    def startAll(self, process_list):
        if not process_list:
            raise ValueError("[CORELOGIC] 所输入的进程列表为空，无法执行进程.")
        for process in process_list:
            if not isinstance(process, WorkerProcess):
                raise ValueError(
                    "[CORELOGIC] 错误：进程列表中的所有对象必须是WorkerProcess或其子类的实例.")
        for process in process_list:
            process.start()
            self.runningprocesses.append(process)

    # 停止所有进程
    def stopAll(self):
        for process in self.runningprocesses:
            process.stop()  # 停止所有子进程的循环
        for process in self.runningprocesses:
            process.join()  # 等待所有子进程停止
        self.runningprocesses.clear()  # 清空所有子进程

# ---------------------------------------------------------------- 自定义类
# 摄像头OpenCV图像管理类
class CameraImageManager:
    # 构造函数
    def __init__(self, cap):
        # 以下是类内置参数
        self.imageWidth = 640  # 统一OpenCV图像宽度
        self.imageHeight = 640  # 统一OpenCV图像高度
        self.imageChannels = 3  # 统一OpenCV图像通道数
        self.semaphoreNumber = 3  # 信号量的数量

        self.newimage = np.zeros((self.imageWidth, self.imageHeight, self.imageChannels), dtype=np.uint8)  # 新的从摄像头导入的一帧
        self.newimagesemaphore = threading.Semaphore(self.semaphoreNumber)  # 信号量，用于控制对图像的访问

        self.detectionimage = np.zeros((self.imageWidth, self.imageHeight, self.imageChannels),
                                       dtype=np.uint8)  # 目标检测结果图像帧
        self.detectionimagesemaphore = threading.Semaphore(self.semaphoreNumber)  # 目标检测结果图像的信号量

        self.mapimage = np.zeros((self.imageWidth, self.imageHeight, self.imageChannels), dtype=np.uint8)  # 小地图图像
        self.mapimagesemaphore = threading.Semaphore(self.semaphoreNumber)  # 小地图图像的信号量

        # 检查传入的 cap 参数是否为 cv2.VideoCapture 类型
        if not isinstance(cap, cv2.VideoCapture):
            raise TypeError("[CAMERAIMAGE] cap 参数必须为 cv2.VideoCapture 类型")
        self.cap = cap  # OpenCV 视频流
        self.framecount = 0 # 记录已经读取的帧数

    # 从Cap中获取图像
    def getImageFromCap(self):
        if self.cap.isOpened():
            frame_position = self.cap.get(cv2.CAP_PROP_POS_FRAMES)
            ret, image = self.cap.read()
            if ret == True:
                image_resized = cv2.resize(image, (640, 640)) # 将图像拉伸至640x640
                # 如果原图像大小与目标大小不同，可以使用不同的插值方法，例如 cv2.INTER_AREA
                # image_resized = cv2.resize(image, (640, 640), interpolation=cv2.INTER_AREA)
                # 确保图像为RGB格式
                if len(image_resized.shape) == 2:  # 如果是灰度图，则转换为RGB
                    image_resized = cv2.cvtColor(image_resized, cv2.COLOR_GRAY2RGB)
                elif image_resized.shape[2] == 1:  # 如果是单通道图像，则转换为RGB
                    image_resized = cv2.cvtColor(image_resized, cv2.COLOR_GRAY2RGB)
                self.inputCameraImage(image_resized)
                self.framecount += 10
                print(f"[CAMERAIMAGE] 获取新图像成功，计数为 {self.framecount}")
            # 将帧位置设置为下一帧
            self.cap.set(cv2.CAP_PROP_POS_FRAMES, frame_position + 1)

    # 指定Cap读取一个新的文件
    # inputdata：新的文件
    def openCap(self, inputdata):
        self.cap.release() # 需要重新初始化一次
        self.cap.open(inputdata)
        print(f"[CAMERAIMAGE] cap 打开 {inputdata}")
        self.framecount = 0

    # 释放掉Cap
    def releaseCap(self):
        self.cap.release()
        print(f"[CAMERAIMAGE] 释放cap")
        self.framecount = 0

    # 输入新的摄像头图像
    def inputCameraImage(self, newimage=None):
        if newimage is None:
            print("[CAMERAIMAGE] 没有图像。")
            return
        if not isinstance(newimage, np.ndarray):
            print("[CAMERAIMAGE] 图像不是numpy数组。")
            return
        if newimage.shape != (self.imageWidth, self.imageHeight, self.imageChannels):
            print("[CAMERAIMAGE] 输入OpenCV图像规格不符合(width={}, height={}, channels=3).".format(self.imageWidth,
                                                                                                    self.imageHeight))
            return
        # 尝试获取锁
        lock_acquired = self.newimagesemaphore.acquire(blocking=False)
        if lock_acquired:
            try:
                self.newimage = newimage
                # np.copyto(self.newimage, newimage)  # 将图像拷贝到self.newimage中
            finally:
                self.newimagesemaphore.release()  # 释放锁
        else:
            print("[CAMERAIMAGE] 尝试获取锁失败，另一个进程正在占用 self.newimagesemaphore。")

    # 获取新的摄像头图像
    def outputCameraImage(self):
        # 尝试获取锁
        lock_acquired = self.newimagesemaphore.acquire(blocking=False)
        if lock_acquired:
            try:
                # 返回当前图像的拷贝
                return np.copy(self.newimage)
            finally:
                # 释放锁
                self.newimagesemaphore.release()
        else:
            print("[CAMERAIMAGE] 尝试获取图像失败.")
            return None

    # 输入新的detect后图像
    def inputDetectionImage(self, newimage=None):
        if newimage is None:
            print("[MAPIMAGE] 没有图像。")
            return
        if not isinstance(newimage, np.ndarray):
            print("[MAPIMAGE] 图像不是numpy数组。")
            return
        if newimage.shape != (self.imageWidth, self.imageHeight, self.imageChannels):
            print("[MAPIMAGE] 输入OpenCV图像规格不符合(width={}, height={}, channels=3).".format(self.imageWidth,
                                                                                                    self.imageHeight))
            return
        # 尝试获取锁
        lock_acquired = self.detectionimagesemaphore.acquire(blocking=False)
        if lock_acquired:
            try:
                np.copyto(self.detectionimage, newimage)  # 将图像拷贝到self.newimage中
            finally:
                self.detectionimagesemaphore.release()  # 释放锁
        else:
            print("[DETECTIONIMAGE] 尝试获取锁失败，另一个进程正在占用 self.mapimagesemaphore。")

    # 获取新的detect后图像
    def outputDetectionImage(self):
        # 尝试获取锁
        lock_acquired = self.detectionimagesemaphore.acquire(blocking=False)
        if lock_acquired:
            try:
                # 返回当前图像的拷贝
                return np.copy(self.detectionimage)
            finally:
                # 释放锁
                self.detectionimagesemaphore.release()
        else:
            print("[DETECTIONIMAGE] 尝试获取图像失败.")
            return None

    # 输入新的map图像
    def inputMapImage(self, newimage=None):
        if newimage is None:
            print("[MAPIMAGE] 没有图像。")
            return
        if not isinstance(newimage, np.ndarray):
            print("[MAPIMAGE] 图像不是numpy数组。")
            return
        if newimage.shape != (self.imageWidth, self.imageHeight, self.imageChannels):
            print("[MAPIMAGE] 输入OpenCV图像规格不符合(width={}, height={}, channels=3).".format(self.imageWidth,
                                                                                                    self.imageHeight))
            return
        # 尝试获取锁
        lock_acquired = self.mapimagesemaphore.acquire(blocking=False)
        if lock_acquired:
            try:
                np.copyto(self.mapimage, newimage)  # 将图像拷贝到self.newimage中
            finally:
                self.mapimagesemaphore.release()  # 释放锁
        else:
            print("[MAPIMAGE] 尝试获取锁失败，另一个进程正在占用 self.newimagesemaphore。")

    # 获取新的map图像
    def outputMapImage(self):
        # 尝试获取锁
        lock_acquired = self.mapimagesemaphore.acquire(blocking=False)
        if lock_acquired:
            try:
                # 返回当前图像的拷贝
                return np.copy(self.mapimage)
            finally:
                # 释放锁
                self.mapimagesemaphore.release()
        else:
            print("[DETECTIONIMAGE] 尝试获取图像失败.")
            return None

# ---------------------------------------------------------------- 工具函数
# 输入相对于当前脚本的位置，输出绝对位置
# 输入：filepath 相对目录位置字符串
# 输出：relative_path 绝对目录位置字符串
def toAbsolutePath(current_script_path, filepath):
    current_dir = os.path.dirname(os.path.abspath(current_script_path))  # 获取当前脚本的绝对路径
    relative_path = os.path.join(current_dir, filepath)
    return relative_path

