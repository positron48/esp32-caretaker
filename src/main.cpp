#include <Arduino.h>
#include "task_stats.h"
#include "log_task.h"
#include "wifi_manager.h"
#include "http_server_task.h"
#include "camera_config.h"
#include <esp_camera.h>
#include "motor_control.h"
#include "bluetooth_task.h"
#include "config.h"

// LED pin
const int ledPin = LED_PIN;

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
    config.xclk_freq_hz = XCLK_FREQ;
    config.pixel_format = PIXEL_FORMAT;

    // Init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        Serial.println("PSRAM found, using higher resolution");
        config.frame_size = FRAMESIZE_XGA; // Higher resolution for initialization
        config.jpeg_quality = 10; // Better quality
        config.fb_count = 2;
    } else {
        Serial.println("PSRAM not found, using lower resolution");
        config.frame_size = FRAMESIZE_HVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }

    // Set to desired frame size after initialization
    sensor_t * s = esp_camera_sensor_get();
    
    // Additional settings from the example
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);
        s->set_brightness(s, 1);
        s->set_saturation(s, -2);
    }
    
    Serial.println("Camera initialized successfully");
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

    // Initialize LED if enabled
    #if FEATURE_LED_CONTROL_ENABLED
    ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION);
    ledcAttachPin(ledPin, LED_CHANNEL);
    ledcWrite(LED_CHANNEL, 0); // Start with LED off
    Serial.println("LED control initialized");
    #endif

    // Initialize camera
    if(!initCamera()) {
        Serial.println("Camera initialization failed");
        return;
    }

    // Initialize motor control
    initMotors();

    // Initialize BLE if enabled
    #if FEATURE_BLUETOOTH_ENABLED
    initBluetooth();
    Serial.println("Bluetooth initialized");
    #endif

    // Initialize WiFi
    initWiFi();

    // Create log task on core 1 if enabled
    #if FEATURE_TASK_STATS_ENABLED
    xTaskCreatePinnedToCore(
        TaskLog,
        "Log",
        2048,
        NULL,
        1,
        NULL,
        1
    );
    Serial.println("Task stats logging enabled");
    #endif

    // Create HTTP server task on core 0
    xTaskCreatePinnedToCore(
        TaskHttpServer,
        "HttpServer",
        4096,
        NULL,
        1,
        NULL,
        0
    );
    
    // Create Bluetooth task on core 0 if enabled
    #if FEATURE_BLUETOOTH_ENABLED
    xTaskCreatePinnedToCore(
        TaskBluetooth,
        "Bluetooth",
        4096,
        NULL,
        1,
        NULL,
        0
    );
    #endif
}

void loop() {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}