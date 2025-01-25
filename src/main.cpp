#include <Arduino.h>

// Пин встроенного светодиода на ESP32-CAM (чаще всего GPIO 4 для AI Thinker)
const int ledPin = 4;

// Декларация задач
void TaskBlink(void *pvParameters);
void TaskLog(void *pvParameters);

void setup() {
    // Инициализация последовательного порта
    Serial.begin(115200);
    Serial.println("ESP32-CAM LED Blink Example with FreeRTOS");

    // Инициализация пина светодиода как выход
    pinMode(ledPin, OUTPUT);

    // Создание задачи для мигания светодиода
    xTaskCreate(
        TaskBlink,   // Функция задачи
        "Blink",     // Имя задачи
        1024,        // Размер стека задачи
        NULL,        // Параметры задачи
        1,           // Приоритет задачи
        NULL         // Дескриптор задачи
    );

    // Создание задачи для вывода логов
    xTaskCreate(
        TaskLog,     // Функция задачи
        "Log",       // Имя задачи
        1024,        // Размер стека задачи
        NULL,        // Параметры задачи
        1,           // Приоритет задачи
        NULL         // Дескриптор задачи
    );
}

void loop() {
    // Пусто, так как задачи выполняются в фоновом режиме
}

// Задача для мигания светодиода
void TaskBlink(void *pvParameters) {
    while (true) {
        digitalWrite(ledPin, HIGH);   // Включить светодиод
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Задержка 1 секунда
        digitalWrite(ledPin, LOW);    // Выключить светодиод
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Задержка 1 секунда
    }
}

// Задача для вывода логов
void TaskLog(void *pvParameters) {
    while (true) {
        Serial.println("LED is toggling");
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Задержка 2 секунды
    }
} 