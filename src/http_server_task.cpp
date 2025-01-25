#include "include/http_server_task.h"
#include "include/task_stats.h"
#include <Arduino.h>

WebServer server(80);

// Обработчик корневого маршрута
void handleRoot() {
    updateTaskStats(httpTaskStats, true);
    server.send(200, "text/plain", "Hello World!");
    updateTaskStats(httpTaskStats, false);
}

void TaskHttpServer(void *pvParameters) {
    // Настройка маршрутов
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started on port 80");

    while (true) {
        updateTaskStats(httpTaskStats, true);
        server.handleClient();
        updateTaskStats(httpTaskStats, false);
        
        // Небольшая задержка чтобы не нагружать процессор
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
} 