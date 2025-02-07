#include "blink_task.h"
#include <Arduino.h>

const int ledPin = 4;

void TaskBlink(void *pvParameters) {
    // Настройка PWM для светодиода
    ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION);
    ledcAttachPin(ledPin, LED_CHANNEL);
    
    // Вычисляем значение для 0.5% яркости
    const int pwmValue = (4095 * 1) / 100;  // 0.5% от максимума
    
    while (true) {
        ledcWrite(LED_CHANNEL, pwmValue);  // Включить светодиод на 0.5% яркости
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        ledcWrite(LED_CHANNEL, 0);  // Выключить светодиод
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
} 