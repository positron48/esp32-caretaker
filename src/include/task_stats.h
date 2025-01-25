#pragma once

struct TaskStats {
    unsigned long totalTime;     // Общее время работы в микросекундах
    unsigned long activeTime;    // Время активной работы в микросекундах
    unsigned long lastCheck;     // Время последней проверки
    unsigned long lastActive;    // Время начала активной работы
    bool isActive;              // Флаг активности задачи
};

void updateTaskStats(TaskStats &stats, bool startActive);
float calculateCPUUsage(TaskStats &stats);
void resetStats(TaskStats &stats);

extern TaskStats core0Stats;
extern TaskStats core1Stats; 