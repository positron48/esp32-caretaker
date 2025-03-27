#pragma once

#include "config.h" // Include config.h for constants

// Структура данных для хранения положения джойстиков и кнопок
struct JoystickData {
    int16_t x = 0;    // левый джойстик, x-axis
    int16_t y = 0;    // левый джойстик, y-axis
    int16_t rx = 0;   // правый джойстик, x-axis
    int16_t ry = 0;   // правый джойстик, y-axis
    uint32_t buttons = 0; // битовые флаги для кнопок
};

// Глобальный флаг активации BT джойстика
extern bool btControlEnabled; 