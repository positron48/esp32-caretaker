#pragma once

#include <Arduino.h>

// Stream configuration constants
constexpr size_t HDR_BUF_LEN = 64;
constexpr size_t MAX_FRAME_SIZE = 32 * 1024; // Уменьшаем максимальный размер кадра до 32KB
constexpr size_t STREAM_TASK_STACK_SIZE = 4096;
constexpr uint8_t STREAM_TASK_PRIORITY = 1;
constexpr uint8_t STREAM_TASK_CORE = 1;
constexpr uint32_t STREAM_DELAY_MS = 50; // Уменьшаем задержку между кадрами

// Stream string constants
constexpr const char* PART_BOUNDARY = "123456789000000000000987654321";
constexpr const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=123456789000000000000987654321";
constexpr const char* STREAM_BOUNDARY = "\r\n--123456789000000000000987654321\r\n";
constexpr const char* STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"; 