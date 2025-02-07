#include "stream_task.h"

// Initialize global variables
bool isStreaming = false;
uint8_t* streamBuffer = nullptr;
size_t streamBufferSize = 0;
WebServer* streamServer = nullptr;
WiFiClient streamClient;

void initStreamHandler(WebServer* server) {
    if (!streamBuffer) {
        streamBuffer = (uint8_t*)ps_malloc(MAX_FRAME_SIZE);
        if (!streamBuffer) {
            Serial.println("Failed to allocate stream buffer");
            return;
        }
    }
    streamServer = server;
    isStreaming = false;
}

void cleanupStreamHandler() {
    if (streamBuffer) {
        free(streamBuffer);
        streamBuffer = nullptr;
    }
    streamServer = nullptr;
    isStreaming = false;
}

void handleStartStream() {
    if (!isStreaming) {
        Serial.println("Starting stream...");
        
        streamClient = streamServer->client();
        
        // Send HTTP response headers
        streamClient.println("HTTP/1.1 200 OK");
        streamClient.println("Access-Control-Allow-Origin: *");
        streamClient.printf("Content-Type: %s\r\n", STREAM_CONTENT_TYPE);
        streamClient.println("Cache-Control: no-cache, no-store, must-revalidate");
        streamClient.println("Pragma: no-cache");
        streamClient.println("Expires: -1");
        streamClient.println("Connection: keep-alive");
        streamClient.println();
        streamClient.flush();
        
        if (!streamClient.connected()) {
            Serial.println("Client disconnected after headers");
            return;
        }
        
        Serial.println("Headers sent, starting stream task");
        
        isStreaming = true;
        xTaskCreatePinnedToCore(
            streamTask,
            "StreamTask",
            STREAM_TASK_STACK_SIZE * 2,
            nullptr,
            STREAM_TASK_PRIORITY,
            nullptr,
            STREAM_TASK_CORE
        );
    } else {
        Serial.println("Stream already running");
        streamServer->send(409, "text/plain", "Stream already running");
    }
}

void handleStopStream() {
    if (isStreaming) {
        Serial.println("Stopping stream");
        isStreaming = false;
    }
    streamServer->send(200, "text/plain", "Stream stopped");
}

bool sendMJPEGFrame(const uint8_t* buf, size_t len) {
    if (!streamClient.connected()) {
        Serial.println("Client disconnected in sendMJPEGFrame");
        return false;
    }

    // Send boundary
    if (streamClient.write(STREAM_BOUNDARY, strlen(STREAM_BOUNDARY)) != strlen(STREAM_BOUNDARY)) {
        Serial.println("Failed to send boundary");
        return false;
    }

    // Send JPEG header
    char hdrBuf[HDR_BUF_LEN];
    size_t hdrLen = snprintf(hdrBuf, HDR_BUF_LEN, STREAM_PART, len);
    if (streamClient.write(hdrBuf, hdrLen) != hdrLen) {
        Serial.println("Failed to send JPEG header");
        return false;
    }

    // Send JPEG data in chunks
    const size_t chunkSize = 8192; // Увеличиваем размер чанка до 8KB
    size_t remaining = len;
    const uint8_t* ptr = buf;
    
    while (remaining > 0) {
        size_t toWrite = (remaining > chunkSize) ? chunkSize : remaining;
        size_t written = streamClient.write(ptr, toWrite);
        if (written != toWrite) {
            Serial.printf("Failed to send JPEG data chunk: %u != %u\n", written, toWrite);
            return false;
        }
        ptr += written;
        remaining -= written;
    }
    
    // Делаем flush только после отправки всего кадра
    streamClient.flush();
    return true;
}

void streamTask(void* parameter) {
    Serial.println("Stream task started");
    vTaskDelay(pdMS_TO_TICKS(200)); // Уменьшаем начальную задержку

    uint32_t lastFrameTime = millis();
    uint32_t frameCount = 0;
    uint32_t errorCount = 0;
    const uint32_t MAX_ERRORS = 3;

    while (isStreaming && streamClient.connected()) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Failed to get camera frame");
            if (++errorCount > MAX_ERRORS) {
                Serial.println("Too many camera errors, stopping stream");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        if (fb->len > MAX_FRAME_SIZE) {
            Serial.printf("Frame too large: %u bytes\n", fb->len);
            esp_camera_fb_return(fb);
            continue;
        }

        if (!sendMJPEGFrame(fb->buf, fb->len)) {
            Serial.println("Failed to send frame");
            esp_camera_fb_return(fb);
            break;
        }

        esp_camera_fb_return(fb);
        frameCount++;
        errorCount = 0;

        // Print stats every 30 frames
        if (frameCount % 30 == 0) {
            uint32_t elapsed = (millis() - lastFrameTime) / 1000;
            if (elapsed > 0) {
                Serial.printf("Stream stats: %u frames, %u fps\n", 
                    frameCount, frameCount / elapsed);
            }
            lastFrameTime = millis(); // Сбрасываем время для следующего расчета FPS
        }

        // Maintain frame rate
        uint32_t frameTime = millis() - lastFrameTime;
        if (frameTime < STREAM_DELAY_MS) {
            vTaskDelay(pdMS_TO_TICKS(STREAM_DELAY_MS - frameTime));
        }
        lastFrameTime = millis();
    }

    Serial.println("Stream task ending");
    isStreaming = false;
    streamClient.stop();
    vTaskDelete(nullptr);
}

void setupStreamTask(WebServer* server) {
    initStreamHandler(server);
    server->on("/stream", HTTP_GET, handleStartStream);
    server->on("/stopstream", HTTP_GET, handleStopStream);
} 