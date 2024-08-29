#语音播报端点为flag确认发送
import time
import numpy as np
from PySide6.QtCore import Signal, Slot  # QCoreApplication用于命令行Qt程序，用不上
from drivers.rpmsg import RPMsg
from drivers.rpmsg import *
from drivers.speakout import *
from classes.roadDeviationDetector import *
from classes.gpsdecode import *
from UIFunctions import *
import time
import array


class SlaveDeviceThread(QObject):
    dht_update=Signal(str)#温湿度
    down_update=Signal(str)#摔倒
    cam_update=Signal(str)#相机位置
    gps_update=Signal(str)#GPS
    loc_detected=Signal(str)#地理位置
    speak=Signal(int)
    mtosT=Signal(str)#响应时间
    def __init__(self, parent=None):
        super().__init__(parent)
        self.running = False
        self.dht_queue=Queue()
        self.gps_queue=Queue()
        self.cam_queue=Queue()
        self.t_queue=Queue()
        self.test_road_lists = [
            (113.32742024872587, 23.096267632545597, 113.32302142594145, 23.0962133536494),  # 新港中路（西）
            (113.32746852848814, 23.09624789476786, 113.32980741474913, 23.096272566989867),  # 新港中路（东）
            (113.32738269779966, 23.096351518069685, 113.32739342663572, 23.09855226074607),  # 赤岗北路
            (113.32739879105375, 23.09629230476447, 113.32743097756193, 23.09346977360184)  # 赤岗路（南）
        ]   # 位于广州海珠区附近的道路
        self.speaker = Speaker() # 解包、播报信息
        self.deviation_detector = DeviationDetector() # 是否沿道路行走

        self.api_key = "e0b93d4f1620b219cc59584de3bb01ee"
        self.rpmsg0 = RPMsg("/dev/rpmsg_ctrl_lawrence_10",
                   "/dev/rpmsg_lawrence_10",
                   "hello", 11, 0)
        self.rpmsg0.openCtrlDevice()
        self.rpmsg1 = RPMsg("/dev/rpmsg_ctrl_lawrence_20",
                  "/dev/rpmsg_lawrence_20",
                  "world", 22, 0)
        self.rpmsg1.openCtrlDevice()
        self.rpmsg2 = RPMsg("/dev/rpmsg_ctrl_lawrence_30",
                   "/dev/rpmsg_lawrence_30",
                   "hello", 33, 0)
        self.rpmsg2.openCtrlDevice()
    # 创建第四个端点
        self.rpmsg3 = RPMsg("/dev/rpmsg_ctrl_lawrence_40",
                   "/dev/rpmsg_lawrence_40",
                   "world", 44, 0)
        self.rpmsg3.openCtrlDevice()
        self.rpmsg0.createEndpoint()
        self.rpmsg1.createEndpoint()
        self.rpmsg2.createEndpoint()
        self.rpmsg3.createEndpoint()
        self.rpmsg0.openEptDevice()
        self.rpmsg1.openEptDevice()
        self.rpmsg2.openEptDevice()
        self.rpmsg3.openEptDevice()
        self.count = 15
        self.time1=0

        self.jy61pdata = np.zeros((3, 3))	# JY61P接收缓存
        data_history = []
        self.var=None
        self.timetosend()
        
    def slavetask(self):
        self.running=True
        print("Slave running")
        while self.running:
            # try:
            #DHT11
                # self.msleep(100)   # 睡眠1秒
                start_time=time.time()
                self.rpmsg0.writeEptDevice() # 将已有的数据写入到端点里面去
                end_ime=time.time()
                self.time1=end_ime-start_time
                
                time_formatted=round(self.time1*1000,2)
                print("time1:",time_formatted)#slave响应时间
                self.t_queue.put(str(time_formatted))
                print("[OPENAMP] WAIT data from DHT11")
                self.rpmsg0.pollEptDeviceWithReadEvent()

                time.sleep(1)   # 睡眠1秒
                print("[OPENAMP] read data from DHT11")

                self.rpmsg0data = self.rpmsg0.readEptDevice()
                
                if self.rpmsg0data is not None:
                    result = slaveDHT11Analysis(self.rpmsg0data)
                else:
                    continue
                if result:
                    result_str="({}%,{}°C)".format(result[0],result[1])
                    print("[OPENAMP] DHT11 received data: ", result_str)
                    self.dht_queue.put(str(result_str))
                    #formatted_result=tuple(round(value,2) for value in result)
                    #print("formatted data: ", formatted_result) 
            #GPS
                self.rpmsg1.writeEptDevice()
                print("[OPENAMP] WAIT data from GPS")
                self.rpmsg1.pollEptDeviceWithReadEvent()
                time.sleep(1)
                print("[OPENAMP] read data from GPS")
                self.rpmsg1data = self.rpmsg1.readEptDevice()
                if type(self.rpmsg1data) is not type(self.var):
                    result = slaveGPSAnalysis(self.rpmsg1data)
                    print("result:")
                    print(result)
                    #result = (113.32718957875059, 23.09615907473295)
                    if result and result!=(0.0,0.0):
                        print("[OPENAMP] GPS received data: ", result)
                        current_location=result
                        #current_location = (113.32718957875059, 23.09615907473295)
                        isnearroad, isnearcrossing, isalongroad, nearest_road = self.deviation_detector.CompleteRoadDeviationProcess(self.test_road_lists, current_location)  # 分析行人是否沿着道路行走
                        # 开始播报是否沿着道路行走了，{}里面的内容是字符串
                        #print(f'[播报5]: {SpeakDeviationRoad(isalongroad, isnearroad, isnearcrossing)} \r\n')   # todo：需要传入slave端：005 是否沿着道路行走
                        # formatted_result=tuple(f'{value:.2f}' for value in result)
                        formatted_result=tuple(round(value,2) for value in result)
                        print("formatted data: ", formatted_result)
                        formatted_address,citycode=decode_address(result[0],result[1],self.api_key)
                        weather_data=get_weather(self.api_key,citycode)
                        current_weather = weather_data['forecasts'][0]['casts'][0]
                        weather_result=current_weather['dayweather']

                        print("地理信息：",formatted_address,"当前天气：",weather_result)
                        self.gps_queue.put((str(formatted_result),str(formatted_address),str(weather_result)))
                        print("gps传出")
                        time.sleep(1)

            # #JY61P
                time.sleep(1)    # 睡眠1秒
                self.rpmsg3.writeEptDevice() # 将已有的数据写入到端点里面去
                print("[OPENAMP] WAIT data from JY61P")
                self.rpmsg3.pollEptDeviceWithReadEvent()

                # time.sleep(1)   # 睡眠1秒
                print("[OPENAMP] read data from JY61P")
            # 以下是数据解析
                data = self.rpmsg3.readEptDevice()
                rpmsg3_result=slaveJY61PAnalysis(self.jy61pdata, data)
                #print("len:",len(rpmsg3_result))
                print(type(rpmsg3_result))
                if rpmsg3_result is not None:
                    for i in range(len(rpmsg3_result[0])):
                        if rpmsg3_result[0][i]<=3:
                            status="未摔倒"
                        else:
                            status="摔倒"

                    for i in range(len(rpmsg3_result[2])):
                        # if rpmsg3_result[2][i]<=6:
                        if rpmsg3_result[2][1] >= -40 and rpmsg3_result[2][1] <= 40 and rpmsg3_result[2][0] >= -20 and rpmsg3_result[2][0] <= 20:
                            cam="相机视角正常"
                        else:
                            cam="相机视角异常"
                    self.cam_queue.put((status,cam))
                else:
                    continue
                time.sleep(1)
            #SYN6288
                
                

    def start_device(self):
        self.slave_process=Process(target=self.slavetask)
        self.slave_process.start()

    def stop_device(self):
        self.running = False
        self.rpmsg0.closeEptDevice()
        self.rpmsg0.closeCtrlDevice()
        self.rpmsg1.closeEptDevice()
        self.rpmsg1.closeCtrlDevice()
        self.rpmsg2.closeEptDevice()
        self.rpmsg2.closeCtrlDevice()
        self.rpmsg3.closeEptDevice()
        self.rpmsg3.closeCtrlDevice()
        self.slave_process.terminate()

    def timetosend(self):
        self.timerS = QTimer()
        self.timerS.timeout.connect(self.emit_signal_to_UI)
        self.timerS.start(1)  # 每毫秒发射一次信号

    def emit_signal_to_UI(self):
        if not self.dht_queue.empty():
            dht=self.dht_queue.get()
            print("dht:",dht)
            self.dht_update.emit(dht)
        if not self.gps_queue.empty():
            tuple_result=self.gps_queue.get()
            gps,address,wethr=tuple_result
            self.gps_update.emit(gps)
            address1="位置：{}， 当前天气：{}".format(address,wethr)
            self.loc_detected.emit(address1)
        if not self.cam_queue.empty():
            tuple_result2=self.cam_queue.get()
            print("cam_queue:",tuple_result2)
            status,cam=tuple_result2
            self.down_update.emit(status)
            self.cam_update.emit(cam)
            #self.rpmsg2.writeSpeaker(f"{status}, {cam}")
        if not self.t_queue.empty():
            time=self.t_queue.get()
            
            self.mtosT.emit(time)

    @Slot(str)
    def handle_broadcast(self,content):
        self.rpmsg2.writeEptDevice()
        time.sleep(1)
        self.rpmsg2.pollEptDeviceWithReadEvent()
        data = self.rpmsg2.readEptDevice()
        print("Speaker returned:", data)
        if data== "0" or data== "31":#如果flag有变，在这里修改data的判断值
        #if data== None:
            print("发送播报:",content)
            self.rpmsg2.writeSpeaker(content)
            #print("sent!")
            array=bytearray.fromhex(content)
            self.speaker.object_number, self.speaker.zebraline_number, self.speaker.traffic_light_number, self.speaker.stairs_number = SpkUnpackHead(array)
            self.speaker.objects = UnpackObject(array, self.speaker.object_number)
            self.speaker.zebralines = UnpackZebraline(array, self.speaker.zebraline_number)
            self.speaker.traffic_lights = UnpackTrafficLight(array, self.speaker.traffic_light_number)
            self.speaker.stairs = UnpackStairs(array, self.speaker.stairs_number)

            print(f'[播报1]: {SpeakObstacles(self.speaker.objects)}')   # todo：需要传入slave端：001 障碍物识别结果
            zebrastring, trafficstring = SpeakZebraTraffic(self.speaker.zebralines, self.speaker.traffic_lights) # todo：需要传入slave端：002 斑马线识别结果、003 交通灯识别结果
            print(f'[播报2,3]: {zebrastring} {trafficstring}')
            print(f'[播报4]" {SpeakStairs(self.speaker.stairs)}') # todo：需要传入slave端：004 楼梯识别结果
        else:
            print("wait for permission")
    	
