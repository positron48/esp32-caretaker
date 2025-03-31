#include "bluetooth_task.h"
#include <Arduino.h>
#include "config.h"

#if FEATURE_BLUETOOTH_ENABLED

#include <NimBLEDevice.h>
#include "joystick_config.h"
#include "motor_control.h"

// Глобальные переменные для доступа из других модулей
JoystickData joystickData;
bool hasNewJoystickData = false;
bool bleConnected = false;
bool btControlEnabled = false;

// Внутренние переменные
static bool deviceFound = false;
static NimBLEAddress* joystickAddress = nullptr;
static NimBLEClient* pClient = nullptr;
static NimBLERemoteCharacteristic* joystickCharacteristic = nullptr;
static bool serviceDiscoveryCompleted = false;

// Время последнего сканирования
static unsigned long lastScanTime = 0;

// Статус соединения для веб-интерфейса
String btConnectionStatus = "Не активировано";

// Класс для обработки найденных устройств во время сканирования
class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        Serial.printf("BLE: Найдено устройство: %s\n", advertisedDevice->toString().c_str());
        
        // Проверяем, совпадает ли имя устройства с искомым
        if (advertisedDevice->haveName() && advertisedDevice->getName() == JOYSTICK_DEVICE_NAME) {
            Serial.println("BLE: Обнаружен ExpressLRS Joystick!");
            deviceFound = true;
            
            // Сохраняем адрес устройства
            if (joystickAddress != nullptr) {
                delete joystickAddress;
            }
            joystickAddress = new NimBLEAddress(advertisedDevice->getAddress());
            
            // Остановка сканирования после нахождения нужного устройства
            NimBLEDevice::getScan()->stop();
            
            // Обновляем статус
            btConnectionStatus = "Устройство найдено, подключение...";
        }
    }
};

// Класс для управления подключением к серверу
class MyClientCallback : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pclient) {
        bleConnected = true;
        Serial.println("BLE: Подключено к ExpressLRS Joystick");
        btConnectionStatus = "Подключено";
    }

    void onDisconnect(NimBLEClient* pclient) {
        bleConnected = false;
        Serial.println("BLE: Соединение с ExpressLRS Joystick разорвано");
        joystickCharacteristic = nullptr;
        serviceDiscoveryCompleted = false;
        btConnectionStatus = "Соединение разорвано";
    }

    // Обработка авторизации
    bool onConnParamsUpdateRequest(NimBLEClient* pclient, const ble_gap_upd_params* params) {
        Serial.println("BLE: Запрос на обновление параметров соединения");
        return true;
    }

    uint32_t onPassKeyRequest() {
        Serial.println("BLE: Запрос на ввод ключа");
        // Возвращаем стандартный ключ (можно изменить при необходимости)
        return 123456;
    }

    bool onConfirmPIN(uint32_t pin) {
        Serial.printf("BLE: Запрос на подтверждение PIN: %d\n", pin);
        return true; // Автоматически подтверждаем PIN
    }
};

// Функция для сканирования устройств
void scanForBluetoothDevices() {
    Serial.println("BLE: Начинаю сканирование BLE устройств...");
    btConnectionStatus = "Сканирование...";
    
    NimBLEScan* pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
    pBLEScan->setActiveScan(true); // Активное сканирование использует больше энергии, но находит устройства быстрее
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    
    deviceFound = false; // Сбрасываем флаг перед новым сканированием
    pBLEScan->start(BLE_SCAN_TIME, false);
    
    Serial.println("BLE: Сканирование завершено");
    if (!deviceFound) {
        btConnectionStatus = "Устройство не найдено";
    }
    lastScanTime = millis();
}

