#include "http_server_task.h"
#include "stream_task.h"
#include "motor_control.h"
#include "ArduinoJson.h"
#include "html_content.h"
#include "bluetooth_task.h"
#include "config.h"

// Global variables for feature state
#if !FEATURE_BLUETOOTH_ENABLED
// When feature is disabled, define it here since bluetooth_task.cpp might not be compiled
bool btControlEnabled = false;
#endif

void TaskHttpServer(void* parameter) {

    // Initialize motors
    initMotors();

    WebServer server(80);

    // Setup routes
    server.on("/", HTTP_GET, [&server]() {
        // Send gzipped HTML content with appropriate headers
        server.sendHeader("Content-Encoding", "gzip");
        server.sendHeader("Cache-Control", "max-age=86400");
        server.send_P(200, "text/html", (const char*)INDEX_HTML, INDEX_HTML_SIZE);
    });

    // LED control endpoint - conditionally register if LED control is enabled
    #if FEATURE_LED_CONTROL_ENABLED
    server.on("/led", HTTP_GET, [&server]() {
        int pwmValue = 0;
        
        // Check if brightness parameter is provided for smooth control
        if (server.hasArg("brightness")) {
            // Get brightness value (0-100)
            int brightness = server.arg("brightness").toInt();
            
            // Convert brightness value to PWM value (0-4095)
            // Using maximum from LED_RESOLUTION (12 bit = 4095)
            pwmValue = map(brightness, 0, 100, 0, (1 << LED_RESOLUTION) - 1);
        }
        
        ledcWrite(LED_CHANNEL, pwmValue);
        server.send(200, "text/plain", "OK");
    });
    #endif

    // Quality control endpoint with new parameters for resolution and compression
    server.on("/quality", HTTP_GET, [&server]() {
        String resolution = server.arg("resolution");
        String quality = server.arg("quality");
        
        sensor_t * s = esp_camera_sensor_get();
        
        if (s != nullptr) {
            // Set resolution based on the parameter
            if (resolution == "QQVGA") {
                s->set_framesize(s, FRAMESIZE_QQVGA); // 160x120
            } else if (resolution == "QCIF") {
                s->set_framesize(s, FRAMESIZE_QCIF); // 176x144
            } else if (resolution == "HQVGA") {
                s->set_framesize(s, FRAMESIZE_HQVGA); // 240x176
            } else if (resolution == "240X240") {
                s->set_framesize(s, FRAMESIZE_240X240); // 240x240
            } else if (resolution == "QVGA") {
                s->set_framesize(s, FRAMESIZE_QVGA); // 320x240
            } else if (resolution == "CIF") {
                s->set_framesize(s, FRAMESIZE_CIF); // 400x296
            } else if (resolution == "HVGA") {
                s->set_framesize(s, FRAMESIZE_HVGA); // 480x320
            } else if (resolution == "VGA") {
                s->set_framesize(s, FRAMESIZE_VGA); // 640x480
            } else if (resolution == "SVGA") {
                s->set_framesize(s, FRAMESIZE_SVGA); // 800x600
            } else if (resolution == "XGA") {
                s->set_framesize(s, FRAMESIZE_XGA); // 1024x768
            } else if (resolution == "HD") {
                s->set_framesize(s, FRAMESIZE_HD); // 1280x720
            } else if (resolution == "SXGA") {
                s->set_framesize(s, FRAMESIZE_SXGA); // 1280x1024
            } else if (resolution == "UXGA") {
                s->set_framesize(s, FRAMESIZE_UXGA); // 1600x1200
            }
            
            // Set quality (compression) if provided
            if (quality.length() > 0) {
                int qualityValue = quality.toInt();
                qualityValue = constrain(qualityValue, 10, 63);
                s->set_quality(s, qualityValue);
            }
            
            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "Camera sensor not found");
        }
    });

    // Get camera settings
    server.on("/camera/settings", HTTP_GET, [&server]() {
        sensor_t * s = esp_camera_sensor_get();
        
        if (s != nullptr) {
            StaticJsonDocument<200> doc;
            
            // Map framesize to string representation
            int framesize = s->status.framesize;
            const char* resolution = "QVGA"; // Default
            
            switch(framesize) {
                case FRAMESIZE_QQVGA:
                    resolution = "QQVGA";
                    break;
                case FRAMESIZE_QCIF:
                    resolution = "QCIF";
                    break;
                case FRAMESIZE_HQVGA:
                    resolution = "HQVGA";
                    break;
                case FRAMESIZE_240X240:
                    resolution = "240X240";
                    break;
                case FRAMESIZE_QVGA:
                    resolution = "QVGA";
                    break;
                case FRAMESIZE_CIF:
                    resolution = "CIF";
                    break;
                case FRAMESIZE_HVGA:
                    resolution = "HVGA";
                    break;
                case FRAMESIZE_VGA:
                    resolution = "VGA";
                    break;
                case FRAMESIZE_SVGA:
                    resolution = "SVGA";
                    break;
                case FRAMESIZE_XGA:
                    resolution = "XGA";
                    break;
                case FRAMESIZE_HD:
                    resolution = "HD";
                    break;
                case FRAMESIZE_SXGA:
                    resolution = "SXGA";
                    break;
                case FRAMESIZE_UXGA:
                    resolution = "UXGA";
                    break;
                default:
                    resolution = "QVGA";
            }
            
            doc["resolution"] = resolution;
            doc["quality"] = s->status.quality;
            doc["framesize"] = framesize; // Add numeric framesize for compatibility
            
            String response;
            serializeJson(doc, response);
            server.send(200, "application/json", response);
        } else {
            server.send(500, "text/plain", "Camera sensor not found");
        }
    });

    // Control endpoint for joystick and sliders
    server.on("/control", HTTP_POST, [&server]() {
        StaticJsonDocument<200> doc;
        if (server.hasArg("plain")) {
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (error) {
                server.send(400, "text/plain", "Invalid JSON");
                return;
            }

            const char* mode = doc["mode"] | "joystick";

            // If switching control mode - always process, even with active BT
            if (doc.containsKey("mode") && !doc.containsKey("x") && !doc.containsKey("y") && 
                !doc.containsKey("left") && !doc.containsKey("right")) {
                
                if (strcmp(mode, "joystick") == 0) {
                    currentControlMode = ControlMode::JOYSTICK;
                } else if (strcmp(mode, "sliders") == 0) {
                    currentControlMode = ControlMode::SLIDERS;
                }
                
                server.send(200, "text/plain", "Control mode changed");
                return;
            }

            // Process control commands - only if BT is not active or not connected
            #if FEATURE_BLUETOOTH_ENABLED
            if (btControlEnabled && bleConnected) {
                server.send(200, "text/plain", "BT control active");
                return;
            }
            #endif

            // Process control commands
            if (strcmp(mode, "joystick") == 0) {
                currentControlMode = ControlMode::JOYSTICK;
                float x = doc["x"] | 0.0f;
                float y = doc["y"] | 0.0f;
                processJoystickControl(x, y);
            } else if (strcmp(mode, "sliders") == 0) {
                currentControlMode = ControlMode::SLIDERS;
                float left = doc["left"] | 0.0f;
                float right = doc["right"] | 0.0f;
                processSlidersControl(left, right);
            }
            
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "No data received");
        }
    });
    
    // Bluetooth control endpoint - conditionally register if Bluetooth is enabled
    #if FEATURE_BLUETOOTH_ENABLED
    server.on("/bt", HTTP_GET, [&server]() {
        String state = server.arg("state");
        
        // Parameter to determine current control mode
        String mode = server.arg("mode");
        
        if (state == "on") {
            btControlEnabled = true;
            
            // Set control mode based on parameter
            if (mode == "sliders") {
                currentControlMode = ControlMode::SLIDERS;
            } else {
                currentControlMode = ControlMode::JOYSTICK;
            }
            
            server.send(200, "text/plain", "BT enabled");
        } else if (state == "off") {
            btControlEnabled = false;
            
            // Set control mode based on parameter
            if (mode == "sliders") {
                currentControlMode = ControlMode::SLIDERS;
            } else {
                currentControlMode = ControlMode::JOYSTICK;
            }
            
            server.send(200, "text/plain", "BT disabled");
        } else {
            server.send(400, "text/plain", "Invalid parameter");
        }
    });
    
    // Bluetooth status endpoint
    server.on("/bt/status", HTTP_GET, [&server]() {
        StaticJsonDocument<200> doc;
        doc["enabled"] = btControlEnabled;
        doc["connected"] = bleConnected;
        doc["status"] = getBtStatus();
        
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    });
    #endif

    setupStreamTask(&server);

    server.begin();
    Serial.println("HTTP server started");

    while (true) {
        server.handleClient();
        // Use vTaskDelay instead of delay for better task scheduling
        vTaskDelay(pdMS_TO_TICKS(50));
    }
} 