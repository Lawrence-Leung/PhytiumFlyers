# rpmsg.py
# OpenAMP rpmsg 通信协议程序
# by Lawrence Leung 2024
# 2924.3.18 更新
#
import fcntl
import os
import select
import time
import numpy as np
import struct
import matplotlib.pyplot as plt
import cv2
from ctypes import *
from ioctl_opt import IOW, IO

# ---------------------------------------------------------------- OpenAMP RPMSG 通信机制

# 对应C语言的 struct rpmsg_endpoint_info 结构体，位于<linux/rpmsg.h>头文件
class RpmsgEptinfo(Structure):
    _fields_ = [
        ("name", c_char * 32),
        ("src", c_uint32),
        ("dst", c_uint32)
    ]

# 对应C语言的 struct pollfd 结构体，位于<poll.h>头文件
class PollFd(Structure):
    _fields_ = [
        ("fd", c_int),
        ("events", c_short),
        ("revents", c_short)
    ]

# 移植示例程序，需要包括以下几个流程：
# 打开endpoint0、endpoint1 控制器
# ioctl，创建rpmsg_endpoint设备文件，背后靠驱动执行
# 打开rpmsg文件描述符
# 从远程处理器接收信息
# 从远程处理器发送信息

# RPMSG类。下面定义每个rpmsg_ctrl对应一个endpoint。
class RPMsg:
    def __init__(self,
                 ctrl_device_path: object,
                 ept_device_path: object,
                 info_name: object,
                 info_src: object,
                 info_dst: object) -> object:
        # 设置endpoint字段
        self.ctrl_device_path = ctrl_device_path  # /dev目录的rpmsg_ctrl文件地址
        self.ept_device_path = ept_device_path  # /dev目录的rpmsg_ept文件地址
        self.info_name = info_name  # slave端为这个/dev/rpmsg_ctrl所创建的endpoint名称
        self.info_src = info_src    # 上述endpoint的src
        self.info_dst = info_dst    # 上述endpoint的dst
        self.ret = None     # 一个静态的返回值

        # 导入OpenAMP的rpmsg通信时使用的宏定义，来源于<linux/rpmsg.h>头文件，/usr/include目录。
        # 用于ioctl，也就是与Linux底层驱动程序直接交互
        # 创建通信端点
        self.RPMSG_CREATE_EPT_IOCTL = IOW(0xb5, 0x1, RpmsgEptinfo)  # 注意这个地方用的是RpmsgEptInfo
        # 销毁通信端点
        self.RPMSG_DESTROY_EPT_IOCTL = IO(0xb5, 0x2)
        # 任意地址，这个地址是Linux内核固定的，一般不随意更改
        self.RPMSG_ADDR_ANY = 0xFFFFFFFF

    # 打开rpmsg_ctrl设备
    def openCtrlDevice(self):
        # 打开文件描述符
        self.ctrlfd = os.open(self.ctrl_device_path, os.O_RDWR | os.O_NONBLOCK)
        if self.ctrlfd is not None:
            print("[OPENAMP] Open control device '%s' successfully" % self.ctrl_device_path)
            return True
        else:
            print("[OPENAMP] Open control device '%s' failed" % self.ctrl_device_path)
            return False

    # 创建端点
    def createEndpoint(self):
        # 检查文件是否存在
        if not os.path.exists(self.ept_device_path):
            # 如果文件不存在，则创建端点
            self.eptinfo = RpmsgEptinfo()
            self.eptinfo.name = self.info_name.encode('ascii')  # 将字符串转换为字节串
            self.eptinfo.src = self.info_src
            self.eptinfo.dst = self.info_dst
            # 创建端点驱动程序
            ret = fcntl.ioctl(self.ctrlfd, self.RPMSG_CREATE_EPT_IOCTL, self.eptinfo)
        else:
            print(f"[OPENAMP] {self.ept_device_path} already exists. Skipping endpoint creation.")

    # 打开rpmsg_endpoint设备
    def openEptDevice(self):
        # 打开文件描述符
        self.eptfd = os.open(self.ept_device_path, os.O_RDWR | os.O_NONBLOCK)
        if self.eptfd is not None:
            print("[OPENAMP] Open endpoint '%s' successfully" % self.ept_device_path)
            # 规定ept文件描述符所需要完成的功能：读取功能
            self.eptpoller = select.poll()
            self.eptpoller.register(self.eptfd, select.POLLIN)
            self.eptpollevents = None
            return True

        else:
            print("[OPENAMP] Open endpoint '%s' failed" % self.ept_device_path)
            return False

    # 向rpmsg_endpoint设备写入数据
    def writeEptDevice(self):
        # todo: demo only，后续需要加入业务代码，如JSON之类的
        string = "I'm Lawrence '%s" % self.ept_device_path
        buf0 = string.encode('ascii')   # 为字符串编码为ASCII
        buf0 = buf0.ljust(32, b'\x00')  # 确保不超过32字节

        # 开始写入数据
        self.ret = os.write(self.eptfd, buf0)
        if self.ret < 0:
            print("[OPENAMP] Write endpoint '%s' failed" % self.ept_device_path)

    # 等待事件列表发生了读取设备文件可用的提示
    def pollEptDeviceWithReadEvent(self):
        if self.eptpoller is None:
            print("[OPENAMP] Endpoint eptpoller not initialized")
        else:
            try:
                self.eptpollevents = self.eptpoller.poll(100)
                if not self.eptpollevents:
                    print("[OPENAMP] No events occurred within the timeout period")
                else:
                    for fd, event in self.eptpollevents:
                        print("[OPENAMP] Data is ready to be read")
                        # todo：读取数据
            except Exception as e:
                print("[OPENAMP] Error on pollEptDeviceWithReadEvent: %s" % e)

    # 开始从设备文件中读取数据
    def readEptDevice(self):
        if not self.eptpollevents: # 没有初始化
            print("[OPENAMP] Endpoint eptpoller not initialized")
            return None
        else:   # POLLIN事件发生了，意味着可以读取数据了
            for _, event in self.eptpollevents:
                if event & select.POLLIN:
                    try:
                    	while True:
		                    self.r_buf0 = os.read(self.eptfd, 32)
		                    if len(self.r_buf0) == 0:
		                        print("[OPENAMP] No data read from %s" % self.ept_device_path)
		                    else:
		                        print(f"[OPENAMP] Read data: {self.r_buf0.hex()}")
		                        return self.r_buf0.hex()    # 将数据以16进制的形式返回

                    except OSError as e:
                        print(f"[OPENAMP] Reading failed: {e}")
                else:
                    # todo 后续如果有需要增加更多的那就加入
                    return None

    # 关闭endpoint文件设备
    def closeEptDevice(self):
        try:
            os.close(self.eptfd)
        except OSError as e:
            print(f"[ERROR] Failed to close endpoint device. {e}")

    # 关闭ctrl文件设备
    def closeCtrlDevice(self):
        try:
            os.close(self.ctrlfd)
        except OSError as e:
            print(f"[ERROR] Failed to close ctrl device. {e}")

