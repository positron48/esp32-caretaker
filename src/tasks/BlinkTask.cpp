#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "BlinkTask.h"

static const char* TAG = "BlinkTask";

// Конфигурация LEDC
#define LEDC_MODE              LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL           LEDC_CHANNEL_0

void TaskBlink(void *pvParameters) {
    // Задача мигания светодиода
    while (true) {           
        // Установка яркости до 0.5%
        uint32_t duty = (uint32_t)(255 * 0.005); // 0.5% яркости
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Задержка 1 секунда

        // Выключение светодиода (0% яркости)
        duty = 0;
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Задержка 1 секунда
    }
} 