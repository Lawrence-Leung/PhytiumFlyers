# -*- coding: utf-8 -*-
# detectResultPackUp.py
# 将所有的检测好的数据，按照特定格式打包好
# by Lawrence Leung 2024
# 飞腾风驰队
# 2024.5.4 更新

import struct

'''
帧定义：最多1004byte

帧开头：
[0] uint8 0xAA 字头
[1] uint8 物体数量（除了zebraline, traffic light, stairs以及非必要物体外），max=78
[2] uint8 斑马线数量（zebraline），max=5
[3] uint8 红绿灯数量（traffic light），max=10
[4] uint8 楼梯数量（stairs），max=5

后续的物体帧：
[0] uint8 0x1A 字头
[1~2] uint16 中心x
[3~4] uint16 中心y
[5] uint8 class类别号
[6] uint8 distance（实际距离*10，保留uint8）
[7] int8 verticalangle（垂直角度，-127~127，保留uint8取整）
[8~9] int16 horizontalangle（水平角度，实际角度*10，保留int16取整）
[10] uint8 0x1B 字尾

后续的斑马线帧：
[0] uint8 0x2A 字头
[1~2] uint16 中心x
[3~4] uint16 中心y
[5~6] int16 deg（实际角度*10，保留int16取整）（注意：90deg和0deg没有特别区别）
[7] uint8 0x2B 字尾

后续的红绿灯帧：
[0] uint8 0x3A 字头
[1~2] uint16 中心x
[3~4] uint16 中心y
[5] uint8 亮灯状态：undefined:0, red:1, green:2
[6] uint8 0x3B 字尾

后续的楼梯帧：
[0] uint8 0x4A 字头
[1~2] uint16 中心x
[3~4] uint16 中心y
[5] uint8 楼梯级数
[6] uint8 0x4B 字尾

最后加入字符：
[0] uint8 0xBB 字尾

总共可以塞入1024字节数据中去。

注意：一般可以每一个目标检测帧发送一次。这样比较方便处理。而不是每一个快速帧发送一次
'''