# ---------------------------------------------------------------- 传感器数据处理

# 计算DHT11的校验码（位于Master端）
def slaveDHT11CalculateCheck(temp, humi):
    return (humi + temp) % 256

# 分析Slave端DHT11收到的数据
def slaveDHT11Analysis(dht11data):
    # 检查数据长度是否正确
    if len(dht11data) != 10:
        print("[OPENAMP] Invalid data length")
        return None
    try:
        humi = int(dht11data[2:4], 16)
        temp = int(dht11data[4:6], 16)
        received_check = int(dht11data[6:8], 16)
        print(f"humi: {humi} temp {temp} received check {received_check}")
    except ValueError:
        print("[OPENAMP] Invalid data format")
        return None
    # 计算校验码
    calculated_check = slaveDHT11CalculateCheck(temp, humi)
    # 检查校验码是否匹配
    if received_check == calculated_check:
        # 数据帧校验通过，返回湿度和温度数据
        return (humi, temp)
    else:
        print("[OPENAMP] Checksum mismatch. Discarding frame.")
        return None

# 计算GPS的校验码（位于Master端）
def slaveGPSCalculateCheck(latitude, longitude):
    code = latitude + longitude
    if -90 * 10 ** 6 <= latitude <= 90 * 10 ** 6:
        if latitude > 0:
            code += 100
    if -180 * 10 ** 6 <= longitude <= 180 * 10 ** 6:
        if longitude > 0:
            code += 10
    return code

# 分析Slave端GPS收到的数据
def slaveGPSAnalysis(gpsdata):
    # 检查数据长度是否正确
    if len(gpsdata) != 28:  # 14个字节转为hex字符串是28个字符
        print("[OPENAMP] GPS Invalid data length")
        return None
    # 将hex字符串转换为对应的整数值
    try:
        latitude = int(gpsdata[2:10], 16)
        longitude = int(gpsdata[10:18], 16)
        received_check = int(gpsdata[19:26], 16)
    except ValueError:
        print("[OPENAMP] GPS Invalid data format")
        return None
    # 计算校验码
    calculated_check = slaveGPSCalculateCheck(latitude, longitude)
    # 检查校验码是否匹配
    if received_check == calculated_check:
        # 数据帧校验通过，返回经度和纬度数据
        return (latitude / 10 ** 6, longitude / 10 ** 6)
    else:
        print("[OPENAMP] GPS Checksum mismatch. Discarding frame.")
        return None

