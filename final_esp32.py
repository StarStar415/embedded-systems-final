from machine import Pin, PWM
from machine import UART
from umqtt.simple import MQTTClient
import utime, xtools

xtools.connect_wifi_led()
# 連結 8051 喇叭
speaker = Pin(15, Pin.OUT)
# 與 8051 UART 連結 , 設定 tx rx 的 GPIO
com = UART(2, 9600, tx=17, rx=16)
com.init(9600)

# 設定自己的 username 和 api key 以及 feed 名稱

# ADAFRUIT_IO_USERNAME = ""
# ADAFRUIT_IO_KEY      = ""
cnt=1

# 保存接收到的 MQTT 消息
received_msgs = {
    "finalNumber": None,
    "finalAsciiCode": None
}

# 訂閱要使用的 FEEDS
def subscribe_to_feeds(client, username):
    feeds = ["finalAscii", "finalNumber"]
    for feed in feeds:
        topic = f"{username}/feeds/{feed}"
        print(topic)
        client.subscribe(topic)

# MQTT 客戶端
client = MQTTClient (
    client_id = xtools.get_id(),
    server = "io.adafruit.com",
    user = ADAFRUIT_IO_USERNAME,
    password = ADAFRUIT_IO_KEY,
    ssl = False,
)

# 解析收到的訊息
def sub_cb(topic, msg):
    topic = topic.decode()
    msg = msg.decode()
    print(msg)
    # 判斷猜數字或是解碼 ASCII
    if "finalNumber" in topic:
        com.write(f"{msg}\r\n") # 傳送訊息給 8051
        received_msgs["finalNumber"] = msg
        
    elif "finalAscii" in topic:
        msg = msg.upper()
        com.write(f"{msg}\r\n") # 傳送訊息給 8051
        received_msgs["finalAsciiCode"] = msg
                
    
client.set_callback(sub_cb)   # 指定回撥函數來接收訊息
client.connect()              # 連線

subscribe_to_feeds(client, ADAFRUIT_IO_USERNAME)

# 接收 8051 回傳的資料
while True:
    client.check_msg()
    if com.any() > 0:
       a = com.readline()
       print(a.decode().strip())
       combined_msg = f"{cnt}: {received_msgs['finalNumber']} -- {a.decode().strip()}"
       # 猜數字成功 顯示成功訊息並且嘗試了幾次，將計數歸零
       if "4A0B" in a.decode().strip():
           if cnt == 1:
                tmp = "try"
           else:
                tmp = "tries"
           combined_msg = f"SUCCESS!! use {cnt} {tmp} Ans:{received_msgs['finalNumber']}  "
           client.publish("StarStar415/feeds/finalNumberAns", combined_msg)
           cnt=1
       # 提示模式 publish 提示格式
       elif "-" in a.decode().strip():
           combined_msg = f"Hint: {a.decode().strip()}"
           client.publish("StarStar415/feeds/finalNumberAns", combined_msg)
       # 摩斯密碼的格式 
       elif a.decode().strip() >= 'A' and a.decode().strip() <= 'Z':
           client.publish("StarStar415/feeds/finalMorseCode", a.decode())
       # 其餘皆為正常猜數字模式 並將計數次數增加
       else:
           client.publish("StarStar415/feeds/finalNumberAns", combined_msg)
           cnt += 1
    
