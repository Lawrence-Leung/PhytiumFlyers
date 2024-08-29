# -*- coding: utf-8 -*-
# roadDeviationDetector.py
# 功能：检测当前视障人士是否沿道路行走的判断功能
# by Lawrence Leung 2024
# 2024.5.10
import math
from queue import Queue
from typing import Tuple, Optional, List
import numpy as np


class DeviationDetector():
    def __init__(self):
        self.road_list = []
        self.recent_locations = Queue()

    # 清除当前的道路列表
    def ClearRoadList(self):
        self.road_list.clear()

    # 加载到道路列表中
    def LoadToRoadList(self, loaded_list: list):
        for item in loaded_list:
            if isinstance(item, tuple) and len(item) == 4:
                self.road_list.append(item)
            for i in item:
                i = float(i)    # 强制转换类型为float

    # 合并端点
    def MergePoints(self):
        n = len(self.road_list)
        merged = [list(item) for item in self.road_list]  # 将tuple转为list以便修改数据
        threshold = 4 * 10e-5  # 设置合并阈值

        # 创建一个点的列表
        points = {}
        for i in range(n):
            x1, y1, x2, y2 = merged[i]
            points[(x1, y1)] = (x1, y1)
            points[(x2, y2)] = (x2, y2)

        # 找出需要合并的点，并更新它们的坐标
        for key1 in list(points.keys()):
            for key2 in list(points.keys()):
                if key1 != key2 and Distance(points[key1], points[key2]) <= threshold:
                    # 合并两个点，并更新所有引用这两个点的坐标
                    merged_point = UpdatePoint(points[key1], points[key2])
                    points[key1] = merged_point
                    points[key2] = merged_point

        # 更新原始道路列表的点坐标
        for i in range(n):
            x1, y1, x2, y2 = merged[i]
            merged[i] = [points[(x1, y1)][0], points[(x1, y1)][1], points[(x2, y2)][0], points[(x2, y2)][1]]

        # 移除零长度的道路
        road_lists_updated = []
        for x1, y1, x2, y2 in merged:
            if not (x1 == x2 and y1 == y2):
                road_lists_updated.append((x1, y1, x2, y2))

        return road_lists_updated

    # 向队列添加位置信息
    def AddLocationToQueue(self, latest_location):
        # 首先检查输入的latest_location是否符合要求
        if isinstance(latest_location, tuple) and len(latest_location) == 2 and all(
                isinstance(item, float) for item in latest_location):
            # 检查队列长度并更新
            while self.recent_locations.qsize() >= 6:   #todo 需要调节的参数
                self.recent_locations.get()  # 如果队列长度大于或等于6，出队直到长度为6-1=5
            self.recent_locations.put(latest_location)  # 将新位置加入队列

    # 从地理坐标序列中，检测当前位置、运动方向
    # 注意：可能会生成(0, 0)
    def GetCurrentLocationAndDirection(self) -> Tuple[
        Optional[Tuple[float, float]], Optional[float]]:

        if self.recent_locations.qsize() < 2:
            return None, None

        # 将队列转换为列表，以取得所有的元素
        location_list = list(self.recent_locations.queue)
        location_list_length = math.floor(len(location_list) / 2)
        direction_list = [] # 收集各种方向的可能性，然后全部求平均

        if len(location_list) % 2 == 1: # 长度为奇数
            for i in range(location_list_length):
                direction_list.append(CalculateDirection(location_list[i], location_list[i + location_list_length + 1]))
        else:   # 长度为偶数
            for i in range(location_list_length):
                direction_list.append(CalculateDirection(location_list[i], location_list[i + location_list_length]))

        direction = np.average(direction_list)  # 全部求平均
        current_location = location_list[-1]

        return current_location, direction

    # 搜索当前位置与当前图内所有边（路径）的距离排序，必须得距离80米内，则判断为在路附近；若有多条道路（≥2）的距离均短于30米，则判断为路口
    # 输入：
    #   current_location(x, y)
    # 输出：
    #   isnearroad：bool, 是否为靠近道路模式，如果是，那么触发后续自动判断道路方向和当前方向是否一致or相反
    #   isnearcrossing：bool, 是否为靠近十字路口模式，如果是，那么不触发自动判断道路方向和当前方向的一致性；如果否，那么判断。
    #   nearestroad: tuple(x1, y1, x2, y2)：距离最近的道路，有可能是空的
    # 几个模式设定如下：
    # 1.offroad模式
    # 2.在路上、正常
    # 3.在路上、角度异常
    # 4.在路口
    def FindNearestRoads(self, current_location: Tuple[float, float]) -> \
    Tuple[bool, bool, Optional[Tuple[float, float, float, float]]]:

        distances = [(PointToLineDistance(current_location[0], current_location[1], x1, y1, x2, y2), (x1, y1, x2, y2))
                     for x1, y1, x2, y2 in self.road_list]
        distances.sort()

        if not distances:
            return False, False, None

        nearest_road_distance, nearest_road = distances[0]
        second_nearest_road_distance = distances[1][0] if len(distances) > 1 else float('inf')

        isnearroad = nearest_road_distance < 1.5e-4   # 125m：距离单条路的垂直距离阈值   #todo：需要调节的参数
        isnearcrossing = nearest_road_distance < 2e-4 and second_nearest_road_distance < 2e-4   #todo：需要调节的参数
            # 200m 距离两条路的垂直距离阈值

        return isnearroad, isnearcrossing, nearest_road if isnearroad else None

    # 比较是否在道路上
    # 输入：
    # nearest_road：tuple:(x1, y1, x2, y2)，最近的道路
    # current_direction: float，当前方向
    # 输出：
    # isalongroad：bool，是否沿着道路行走
    def CompareIsOnRoad(self, nearest_road: Tuple[float, float, float, float], current_direction: float):
        x1, y1, x2, y2 = nearest_road
        road_direction = CalculateDirection((x1, y1), (x2, y2))

        # 接下来判断当前方向和道路方向是否一致或差多少
        difference = road_direction - current_direction
        if difference >= 180:
            while (difference >= 180):
                difference = difference - 360
        if difference < -180:
            while (difference < -180):
                difference = difference + 360
        isalongroad = False
        if -30 < difference < 30 or -180 <= difference <= -150 or 150 <= difference < 180:  # todo：偏差30°
            isalongroad = True
        return isalongroad

    # 全套流程单个循环
    # 输入：
    #   test_road_lists：地理API搜索到的各种道路的list
    #   new_location：(x, y)：由GPS来的新的坐标
    # 输出：
    #   isnearroad：是否在道路附近，最优先判断
    #   isnearcrossing：是否在十字路口附近
    #   isalongroad：是否沿着道路行走
    #   nearest_road：最临近的道路
    def CompleteRoadDeviationProcess(self, test_road_lists:list, new_location:Tuple[float, float]):
        # step 1 导入已知的道路列表
        self.ClearRoadList()
        self.LoadToRoadList(test_road_lists)
        # step 2 合并端点
        self.MergePoints()
        # step 3 加入一个新的坐标，输出当前方向和当前位置
        self.AddLocationToQueue(new_location)
        current_location, direction = self.GetCurrentLocationAndDirection()
        if current_location is None or direction is None:
            current_location = (0.0, 0.0)
            direction = 0.0
        # step 4 找到最近的道路，并计算角度偏移
        isnearroad, isnearcrossing, nearest_road = self.FindNearestRoads(current_location)
        if nearest_road is None:
            nearest_road = (0, 0, 0, 0)
        isalongroad = self.CompareIsOnRoad(nearest_road, direction)
        return isnearroad, isnearcrossing, isalongroad, nearest_road


