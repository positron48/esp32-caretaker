#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "LogTask.h"

static const char* TAG = "LogTask";

void TaskLog(void *pvParameters) {
    while (true) {
        ESP_LOGI(TAG, "LED is toggling");
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Задержка 1 секунда
    }
} 