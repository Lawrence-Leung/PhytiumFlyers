# -*- coding: utf-8 -*-
# yolov5v8.py
# 基于YOLOv8算法目标框，实现单目测距转换
# by Lawrence Leung 2024

import os
import cv2
import numpy as np
import onnxruntime
import time
from classes import coordination

# YOLOv8n的OpenImagesv7数据集的800个label classes，在此列出来
CLASSESv8n = ['Accordion', 'Adhesive tape', 'Aircraft', 'Airplane', 'Alarm clock', 'Alpaca', 'Ambulance', 'Animal', 'Ant',
           'Antelope', 'Apple', 'Armadillo', 'Artichoke', 'Auto part', 'Axe', 'Backpack', 'Bagel', 'Baked goods',
           'Balance beam', 'Ball', 'Balloon', 'Banana', 'Band-aid', 'Banjo', 'Barge', 'Barrel', 'Baseball bat',
           'Baseball glove', 'Bat (Animal)', 'Bathroom accessory', 'Bathroom cabinet', 'Bathtub', 'Beaker', 'Bear',
           'Bed', 'Bee', 'Beehive', 'Beer', 'Beetle', 'Bell pepper', 'Belt', 'Bench', 'Bicycle', 'Bicycle helmet',
           'Bicycle wheel', 'Bidet', 'Billboard', 'Billiard table', 'Binoculars', 'Bird', 'Blender', 'Blue jay', 'Boat',
           'Bomb', 'Book', 'Bookcase', 'Boot', 'Bottle', 'Bottle opener', 'Bow and arrow', 'Bowl', 'Bowling equipment',
           'Box', 'Boy', 'Brassiere', 'Bread', 'Briefcase', 'Broccoli', 'Bronze sculpture', 'Brown bear', 'Building',
           'Bull', 'Burrito', 'Bus', 'Bust', 'Butterfly', 'Cabbage', 'Cabinetry', 'Cake', 'Cake stand', 'Calculator',
           'Camel', 'Camera', 'Can opener', 'Canary', 'Candle', 'Candy', 'Cannon', 'Canoe', 'Cantaloupe', 'Car',
           'Carnivore', 'Carrot', 'Cart', 'Cassette deck', 'Castle', 'Cat', 'Cat furniture', 'Caterpillar', 'Cattle',
           'Ceiling fan', 'Cello', 'Centipede', 'Chainsaw', 'Chair', 'Cheese', 'Cheetah', 'Chest of drawers', 'Chicken',
           'Chime', 'Chisel', 'Chopsticks', 'Christmas tree', 'Clock', 'Closet', 'Clothing', 'Coat', 'Cocktail',
           'Cocktail shaker', 'Coconut', 'Coffee', 'Coffee cup', 'Coffee table', 'Coffeemaker', 'Coin', 'Common fig',
           'Common sunflower', 'Computer keyboard', 'Computer monitor', 'Computer mouse', 'Container', 'Convenience store',
           'Cookie', 'Cooking spray', 'Corded phone', 'Cosmetics', 'Couch', 'Countertop', 'Cowboy hat', 'Crab', 'Cream',
           'Cricket ball', 'Crocodile', 'Croissant', 'Crown', 'Crutch', 'Cucumber', 'Cupboard', 'Curtain', 'Cutting board',
           'Dagger', 'Dairy Product', 'Deer', 'Desk', 'Dessert', 'Diaper', 'Dice', 'Digital clock', 'Dinosaur',
           'Dishwasher', 'Dog', 'Dog bed', 'Doll', 'Dolphin', 'Door', 'Door handle', 'Doughnut', 'Dragonfly', 'Drawer',
           'Dress', 'Drill (Tool)', 'Drink', 'Drinking straw', 'Drum', 'Duck', 'Dumbbell', 'Eagle', 'Earrings',
           'Egg (Food)', 'Elephant', 'Envelope', 'Eraser', 'Face powder', 'Facial tissue holder', 'Falcon',
           'Fashion accessory', 'Fast food', 'Fax', 'Fedora', 'Filing cabinet', 'Fire hydrant', 'Fireplace', 'Fish',
           'Flag', 'Flashlight', 'Flower', 'Flowerpot', 'Flute', 'Flying disc', 'Food', 'Food processor', 'Football',
           'Football helmet', 'Footwear', 'Fork', 'Fountain', 'Fox', 'French fries', 'French horn', 'Frog', 'Fruit',
           'Frying pan', 'Furniture', 'Garden Asparagus', 'Gas stove', 'Giraffe', 'Girl', 'Glasses', 'Glove', 'Goat',
           'Goggles', 'Goldfish', 'Golf ball', 'Golf cart', 'Gondola', 'Goose', 'Grape', 'Grapefruit', 'Grinder',
           'Guacamole', 'Guitar', 'Hair dryer', 'Hair spray', 'Hamburger', 'Hammer', 'Hamster', 'Hand dryer',
           'Handbag', 'Handgun', 'Harbor seal', 'Harmonica', 'Harp', 'Harpsichord', 'Hat', 'Headphones', 'Heater',
           'Hedgehog', 'Helicopter', 'Helmet', 'High heels', 'Hiking equipment', 'Hippopotamus', 'Home appliance',
           'Honeycomb', 'Horizontal bar', 'Horse', 'Hot dog', 'House', 'Houseplant', 'Human arm', 'Human beard',
           'Human body', 'Human ear', 'Human eye', 'Human face', 'Human foot', 'Human hair', 'Human hand', 'Human head',
           'Human leg', 'Human mouth', 'Human nose', 'Humidifier', 'Ice cream', 'Indoor rower', 'Infant bed', 'Insect',
           'Invertebrate', 'Ipod', 'Isopod', 'Jacket', 'Jacuzzi', 'Jaguar (Animal)', 'Jeans', 'Jellyfish', 'Jet ski',
           'Jug', 'Juice', 'Kangaroo', 'Kettle', 'Kitchen & dining room table', 'Kitchen appliance', 'Kitchen knife',
           'Kitchen utensil', 'Kitchenware', 'Kite', 'Knife', 'Koala', 'Ladder', 'Ladle', 'Ladybug', 'Lamp', 'Land vehicle',
           'Lantern', 'Laptop', 'Lavender (Plant)', 'Lemon', 'Leopard', 'Light bulb', 'Light switch', 'Lighthouse',
           'Lily', 'Limousine', 'Lion', 'Lipstick', 'Lizard', 'Lobster', 'Loveseat', 'Luggage and bags', 'Lynx', 'Magpie',
           'Mammal', 'Man', 'Mango', 'Maple', 'Maracas', 'Marine invertebrates', 'Marine mammal', 'Measuring cup',
           'Mechanical fan', 'Medical equipment', 'Microphone', 'Microwave oven', 'Milk', 'Miniskirt', 'Mirror',
           'Missile', 'Mixer', 'Mixing bowl', 'Mobile phone', 'Monkey', 'Moths and butterflies', 'Motorcycle', 'Mouse',
           'Muffin', 'Mug', 'Mule', 'Mushroom', 'Musical instrument', 'Musical keyboard', 'Nail (Construction)',
           'Necklace', 'Nightstand', 'Oboe', 'Office building', 'Office supplies', 'Orange', 'Organ (Musical Instrument)',
           'Ostrich', 'Otter', 'Oven', 'Owl', 'Oyster', 'Paddle', 'Palm tree', 'Pancake', 'Panda', 'Paper cutter',
           'Paper towel', 'Parachute', 'Parking meter', 'Parrot', 'Pasta', 'Pastry', 'Peach', 'Pear', 'Pen', 'Pencil case',
           'Pencil sharpener', 'Penguin', 'Perfume', 'Person', 'Personal care', 'Personal flotation device', 'Piano',
           'Picnic basket', 'Picture frame', 'Pig', 'Pillow', 'Pineapple', 'Pitcher (Container)', 'Pizza', 'Pizza cutter',
           'Plant', 'Plastic bag', 'Plate', 'Platter', 'Plumbing fixture', 'Polar bear', 'Pomegranate', 'Popcorn', 'Porch',
           'Porcupine', 'Poster', 'Potato', 'Power plugs and sockets', 'Pressure cooker', 'Pretzel', 'Printer', 'Pumpkin',
           'Punching bag', 'Rabbit', 'Raccoon', 'Racket', 'Radish', 'Ratchet (Device)', 'Raven', 'Rays and skates',
           'Red panda', 'Refrigerator', 'Remote control', 'Reptile', 'Rhinoceros', 'Rifle', 'Ring binder', 'Rocket',
           'Roller skates', 'Rose', 'Rugby ball', 'Ruler', 'Salad', 'Salt and pepper shakers', 'Sandal', 'Sandwich',
           'Saucer', 'Saxophone', 'Scale', 'Scarf', 'Scissors', 'Scoreboard', 'Scorpion', 'Screwdriver', 'Sculpture',
           'Sea lion', 'Sea turtle', 'Seafood', 'Seahorse', 'Seat belt', 'Segway', 'Serving tray', 'Sewing machine',
           'Shark', 'Sheep', 'Shelf', 'Shellfish', 'Shirt', 'Shorts', 'Shotgun', 'Shower', 'Shrimp', 'Sink', 'Skateboard',
           'Ski', 'Skirt', 'Skull', 'Skunk', 'Skyscraper', 'Slow cooker', 'Snack', 'Snail', 'Snake', 'Snowboard', 'Snowman',
           'Snowmobile', 'Snowplow', 'Soap dispenser', 'Sock', 'Sofa bed', 'Sombrero', 'Sparrow', 'Spatula', 'Spice rack',
           'Spider', 'Spoon', 'Sports equipment', 'Sports uniform', 'Squash (Plant)', 'Squid', 'Squirrel', 'Stairs',
           'Stapler', 'Starfish', 'Stationary bicycle', 'Stethoscope', 'Stool', 'Stop sign', 'Strawberry', 'Street light',
           'Stretcher', 'Studio couch', 'Submarine', 'Submarine sandwich', 'Suit', 'Suitcase', 'Sun hat', 'Sunglasses',
           'Surfboard', 'Sushi', 'Swan', 'Swim cap', 'Swimming pool', 'Swimwear', 'Sword', 'Syringe', 'Table',
           'Table tennis racket', 'Tablet computer', 'Tableware', 'Taco', 'Tank', 'Tap', 'Tart', 'Taxi', 'Tea',
           'Teapot', 'Teddy bear', 'Telephone', 'Television', 'Tennis ball', 'Tennis racket', 'Tent', 'Tiara',
           'Tick', 'Tie', 'Tiger', 'Tin can', 'Tire', 'Toaster', 'Toilet', 'Toilet paper', 'Tomato', 'Tool',
           'Toothbrush', 'Torch', 'Tortoise', 'Towel', 'Tower', 'Toy', 'Traffic light', 'Traffic sign', 'Train',
           'Training bench', 'Treadmill', 'Tree', 'Tree house', 'Tripod', 'Trombone', 'Trousers', 'Truck', 'Trumpet',
           'Turkey', 'Turtle', 'Umbrella', 'Unicycle', 'Van', 'Vase', 'Vegetable', 'Vehicle', 'Vehicle registration plate',
           'Violin', 'Volleyball (Ball)', 'Waffle', 'Waffle iron', 'Wall clock', 'Wardrobe', 'Washing machine',
           'Waste container', 'Watch', 'Watercraft', 'Watermelon', 'Weapon', 'Whale', 'Wheel', 'Wheelchair', 'Whisk',
           'Whiteboard', 'Willow', 'Window', 'Window blind', 'Wine', 'Wine glass', 'Wine rack', 'Winter melon', 'Wok',
           'Woman', 'Wood-burning stove', 'Woodpecker', 'Worm', 'Wrench', 'Zebra', 'Zucchini']

