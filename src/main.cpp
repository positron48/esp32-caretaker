#include <Arduino.h>
#include "include/task_stats.h"
#include "include/blink_task.h"
#include "include/log_task.h"
#include "include/wifi_manager.h"
#include "include/http_server_task.h"
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

// LED pin
const int ledPin = 4;

bool initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        Serial.printf("PSRAM found\n");
        config.frame_size = FRAMESIZE_VGA;
        config.jpeg_quality = 10;  // 0-63, lower means higher quality
        config.fb_count = 2;
    } else {
        Serial.printf("PSRAM not found\n");
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }

    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_VGA);
    s->set_quality(s, 10);
    
    return true;
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-CAM Streaming Server");

    // Initialize PSRAM
    if(!psramInit()){
        Serial.println("PSRAM initialization failed");
        return;
    }

    // Initialize camera
    if(!initCamera()) {
        Serial.println("Camera initialization failed");
        return;
    }

    // Initialize LED pin
    pinMode(ledPin, OUTPUT);

    // Initialize WiFi
    initWiFi();

    // Create blink task on core 0
    xTaskCreatePinnedToCore(
        TaskBlink,
        "Blink",
        2048,
        NULL,
        1,
        NULL,
        0
    );

    // Create log task on core 1
    xTaskCreatePinnedToCore(
        TaskLog,
        "Log",
        2048,
        NULL,
        1,
        NULL,
        1
    );

    // Create HTTP server task on core 1
    xTaskCreatePinnedToCore(
        TaskHttpServer,
        "HttpServer",
        4096,
        NULL,
        1,
        NULL,
        1
    );
}

void loop() {
    // Check WiFi connection every 5 seconds
    ensureWiFiConnection();
    delay(5000);
} 