#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "StatsTask.h"

static const char* TAG = "StatsTask";

void TaskStats(void *pvParameters) {
    char *buffer = NULL;
    while (true) {
        // Получение количества задач и вычисление размера буфера
        size_t bufferLen = uxTaskGetNumberOfTasks() * 40;
        buffer = (char *)malloc(bufferLen);
        if (buffer == NULL) {
            ESP_LOGE(TAG, "Не удалось выделить память для статистики");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        // Получение и вывод статистики
        vTaskGetRunTimeStats(buffer);
        ESP_LOGI(TAG, "Run Time Stats:\n%s\n", buffer);
        free(buffer);

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Задержка 5 секунд
    }
} 