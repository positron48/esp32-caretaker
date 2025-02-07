#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include "stream_task.h"

void TaskHttpServer(void* parameter);
extern WebServer server; 