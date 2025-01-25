#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"

// Пин встроенного светодиода на ESP32-CAM (чаще всего GPIO 4 для AI Thinker)
const int ledPin = 4;

// Декларация задач
void TaskBlink(void *pvParameters);
void TaskLog(void *pvParameters);
void TaskStats(void *pvParameters);

// Конфигурация таймера для Run-Time Stats
#define TIMER_DIVIDER         80  // Делитель: таймер считает каждую микросекунду
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // Перевод в секунды

static gptimer_handle_t timer = NULL;

// Функция счетчика времени (Run-Time Counter)
extern "C" {
    void vConfigureTimerForRunTimeStats(void) {
        gptimer_config_t config = {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 1000000,  // 1 микросекунда
        };

        // Создание нового таймера
        gptimer_new_timer(&config, &timer);

        // Регистрация колбэков (в данном случае нет необходимости в обработчике событий)
        gptimer_event_callbacks_t cbs = {
            .on_alarm = NULL,
        };
        gptimer_register_event_callbacks(timer, &cbs, NULL);

        // Запуск таймера
        gptimer_start(timer);
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

void TaskBlink(void *pvParameters) {
    // Инициализация пина светодиода как выход
    gpio_reset_pin((gpio_num_t)ledPin);
    gpio_set_direction((gpio_num_t)ledPin, GPIO_MODE_OUTPUT);

    while (true) {
        gpio_set_level((gpio_num_t)ledPin, 1);   // Включить светодиод
        vTaskDelay(1000 / portTICK_PERIOD_MS);   // Задержка 1 секунда
        gpio_set_level((gpio_num_t)ledPin, 0);   // Выключить светодиод
        vTaskDelay(1000 / portTICK_PERIOD_MS);   // Задержка 1 секунда
    }
}

void TaskLog(void *pvParameters) {
    while (true) {
        printf("LED is toggling\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Задержка 1 секунда
    }
}

void TaskStats(void *pvParameters) {
    char *buffer = NULL;
    while (true) {
        // Получение размера буфера
        size_t bufferLen = uxTaskGetNumberOfTasks() * 40;
        buffer = (char *)malloc(bufferLen);
        if (buffer == NULL) {
            printf("Failed to allocate memory for stats\n");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        // Получение и вывод статистики
        vTaskGetRunTimeStats(buffer);
        printf("Run Time Stats:\n%s\n", buffer);
        free(buffer);

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Задержка 5 секунд
    }
} 