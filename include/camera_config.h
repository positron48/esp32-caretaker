#pragma once

#include <Arduino.h>
#include <esp_camera.h>
#include <config.h>

// Camera configuration
constexpr framesize_t FRAME_SIZE = CAMERA_FRAMESIZE;
constexpr int JPEG_QUALITY = CAMERA_JPEG_QUALITY;  
constexpr int XCLK_FREQ = CAMERA_XCLK_FREQ;  
constexpr pixformat_t PIXEL_FORMAT = CAMERA_PIXEL_FORMAT;
constexpr int FB_COUNT = CAMERA_FB_COUNT;  

// Function declarations
bool initCamera();