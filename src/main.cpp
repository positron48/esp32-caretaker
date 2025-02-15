#include <Arduino.h>
#include "task_stats.h"
#include "log_task.h"
#include "wifi_manager.h"
#include "http_server_task.h"
#include "camera_config.h"
#include <esp_camera.h>
#include "SPIFFS.h"
#include <ArduinoOTA.h>

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
        Serial.println("PSRAM found");
    } else {
        Serial.println("PSRAM not found");
    }

    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = JPEG_QUALITY;
    config.fb_count = FB_COUNT;

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }

    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, FRAME_SIZE);
    s->set_quality(s, JPEG_QUALITY);
    return true;
}

void initOTA() {
    ArduinoOTA.setHostname("esp32cam");
    ArduinoOTA.onStart([]() {
        Serial.println("Start OTA update");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA update finished");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("OTA Progress: %u%%\r", (progress * 100) / total);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u] during OTA update: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("OTA is ready");
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-CAM Streaming Server");

    // Initialize PSRAM
    if(!psramInit()){
        Serial.println("PSRAM initialization failed");
        return;
    }

    // Initialize LED
    ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION);
    ledcAttachPin(ledPin, LED_CHANNEL);
    ledcWrite(LED_CHANNEL, 0); // Start with LED off

    // Initialize SPIFFS
    if(!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // Initialize camera
    if(!initCamera()) {
        Serial.println("Camera initialization failed");
        return;
    }

    // Initialize WiFi
    initWiFi();

    // Initialize OTA updates
    initOTA();

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
    ArduinoOTA.handle();
    // Check WiFi connection every 5 seconds
    // ensureWiFiConnection();
    // delay(500);
} 