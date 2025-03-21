#pragma once

#include <Arduino.h>

// L298N pins
#define MOTOR_LEFT_IN1 12 //in1
#define MOTOR_LEFT_IN2 13 //in2
#define MOTOR_RIGHT_IN1 15 //in3
#define MOTOR_RIGHT_IN2 14 //in4

// PWM configuration
#define MOTOR_PWM_CHANNEL_LEFT1 2  // PWM channel for left motor IN1
#define MOTOR_PWM_CHANNEL_LEFT2 3  // PWM channel for left motor IN2
#define MOTOR_PWM_CHANNEL_RIGHT1 4 // PWM channel for right motor IN1
#define MOTOR_PWM_CHANNEL_RIGHT2 5 // PWM channel for right motor IN2
#define MOTOR_PWM_FREQ 10000
#define MOTOR_PWM_RESOLUTION 8  // 8-bit resolution (0-255)

// Motor control parameters
#define MOTOR_DEADZONE 0.15f      // Значения стика меньше этого игнорируются
#define MOTOR_MIN_POWER 90        // Минимальное значение PWM для начала движения
#define MOTOR_MAX_POWER 255       // Максимальное значение PWM
#define MOTOR_TURN_THRESHOLD 0.8f // Порог для включения разворота на месте

// Control modes
enum class ControlMode {
    JOYSTICK,
    SLIDERS
};

// Motor control functions
void initMotors();
void setMotorSpeed(int pwmLeft, int pwmRight);
void processJoystickControl(float x, float y);
void processSlidersControl(float leftSlider, float rightSlider);
void stopMotors(); 