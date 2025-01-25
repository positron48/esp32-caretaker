#include "include/blink_task.h"
#include "include/task_stats.h"
#include <Arduino.h>

const int ledPin = 4;

void TaskBlink(void *pvParameters) {
    pinMode(ledPin, OUTPUT);
    
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