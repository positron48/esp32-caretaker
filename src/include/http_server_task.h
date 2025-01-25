#pragma once
#include <WebServer.h>

void TaskHttpServer(void *pvParameters);

// Глобальный объект сервера, чтобы он был доступен для обработчиков
extern WebServer server; 