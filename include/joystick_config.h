#pragma once

// Название Bluetooth джойстика, к которому нужно подключиться
#define JOYSTICK_DEVICE_NAME "ExpressLRS Joystick"

// Время сканирования BLE устройств в секундах
#define BLE_SCAN_TIME 5

// Период повторного сканирования, если устройство не найдено (в мс)
#define BLE_RESCAN_INTERVAL 10000

// Параметры левого джойстика X
#define LEFT_JOYSTICK_X_MIN 1500
#define LEFT_JOYSTICK_X_MAX 32767
#define LEFT_JOYSTICK_X_CENTER 17000

// Параметры левого джойстика Y
#define LEFT_JOYSTICK_Y_MIN 19
#define LEFT_JOYSTICK_Y_MAX 31527
#define LEFT_JOYSTICK_Y_CENTER 16693

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