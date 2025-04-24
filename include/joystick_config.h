#pragma once

#include "config.h" // Include config.h for constants

// Data structure for storing joystick positions and buttons
struct JoystickData {
    int16_t x = 0;        // left joystick, x-axis
    int16_t y = 0;        // left joystick, y-axis
    int16_t rx = 0;       // right joystick, x-axis
    int16_t ry = 0;       // right joystick, y-axis
    uint32_t buttons = 0; // bit flags for buttons
};

// Global flag for BT joystick activation
extern bool btControlEnabled; 