# 分析Slave端加速度传感器收到的数据
def slaveJY61PAnalysis(jy61pdata, inputbitstream):
	if inputbitstream is None:
		return None
	else:
		# 检查数据长度是否为24字节
		if len(inputbitstream) != 24 * 2:  # 每个字节由两个十六进制字符表示
		    print("[JY61P] 数据长度不符，应为24字节")
		    return None
		# 将16进制字符串转换为bytes，然后反转每4字节解析为一个float32
		data_bytes = bytes.fromhex(inputbitstream)
		data_floats = [struct.unpack('<f', data_bytes[i:i + 4])[0] for i in range(0, len(data_bytes), 4)]
		# 第5个float32决定数据类型
		data_type = data_floats[4]
		if data_type == 1.0:
		    print("[JY61P] 加速度")
		    row_index = 0
		elif data_type == 2.0:
		    print("[JY61P] 角速度")
		    row_index = 1
		elif data_type == 3.0:
		    print("[JY61P] 偏移角度")
		    row_index = 2
		else:
		    print("[JY61P] 未知数据类型")
		    return None
		# 更新jy61pdata数组
		for i in range(3):
		    jy61pdata[row_index, i] = data_floats[i + 1]  # 第2~4个数值对应到数组中
		print("[JY61P] Updated：\n", jy61pdata)



def update_and_visualize_data(jy61pdata, data_history):
    # 更新数据历史，保留最新的100个数据点
    data_history.append(jy61pdata)
    data_history = data_history[-100:]
    # 创建图形和子图
    plt.figure(figsize=(10, 6))
    # 加速度子图
    plt.subplot(3, 1, 1)
    plt.title("Acceleration")
    plt.plot([d[0, 0] for d in data_history], label='Pitch')
    plt.plot([d[0, 1] for d in data_history], label='Roll')
    plt.plot([d[0, 2] for d in data_history], label='Yaw')
    plt.legend()
    # 角速度子图
    plt.subplot(3, 1, 2)
    plt.title("Angular Velocity")
    plt.plot([d[1, 0] for d in data_history], label='Pitch')
    plt.plot([d[1, 1] for d in data_history], label='Roll')
    plt.plot([d[1, 2] for d in data_history], label='Yaw')
    plt.legend()
    # 偏移角度子图
    plt.subplot(3, 1, 3)
    plt.title("Offset Angle")
    plt.plot([d[2, 0] for d in data_history], label='Pitch')
    plt.plot([d[2, 1] for d in data_history], label='Roll')
    plt.plot([d[2, 2] for d in data_history], label='Yaw')
    plt.legend()
    # 将matplotlib绘图转换为OpenCV图像
    plt.tight_layout()
    plt.gcf().canvas.draw()
    w, h = plt.gcf().get_size_inches() * plt.gcf().get_dpi()
    img = np.frombuffer(plt.gcf().canvas.tostring_rgb(), dtype='uint8').reshape(int(h), int(w), 3)
    
    # 使用OpenCV显示图像
    cv2.imshow("JY61P Data Visualization", img)
    cv2.waitKey(1)
    plt.close()


