#pragma once

#include <Arduino.h>
#include <esp_camera.h>

// Camera pins for ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Camera configuration
constexpr framesize_t FRAME_SIZE = FRAMESIZE_VGA;
constexpr int JPEG_QUALITY = 12;  // 0-63 (lower means higher quality)
constexpr int XCLK_FREQ = 20000000;  // 20MHz
constexpr pixformat_t PIXEL_FORMAT = PIXFORMAT_JPEG;
constexpr int FB_COUNT = 2;  // Frame buffer count

// Function declarations
bool initCamera();