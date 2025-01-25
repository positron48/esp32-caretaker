#include <Arduino.h>
#include "include/task_stats.h"
#include "include/blink_task.h"
#include "include/log_task.h"
#include "include/wifi_manager.h"
#include "include/http_server_task.h"

// Пин встроенного светодиода на ESP32-CAM (чаще всего GPIO 4 для AI Thinker)
const int ledPin = 4;

void setup() {
    // Инициализация последовательного порта
    Serial.begin(115200);
    Serial.println("ESP32-CAM LED Blink Example with CPU Usage Monitoring (microseconds)");

    // Инициализация пина светодиода как выход
    pinMode(ledPin, OUTPUT);

    // Инициализация WiFi
    initWiFi();

    // Инициализация времени для статистики
    unsigned long startTime = micros();
    blinkTaskStats.lastCheck = startTime;
    logTaskStats.lastCheck = startTime;
    httpTaskStats.lastCheck = startTime;
    
    // Создание задачи для мигания светодиода на ядре 0
    xTaskCreatePinnedToCore(
        TaskBlink,   // Функция задачи
        "Blink",     // Имя задачи
        2048,        // Увеличили размер стека для доп. вычислений
        NULL,        // Параметры задачи
        1,           // Приоритет задачи
        NULL,        // Дескриптор задачи
        0            // Ядро, на котором будет выполняться задача
    );

    // Создание задачи для вывода логов на ядре 1
    xTaskCreatePinnedToCore(
        TaskLog,     // Функция задачи
        "Log",       // Имя задачи
        2048,        // Увеличили размер стека для доп. вычислений
        NULL,        // Параметры задачи
        1,           // Приоритет задачи
        NULL,        // Дескриптор задачи
        1            // Ядро, на котором будет выполняться задача
    );

    // Создаем задачу для HTTP сервера на ядре 1
    xTaskCreatePinnedToCore(
        TaskHttpServer,
        "HttpServer",
        4096,        // Увеличенный размер стека для HTTP сервера
        NULL,
        1,
        NULL,
        1
    );
}

void loop() {
    // Проверка состояния WiFi каждые 5 секунд
    ensureWiFiConnection();
    delay(5000);
} 