# ---------------------------------------------------------------- 主函数 main()
# 仅在调试这个脚本的代码时使用，不是真正的主程序，后续会整合到主程序里面去
if __name__ == "__main__":
    #创建第一个端点
    rpmsg0 = RPMsg("/dev/rpmsg_ctrl_lawrence_10",
                   "/dev/rpmsg_lawrence_10",
                   "hello", 11, 0)
    rpmsg0.openCtrlDevice()
    # 创建第二个端点
    #rpmsg1 = RPMsg("/dev/rpmsg_ctrl_lawrence_20",
    #               "/dev/rpmsg_lawrence_20",
    #               "world", 22, 0)
    #rpmsg1.openCtrlDevice()
    #创建第三个端点
    rpmsg2 = RPMsg("/dev/rpmsg_ctrl_lawrence_30",
                   "/dev/rpmsg_lawrence_30",
                   "hello", 33, 0)
    rpmsg2.openCtrlDevice()
    # 创建第四个端点
    rpmsg3 = RPMsg("/dev/rpmsg_ctrl_lawrence_40",
                   "/dev/rpmsg_lawrence_40",
                   "world", 44, 0)
    rpmsg3.openCtrlDevice()
   

    rpmsg0.createEndpoint()
    #rpmsg1.createEndpoint()
    rpmsg2.createEndpoint()
    rpmsg3.createEndpoint()

    # 上面三个步骤已经成功了，接下来需要执行下面的步骤
    rpmsg0.openEptDevice()
    #rpmsg1.openEptDevice()
    rpmsg2.openEptDevice()
    rpmsg3.openEptDevice()

    # 下面将飞腾的demo移植进去：
    #count = 15

    # 所返回的数据：
    # rpmsg0data = None
    # rpmsg1data = None
    
    jy61pdata = np.zeros((3, 3))	# JY61P接收缓存
    data_history = []	# JY61P画图使用，debug only
    
    while True:
        try:
            #DHT11
            time.sleep(0.1)   # 睡眠1秒
			
            rpmsg0.writeEptDevice() # 将已有的数据写入到端点里面去
            print("[OPENAMP] WAIT data from DHT11")
            rpmsg0.pollEptDeviceWithReadEvent()

            time.sleep(0.1)   # 睡眠1秒
            print("[OPENAMP] read data from DHT11")
            # 以下是数据解析
            rpmsg0data = rpmsg0.readEptDevice()
            # 计算校验码
            #result = slaveDHT11Analysis(rpmsg0data)
            #if result:
            #    print("[OPENAMP] DHT11接收到数据 ", result)
            #time.sleep(1)   # 睡眠1秒
            
            #GPS
            time.sleep(0.1)   # 睡眠1秒
            #rpmsg1.writeEptDevice() # 将已有的数据写入到端点里面去
            print("[OPENAMP] WAIT data from GPS")
            #rpmsg1.pollEptDeviceWithReadEvent()

            time.sleep(0.1)   # 睡眠1秒
            print("[OPENAMP] read data from GPS")
            # 以下是数据解析
            #data = rpmsg1.readEptDevice()
            
            #SYN6288
            time.sleep(0.1)   # 睡眠()秒
            rpmsg2.writeEptDevice() # 将已有的数据写入到端点里面去
            #print("[OPENAMP] WAIT data from SYN6288")
            #rpmsg2.pollEptDeviceWithReadEvent()	#读取数据——syn6288不发送数据
            #time.sleep(0.1)   # 睡眠()秒
            #print("[OPENAMP] read data from SYN6288")
            #data = rpmsg2.readEptDevice()	#数据解析
            
            #JY61P
            time.sleep(0.1)   # 睡眠1秒
            rpmsg3.writeEptDevice() # 将已有的数据写入到端点里面去
            print("[OPENAMP] WAIT data from JY61P")
            rpmsg3.pollEptDeviceWithReadEvent()

            time.sleep(0.1)   # 睡眠1秒
            print("[OPENAMP] read data from JY61P")
            # 以下是数据解析
            data = rpmsg3.readEptDevice()
            slaveJY61PAnalysis(jy61pdata, data)
            #if data is not None:
            #    update_and_visualize_data(jy61pdata, data_history)
            
            
            
            # 向第二个端点写入数据
            #rpmsg1.writeEptDevice()
            # 等待第二个端点的事件
            #print("[OPENAMP] WAIT data from GPS")
            #rpmsg1.pollEptDeviceWithReadEvent()
            #time.sleep(1)   # 睡眠1秒
            # 从第二个端点读取数据
            #print("[OPENAMP] read data from GPS")
            #rpmsg1data = rpmsg1.readEptDevice()
            #result = slaveGPSAnalysis(rpmsg1data)
            #if result:
            #    print("[OPENAMP] GPS接收到数据 ", result)

        except Exception as e:
            print(f"[OPENAMP] Example operation failed. {e}")
            rpmsg0.closeEptDevice()
            rpmsg0.closeCtrlDevice()
            #rpmsg1.closeEptDevice()
            #rpmsg1.closeCtrlDevice()
            rpmsg2.closeEptDevice()
            rpmsg2.closeCtrlDevice()
            rpmsg3.closeEptDevice()
            rpmsg3.closeCtrlDevice()
            exit()
        except KeyboardInterrupt:
            print("[ERROR] Polling interrupted by the user.")
            rpmsg0.closeEptDevice()
            rpmsg0.closeCtrlDevice()
            #rpmsg1.closeEptDevice()
            #rpmsg1.closeCtrlDevice()
            rpmsg2.closeEptDevice()
            rpmsg2.closeCtrlDevice()
            rpmsg3.closeEptDevice()
            rpmsg3.closeCtrlDevice()
            exit()

        #count -= 1

    rpmsg0.closeEptDevice()
    rpmsg0.closeCtrlDevice()
    #rpmsg1.closeEptDevice()
    #rpmsg1.closeCtrlDevice()
    rpmsg2.closeEptDevice()
    rpmsg2.closeCtrlDevice()
    rpmsg3.closeEptDevice()
    rpmsg3.closeCtrlDevice()

    #todo: 把程序写上去

