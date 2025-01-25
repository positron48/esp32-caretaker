#!/bin/bash

# Загружаем переменные из .env
source .env

# Создаем строку с флагами
echo "-DWIFI_SSID=\\\"$WIFI_SSID\\\" -DWIFI_PASSWORD=\\\"$WIFI_PASSWORD\\\"" 