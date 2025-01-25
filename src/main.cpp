#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/ledc.h"

// Включение заголовочных файлов задач
#include "tasks/BlinkTask.h"
#include "tasks/LogTask.h"
#include "tasks/StatsTask.h"

static const char* TAG = "Main";

// Пин встроенного светодиода на ESP32-CAM (чаще всего GPIO4 для AI Thinker)
const int ledPin = 4;

// Конфигурация таймера для Run-Time Stats
#define TIMER_DIVIDER         80           // Делитель: таймер считает каждую микросекунду
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // Перевод в секунды

static gptimer_handle_t timer = NULL;

// Конфигурация LEDC
#define LEDC_TIMER             LEDC_TIMER_0
#define LEDC_MODE              LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO         (ledPin)               // GPIO4
#define LEDC_CHANNEL           LEDC_CHANNEL_0
#define LEDC_DUTY_RES          LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY         (5000)                 // 5 kHz

// Функция счетчика времени (Run-Time Counter)
extern "C" {
    void vConfigureTimerForRunTimeStats(void) {
        gptimer_config_t config;
        config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
        config.direction = GPTIMER_COUNT_UP;
        config.resolution_hz = 1000000;           // 1 микросекунда

        // Создание нового таймера
        esp_err_t err = gptimer_new_timer(&config, &timer);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Не удалось создать GPTimer: %d", err);
            return;
        }

        // Регистрация колбэков (нет обработчика событий)
        gptimer_event_callbacks_t cbs = {0};       // Инициализация нулями
        cbs.on_alarm = NULL;
        err = gptimer_register_event_callbacks(timer, &cbs, NULL);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Не удалось зарегистрировать колбэки таймера: %d", err);
            return;
        }

        // Запуск таймера
        err = gptimer_start(timer);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Не удалось запустить GPTimer: %d", err);
            return;
        }
    }

    unsigned long long ullGetRunTimeCounterValue(void) {
        uint64_t timer_val = 0;
        gptimer_get_raw_count(timer, &timer_val);
        return timer_val;
    }
}

extern "C" void app_main() {
    // Конфигурация таймера для Run-Time Stats
    vConfigureTimerForRunTimeStats();

    // Конфигурация LEDC
    ledc_timer_config_t ledc_timer;
    ledc_timer.speed_mode      = LEDC_MODE;
    ledc_timer.timer_num       = LEDC_TIMER;
    ledc_timer.duty_resolution = LEDC_DUTY_RES;
    ledc_timer.freq_hz         = LEDC_FREQUENCY;
    ledc_timer.clk_cfg         = LEDC_AUTO_CLK;
    ledc_timer.deconfigure     = false;                  // Не перезагружать другие каналы

    esp_err_t ledc_err = ledc_timer_config(&ledc_timer);
    if (ledc_err != ESP_OK) {
        ESP_LOGE(TAG, "Не удалось сконфигурировать LEDC таймер: %d", ledc_err);
    }

    ledc_channel_config_t ledc_channel;
    ledc_channel.speed_mode  = LEDC_MODE;
    ledc_channel.channel     = LEDC_CHANNEL;
    ledc_channel.timer_sel   = LEDC_TIMER;
    ledc_channel.intr_type   = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num    = LEDC_OUTPUT_IO;
    ledc_channel.duty        = 0;                      // Начальная яркость 0%
    ledc_channel.hpoint      = 0;
    // ledc_channel.flags       = 0;                      // Нет специальных флагов

    ledc_err = ledc_channel_config(&ledc_channel);
    if (ledc_err != ESP_OK) {
        ESP_LOGE(TAG, "Не удалось сконфигурировать LEDC канал: %d", ledc_err);
    }

    // Установка яркости до 20%
    uint32_t duty = (uint32_t)(255 * 0.2);        // Для 8-битного разрешения: 255 * 20% = 51
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, duty);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

    // Инициализация задач
    xTaskCreatePinnedToCore(
        TaskBlink,   // Функция задачи
        "Blink",     // Имя задачи
        2048,        // Размер стека задачи
        NULL,        // Параметры задачи
        1,           // Приоритет задачи
        NULL,        // Дескриптор задачи
        0            // Ядро, на котором будет выполняться задача
    );

    xTaskCreatePinnedToCore(
        TaskLog,
        "Log",
        2048,
        NULL,
        1,
        NULL,
        1
    );

    xTaskCreatePinnedToCore(
        TaskStats,
        "Stats",
        4096,
        NULL,
        1,
        NULL,
        1
    );
} 