#--------------------------------------------------------------------------------------------------#
# 功能函数部分

# 求两点间距离
def Distance(p1, p2):
    return math.sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2)

# 更新点的位置到它们的几何中心
def UpdatePoint(p1, p2):
    return ((p1[0] + p2[0]) / 2, (p1[1] + p2[1]) / 2)

# 计算方向，以角度为单位，从point1到point2
# 输入：point1 & point2：(经度，纬度）格式
# 输出：角度，正北为正向，顺时针，范围从-180到180
def CalculateDirection(point1: Tuple[float, float], point2: Tuple[float, float]) -> float:
    dy = point2[1] - point1[1]
    dx = point2[0] - point1[0]
    angle = math.degrees(math.atan2(dy, dx))  # Calculate the angle in degrees
    north_angle = (450 - angle) % 360  # Convert to North-based degrees
    if north_angle > 180:
        north_angle -= 360  # [-180, 180)
    return north_angle

# 计算垂直距离：点(px, py) 与线(x1, y1, x2, y2)的距离。输出为float
def PointToLineDistance(px: float, py: float, x1: float, y1: float, x2: float, y2: float) -> float:
    if (x1, y1) == (x2, y2):
        return math.sqrt((px - x1) ** 2 + (py - y1) ** 2)
    # Line segment vector
    lx = x2 - x1
    ly = y2 - y1
    # Length of the line segment
    l_norm = math.sqrt(lx ** 2 + ly ** 2)
    # Projection length of point p onto the line
    projection_length = ((px - x1) * lx + (py - y1) * ly) / l_norm
    # Clamp projection to the segment
    projection_length = max(0, min(l_norm, projection_length))
    # Closest point on the line segment to point p
    closest_x = x1 + projection_length * lx / l_norm
    closest_y = y1 + projection_length * ly / l_norm
    # Distance from point p to the closest point on the line segment
    distance = math.sqrt((px - closest_x) ** 2 + (py - closest_y) ** 2)
    return distance