// Функция для обнаружения сервисов и характеристик
bool discoverServicesAndCharacteristics() {
    if (!bleConnected || pClient == nullptr) {
        Serial.println("BLE: Невозможно обнаружить сервисы - нет подключения");
        return false;
    }
    
    Serial.println("BLE: Начинаем обнаружение сервисов...");
    btConnectionStatus = "Обнаружение сервисов...";
    
    // Получаем список всех сервисов
    std::vector<NimBLERemoteService*>* services = pClient->getServices(true);
    if (services->empty()) {
        Serial.println("BLE: Не найдено ни одного сервиса");
        btConnectionStatus = "Сервисы не найдены";
        return false;
    }
    
    Serial.printf("BLE: Найдено %d сервисов\n", services->size());
    
    // Проходим по всем сервисам
    for (NimBLERemoteService* service : *services) {
        if (service == nullptr) continue;
        
        Serial.printf("BLE: Сервис: %s\n", service->getUUID().toString().c_str());
        
        // Получаем список всех характеристик сервиса
        std::vector<NimBLERemoteCharacteristic*>* characteristics = service->getCharacteristics(true);
        if (characteristics->empty()) {
            Serial.println("  BLE: Характеристики не найдены");
            continue;
        }
        
        // Проходим по всем характеристикам
        for (NimBLERemoteCharacteristic* characteristic : *characteristics) {
            if (characteristic == nullptr) continue;
            
            Serial.printf("  BLE: Характеристика: %s\n", characteristic->getUUID().toString().c_str());
            Serial.printf("    Свойства: %s", characteristic->canRead() ? "Чтение " : "");
            Serial.printf("%s", characteristic->canWrite() ? "Запись " : "");
            Serial.printf("%s", characteristic->canNotify() ? "Уведомления " : "");
            Serial.printf("%s", characteristic->canIndicate() ? "Индикация " : "");
            Serial.println();
            
            // Пытаемся найти характеристику, которая может отправлять уведомления
            // Обычно это характеристика управления или данных джойстика
            if (characteristic->canNotify() && joystickCharacteristic == nullptr) {
                joystickCharacteristic = characteristic;
                Serial.println("BLE: Найдена подходящая характеристика для джойстика");
                
                // Регистрируем колбэк для обработки уведомлений
                if(joystickCharacteristic->subscribe(true, 
                    [](NimBLERemoteCharacteristic* pBLERemoteCharacteristic, 
                       uint8_t* pData, 
                       size_t length, 
                       bool isNotify) {
                        
                        // Обрабатываем полученные данные без вывода в консоль
                        if (length >= 14) {
                            // Извлекаем сырые данные без преобразований
                            int16_t rawLY = (pData[11] << 8) | pData[10];
                            int16_t rawLX = (pData[13] << 8) | pData[12];
                            int16_t rawRX = (pData[3] << 8) | pData[2];
                            int16_t rawRY = (pData[5] << 8) | pData[4];
                            uint32_t buttons = ((uint32_t)pData[9] << 24) | 
                                              ((uint32_t)pData[8] << 16) | 
                                              ((uint32_t)pData[7] << 8) | 
                                              pData[6];
                            
                            // Минимально необходимые данные для работы системы
                            joystickData.x = rawLX;
                            joystickData.y = rawLY;
                            joystickData.rx = rawRX;
                            joystickData.ry = rawRY;
                            joystickData.buttons = buttons;
                            hasNewJoystickData = true;
                        }
                    }, false)) {
                    Serial.println("BLE: Подписка на уведомления настроена");
                    btConnectionStatus = "Готов к работе";
                } else {
                    Serial.println("BLE: Ошибка при настройке подписки на уведомления");
                    btConnectionStatus = "Ошибка подписки на уведомления";
                    joystickCharacteristic = nullptr;
                }
            }
        }
    }
    
    // Проверяем, нашли ли мы необходимую характеристику
    if (joystickCharacteristic == nullptr) {
        Serial.println("BLE: Не удалось найти подходящую характеристику для джойстика");
        btConnectionStatus = "Не найдена характеристика джойстика";
        return false;
    }
    
    serviceDiscoveryCompleted = true;
    return true;
}

// Функция подключения к джойстику
bool connectToJoystick() {
    if (joystickAddress == nullptr) {
        Serial.println("BLE: Ошибка: адрес устройства не найден");
        btConnectionStatus = "Ошибка: адрес устройства не найден";
        return false;
    }

    Serial.printf("BLE: Подключение к ExpressLRS Joystick (%s)...\n", joystickAddress->toString().c_str());
    
    if (pClient != nullptr) {
        if(pClient->isConnected()) {
            pClient->disconnect();
        }
        NimBLEDevice::deleteClient(pClient);
    }
    
    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback(), false);
    
    // Настройки безопасности
    pClient->setConnectTimeout(10);
    
    // Подключение к устройству
    if (pClient->connect(*joystickAddress)) {
        Serial.println("BLE: Подключение успешно!");
        
        // Обнаруживаем сервисы и характеристики
        if (!discoverServicesAndCharacteristics()) {
            Serial.println("BLE: Не удалось настроить работу с джойстиком");
            btConnectionStatus = "Ошибка настройки джойстика";
            pClient->disconnect();
            return false;
        }
        
        return true;
    } else {
        Serial.println("BLE: Ошибка подключения!");
        btConnectionStatus = "Ошибка подключения";
        return false;
    }
}

// Инициализация Bluetooth
void initBluetooth() {
    NimBLEDevice::init("");
    Serial.println("BLE: Инициализация выполнена");
    
    // Настройка безопасности BLE для поддержки pairing
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    Serial.println("BLE: Настройка безопасности выполнена");
}

// Отключение от Bluetooth-устройства
void disconnectBluetooth() {
    if (pClient != nullptr && pClient->isConnected()) {
        pClient->disconnect();
    }
    
    bleConnected = false;
    hasNewJoystickData = false;
    joystickCharacteristic = nullptr;
    serviceDiscoveryCompleted = false;
    btConnectionStatus = "Отключено";
}

// Получение текущего статуса Bluetooth для веб-интерфейса
String getBtStatus() {
    // Просто возвращаем текущий статус без дополнительной информации о режиме
    return btConnectionStatus;
}

