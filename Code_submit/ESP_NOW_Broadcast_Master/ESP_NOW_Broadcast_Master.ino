#include <esp_now.h>
#include <WiFi.h>
#include "esp_camera.h"

#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22


uint8_t receiverMAC[] = {0xFC, 0xB4, 0x67, 0x56, 0xF9, 0x04}; 

camera_config_t config = {
  .pin_pwdn = PWDN_GPIO_NUM,
  .pin_reset = RESET_GPIO_NUM,
  .pin_xclk = XCLK_GPIO_NUM,
  .pin_sccb_sda = SIOD_GPIO_NUM,
  .pin_sccb_scl = SIOC_GPIO_NUM,

  .pin_d7 = Y9_GPIO_NUM,
  .pin_d6 = Y8_GPIO_NUM,
  .pin_d5 = Y7_GPIO_NUM,
  .pin_d4 = Y6_GPIO_NUM,
  .pin_d3 = Y5_GPIO_NUM,
  .pin_d2 = Y4_GPIO_NUM,
  .pin_d1 = Y3_GPIO_NUM,
  .pin_d0 = Y2_GPIO_NUM,
  .pin_vsync = VSYNC_GPIO_NUM,
  .pin_href = HREF_GPIO_NUM,
  .pin_pclk = PCLK_GPIO_NUM,

  .xclk_freq_hz = 20000000,
  .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,
  .pixel_format = PIXFORMAT_JPEG, 
  .frame_size = FRAMESIZE_SVGA, 
  .jpeg_quality = 10,          
  .fb_count = 1
};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {

  Serial.begin(115200);
  Serial.println();
  delay(10000);

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("ESP-NOW Initialized and Peer Added");
  delay(15000);
}

void loop() {

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  Serial.printf("Captured Image Size: %zu bytes\n", fb->len);

  uint32_t imageSize = fb->len;
  esp_now_send(receiverMAC, (uint8_t *) &imageSize, sizeof(imageSize));
  delay(100); 

  size_t totalBytes = fb->len;
  size_t bytesSent = 0;
  const uint8_t *data = fb->buf;

  while (bytesSent < totalBytes) {

    size_t chunkSize = (totalBytes - bytesSent > 240) ? 240 : (totalBytes - bytesSent);
    uint8_t buffer[240];
    memcpy(buffer, data + bytesSent, chunkSize);


    esp_err_t result = esp_now_send(receiverMAC, buffer, chunkSize);
    if (result == ESP_OK) {
      Serial.printf("Sent %zu bytes\n", chunkSize);
    } else {
      Serial.println("Error sending the data");
    }

    bytesSent += chunkSize;
    delay(10); 
  }

  esp_camera_fb_return(fb);

  delay(10000);
}
