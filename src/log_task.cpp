#include "include/log_task.h"
#include "include/task_stats.h"
#include "include/http_server_task.h"
#include <Arduino.h>

void TaskLog(void *pvParameters) {
    while (true) {
        updateTaskStats(logTaskStats, true);
        
        Serial.println("\nTask Usage Statistics:");
        
        // Статистика для Blink Task
        float blinkUsage = calculateCPUUsage(blinkTaskStats);
        Serial.printf("%s (Core %d): %.4f%% (Active: %lu us / Total: %lu us)\n",
            blinkTaskStats.taskName,
            blinkTaskStats.coreId,
            blinkUsage,
            blinkTaskStats.activeTime,
            blinkTaskStats.totalTime
        );

        // Статистика для HTTP Server Task
        float httpUsage = calculateCPUUsage(httpTaskStats);
        Serial.printf("%s (Core %d): %.4f%% (Active: %lu us / Total: %lu us)\n",
            httpTaskStats.taskName,
            httpTaskStats.coreId,
            httpUsage,
            httpTaskStats.activeTime,
            httpTaskStats.totalTime
        );

        // Статистика для Log Task
        float logUsage = calculateCPUUsage(logTaskStats);
        Serial.printf("%s (Core %d): %.4f%% (Active: %lu us / Total: %lu us)\n",
            logTaskStats.taskName,
            logTaskStats.coreId,
            logUsage,
            logTaskStats.activeTime,
            logTaskStats.totalTime
        );

        // Информация о HTTP сервере
        Serial.println("------------------------");

        updateTaskStats(logTaskStats, false);

        // Сброс статистики всех задач
        resetStats(blinkTaskStats);
        resetStats(logTaskStats);
        resetStats(httpTaskStats);
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
} 