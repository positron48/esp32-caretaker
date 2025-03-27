#pragma once

#include <Arduino.h>
#include "config.h" // Include config.h for constants

// Control modes
enum class ControlMode {
    JOYSTICK,
    SLIDERS
};

// Глобальная переменная для хранения текущего режима управления
extern ControlMode currentControlMode;

// Motor control functions
void initMotors();
void setMotorSpeed(int pwmLeft, int pwmRight);
void processJoystickControl(float x, float y);
void processJoystickControlWithValues(float x, float y, int* leftMotor, int* rightMotor);
void processSlidersControl(float leftSlider, float rightSlider);
void stopMotors(); 