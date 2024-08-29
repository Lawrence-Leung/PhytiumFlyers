import requests
import xml.etree.ElementTree as ET

def parse_xml_to_dict(xml_data):
    root = ET.fromstring(xml_data)
    result = {}
    for child in root:
        result[child.tag] = parse_element(child)
    return result

def parse_element(element):
    result = {}
    if element.text:
        result[element.tag] = element.text.strip()
    else:
        result[element.tag] = {}
        for child in element:
            result[element.tag][child.tag] = parse_element(child)
    return result

def amap_reverse_geocode(latitude, longitude, api_key):
    url = f"https://restapi.amap.com/v3/geocode/regeo?output=xml&location={longitude},{latitude}&key={api_key}&radius=1000&extensions=all"
    response = requests.get(url)
    if response.status_code == 200:
        return response.text
    else:
        return "请求失败"

def decode_address(latitude,longitude):
    api_key = "e0b93d4f1620b219cc59584de3bb01ee"
    result = amap_reverse_geocode(latitude, longitude, api_key)
 
    parsed_data = ET.fromstring(result)
    formatted_address = parsed_data.find("./regeocode/formatted_address").text#查找
    return formatted_address

# latitude = 23.037395
# # 39.991957
# longitude = 113.393231
# # 116.310003
# # 替换为你的高德地图API密钥


# # 执行逆地理编码
# result = amap_reverse_geocode(latitude, longitude, api_key)
# parsed_data = parse_xml_to_dict(result)#以字典格式存储

    # formatted_address = parsed_data['regeocode']['formatted_address']
# print("地址信息：", formatted_address)
