#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <esp_camera.h>
#include "stream_constants.h"

// Global variables
extern bool isStreaming;
extern uint8_t* streamBuffer;
extern size_t streamBufferSize;
extern WebServer* streamServer;

// Function declarations
void initStreamHandler(WebServer* server);
void cleanupStreamHandler();
void handleStartStream();
void handleStopStream();
void streamTask(void* parameter);
esp_err_t captureFrame();
void setupStreamTask(WebServer* server); 