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

    setupStreamTask(&server);

    server.begin();
    Serial.println("HTTP server started");

    while (true) {
        server.handleClient();
        // Используем vTaskDelay вместо delay для лучшего планирования задач
        vTaskDelay(pdMS_TO_TICKS(50));
    }
} 