#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include "stream_task.h"

// External declarations
extern WebServer server;

// Task function
void TaskHttpServer(void* parameter); 