# -*- coding: utf-8 -*-
# speakout.py
# 语音输出
# by Lawrence Leung 2024
# 飞腾风驰队
# 2024.5.4 更新
import math

# 用于存储的结构体，这里暂时用Python代替着，后续如果有需要就改成C
class Speaker:
    def __init__(self):
        self.object_number = int(0)
        self.zebraline_number = int(0)
        self.traffic_light_number = int(0)
        self.stairs_number = int(0)

        self.objects = []
        self.zebralines = []
        self.traffic_lights = []
        self.stairs = []

# 将头字节进行解包
def SpkUnpackHead(data: bytearray):
    if data[0] != 0xaa:
        print('[SPEAKER] 包头错误')
        return 0, 0, 0, 0

    object_number = data[1]
    zebraline_number = data[2]
    traffic_light_number = data[3]
    stairs_number = data[4]

    return object_number, zebraline_number, traffic_light_number, stairs_number

# 提取物体的数量
# 输入：data 原始字节流；object_number：原始的物体数量
def UnpackObject(data: bytearray, object_number: int):
    objects = []
    i = 0
    while i < len(data) - 1:
        # Search for start and end marker
        if data[i] == 0x1A:
            end_idx = data.find(0x1B, i)
            # Check if end marker found and length is 11 bytes
            if end_idx != -1 and end_idx - i == 10:
                segment = data[i + 1:end_idx]  # Exclude markers for processing
                if len(segment) == 9:
                    x = int.from_bytes(segment[0:2], 'big')
                    y = int.from_bytes(segment[2:4], 'big')
                    class_type = segment[4]
                    distance = float(segment[5]) / 10.0
                    ver = int.from_bytes(segment[6:7], 'big', signed=True)
                    hor = float(int.from_bytes(segment[7:9], 'big', signed=True)) / 10.0
                    objects.append((x, y, class_type, distance, ver, hor))
                i = end_idx  # Move index past the end marker
        i += 1

    # 这里有bug!
    # if len(objects) != object_number and len(objects) != object_number - 1:
    #    print('[SPEAKER] 物体数量错误, 数量为：', len(objects))
    #    return []

    # print(objects)
    # print('[SPEAKER] 物体数量正确')
    return objects

# 提取斑马线数量
# 输入：data 原始字节流；object_number：原始的物体数量
def UnpackZebraline(data: bytearray, object_number: int):
    result = []
    i = 0
    while i < len(data) - 1:
        if data[i] == 0x2A:
            end_idx = data.find(0x2B, i)
            # Check if end marker found and length is 8 bytes
            if end_idx != -1 and end_idx - i == 7:
                segment = data[i + 1:end_idx]  # Exclude markers for processing
                if len(segment) == 6:
                    x = int.from_bytes(segment[0:2], 'big')
                    y = int.from_bytes(segment[2:4], 'big')
                    deg = float(int.from_bytes(segment[4:6], 'big', signed=True)) / 10.0
                    result.append((x, y, deg))
                i = end_idx  # Move index past the end marker
        i += 1

    if len(result) != object_number:
        print('[SPEAKER] 斑马线数量错误')
        return []

    # print(objects)
    # print('[SPEAKER] 斑马线数量正确')
    return result

# 提取红绿灯数量
# 输入：data 原始字节流；object_number：原始的物体数量
def UnpackTrafficLight(data: bytearray, object_number: int):
    result = []
    i = 0
    while i < len(data) - 1:
        if data[i] == 0x3A:
            end_idx = data.find(0x3B, i)
            # Check if end marker found and length is 7 bytes
            if end_idx != -1 and end_idx - i == 6:
                segment = data[i + 1:end_idx]  # Exclude markers for processing
                if len(segment) == 5:
                    x = int.from_bytes(segment[0:2], 'big')
                    y = int.from_bytes(segment[2:4], 'big')
                    lightstatus = segment[4]
                    result.append((x, y, lightstatus))
                i = end_idx  # Move index past the end marker
        i += 1

    if len(result) != object_number:
        print('[SPEAKER] 交通灯数量错误')
        return []

    # print(objects)
    # print('[SPEAKER] 交通灯数量正确')
    return result

# 提取楼梯数量
# 输入：data 原始字节流；object_number：原始的物体数量
def UnpackStairs(data: bytearray, object_number: int):
    result = []
    i = 0
    while i < len(data) - 1:
        if data[i] == 0x4A:
            end_idx = data.find(0x4B, i)
            # Check if end marker found and length is 7 bytes
            if end_idx != -1 and end_idx - i == 6:
                segment = data[i + 1:end_idx]  # Exclude markers for processing
                if len(segment) == 5:
                    x = int.from_bytes(segment[0:2], 'big')
                    y = int.from_bytes(segment[2:4], 'big')
                    stairsnumbers = segment[4]
                    result.append((x, y, stairsnumbers))
                i = end_idx  # Move index past the end marker
        i += 1

    if len(result) != object_number:
        print('[SPEAKER] 楼梯数量错误')
        return []

    # print(objects)
    # print('[SPEAKER] 楼梯数量正确')
    return result

