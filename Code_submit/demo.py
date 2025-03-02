import paho.mqtt.client as mqtt
import time
import os
import threading
from roboflow import Roboflow
from ultralytics import YOLO

#MQTT
mqtt_broker = "test.mosquitto.org"
mqtt_port = 1883
mqtt_topic = "drone/send/pic"

#Roboflow
model=YOLO("best_train_800x600.pt")

# Biến để lưu trữ dữ liệu ảnh
image_data = bytearray()
image_ready = False
lock = threading.Lock()

# Hàm callback khi nhận được gói tin từ MQTT
def on_message(client, userdata, msg):
    global image_data, image_ready
    segment = msg.payload  # Dữ liệu nhị phân

    with lock:
        image_data += segment

    # Kiểm tra nếu đây là gói cuối cùng (giả định gói cuối nhỏ hơn 1024 byte)
    if len(segment) < 1024:
        image_ready = True
        print("Received complete image.")

# Hàm xử lý và phân tích ảnh
def process_image():
    global image_ready, image_data
    while True:
        if image_ready:
            with lock:
                full_image = bytes(image_data)
                image_data = bytearray()
                image_ready = False
            
            # Lưu ảnh vào file tạm thời
            temp_filename = "temp_image.jpg"
            try:
                with open(temp_filename, "wb") as img_file:
                    img_file.write(full_image)
                print(f"Image saved temporarily as {temp_filename}")
                #detect
                results=model.predict(temp_filename, save=True, project="received_images", exist_ok=True)
            
                per_cnt=0
                for result in results:
                    for box in result.boxes.data:
                        cls=int(box[-1])
                        if cls==0:
                            per_cnt+=1
                print(f"Số lượng người nhận diện: {per_cnt}")
            except Exception as e:
                print(f"Error processing image: {e}")
            finally:
                # delete temp_file
                if os.path.exists(temp_filename):
                    os.remove(temp_filename)

# Thiết lập MQTT
client = mqtt.Client()
client.on_message = on_message

try:
    client.connect(mqtt_broker, mqtt_port, 60)
except Exception as e:
    print(f"Failed to connect to MQTT broker: {e}")
    exit(1)

client.subscribe(mqtt_topic)

# Chạy MQTT trong một thread riêng để không bị block
client.loop_start()
print("Waiting for image data...")

# Tạo luồng xử lý ảnh
image_thread = threading.Thread(target=process_image)
image_thread.daemon = True
image_thread.start()

try:
    while True:
        time.sleep(1)  # Thời gian chờ ngắn để giữ chương trình chạy
except KeyboardInterrupt:
    print("Process interrupted by user.")
finally:
    client.loop_stop()
    client.disconnect()
    print("Disconnected from MQTT broker.")