# COCO数据集（YOLOv5）

CLASSESv5n = ['person', 'bicycle', 'car', 'motorcycle', 'airplane', 'bus', 'train', 'truck', 'boat', 'traffic light',
           'fire hydrant', 'stop sign', 'parking meter', 'bench', 'bird', 'cat', 'dog', 'horse', 'sheep', 'cow',
           'elephant', 'bear', 'zebra', 'giraffe', 'backpack', 'umbrella', 'handbag', 'tie', 'suitcase', 'frisbee',
           'skis', 'snowboard', 'sports ball', 'kite', 'baseball bat', 'baseball glove', 'skateboard', 'surfboard',
           'tennis racket', 'bottle', 'wine glass', 'cup', 'fork', 'knife', 'spoon', 'bowl', 'banana', 'apple',
           'sandwich', 'orange', 'broccoli', 'carrot', 'hot dog', 'pizza', 'donut', 'cake', 'chair', 'couch',
           'potted plant', 'bed', 'dining table', 'toilet', 'tv', 'laptop', 'mouse', 'remote', 'keyboard', 'cell phone',
           'microwave', 'oven', 'toaster', 'sink', 'refrigerator', 'book', 'clock', 'vase', 'scissors', 'teddy bear',
           'hair drier', 'toothbrush']

