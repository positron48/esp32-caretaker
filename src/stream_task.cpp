#include "stream_task.h"
#include "stream_constants.h"

// Initialize global variables
bool isStreaming = false;
uint8_t* streamBuffer = nullptr;
size_t streamBufferSize = 0;
WebServer* streamServer = nullptr;
WiFiClient streamClient;
ra_filter_t ra_filter;

// Initialize frame rate filter
ra_filter_t* ra_filter_init(ra_filter_t* filter, size_t sample_size) {
    memset(filter, 0, sizeof(ra_filter_t));

    filter->values = (int*)malloc(sample_size * sizeof(int));
    if (!filter->values) {
        return nullptr;
    }
    memset(filter->values, 0, sample_size * sizeof(int));

    filter->size = sample_size;
    return filter;
}

// Run frame rate filter calculation
int ra_filter_run(ra_filter_t* filter, int value) {
    if (!filter->values) {
        return value;
    }
    filter->sum -= filter->values[filter->index];
    filter->values[filter->index] = value;
    filter->sum += filter->values[filter->index];
    filter->index++;
    filter->index = filter->index % filter->size;
    if (filter->count < filter->size) {
        filter->count++;
    }
    return filter->sum / filter->count;
}

void initStreamHandler(WebServer* server) {
    if (!streamBuffer) {
        streamBuffer = (uint8_t*)ps_malloc(MAX_FRAME_SIZE);
        if (!streamBuffer) {
            Serial.println("Failed to allocate stream buffer");
            return;
        }
        Serial.printf("Allocated stream buffer of %u bytes on PSRAM\n", MAX_FRAME_SIZE);
    }
    streamServer = server;
    isStreaming = false;
    
    // Initialize frame rate filter with 20 samples (like in example)
    ra_filter_init(&ra_filter, 20);
    
    Serial.println("Stream handler initialized successfully");
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
    const size_t chunkSize = 8192; // Optimized chunk size (8KB)
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
    
    // Flush only after sending the entire frame
    streamClient.flush();
    return true;
}

void streamTask(void* parameter) {
    Serial.println("Stream task started on core " + String(xPortGetCoreID()));
    vTaskDelay(pdMS_TO_TICKS(200)); // Initial delay

    int64_t lastFrameTime = 0;
    int64_t currentTime = 0;
    uint32_t frameCount = 0;
    uint32_t errorCount = 0;
    const uint32_t MAX_ERRORS = 3;
    
    // Initialize the last frame time
    lastFrameTime = esp_timer_get_time();

    while (isStreaming && streamClient.connected()) {
        int64_t fr_start = esp_timer_get_time();
        
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
        
        int64_t fr_ready = esp_timer_get_time();
        int64_t fr_encode = fr_ready; // Initialize for JPEG format

        if (fb->format != PIXFORMAT_JPEG) {
            // Convert to JPEG if needed
            bool jpeg_converted = frame2jpg(fb, 80, &streamBuffer, &streamBufferSize);
            esp_camera_fb_return(fb);
            fb = nullptr;
            if (!jpeg_converted) {
                Serial.println("JPEG compression failed");
                if (++errorCount > MAX_ERRORS) {
                    break;
                }
                continue;
            }
            
            if (!sendMJPEGFrame(streamBuffer, streamBufferSize)) {
                Serial.println("Failed to send frame");
                break;
            }
            
            fr_encode = esp_timer_get_time();
        } else {
            // Direct JPEG format
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
            
            fr_encode = esp_timer_get_time();
        }
        
        if (fb) {
            esp_camera_fb_return(fb);
            fb = nullptr;
        }
        
        frameCount++;
        errorCount = 0;
        
        // Calculate timings
        int64_t fr_end = esp_timer_get_time();
        int64_t ready_time = (fr_ready - fr_start) / 1000;
        int64_t encode_time = (fr_encode - fr_ready) / 1000;
        int64_t process_time = (fr_encode - fr_start) / 1000;
        
        int64_t frame_time = fr_end - lastFrameTime;
        lastFrameTime = fr_end;
        frame_time /= 1000; // Convert to ms
        
        // Use the frame rate filter
        uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);
        
        // Print stats every 30 frames
        if (frameCount % 30 == 0) {
            Serial.printf("MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps), %u+%u=%u\n",
                (fb ? fb->len : streamBufferSize),
                (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
                avg_frame_time, 1000.0 / avg_frame_time,
                (uint32_t)ready_time, (uint32_t)encode_time, (uint32_t)process_time
            );
        }

        // Maintain frame rate with adaptive delay
        uint32_t elapsed = (fr_end - lastFrameTime) / 1000;
        if (elapsed < STREAM_DELAY_MS) {
            vTaskDelay(pdMS_TO_TICKS(STREAM_DELAY_MS - elapsed));
        } else {
            // Give other tasks on the same core a chance to run
            vTaskDelay(1);
        }
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