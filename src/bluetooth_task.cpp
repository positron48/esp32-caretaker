#include "bluetooth_task.h"
#include <Arduino.h>
#include "config.h"

#if FEATURE_BLUETOOTH_ENABLED

#include <NimBLEDevice.h>
#include "joystick_config.h"
#include "motor_control.h"

// Global variables for access from other modules
JoystickData joystickData;
bool hasNewJoystickData = false;
bool bleConnected = false;
bool btControlEnabled = false;

// Internal variables
static bool deviceFound = false;
static NimBLEAddress* joystickAddress = nullptr;
static NimBLEClient* pClient = nullptr;
static NimBLERemoteCharacteristic* joystickCharacteristic = nullptr;
static bool serviceDiscoveryCompleted = false;

// Time of last scan
static unsigned long lastScanTime = 0;

// Connection status for web interface
String btConnectionStatus = "Not activated";

// Class for handling devices found during scanning
class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        Serial.printf("BLE: Device found: %s\n", advertisedDevice->toString().c_str());
        
        // Check if the device name matches the one we're looking for
        if (advertisedDevice->haveName() && advertisedDevice->getName() == JOYSTICK_DEVICE_NAME) {
            Serial.println("BLE: ExpressLRS Joystick detected!");
            deviceFound = true;
            
            // Save the device address
            if (joystickAddress != nullptr) {
                delete joystickAddress;
            }
            joystickAddress = new NimBLEAddress(advertisedDevice->getAddress());
            
            // Stop scanning after finding the needed device
            NimBLEDevice::getScan()->stop();
            
            // Update status
            btConnectionStatus = "Device found, connecting...";
        }
    }
};

// Class for managing connection to the server
class MyClientCallback : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pclient) {
        bleConnected = true;
        Serial.println("BLE: Connected to ExpressLRS Joystick");
        btConnectionStatus = "Connected";
    }

    void onDisconnect(NimBLEClient* pclient) {
        bleConnected = false;
        Serial.println("BLE: Connection to ExpressLRS Joystick lost");
        joystickCharacteristic = nullptr;
        serviceDiscoveryCompleted = false;
        btConnectionStatus = "Connection lost";
    }

    // Handling authorization
    bool onConnParamsUpdateRequest(NimBLEClient* pclient, const ble_gap_upd_params* params) {
        Serial.println("BLE: Authorization request");
        return true;
    }

    uint32_t onPassKeyRequest() {
        Serial.println("BLE: PIN request");
        // Return standard key (can be changed if needed)
        return 123456;
    }

    bool onConfirmPIN(uint32_t pin) {
        Serial.printf("BLE: PIN confirmation request: %d\n", pin);
        return true; // Automatically confirm PIN
    }
};

// Function for scanning devices
void scanForBluetoothDevices() {
    Serial.println("BLE: Starting BLE device scan...");
    btConnectionStatus = "Scanning...";
    
    NimBLEScan* pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
    pBLEScan->setActiveScan(true); // Active scan uses more power but finds devices faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    
    deviceFound = false; // Reset flag before new scan
    pBLEScan->start(BLE_SCAN_TIME, false);
    
    Serial.println("BLE: Scan completed");
    if (!deviceFound) {
        btConnectionStatus = "Device not found";
    }
    lastScanTime = millis();
}