#--------------------------------------------------------------------------------------------------#
# 主函数，for debug only。
if __name__ == "__main__":
    test_road_lists = [
        (113.32742024872587, 23.096267632545597, 113.32302142594145, 23.0962133536494),   # 新港中路（西）
        (113.32746852848814, 23.09624789476786, 113.32980741474913, 23.096272566989867),    # 新港中路（东）
        (113.32738269779966, 23.096351518069685, 113.32739342663572, 23.09855226074607),   # 赤岗北路
        (113.32739879105375, 23.09629230476447, 113.32743097756193, 23.09346977360184)    # 赤岗路（南）
    ]

    deviation_detector = DeviationDetector()    # 创建class对象示例
    
    # # step 1 合并端点
    # # --------------------------------------
    # deviation_detector.MergePoints()
    # print(deviation_detector.road_list)

    # step 2 计算速度与当前的方向
    # 先提前输入一些location
    # --------------------------------------
    test_locations = [(113.32357396099852, 23.09613933693842),
                      (113.32375635121153, 23.09613933693842),
                      (113.32390655491636, 23.096104795792186),
                      (113.32391191933439, 23.09584326968273),     # 假设中间行人跑到了一个别的地方
                      (113.32392801258848, 23.096015975661313),    # 假设中间行人又跑到了一个别的地方
                      (113.32405139420317, 23.096203484758096),
                      (113.32426060650633, 23.096173878076005)]
    for loc in test_locations:
        # deviation_detector.AddLocationToQueue(loc)     # 将新坐标添加进去
        isnearroad, isnearcrossing, isalongroad, nearest_road = deviation_detector.CompleteRoadDeviationProcess(test_road_lists, loc)
        print(f'IsNearRoad = {isnearroad}, IsNearCrossing = {isnearcrossing}, IsAlongRoad = {isalongroad}, NearestRoad = {nearest_road}')
    # # 尝试加入一个新的坐标，观察队列变化
    # new_location = (113.32718957875059, 23.09615907473295)
    # deviation_detector.AddLocationToQueue(new_location)    # 将新坐标添加进去
    # # 输出队列中的元素以验证
    # # print(deviation_detector.recent_locations.queue)
    # current_location, direction = deviation_detector.GetCurrentLocationAndDirection()
    # print(f'Current Location: {current_location}, Direction: {direction} deg')
    # road_direction = CalculateDirection((113.32742024872587, 23.096267632545597), (113.32302142594145, 23.0962133536494))
    # print(f'Road Direction: {road_direction}')
    #
    # # step 3 计算位置
    # # --------------------------------------
    # isnearroad, isnearcrossing, nearest_road = deviation_detector.FindNearestRoads(current_location)
    # print(f'IsNearRoad = {isnearroad}, IsNearCrossing = {isnearcrossing}, NearestRoad = {nearest_road}')
    #
    # # step 4 计算角度偏移
    # # --------------------------------------
    # isalongroad = deviation_detector.CompareIsOnRoad(nearest_road, direction)
    # print(f'IsAlongRoad = {isalongroad}')