// Bluetooth task
void TaskBluetooth(void *pvParameters) {
    Serial.println("BLE: Запуск задачи Bluetooth");
    
    while(true) {
        // Поиск и подключение к джойстику только если BT активирован в настройках
        if (btControlEnabled) {
            // Сохраняем текущее время для использования в нескольких местах
            unsigned long currentTime = millis();
            
            if (!bleConnected && (currentTime - lastScanTime > BLE_RESCAN_INTERVAL || lastScanTime == 0)) {
                Serial.println("BLE: Выполняем сканирование...");
                scanForBluetoothDevices();
                
                if (deviceFound) {
                    connectToJoystick();
                }
            }
            
            // Обработка данных джойстика если он подключен
            if (bleConnected && hasNewJoystickData) {
                // Обработка будет зависеть от текущего режима управления
                if (currentControlMode == ControlMode::JOYSTICK) {
                    // В режиме джойстика используем правый стик
                    
                    // Преобразуем сырые значения в нормализованный диапазон [-1.0, 1.0]
                    // Для правого стика (rx, ry) используем те же параметры калибровки что и для левого
                    float normX = 0.0f;
                    if (joystickData.rx < LEFT_JOYSTICK_X_CENTER) {
                        normX = -1.0f + (joystickData.rx - LEFT_JOYSTICK_X_MIN) / 
                                        (float)(LEFT_JOYSTICK_X_CENTER - LEFT_JOYSTICK_X_MIN);
                    } else {
                        normX = (joystickData.rx - LEFT_JOYSTICK_X_CENTER) / 
                                (float)(LEFT_JOYSTICK_X_MAX - LEFT_JOYSTICK_X_CENTER);
                    }
                    
                    float normY = 0.0f;
                    if (joystickData.ry < LEFT_JOYSTICK_Y_CENTER) {
                        normY = -1.0f + (joystickData.ry - LEFT_JOYSTICK_Y_MIN) / 
                                        (float)(LEFT_JOYSTICK_Y_CENTER - LEFT_JOYSTICK_Y_MIN);
                    } else {
                        normY = (joystickData.ry - LEFT_JOYSTICK_Y_CENTER) / 
                                (float)(LEFT_JOYSTICK_Y_MAX - LEFT_JOYSTICK_Y_CENTER);
                    }
                    
                    // Ограничение диапазона и фильтрация
                    if (fabs(normX) < 0.1f) normX = 0.0f;
                    if (fabs(normY) < 0.1f) normY = 0.0f;
                    
                    if (normX < -1.0f) normX = -1.0f;
                    if (normX > 1.0f) normX = 1.0f;
                    if (normY < -1.0f) normY = -1.0f;
                    if (normY > 1.0f) normY = 1.0f;
                    
                    // Вызываем функцию управления с нормализованными значениями
                    int leftMotorValue, rightMotorValue;
                    processJoystickControlWithValues(normX, normY, &leftMotorValue, &rightMotorValue);
                } 
                else if (currentControlMode == ControlMode::SLIDERS) {
                    // В режиме слайдеров используем левый стик Y и правый стик Y как отдельные слайдеры
                    
                    // Преобразуем сырые значения в нормализованный диапазон [-1.0, 1.0]
                    float leftSlider = 0.0f;
                    if (joystickData.y < LEFT_JOYSTICK_Y_CENTER) {
                        leftSlider = -1.0f + (joystickData.y - LEFT_JOYSTICK_Y_MIN) / 
                                        (float)(LEFT_JOYSTICK_Y_CENTER - LEFT_JOYSTICK_Y_MIN);
                    } else {
                        leftSlider = (joystickData.y - LEFT_JOYSTICK_Y_CENTER) / 
                                (float)(LEFT_JOYSTICK_Y_MAX - LEFT_JOYSTICK_Y_CENTER);
                    }
                    
                    float rightSlider = 0.0f;
                    if (joystickData.ry < LEFT_JOYSTICK_Y_CENTER) {
                        rightSlider = -1.0f + (joystickData.ry - LEFT_JOYSTICK_Y_MIN) / 
                                        (float)(LEFT_JOYSTICK_Y_CENTER - LEFT_JOYSTICK_Y_MIN);
                    } else {
                        rightSlider = (joystickData.ry - LEFT_JOYSTICK_Y_CENTER) / 
                                (float)(LEFT_JOYSTICK_Y_MAX - LEFT_JOYSTICK_Y_CENTER);
                    }
                    
                    // Ограничение диапазона и фильтрация
                    if (fabs(leftSlider) < 0.1f) leftSlider = 0.0f;
                    if (fabs(rightSlider) < 0.1f) rightSlider = 0.0f;
                    
                    if (leftSlider < -1.0f) leftSlider = -1.0f;
                    if (leftSlider > 1.0f) leftSlider = 1.0f;
                    if (rightSlider < -1.0f) rightSlider = -1.0f;
                    if (rightSlider > 1.0f) rightSlider = 1.0f;
                    
                    // Вызываем функцию управления слайдерами
                    processSlidersControl(leftSlider, rightSlider);
                }
                
                hasNewJoystickData = false;
            }
        } else {
            // Если BT отключен в настройках и есть активное соединение, отключаемся
            if (bleConnected) {
                disconnectBluetooth();
            }
        }
        
        // Добавим небольшую задержку для экономии ресурсов
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Уменьшаем задержку для более быстрой реакции
    }
}
#endif // FEATURE_BLUETOOTH_ENABLED 