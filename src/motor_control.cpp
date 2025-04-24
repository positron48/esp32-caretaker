#include "motor_control.h"
#include <math.h>

// Global variable for control mode
ControlMode currentControlMode = ControlMode::JOYSTICK;

// Static variables for storing previous motor signal values
static int lastLeftMotorValue = 0;
static int lastRightMotorValue = 0;

// Helper function for nonlinear transformation of joystick values
float processJoystickAxis(float value) {
    // Apply deadzone
    if (fabs(value) < MOTOR_DEADZONE) {
        return 0.0f;
    }
    
    // Normalize value after deadzone
    float normalized = (fabs(value) - MOTOR_DEADZONE) / (1.0f - MOTOR_DEADZONE);
    
    // Apply quadratic characteristic for more precise control at low speeds
    normalized = normalized * normalized;
    
    // Scale to PWM range considering minimum power
    float pwm = MOTOR_MIN_POWER + normalized * (MOTOR_MAX_POWER - MOTOR_MIN_POWER);
    
    // Return with original value sign
    return value > 0 ? pwm : -pwm;
}

void initMotors() {
    // Setup PWM channels
    ledcSetup(MOTOR_PWM_CHANNEL_LEFT1, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
    ledcSetup(MOTOR_PWM_CHANNEL_LEFT2, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
    ledcSetup(MOTOR_PWM_CHANNEL_RIGHT1, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
    ledcSetup(MOTOR_PWM_CHANNEL_RIGHT2, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
    
    // Attach PWM channels to pins
    ledcAttachPin(MOTOR_LEFT_IN1, MOTOR_PWM_CHANNEL_LEFT1);
    ledcAttachPin(MOTOR_LEFT_IN2, MOTOR_PWM_CHANNEL_LEFT2);
    ledcAttachPin(MOTOR_RIGHT_IN1, MOTOR_PWM_CHANNEL_RIGHT1);
    ledcAttachPin(MOTOR_RIGHT_IN2, MOTOR_PWM_CHANNEL_RIGHT2);
    
    // Initially stop motors
    stopMotors();
}

void setMotorSpeed(int pwmLeft, int pwmRight) {
    // Check if motor values have changed
    bool valuesChanged = (pwmLeft != lastLeftMotorValue) || (pwmRight != lastRightMotorValue);
    
    // Save new values
    lastLeftMotorValue = pwmLeft;
    lastRightMotorValue = pwmRight;
    
    // Left motor
    if (pwmLeft > 0) {
        ledcWrite(MOTOR_PWM_CHANNEL_LEFT1, pwmLeft);
        ledcWrite(MOTOR_PWM_CHANNEL_LEFT2, 0);
    } else {
        ledcWrite(MOTOR_PWM_CHANNEL_LEFT1, 0);
        ledcWrite(MOTOR_PWM_CHANNEL_LEFT2, -pwmLeft);
    }
    
    // Right motor
    if (pwmRight > 0) {
        ledcWrite(MOTOR_PWM_CHANNEL_RIGHT1, pwmRight);
        ledcWrite(MOTOR_PWM_CHANNEL_RIGHT2, 0);
    } else {
        ledcWrite(MOTOR_PWM_CHANNEL_RIGHT1, 0);
        ledcWrite(MOTOR_PWM_CHANNEL_RIGHT2, -pwmRight);
    }
    
    // Log changes if they occurred
    if (valuesChanged) {
        // Normalize values to the range [-1.0, 1.0] for output
        float normLeftMotor = pwmLeft / (float)MOTOR_MAX_POWER;
        float normRightMotor = pwmRight / (float)MOTOR_MAX_POWER;
        
        Serial.printf("MOTORS: left=%.2f, right=%.2f\n", normLeftMotor, normRightMotor);
    }
}

void processJoystickControl(float x, float y) {
    // Processing input values
    float processedX = processJoystickAxis(x);
    float processedY = processJoystickAxis(y);
    
    float leftMotor = 0;
    float rightMotor = 0;
    
    // Check for in-place rotation mode
    if (fabs(x) > MOTOR_TURN_THRESHOLD && fabs(y) < MOTOR_DEADZONE) {
        // In-place rotation - one motor forward, the other backward
        float turnPower = fabs(processedX);
        if (x > 0) {
            // Turn right
            leftMotor = turnPower;
            rightMotor = -turnPower;
        } else {
            // Turn left
            leftMotor = -turnPower;
            rightMotor = turnPower;
        }
    } else {
        // Normal movement
        leftMotor = processedY;
        rightMotor = processedY;
        
        // Apply turn by slowing one track
        if (fabs(processedX) > 0) {
            float turnFactor = 1.0f - fabs(processedX / MOTOR_MAX_POWER);
            if (x > 0) {
                // Turn right
                rightMotor *= turnFactor;
            } else {
                // Turn left
                leftMotor *= turnFactor;
            }
        }
    }
    
    // Apply constraints and send to motors
    int pwmLeft = constrain(leftMotor, -MOTOR_MAX_POWER, MOTOR_MAX_POWER);
    int pwmRight = constrain(rightMotor, -MOTOR_MAX_POWER, MOTOR_MAX_POWER);
    
    setMotorSpeed(pwmLeft, pwmRight);
}

// New function to get motor values without sending to driver
void processJoystickControlWithValues(float x, float y, int* leftMotor, int* rightMotor) {
    // Processing input values
    float processedX = processJoystickAxis(x);
    float processedY = processJoystickAxis(y);
    
    float leftMotorValue = 0;
    float rightMotorValue = 0;
    
    // Check for in-place rotation mode
    if (fabs(x) > MOTOR_TURN_THRESHOLD && fabs(y) < MOTOR_DEADZONE) {
        // In-place rotation - one motor forward, the other backward
        float turnPower = fabs(processedX);
        if (x > 0) {
            // Turn right
            leftMotorValue = turnPower;
            rightMotorValue = -turnPower;
        } else {
            // Turn left
            leftMotorValue = -turnPower;
            rightMotorValue = turnPower;
        }
    } else {
        // Normal movement
        leftMotorValue = processedY;
        rightMotorValue = processedY;
        
        // Apply turn by slowing one track
        if (fabs(processedX) > 0) {
            float turnFactor = 1.0f - fabs(processedX / MOTOR_MAX_POWER);
            if (x > 0) {
                // Turn right
                rightMotorValue *= turnFactor;
            } else {
                // Turn left
                leftMotorValue *= turnFactor;
            }
        }
    }
    
    // Apply constraints and send to driver
    *leftMotor = constrain(leftMotorValue, -MOTOR_MAX_POWER, MOTOR_MAX_POWER);
    *rightMotor = constrain(rightMotorValue, -MOTOR_MAX_POWER, MOTOR_MAX_POWER);
    
    // Also send to motors
    setMotorSpeed(*leftMotor, *rightMotor);
}

void processSlidersControl(float leftSlider, float rightSlider) {
    // Apply deadzone
    if (abs(leftSlider) < MOTOR_DEADZONE) leftSlider = 0;
    if (abs(rightSlider) < MOTOR_DEADZONE) rightSlider = 0;

    // Apply quadratic characteristic for better control at low speeds
    float leftPower = leftSlider * abs(leftSlider);   // Preserves sign but makes control more precise
    float rightPower = rightSlider * abs(rightSlider);

    // Scale to PWM range and apply minimum power threshold
    int leftPWM = 0;
    int rightPWM = 0;

    if (leftPower != 0) {
        leftPWM = (int)(abs(leftPower) * (MOTOR_MAX_POWER - MOTOR_MIN_POWER) + MOTOR_MIN_POWER);
        if (leftPower < 0) leftPWM = -leftPWM;
    }

    if (rightPower != 0) {
        rightPWM = (int)(abs(rightPower) * (MOTOR_MAX_POWER - MOTOR_MIN_POWER) + MOTOR_MIN_POWER);
        if (rightPower < 0) rightPWM = -rightPWM;
    }

    setMotorSpeed(leftPWM, rightPWM);
}

void stopMotors() {
    // Set all PWM channels to 0
    ledcWrite(MOTOR_PWM_CHANNEL_LEFT1, 0);
    ledcWrite(MOTOR_PWM_CHANNEL_LEFT2, 0);
    ledcWrite(MOTOR_PWM_CHANNEL_RIGHT1, 0);
    ledcWrite(MOTOR_PWM_CHANNEL_RIGHT2, 0);
} 