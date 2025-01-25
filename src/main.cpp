#include <Arduino.h>

// Пин встроенного светодиода на ESP32-CAM (чаще всего GPIO 4 для AI Thinker)
const int ledPin = 4;

void setup() {
    // Инициализация последовательного порта
    Serial.begin(115200);
    Serial.println("ESP32-CAM LED Blink Example");

    // Инициализация пина светодиода как выход
    pinMode(ledPin, OUTPUT);
}

void loop() {
    Serial.println("LED ON");
    digitalWrite(ledPin, HIGH);   // Включить светодиод
    delay(1000);                  // Задержка 1 секунда

    Serial.println("LED OFF");
    digitalWrite(ledPin, LOW);    // Выключить светодиод
    delay(1000);                  // Задержка 1 секунда
} 