# 专门将所有的数据产出打包成字节流（1024byte以内），这个字节流后续将会用作发送给Slave端。当然是跳帧的，不是逐帧的。
# countset：每多少帧发送一次
class PackUpResultClass:
    def __init__(self, countset=8):
        self.counting = 0  # 计数，用于统计当前的上升值
        self.countset = countset  # 计数，用于统计所设置的值

        self.objectlist = []  # 四个列表，当然是临时存放的
        self.zebralinelist = []
        self.trafficlist = []
        self.stairslist = []

        self.string = bytearray()  # 最终拿去处理的string

    # 全部清空
    def clearAllLists(self):
        self.stairslist.clear()
        self.zebralinelist.clear()
        self.trafficlist.clear()
        self.objectlist.clear()
        # print("PackUpResultClass.clearAllLists") #debug 专用

    # 全部加入到List，而不是到String中
    def appendAllToList(self, stairs, zebralines, traffics, objectslists):
        if isinstance(stairs, list) and isinstance(zebralines, list) and isinstance(traffics, list) and isinstance(
                objectslists, list):
            self.stairslist = stairs
            self.zebralinelist = zebralines
            self.trafficlist = traffics
            self.objectlist = objectslists
            # print("[PACKUP] All added!")    #debug 专用
        else:
            print("[PACKUP] Something went wrong!")  # debug 专用

    # 清除整个String
    def clearAllString(self):
        self.string = bytearray()

    # 输出整个String
    def outFullString(self):
        print("[PACKUP] Full String is: ", self.string)
        return self.string

    # 添加字头
    def addStringHead(self):
        tmpstring = bytearray()

        tmpstring += struct.pack('B', 0xAA)  # [0] uint8 0xAA 字头
        tmpstring += struct.pack('B', int(len(self.objectlist)))  # [1] uint8 物体数量
        tmpstring += struct.pack('B', int(len(self.zebralinelist)))  # [2] uint8 斑马线数量
        tmpstring += struct.pack('B', int(len(self.trafficlist)))  # [3] uint8 红绿灯数量
        tmpstring += struct.pack('B', int(len(self.stairslist)))  # [4] uint8 楼梯数量
        self.string += tmpstring

    # 添加物体帧
    def addObjectListToSting(self):
        tmpstring = bytearray()
        for obj in self.objectlist:
            if len(obj) == 6:
                x, y, cl, dist10, angleheight, anglehorizon10 = obj  # 注意！
                tmpstring += struct.pack('B', 0x1A)  # [0] uint8 字头
                tmpstring += struct.pack('H', x)  # [1~2] uint16 x
                tmpstring += struct.pack('H', y)  # [3~4] uint16 y
                tmpstring += struct.pack('B', cl)  # [5] uint8 class
                dist10 = dist10 % 256
                tmpstring += struct.pack('B', dist10)  # [6] uint8 dist*10
                angleheight = max(-127, min(angleheight, 127))
                tmpstring += struct.pack('b', angleheight)  # [7] int8 verangle
                anglehorizon10 = max(-32767, min(anglehorizon10, 32767))
                tmpstring += struct.pack('h', anglehorizon10)  # [8~9] int16 horangle*10
                tmpstring += struct.pack('B', 0x1B)  # [10] uint8 字尾
        self.string += tmpstring

    # 添加斑马线帧
    def addZebraListToString(self):
        tmpstring = bytearray()
        for obj in self.zebralinelist:
            if len(obj) == 3:
                x, y, deg = obj  # 注意！
                tmpstring += struct.pack('B', 0x2A)  # [0] uint8 字头
                tmpstring += struct.pack('H', x)  # [1~2] uint16 中心x
                tmpstring += struct.pack('H', y)  # [3~4] uint16 中心y
                deg = max(-32767, min(deg, 32767))
                tmpstring += struct.pack('h', deg)  # [5~6] int16 deg
                tmpstring += struct.pack('B', 0x2B)  # [7] uint8 字尾
        self.string += tmpstring

    # 添加红绿灯帧
    def addTrafficToString(self):
        tmpstring = bytearray()
        for obj in self.trafficlist:
            if len(obj) == 3:
                x, y, color = obj  # 注意！
                tmpstring += struct.pack('B', 0x3A)  # [0] uint8 字头
                tmpstring += struct.pack('H', x)  # [1~2] uint16 中心x
                tmpstring += struct.pack('H', y)  # [3~4] uint16 中心y
                tmpstring += struct.pack('B', color)  # [5] 亮灯状态
                tmpstring += struct.pack('B', 0x3B)  # [6] 字尾
        self.string += tmpstring

    def addStairToString(self):
        tmpstring = bytearray()
        for obj in self.stairslist:
            if len(obj) == 3:
                x, y, number = obj  # 注意！
                tmpstring += struct.pack('B', 0x4A) # [0] uint8 字头
                tmpstring += struct.pack('H', x)    # [1~2] uint16 中心x
                tmpstring += struct.pack('H', y)    # [3~4] uint16 中心y
                tmpstring += struct.pack('B', number)   # [5] 楼梯级数
                tmpstring += struct.pack('B', 0x4B)
        self.string += tmpstring

    # 添加字尾
    def addStringTail(self):
        self.string += struct.pack('B', 0xBB)  # 最后加入字符：uint8 0xBB 字尾

    # 一键完成所有步骤
    # 如果string非None，那么可以传出去，否则计数再传
    def ComplexFromListsToString(self, stairs, zebralines, traffics, objectslists):
        if self.counting == self.countset:
            self.counting = 0
            self.clearAllString()
            self.appendAllToList(stairs, zebralines, traffics, objectslists)

            self.addStringHead()    # 加字头
            self.addObjectListToSting()  # 加物体帧
            self.addZebraListToString()  # 加斑马线帧
            self.addTrafficToString()  # 加红绿灯帧
            self.addStairToString() # 加楼梯帧
            self.addStringTail()  # 加字尾

            # self.outFullString()  # 打印输出，可debug
            self.clearAllLists()

            return self.string  # 返回打包好的东西
        else:
            self.counting += 1
            return None
