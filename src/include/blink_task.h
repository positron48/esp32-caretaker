#pragma once

// Константы для настройки PWM
const int LED_CHANNEL = 0;  // Канал LEDC (0-15)
const int LED_RESOLUTION = 12;  // Разрешение (1-14 бит)
const int LED_FREQUENCY = 10000;  // Частота PWM в Гц

void TaskBlink(void *pvParameters); 