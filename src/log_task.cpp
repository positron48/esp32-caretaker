#include "include/log_task.h"
#include "include/task_stats.h"
#include <Arduino.h>

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