ClassesTEST = ['stairs', 'null', 'null', 'null', 'null', 'null', 'null']

# 通用类，可以同时支持输入YOLOv5和YOLOv8程序
class YOLOV5V8():
    # 构造函数
    # 输入：onnxpath，字符串，带有.onnx格式YOLOv8文件
    def __init__(self, onnxpath: object, isType='YOLOV8') -> object:
        self.onnx_session = onnxruntime.InferenceSession(onnxpath)  # ONNX框架的Session
        self.input_name = self.getInputName()
        self.output_name = self.getOutputName()
        self.coord_conversion = coordination.CoordConvertion()  # 内置距离推理模块
        self.map = coordination.LittleMap()     # 内置小地图
        self.conf_threshold = 0.5   # 置信度阈值
        self.iou_threshold = 0.5    # IOU阈值
        if isType == 'TEST':
            self.CLASSES = ClassesTEST
        elif isType == 'YOLOV5':
            self.CLASSES = CLASSESv5n  # 使用YOLOv5的COCO80数据集
        else:
            self.CLASSES = CLASSESv8n  # 使用YOLOv8的OpenImagesv7数据集

        # cv2.namedWindow("Map", cv2.WINDOW_NORMAL)   # 创建“小地图”窗口

    def getInputName(self) -> object:
        input_name = []
        for node in self.onnx_session.get_inputs():
            input_name.append(node.name)
        return input_name

    def getOutputName(self) -> object:
        output_name = []
        for node in self.onnx_session.get_outputs():
            output_name.append(node.name)
        return output_name

    def getInputFeed(self, img_tensor: object) -> object:
        input_feed = {}
        for name in self.input_name:
            input_feed[name] = img_tensor
        return input_feed

    # 核心推理函数
    # 输入：frame，OpenCV2格式，输入图像帧
    # 输出：
    #      预测结果：一个tuple，(boxes, scores, classes)
    #      or_img：；拉伸到(640, 640)的原始图像帧
    def inference(self, frame: object) -> object:
        # 1. 图像预处理，这里YOLOv5和YOLOv8是一致的
        or_img = cv2.resize(frame, (640, 640))  # 拉伸图像
        img = or_img[:, :, ::-1].transpose(2, 0, 1)  # BGR2RGB和HWC2CHW
        img = img.astype(dtype=np.float32)
        img /= 255.0
        img = np.expand_dims(img, axis=0)

        # 2. 开始推理
        input_feed = self.getInputFeed(img)
        pred = self.onnx_session.run(None, input_feed)[0]
        # 需要注意：
        # pred变量，对于YOLOv8而言，返回的是：1 * 605 * 8400，而YOLOv5返回的是：1 * 25200 * 85

        # 3. 返回数据：
        boxes, scores, classes = self.normalpred(pred)
        c = classes * 640
        nb = boxes + c[:, np.newaxis]
        id = self.numpy_nms(nb, scores, self.iou_threshold)

        return (boxes[id], scores[id], classes[id]), or_img

    # 将inference()得到的pred变量，直接转换为boxes, scores, classes使用
    # 同时具有处理YOLOv8和YOLOv5两种目标检测模型的功能
    def normalpred(self, pred: object) -> object:
        if pred.shape[1] < pred.shape[2]:   # YOLOv8适用
            pred = np.squeeze(pred).T #1 * 605 * 8400 -> 8400 * 605
            scores = np.max(pred[:, 4:], axis=1)
            classes = np.argmax(pred[:, 4:], axis=1)
            mask = scores > self.conf_threshold #置信度过滤
            boxes = self.xywh_to_x1y1x2y2(pred[mask])
            scores = scores[mask]
            classes = classes[mask]
            return boxes, scores, classes

        else:   # YOLOv5适用
            pred = np.squeeze(pred)
            scores = pred[:, 4]
            classes = np.argmax(pred[:, 5:], axis=1)
            mask = scores > self.conf_threshold  # 置信度过滤
            boxes = self.xywh_to_x1y1x2y2(pred[mask])
            scores = scores[mask]
            classes = classes[mask]
            return boxes, scores, classes

    # 将XYWH格式转换为X1Y1X2Y2格式
    def xywh_to_x1y1x2y2(self, boxes):
        # 提取中心点坐标和宽高
        x_center, y_center, width, height = boxes[:, 0], boxes[:, 1], boxes[:, 2], boxes[:, 3]
        # 计算左上角和右下角坐标
        x1 = x_center - width / 2
        y1 = y_center - height / 2
        x2 = x_center + width / 2
        y2 = y_center + height / 2
        # 将计算结果组合成新的数组
        xyxy_boxes = np.stack((x1, y1, x2, y2), axis=1)
        return xyxy_boxes

    # 非极大抑制
    def numpy_nms(self, boxes, scores, iou_threshold):
        idxs = scores.argsort()  # 按分数 降序排列的索引 [N]
        keep = []
        while idxs.size > 0:  # 统计数组中元素的个数
            max_score_index = idxs[-1]
            max_score_box = boxes[max_score_index][None, :]
            keep.append(max_score_index)
            if idxs.size == 1:
                break
            idxs = idxs[:-1]  # 将得分最大框 从索引中删除； 剩余索引对应的框 和 得分最大框 计算IoU；
            other_boxes = boxes[idxs]  # [?, 4]
            ious = self.box_iou(max_score_box, other_boxes)  # 一个框和其余框比较 1XM
            idxs = idxs[ious[0] <= iou_threshold]

        return keep

    # 功能函数
    def box_area(self, boxes):
        return (boxes[:, 2] - boxes[:, 0]) * (boxes[:, 3] - boxes[:, 1])

    # 功能函数
    def box_iou(self, box1, box2):
        area1 = self.box_area(box1)  # N
        area2 = self.box_area(box2)  # M
        # broadcasting, 两个数组各维度大小 从后往前对比一致， 或者 有一维度值为1；
        lt = np.maximum(box1[:, np.newaxis, :2], box2[:, :2])
        rb = np.minimum(box1[:, np.newaxis, 2:], box2[:, 2:])
        wh = rb - lt
        wh = np.maximum(0, wh)  # [N, M, 2]
        inter = wh[:, :, 0] * wh[:, :, 1]
        iou = inter / (area1[:, np.newaxis] + area2 - inter)
        return iou

    # 画图函数，不带小地图
    # 输入：
    # image：原始图片（640*640），OpenCV2 image格式
    # box_data：(boxes, scores, classes)
    def draw(self, image: object, box_data: object) -> object:
        for box, score, cl in zip(box_data[0], box_data[1], box_data[2]): # 将boxes, scores, classes数据提取出来
            top, left, right, bottom = int(box[0]), int(box[1]), int(box[2]), int(box[3])
            dist, height, width, anglehorizon, angleheight = self.coord_conversion.box2Measurement(box, self.CLASSES[cl])   #SSES[cl])
            # 收集单个目标
            cv2.rectangle(image, (top, left), (right, bottom), (255, 0, 0), 2)  # 目标框显示
            # 将识别结果、识别分数、识别距离显示在目标框旁
            cv2.putText(image, '{0} {1:.2f}'.format(self.CLASSES[cl], score),
                        (top, bottom),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.6, (0, 0, 255), 2)
            # 距离、水平角、垂直角显示在目标框旁
            cv2.putText(image, '{0:.2f}m {1:.2f}hor {2:.2f}hei'.format(
                dist, anglehorizon, angleheight),
                (top, left),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.5, (100, 100, 255), 2)

    # 画图函数，带小地图
    # 输入：
    # image：原始图片（640*640），OpenCV2 image格式
    # box_data：(boxes, scores, classes)
    def drawWithMap(self, image: object, box_data: object) -> object:
        self.map.clearMap()  # 清空地图
        for box, score, cl in zip(box_data[0], box_data[1], box_data[2]): # 将boxes, scores, classes数据提取出来
            top, left, right, bottom = int(box[0]), int(box[1]), int(box[2]), int(box[3])
            #print('class: {}, score: {}'.format(CLASSES[cl], score))
            #print('box coordinate left,top,right,down: [{}, {}, {}, {}]'.format(top, left, right, bottom))


            dist, height, width, anglehorizon, angleheight = self.coord_conversion.box2Measurement(box, self.CLASSES[cl])   #SSES[cl])
            #print('actual dist, height, width: [{}, {}, {}]'.format(dist, height, width))

            # 收集单个目标
            self.map.collectSingleObject(self.CLASSES[cl], dist, anglehorizon)
            cv2.rectangle(image, (top, left), (right, bottom), (255, 0, 0), 2)  # 目标框显示
            # 将识别结果、识别分数、识别距离显示在目标框旁
            cv2.putText(image, '{0} {1:.2f}'.format(self.CLASSES[cl], score),
                        (top, bottom),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.6, (0, 0, 255), 2)
            # 距离、水平角、垂直角显示在目标框旁
            cv2.putText(image, '{0:.2f}m {1:.2f}hor {2:.2f}hei'.format(
                dist, anglehorizon, angleheight),
                (top, left),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.5, (100, 100, 255), 2)
        self.map.showObjects(0) # 显示地图中的所有目标
        self.map.showMapOnWindow("Map") # 在窗口上显示地图
