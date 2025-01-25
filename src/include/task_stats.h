#pragma once

struct TaskStats {
    const char* taskName;        // Имя задачи
    int coreId;                 // Номер ядра
    unsigned long totalTime;     // Общее время работы в микросекундах
    unsigned long activeTime;    // Время активной работы в микросекундах
    unsigned long lastCheck;     // Время последней проверки
    unsigned long lastActive;    // Время начала активной работы
    bool isActive;              // Флаг активности задачи
};

void updateTaskStats(TaskStats &stats, bool startActive);
float calculateCPUUsage(TaskStats &stats);
void resetStats(TaskStats &stats);

// Статистика для каждой задачи
extern TaskStats blinkTaskStats;
extern TaskStats logTaskStats;
extern TaskStats httpTaskStats; 