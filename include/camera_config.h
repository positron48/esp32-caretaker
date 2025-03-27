#pragma once

#include <Arduino.h>
#include <esp_camera.h>
#include <config.h>

// Camera configuration
constexpr int XCLK_FREQ = CAMERA_XCLK_FREQ;  
constexpr pixformat_t PIXEL_FORMAT = CAMERA_PIXEL_FORMAT;
constexpr int FB_COUNT = CAMERA_FB_COUNT;  

// Function declarations
bool initCamera();