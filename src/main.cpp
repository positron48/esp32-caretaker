#include <Arduino.h>

// Пин встроенного светодиода на ESP32-CAM (чаще всего GPIO 4 для AI Thinker)
const int ledPin = 4;

// Структура для хранения статистики выполнения задачи
struct TaskStats {
    unsigned long totalTime;     // Общее время работы в микросекундах
    unsigned long activeTime;    // Время активной работы в микросекундах
    unsigned long lastCheck;     // Время последней проверки
    unsigned long lastActive;    // Время начала активной работы
    bool isActive;              // Флаг активности задачи
};

// Статистика для каждого ядра
TaskStats core0Stats = {0, 0, 0, 0, false};
TaskStats core1Stats = {0, 0, 0, 0, false};

// Декларация задач
void TaskBlink(void *pvParameters);
void TaskLog(void *pvParameters);
void updateTaskStats(TaskStats &stats, bool startActive);
float calculateCPUUsage(TaskStats &stats);
void resetStats(TaskStats &stats);

void setup() {
    // Инициализация последовательного порта
    Serial.begin(115200);
    Serial.println("ESP32-CAM LED Blink Example with CPU Usage Monitoring (microseconds)");

    // Инициализация пина светодиода как выход
    pinMode(ledPin, OUTPUT);

    // Инициализация времени для статистики
    unsigned long startTime = micros();
    core0Stats.lastCheck = startTime;
    core1Stats.lastCheck = startTime;

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
}

void loop() {
    // Пусто, так как задачи выполняются в фоновом режиме
}

// Обновление статистики задачи
void updateTaskStats(TaskStats &stats, bool startActive) {
    unsigned long currentTime = micros();
    unsigned long timeDiff = currentTime - stats.lastCheck;
    
    if (startActive && !stats.isActive) {
        // Задача начинает активную работу
        stats.lastActive = currentTime;
        stats.isActive = true;
    } 
    else if (!startActive && stats.isActive) {
        // Задача завершает активную работу
        stats.activeTime += currentTime - stats.lastActive;
        stats.isActive = false;
    }
    
    stats.totalTime += timeDiff;
    stats.lastCheck = currentTime;
}

// Расчет процента использования CPU
float calculateCPUUsage(TaskStats &stats) {
    if (stats.totalTime == 0) return 0.0;
    return (stats.activeTime * 100.0) / stats.totalTime;
}

// Задача для мигания светодиода
void TaskBlink(void *pvParameters) {
    while (true) {
        updateTaskStats(core0Stats, true);
        digitalWrite(ledPin, HIGH);   // Включить светодиод
        updateTaskStats(core0Stats, false);
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        updateTaskStats(core0Stats, true);
        digitalWrite(ledPin, LOW);    // Выключить светодиод
        updateTaskStats(core0Stats, false);
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Функция сброса статистики
void resetStats(TaskStats &stats) {
    stats.totalTime = 0;
    stats.activeTime = 0;
    stats.lastCheck = micros();
    stats.lastActive = 0;
    stats.isActive = false;
}

// Задача для вывода логов и статистики
void TaskLog(void *pvParameters) {
    while (true) {
        updateTaskStats(core1Stats, true);
        
        float core0Usage = calculateCPUUsage(core0Stats);
        float core1Usage = calculateCPUUsage(core1Stats);
        
        Serial.println("\nCPU Usage Statistics:");
        Serial.print("Core 0 (Blink Task): ");
        Serial.print(core0Usage, 4);
        Serial.print("% (Active: ");
        Serial.print(core0Stats.activeTime);
        Serial.print("us / Total: ");
        Serial.print(core0Stats.totalTime);
        Serial.println("us)");

        Serial.print("Core 1 (Log Task): ");
        Serial.print(core1Usage, 4);
        Serial.print("% (Active: ");
        Serial.print(core1Stats.activeTime);
        Serial.print("us / Total: ");
        Serial.print(core1Stats.totalTime);
        Serial.println("us)");
        Serial.println("------------------------");

        updateTaskStats(core1Stats, false);

        // Сброс статистики обоих ядер
        resetStats(core0Stats);
        resetStats(core1Stats);
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
} 