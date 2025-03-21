#include <Arduino.h>
#include "task_stats.h"
#include "log_task.h"
#include "wifi_manager.h"
#include "http_server_task.h"
#include "camera_config.h"
#include <esp_camera.h>
#include <NimBLEDevice.h>
#include "motor_control.h"

// LED pin
const int ledPin = LED_PIN;

// Название Bluetooth джойстика, к которому нужно подключиться
const char* JOYSTICK_DEVICE_NAME = "ExpressLRS Joystick";

// Время сканирования BLE устройств в секундах
const int SCAN_TIME = 5;

// Флаги статуса Bluetooth
bool deviceFound = false;
bool connected = false;

// Адрес найденного устройства
NimBLEAddress* joystickAddress = nullptr;
NimBLEClient* pClient = nullptr;

// Время последнего сканирования
unsigned long lastScanTime = 0;
// Время последнего лога данных джойстика
unsigned long lastJoystickLogTime = 0;

// Структура для хранения данных джойстика
struct JoystickData {
    int16_t x = 0;  // левый джойстик, x-axis
    int16_t y = 0;  // левый джойстик, y-axis
    int16_t rx = 0; // правый джойстик, x-axis
    int16_t ry = 0; // правый джойстик, y-axis
    uint32_t buttons = 0; // битовые флаги для кнопок
};

JoystickData joystickData;
bool hasNewJoystickData = false;

// Поддержка обнаружения сервисов
NimBLERemoteCharacteristic* joystickCharacteristic = nullptr;
bool serviceDiscoveryCompleted = false;

bool initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = XCLK_FREQ;
    config.pixel_format = PIXEL_FORMAT;

    // Init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        Serial.println("PSRAM found");
    } else {
        Serial.println("PSRAM not found");
    }

    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = JPEG_QUALITY;
    config.fb_count = FB_COUNT;

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }

    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, FRAME_SIZE);
    s->set_quality(s, JPEG_QUALITY);
    return true;
}

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
        }
    }
};

// Класс для управления подключением к серверу
class MyClientCallback : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pclient) {
        connected = true;
        Serial.println("BLE: Подключено к ExpressLRS Joystick");
    }

    void onDisconnect(NimBLEClient* pclient) {
        connected = false;
        Serial.println("BLE: Соединение с ExpressLRS Joystick разорвано");
        joystickCharacteristic = nullptr;
        serviceDiscoveryCompleted = false;
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
    
    NimBLEScan* pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
    pBLEScan->setActiveScan(true); // Активное сканирование использует больше энергии, но находит устройства быстрее
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    
    deviceFound = false; // Сбрасываем флаг перед новым сканированием
    pBLEScan->start(SCAN_TIME, false);
    
    Serial.println("BLE: Сканирование завершено");
    lastScanTime = millis();
}

// Функция для обнаружения сервисов и характеристик
bool discoverServicesAndCharacteristics() {
    if (!connected || pClient == nullptr) {
        Serial.println("BLE: Невозможно обнаружить сервисы - нет подключения");
        return false;
    }
    
    Serial.println("BLE: Начинаем обнаружение сервисов...");
    
    // Получаем список всех сервисов
    std::vector<NimBLERemoteService*>* services = pClient->getServices(true);
    if (services->empty()) {
        Serial.println("BLE: Не найдено ни одного сервиса");
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
                        
                        // Проверяем, прошла ли 1 секунда с момента последнего лога
                        unsigned long currentTime = millis();
                        if (currentTime - lastJoystickLogTime < 1000) {
                            return; // Пропускаем вывод, если прошло менее секунды
                        }
                        
                        // Обновляем время последнего лога
                        lastJoystickLogTime = currentTime;
                        
                        // Выводим сырые данные, сгруппированные по предполагаемым значениям
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
                            
                            // Выводим данные в одну строку
                            Serial.printf("BLE: LX=%d, LY=%d, RX=%d, RY=%d, BTN=0x%08X, H0=%d, H1=%d", 
                                rawLX, rawLY, rawRX, rawRY, buttons, pData[0], pData[1]);
                            
                            // Если есть дополнительные данные, выводим их тоже
                            if (length >= 18) {
                                uint32_t extra = ((uint32_t)pData[17] << 24) | 
                                                ((uint32_t)pData[16] << 16) | 
                                                ((uint32_t)pData[15] << 8) | 
                                                pData[14];
                                Serial.printf(", EXTRA=0x%08X", extra);
                            }
                            
                            Serial.println();
                            
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
                } else {
                    Serial.println("BLE: Ошибка при настройке подписки на уведомления");
                    joystickCharacteristic = nullptr;
                }
            }
        }
    }
    
    // Проверяем, нашли ли мы необходимую характеристику
    if (joystickCharacteristic == nullptr) {
        Serial.println("BLE: Не удалось найти подходящую характеристику для джойстика");
        return false;
    }
    
    serviceDiscoveryCompleted = true;
    return true;
}

// Функция подключения к джойстику
bool connectToJoystick() {
    if (joystickAddress == nullptr) {
        Serial.println("BLE: Ошибка: адрес устройства не найден");
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
            pClient->disconnect();
            return false;
        }
        
        return true;
    } else {
        Serial.println("BLE: Ошибка подключения!");
        return false;
    }
}

// Bluetooth task
void TaskBluetooth(void *pvParameters) {
    Serial.println("BLE: Запуск задачи Bluetooth");
    
    while(true) {
        // Поиск и подключение к джойстику
        if (!connected && (millis() - lastScanTime > 10000 || lastScanTime == 0)) {
            Serial.println("BLE: Выполняем сканирование...");
            scanForBluetoothDevices();
            
            if (deviceFound) {
                connectToJoystick();
            }
        }
        
        // Обработка данных джойстика если он подключен
        if (connected && hasNewJoystickData) {
            
            hasNewJoystickData = false;
        }
        
        // Добавим небольшую задержку для экономии ресурсов
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Уменьшаем задержку для более быстрой реакции
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-CAM Streaming Server");

    // Initialize PSRAM
    if(!psramInit()){
        Serial.println("PSRAM initialization failed");
        return;
    }

    // Initialize LED
    ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION);
    ledcAttachPin(ledPin, LED_CHANNEL);
    ledcWrite(LED_CHANNEL, 0); // Start with LED off

    // Initialize camera
    if(!initCamera()) {
        Serial.println("Camera initialization failed");
        return;
    }

    // Initialize BLE
    NimBLEDevice::init("");
    Serial.println("BLE: Инициализация выполнена");
    
    // Настройка безопасности BLE для поддержки pairing
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    Serial.println("BLE: Настройка безопасности выполнена");

    // Initialize WiFi
    initWiFi();

    // Create log task on core 1
    xTaskCreatePinnedToCore(
        TaskLog,
        "Log",
        2048,
        NULL,
        1,
        NULL,
        1
    );

    // Create HTTP server task on core 1
    xTaskCreatePinnedToCore(
        TaskHttpServer,
        "HttpServer",
        4096,
        NULL,
        1,
        NULL,
        1
    );
    
    // Create Bluetooth task on core 0
    xTaskCreatePinnedToCore(
        TaskBluetooth,
        "Bluetooth",
        4096,
        NULL,
        1,
        NULL,
        0
    );
}

void loop() {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}