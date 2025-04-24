#pragma once

#if FEATURE_BLUETOOTH_ENABLED

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "joystick_config.h"

// Инициализация Bluetooth
void initBluetooth();

// Задача для обработки Bluetooth
void TaskBluetooth(void *pvParameters);

// Отключение от Bluetooth-устройства
void disconnectBluetooth();

// Получение текущего статуса Bluetooth для веб-интерфейса
String getBtStatus();

// Внешняя переменная для доступа к данным джойстика
extern JoystickData joystickData;

// Флаг наличия новых данных
extern bool hasNewJoystickData;

// Флаг подключения
extern bool bleConnected; 

#endif // FEATURE_BLUETOOTH_ENABLED
