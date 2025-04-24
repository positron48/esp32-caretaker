#pragma once

#if FEATURE_BLUETOOTH_ENABLED

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "joystick_config.h"

// Bluetooth initialization
void initBluetooth();

// Task for Bluetooth processing
void TaskBluetooth(void *pvParameters);

// Disconnect from Bluetooth device
void disconnectBluetooth();

// Get current Bluetooth status for web interface
String getBtStatus();

// External variable for accessing joystick data
extern JoystickData joystickData;

// Flag for new data available
extern bool hasNewJoystickData;

// Connection flag
extern bool bleConnected; 

#endif // FEATURE_BLUETOOTH_ENABLED
