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

def decode_address(longitude,latitude,api_key):
    result = amap_reverse_geocode(longitude,latitude, api_key)
    print(f"{longitude,latitude}")
    #print(result)
    if result is None:
    	pass
    parsed_data = ET.fromstring(result)
    if parsed_data is not None:
        formatted_address = parsed_data.find("./regeocode/formatted_address").text#查找
    citycode=parsed_data.find("./regeocode/addressComponent/adcode").text
    return formatted_address,citycode

def get_weather(api_key,citycode):
    url = f"https://restapi.amap.com/v3/weather/weatherInfo?key={api_key}&city={citycode}&extensions=all"
    response = requests.get(url)
    
    if response.status_code == 200:
        return response.json()
    else:
        return None
    
if __name__ == '__main__':

    latitude = 23.09615907473295
# 39.991957
    longitude = 113.32718957875059
# 116.310003, (113.32718957875059, 23.09615907473295)
    api_key = "e0b93d4f1620b219cc59584de3bb01ee"



# # 执行逆地理编码
# result = amap_reverse_geocode(latitude, longitude, api_key)
# parsed_data = parse_xml_to_dict(result)#以字典格式存储

    # formatted_address = parsed_data['regeocode']['formatted_address']
    formatted_address,citycode=decode_address(latitude,longitude,api_key)
    print("citycode:",citycode)
    weather_data=get_weather(api_key,citycode)
    print("解码得地址信息：", formatted_address)
    print("天气信息:",weather_data)
    # 输出当前天气
    current_weather = weather_data['forecasts'][0]['casts'][0]
    print("当前天气：", current_weather['date'], current_weather['dayweather'], "，白天温度:", current_weather['daytemp'], "℃，夜间温度:", current_weather['nighttemp'], "℃")

# # 输出未来两小时的天气变化
#     for i in range(2):
#         hourly_forecast = weather_data['forecasts'][0]['hourly']
#         for forecast in hourly_forecast:
#             print("时间:", forecast['time'], "天气:", forecast['weather'], "温度:", forecast['temperature'], "℃")
#         # print("未来两小时天气变化：", forecast['date'], forecast['dayweather'], "，白天温度:", forecast['daytemp'], "℃，夜间温度:", forecast['nighttemp'], "℃")
