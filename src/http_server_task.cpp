#include "include/http_server_task.h"
#include <Arduino.h>

WebServer server(80);

// Обработчик корневого маршрута
void handleRoot() {
    server.send(200, "text/plain", "Hello World!");
}

void TaskHttpServer(void *pvParameters) {
    // Настройка маршрутов
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started on port 80");

    while (true) {
        server.handleClient();
        // Небольшая задержка чтобы не нагружать процессор
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
} 