# 将障碍物说出来
# 输入：一个list，里面有所有的障碍物的tuple(x, y, class, distance, ver, hor)
# 输出：string
def SpeakObstacles(objects: list):
    # Variable initialization
    dist1 = dist2 = dist3 = dist4 = 0
    closest_class = []
    is_total_crowded = False
    most_crowded = 0

    # Sort objects by distance
    objects.sort(key=lambda obj: obj[3])

    # Categorize objects
    for x, y, obj_class, distance, ver, hor in objects:
        if len(closest_class) < 5 and abs(ver) < 45:
            closest_class.append(obj_class)
        if 0 <= distance <= 2.5:
            dist1 += 1
        elif 2.5 < distance <= 5:
            dist2 += 1
        elif 5 < distance <= 7.5:
            dist3 += 1
        elif 7.5 < distance <= 10:
            dist4 += 1

    # Check crowdedness
    total_objects = dist1 + dist2 + dist3 + dist4
    print(f'[SPEAKER] total_objects = {total_objects}')
    if total_objects >= 10:
        is_total_crowded = True

    # Determine the most crowded distance range
    most_crowded = max(dist1, dist2, dist3, dist4)
    if most_crowded == dist1:
        most_crowded = 1
    elif most_crowded == dist2:
        most_crowded = 2
    elif most_crowded == dist3:
        most_crowded = 3
    elif most_crowded == dist4:
        most_crowded = 4

    # Formulate output strings
    str1 = "前方拥挤" if is_total_crowded else "前方宽松"
    distance_descriptions = {
        1: "近距多障碍",
        2: "中距多障碍",
        3: "远距多障碍",
        4: "极远多障碍"
    }
    str2 = distance_descriptions[most_crowded]
    str3 = ' '.join(map(str, closest_class))

    # Final output
    return f"{str1}, {str2}, {str3}"

# 将斑马线说出来
# 输入：两个list:
# 	zebralist:由大量的(x, y, deg)元组组成
#	trafficlist:由大量的(x, y, lightstatus)元组组成
# 输出：两段文字，string类型：
# 	zebrastring, trafficstring
def SpeakZebraTraffic(zebralist: list, trafficlist: list):
    zebrastring = ""
    trafficstring = ""

    # Processing Zebralist
    if zebralist:
        zebralist.sort(key=lambda obj: obj[1], reverse=True)
        first_zebra = zebralist[0]
        is_zebraline_on_left = first_zebra[0] < 320
        is_zebraline_goes_left = first_zebra[2] < 0
        zebra_x, zebra_y = first_zebra[0], first_zebra[1]

        str1 = f"识别到{len(zebralist)}条斑马线"
        str2 = "脚下斑马线在您左侧" if is_zebraline_on_left else "脚下斑马线在您右侧"
        str3 = "指向左前方" if is_zebraline_goes_left else "指向右前方"

        zebrastring = f"{str1}，{str2}，{str3}"

    # Processing Trafficlist
    if trafficlist:
        traffic_number = len(trafficlist)
        trafficlist.sort(key=lambda obj: obj[1], reverse=True)
        # Remove tuples with lightstatus == 0
        trafficlist = [t for t in trafficlist if t[2] != 0]
        nearest_traffic_signal = 0

        if trafficlist:
            if zebralist:
                # Find nearest traffic signal to the first zebraline position
                nearest = min(trafficlist, key=lambda t: math.hypot(t[0] - zebra_x, t[1] - zebra_y))
                nearest_traffic_signal = nearest[2]

        str4 = f"识别到{traffic_number}个交通灯"
        lightstatus_dict = {1: "最近距离交通灯为红灯", 2: "最近距离交通灯为绿灯"}
        str5 = lightstatus_dict.get(nearest_traffic_signal, "")

        trafficstring = f"{str4}，{str5}"

    return zebrastring, trafficstring

def SpeakStairs(stairs_list: list):
    if not stairs_list:
        return ""  # Return an empty string if the list is empty

        # Sort the list by y in descending order (largest y first)
    stairs_list.sort(key=lambda obj: obj[1], reverse=True)

    # Extract the first tuple as it will have the highest y (closest)
    nearest_stair_x, _, nearest_stair_number = stairs_list[0]

    # Building descriptive strings
    str1 = f"识别到{len(stairs_list)}处阶梯"
    if nearest_stair_x < 213:
        str2 = "最近阶梯在您左侧"
    elif 213 <= nearest_stair_x < 427:
        str2 = "最近阶梯在您前方"
    else:
        str2 = "最近阶梯在您右侧"

    str3 = f"阶梯数{nearest_stair_number}"

    # Combine the string parts to form the final output
    output_string = f"{str1}，{str2}，{str3}"
    return output_string

# 检测是否存在道路偏移
# 输入：
#   isnearroad：首先判断这个，是否在道路附近
#   isnearcrossing：是否位于道路交汇口附近
#   isalongroad：是否沿着道路行走（阈值30°以内）
def SpeakDeviationRoad(isalongroad:bool, isnearroad:bool, isnearcrossing:bool):
    if not isnearroad:
        # str = "未检测到临近道路"
        str = ''
    else:
        if isnearcrossing:
            str = "您位于道路交汇路口附近"
        else:
            if isalongroad:
                str = "识别到道路，您正在沿路方向30度内行走"
            else:
                str = "识别到道路，您正在偏移道路方向行走"
    return str
