#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <esp_camera.h>

#define PART_BOUNDARY "123456789000000000000987654321"
#define STREAM_CONTENT_TYPE "multipart/x-mixed-replace;boundary=" PART_BOUNDARY
#define STREAM_BOUNDARY "\r\n--" PART_BOUNDARY "\r\n"
#define STREAM_PART "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"
#define HDR_BUF_LEN 64

class StreamManager {
protected:
    static bool isStreaming;
    static uint8_t* streamBuffer;
    static size_t streamBufferSize;
    static WebServer* server;
    static const size_t MAX_FRAME_SIZE = 64 * 1024; // 64KB max frame size

public:
    StreamManager(WebServer* webServer);
    ~StreamManager();
    
    static void startStreaming();
    static void stopStreaming();
    static void streamTask(void* parameter);
    static esp_err_t captureFrame();
};

// Task creation helper
void setupStreamTask(WebServer* server); 