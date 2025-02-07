#include "log_task.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void TaskLog(void *pvParameters) {
    static const int statsBufferSize = 1024;
    static char statsBuffer[statsBufferSize];

    static const int listBufferSize = 1024;
    static char listBuffer[listBufferSize];

    while (true) {
        Serial.println("\n============ Task Stats ============");

        // Get runtime stats for CPU usage
        // This requires configGENERATE_RUN_TIME_STATS to be enabled
        vTaskGetRunTimeStats(statsBuffer);
        Serial.println("Run Time Stats:");
        Serial.println(statsBuffer);

        // Get task list with state, priority, stack, and task number
        // Note: vTaskList output depends on configuration and may not include core affinities by default
        // vTaskList(listBuffer);
        // Serial.println("Task List:");
        // Serial.println(listBuffer);

        Serial.println("=====================================");

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
} 