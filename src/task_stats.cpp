#include "include/task_stats.h"
#include <Arduino.h>

TaskStats core0Stats = {0, 0, 0, 0, false};
TaskStats core1Stats = {0, 0, 0, 0, false};

void updateTaskStats(TaskStats &stats, bool startActive) {
    unsigned long currentTime = micros();
    unsigned long timeDiff = currentTime - stats.lastCheck;
    
    if (startActive && !stats.isActive) {
        stats.lastActive = currentTime;
        stats.isActive = true;
    } 
    else if (!startActive && stats.isActive) {
        stats.activeTime += currentTime - stats.lastActive;
        stats.isActive = false;
    }
    
    stats.totalTime += timeDiff;
    stats.lastCheck = currentTime;
}

float calculateCPUUsage(TaskStats &stats) {
    if (stats.totalTime == 0) return 0.0;
    return (stats.activeTime * 100.0) / stats.totalTime;
}

void resetStats(TaskStats &stats) {
    stats.totalTime = 0;
    stats.activeTime = 0;
    stats.lastCheck = micros();
    stats.lastActive = 0;
    stats.isActive = false;
} 