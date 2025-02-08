#pragma once

#include <Arduino.h>
#include <config.h>

// Stream configuration constants
constexpr size_t HDR_BUF_LEN = STR_HDR_BUF_LEN;
constexpr size_t MAX_FRAME_SIZE = STR_MAX_FRAME_SIZE; 
constexpr size_t STREAM_TASK_STACK_SIZE = STR_STREAM_TASK_STACK_SIZE;
constexpr uint8_t STREAM_TASK_PRIORITY = STR_STREAM_TASK_PRIORITY;
constexpr uint8_t STREAM_TASK_CORE = STR_STREAM_TASK_CORE;
constexpr uint32_t STREAM_DELAY_MS = STR_STREAM_DELAY_MS; 

// Stream string constants
constexpr const char* PART_BOUNDARY = STR_PART_BOUNDARY;
constexpr const char* STREAM_CONTENT_TYPE = STR_STREAM_CONTENT_TYPE;
constexpr const char* STREAM_BOUNDARY = STR_STREAM_BOUNDARY;
constexpr const char* STREAM_PART = STR_STREAM_PART; 