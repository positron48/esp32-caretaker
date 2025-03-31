#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <esp_camera.h>
#include "stream_constants.h"

// Frame rate filter structure
typedef struct {
    size_t size;   // number of values used for filtering
    size_t index;  // current value index
    size_t count;  // value count
    int sum;
    int* values;   // array to be filled with values
} ra_filter_t;

// Global variables
extern bool isStreaming;
extern uint8_t* streamBuffer;
extern size_t streamBufferSize;
extern WebServer* streamServer;
extern ra_filter_t ra_filter;

// Function declarations
void initStreamHandler(WebServer* server);
void cleanupStreamHandler();
void handleStartStream();
void handleStopStream();
void handleStreamStatus();
void streamTask(void* parameter);
esp_err_t captureFrame();
void setupStreamTask(WebServer* server);

// Filter functions
ra_filter_t* ra_filter_init(ra_filter_t* filter, size_t sample_size);
int ra_filter_run(ra_filter_t* filter, int value); 