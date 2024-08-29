import time
import numpy as np
from PySide6.QtCore import Signal, Slot 
from drivers.rpmsg import RPMsg
from drivers.rpmsg import *
from drivers.speakout import *
from classes.roadDeviationDetector import *
from classes.gpsdecode import *
from UIFunctions import *
import time
import array
from queue import Empty


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
        self.road_queue=Queue()
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
            #DHT11
                start_time=time.time()
                self.rpmsg0.writeEptDevice() # 将已有的数据写入到端点里面去
                end_ime=time.time()
                self.time1=end_ime-start_time
                time_formatted=round(self.time1*1000,2)
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
                    if result and result!=(0.0,0.0):
                        print("[OPENAMP] GPS received data: ", result)
                        current_location=result
                        isnearroad, isnearcrossing, isalongroad, nearest_road = self.deviation_detector.CompleteRoadDeviationProcess(self.test_road_lists, current_location)  # 分析行人是否沿着道路行走
                        road_string=f'{int(isalongroad)}'+f'{int(isnearcrossing)}'
                        self.rpmsg1.writeSpeaker(road_string)
                        formatted_address,citycode=decode_address(result[0],result[1],self.api_key)
                        weather_data=get_weather(self.api_key,citycode)
                        current_weather = weather_data['forecasts'][0]['casts'][0]
                        weather_result=current_weather['dayweather']self.gps_queue.put((str(formatted_result),str(formatted_address),str(weather_result)))
                        time.sleep(1)

            # #JY61P
                time.sleep(1)    # 睡眠1秒
                self.rpmsg3.writeEptDevice() # 将已有的数据写入到端点里面去
                print("[OPENAMP] WAIT data from JY61P")
                self.rpmsg3.pollEptDeviceWithReadEvent()

                print("[OPENAMP] read data from JY61P")
            # 以下是数据解析
                data = self.rpmsg3.readEptDevice()
                if data is not None:
                    if data[2:]=='30':
                        status="未摔倒"
                    else:
                        status="摔倒"

                    if data[:2]=='30':
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
            status,cam=tuple_result2
            self.down_update.emit(status)
            self.cam_update.emit(cam)
            
        if not self.t_queue.empty():
            time=self.t_queue.get()
            
            self.mtosT.emit(time)

    @Slot(str)
    def handle_broadcast(self,content):
        self.rpmsg2.writeEptDevice()
        time.sleep(1)
        self.rpmsg2.pollEptDeviceWithReadEvent()
        data = self.rpmsg2.readEptDevice()
        if data== "0" or data== "31":#如果flag有变，在这里修改data的判断值
            self.rpmsg2.writeSpeaker(content)
        else:
            print("wait for permission")
    	
