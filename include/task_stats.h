#pragma once

struct TaskStats {
    const char* taskName;        // Task name
    int coreId;                  // Core number
    unsigned long totalTime;     // Total operation time in microseconds
    unsigned long activeTime;    // Active operation time in microseconds
    unsigned long lastCheck;     // Time of last check
    unsigned long lastActive;    // Time of active work start
    bool isActive;               // Task activity flag
};

void updateTaskStats(TaskStats &stats, bool startActive);
float calculateCPUUsage(TaskStats &stats);
void resetStats(TaskStats &stats);

// Statistics for each task
extern TaskStats blinkTaskStats;
extern TaskStats logTaskStats;
extern TaskStats httpTaskStats; 