// Function for discovering services and characteristics
bool discoverServicesAndCharacteristics() {
    if (!bleConnected || pClient == nullptr) {
        Serial.println("BLE: Cannot discover services - no connection");
        return false;
    }
    
    Serial.println("BLE: Starting service discovery...");
    btConnectionStatus = "Service discovery...";
    
    // Get list of all services
    std::vector<NimBLERemoteService*>* services = pClient->getServices(true);
    if (services->empty()) {
        Serial.println("BLE: No services found");
        btConnectionStatus = "Services not found";
        return false;
    }
    
    Serial.printf("BLE: Found %d services\n", services->size());
    
    // Go through all services
    for (NimBLERemoteService* service : *services) {
        if (service == nullptr) continue;
        
        Serial.printf("BLE: Service: %s\n", service->getUUID().toString().c_str());
        
        // Get list of all characteristics of the service
        std::vector<NimBLERemoteCharacteristic*>* characteristics = service->getCharacteristics(true);
        if (characteristics->empty()) {
            Serial.println("  BLE: No characteristics found");
            continue;
        }
        
        // Go through all characteristics
        for (NimBLERemoteCharacteristic* characteristic : *characteristics) {
            if (characteristic == nullptr) continue;
            
            Serial.printf("  BLE: Characteristic: %s\n", characteristic->getUUID().toString().c_str());
            Serial.printf("    Properties: %s", characteristic->canRead() ? "Read " : "");
            Serial.printf("%s", characteristic->canWrite() ? "Write " : "");
            Serial.printf("%s", characteristic->canNotify() ? "Notifications " : "");
            Serial.printf("%s", characteristic->canIndicate() ? "Indication " : "");
            Serial.println();
            
            // Try to find a characteristic that can send notifications
            // Usually this is the control or joystick data characteristic
            if (characteristic->canNotify() && joystickCharacteristic == nullptr) {
                joystickCharacteristic = characteristic;
                Serial.println("BLE: Found suitable characteristic for joystick");
                
                // Register callback for handling notifications
                if(joystickCharacteristic->subscribe(true, 
                    [](NimBLERemoteCharacteristic* pBLERemoteCharacteristic, 
                       uint8_t* pData, 
                       size_t length, 
                       bool isNotify) {
                        
                        // Process received data without output to console
                        if (length >= 14) {
                            // Extract raw data without conversions
                            int16_t rawLY = (pData[11] << 8) | pData[10];
                            int16_t rawLX = (pData[13] << 8) | pData[12];
                            int16_t rawRX = (pData[3] << 8) | pData[2];
                            int16_t rawRY = (pData[5] << 8) | pData[4];
                            uint32_t buttons = ((uint32_t)pData[9] << 24) | 
                                              ((uint32_t)pData[8] << 16) | 
                                              ((uint32_t)pData[7] << 8) | 
                                              pData[6];
                            
                            // Minimum necessary data for system operation
                            joystickData.x = rawLX;
                            joystickData.y = rawLY;
                            joystickData.rx = rawRX;
                            joystickData.ry = rawRY;
                            joystickData.buttons = buttons;
                            hasNewJoystickData = true;
                        }
                    }, false)) {
                    Serial.println("BLE: Notification subscription set up");
                    btConnectionStatus = "Ready to work";
                } else {
                    Serial.println("BLE: Error setting up notification subscription");
                    btConnectionStatus = "Notification subscription error";
                    joystickCharacteristic = nullptr;
                }
            }
        }
    }
    
    // Check if we found the necessary characteristic
    if (joystickCharacteristic == nullptr) {
        Serial.println("BLE: Unable to find suitable characteristic for joystick");
        btConnectionStatus = "Joystick characteristic not found";
        return false;
    }
    
    serviceDiscoveryCompleted = true;
    return true;
}

// Function for connecting to the joystick
bool connectToJoystick() {
    if (joystickAddress == nullptr) {
        Serial.println("BLE: Error: device address not found");
        btConnectionStatus = "Error: device address not found";
        return false;
    }

    Serial.printf("BLE: Connecting to ExpressLRS Joystick (%s)...\n", joystickAddress->toString().c_str());
    
    if (pClient != nullptr) {
        if(pClient->isConnected()) {
            pClient->disconnect();
        }
        NimBLEDevice::deleteClient(pClient);
    }
    
    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback(), false);
    
    // Security settings
    pClient->setConnectTimeout(10);
    
    // Connect to the device
    if (pClient->connect(*joystickAddress)) {
        Serial.println("BLE: Connection successful!");
        
        // Discover services and characteristics
        if (!discoverServicesAndCharacteristics()) {
            Serial.println("BLE: Unable to set up joystick operation");
            btConnectionStatus = "Joystick setup error";
            pClient->disconnect();
            return false;
        }
        
        return true;
    } else {
        Serial.println("BLE: Connection error!");
        btConnectionStatus = "Connection error";
        return false;
    }
}

