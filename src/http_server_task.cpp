#include "http_server_task.h"
#include "stream_task.h"
#include "SPIFFS.h"

void TaskHttpServer(void* parameter) {
    // Initialize SPIFFS
    if(!SPIFFS.begin(true)) {
        Serial.println("An error has occurred while mounting SPIFFS");
        return;
    }

    WebServer server(80);

    // Setup routes
    server.on("/", HTTP_GET, [&server]() {
        if(SPIFFS.exists("/html/index.html")) {
            File file = SPIFFS.open("/html/index.html", "r");
            server.streamFile(file, "text/html");
            file.close();
        } else {
            server.send(404, "text/plain", "File not found");
            Serial.println("index.html not found in SPIFFS");
        }
    });

    // LED control endpoint
    server.on("/led", HTTP_GET, [&server]() {
        String state = server.arg("state");
        int pwmValue = 0;
        
        if (state == "off") {
            pwmValue = 0;
        } else if (state == "mid") {
            pwmValue = (4095 * 10) / 100; // 1% яркости как в TaskBlink
        } else if (state == "high") {
            pwmValue = 4095; // Полная яркость
        }
        
        ledcWrite(LED_CHANNEL, pwmValue);
        server.send(200, "text/plain", "OK");
    });

    // Quality control endpoint
    server.on("/quality", HTTP_GET, [&server]() {
        String mode = server.arg("mode");
        sensor_t * s = esp_camera_sensor_get();
        
        if (s != nullptr) {
            if (mode == "HD") {
                s->set_framesize(s, FRAMESIZE_VGA); // 640x480
                s->set_quality(s, 10); // Лучшее качество для HD
            } else {
                s->set_framesize(s, FRAMESIZE_QVGA); // 320x240
                s->set_quality(s, 12); // Стандартное качество для SD
            }
            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "Camera sensor not found");
        }
    });

    // Control endpoint for joystick
    server.on("/control", HTTP_POST, [&server]() {
        // В будущем здесь будет обработка данных джойстика
        server.send(200, "text/plain", "OK");
    });

    setupStreamTask(&server);

    server.begin();
    Serial.println("HTTP server started");

    while (true) {
        server.handleClient();
        // Используем vTaskDelay вместо delay для лучшего планирования задач
        vTaskDelay(pdMS_TO_TICKS(50));
    }
} 