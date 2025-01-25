#include "include/blink_task.h"
#include "include/task_stats.h"
#include <Arduino.h>

const int ledPin = 4;

void TaskBlink(void *pvParameters) {
    // Настройка PWM для светодиода
    ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION);
    ledcAttachPin(ledPin, LED_CHANNEL);
    
    // Вычисляем значение для 0.5% яркости
    // При 12-битном разрешении максимальное значение 4095
    const int pwmValue = (4095 * 1) / 100;  // 0.5% от максимума
    
    while (true) {
        updateTaskStats(core0Stats, true);
        ledcWrite(LED_CHANNEL, pwmValue);  // Включить светодиод на 0.5% яркости
        updateTaskStats(core0Stats, false);
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        updateTaskStats(core0Stats, true);
        ledcWrite(LED_CHANNEL, 0);  // Выключить светодиод
        updateTaskStats(core0Stats, false);
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
} 