// Bluetooth initialization
void initBluetooth() {
    NimBLEDevice::init("");
    Serial.println("BLE: Initialization completed");
    
    // BLE security settings for supporting pairing
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    Serial.println("BLE: Security settings completed");
}

// Disconnecting from Bluetooth device
void disconnectBluetooth() {
    if (pClient != nullptr && pClient->isConnected()) {
        pClient->disconnect();
    }
    
    bleConnected = false;
    hasNewJoystickData = false;
    joystickCharacteristic = nullptr;
    serviceDiscoveryCompleted = false;
    btConnectionStatus = "Disconnected";
}

// Getting current Bluetooth status for web interface
String getBtStatus() {
    // Simply return current status without additional information about mode
    return btConnectionStatus;
}

// Bluetooth task
void TaskBluetooth(void *pvParameters) {
    Serial.println("BLE: Starting Bluetooth task");
    
    while(true) {
        // Search and connect to joystick only if BT is enabled in settings
        if (btControlEnabled) {
            // Save current time for use in multiple places
            unsigned long currentTime = millis();
            
            if (!bleConnected && (currentTime - lastScanTime > BLE_RESCAN_INTERVAL || lastScanTime == 0)) {
                Serial.println("BLE: Performing scan...");
                scanForBluetoothDevices();
                
                if (deviceFound) {
                    connectToJoystick();
                }
            }
            
            // Process joystick data if connected
            if (bleConnected && hasNewJoystickData) {
                // Processing will depend on current control mode
                if (currentControlMode == ControlMode::JOYSTICK) {
                    // In joystick mode, use right stick
                    
                    // Convert raw values to normalized range [-1.0, 1.0]
                    // For right stick (rx, ry) use the same calibration parameters as for left stick
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
                    
                    // Limit range and filtering
                    if (fabs(normX) < 0.1f) normX = 0.0f;
                    if (fabs(normY) < 0.1f) normY = 0.0f;
                    
                    if (normX < -1.0f) normX = -1.0f;
                    if (normX > 1.0f) normX = 1.0f;
                    if (normY < -1.0f) normY = -1.0f;
                    if (normY > 1.0f) normY = 1.0f;
                    
                    // Call function with normalized values
                    int leftMotorValue, rightMotorValue;
                    processJoystickControlWithValues(normX, normY, &leftMotorValue, &rightMotorValue);
                } 
                else if (currentControlMode == ControlMode::SLIDERS) {
                    // In sliders mode, use left stick Y and right stick Y as separate sliders
                    
                    // Convert raw values to normalized range [-1.0, 1.0]
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
                    
                    // Limit range and filtering
                    if (fabs(leftSlider) < 0.1f) leftSlider = 0.0f;
                    if (fabs(rightSlider) < 0.1f) rightSlider = 0.0f;
                    
                    if (leftSlider < -1.0f) leftSlider = -1.0f;
                    if (leftSlider > 1.0f) leftSlider = 1.0f;
                    if (rightSlider < -1.0f) rightSlider = -1.0f;
                    if (rightSlider > 1.0f) rightSlider = 1.0f;
                    
                    // Call function with sliders
                    processSlidersControl(leftSlider, rightSlider);
                }
                
                hasNewJoystickData = false;
            }
        } else {
            // If BT is disabled in settings and there's an active connection, disconnect
            if (bleConnected) {
                disconnectBluetooth();
            }
        }
        
        // Add small delay to save resources
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Reduce delay for faster reaction
    }
}
#endif // FEATURE_BLUETOOTH_ENABLED 