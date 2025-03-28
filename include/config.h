// Motor Control configuration
#define MOTOR_LEFT_IN1 12  // in1
#define MOTOR_LEFT_IN2 13  // in2
#define MOTOR_RIGHT_IN1 15 // in3
#define MOTOR_RIGHT_IN2 14 // in4

// Motor PWM configuration
#define MOTOR_PWM_CHANNEL_LEFT1 2  // PWM channel for left motor IN1
#define MOTOR_PWM_CHANNEL_LEFT2 3  // PWM channel for left motor IN2
#define MOTOR_PWM_CHANNEL_RIGHT1 4 // PWM channel for right motor IN1
#define MOTOR_PWM_CHANNEL_RIGHT2 5 // PWM channel for right motor IN2
#define MOTOR_PWM_FREQ 10000
#define MOTOR_PWM_RESOLUTION 8     // 8-bit resolution (0-255)

// Motor control parameters
#define MOTOR_DEADZONE 0.05f       // Значения стика меньше этого игнорируются
#define MOTOR_MIN_POWER 30         // Минимальное значение PWM для начала движения
#define MOTOR_MAX_POWER 255        // Максимальное значение PWM
#define MOTOR_TURN_THRESHOLD 0.5f  // Порог для включения разворота на месте


// Константы для led
#define LED_PIN 4
#define LED_CHANNEL 1 // Канал LEDC (0-15)
#define LED_RESOLUTION 12 // Разрешение (1-14 бит)
#define LED_FREQUENCY 10000 // Частота PWM в Гц


// Camera configuration
#define CAMERA_XCLK_FREQ 20000000 // 20MHz
#define CAMERA_PIXEL_FORMAT PIXFORMAT_JPEG
#define CAMERA_FB_COUNT 2 // Frame buffer count

#define STR_HDR_BUF_LEN 64
#define STR_MAX_FRAME_SIZE 64 * 1024 // Увеличиваем максимальный размер кадра до 64KB
#define STR_STREAM_TASK_STACK_SIZE 8192 // Увеличиваем размер стека
#define STR_STREAM_TASK_PRIORITY 2
#define STR_STREAM_TASK_CORE 0 // Переносим на ядро 0
#define STR_STREAM_DELAY_MS 20 // Небольшая задержка для стабильности

// Stream content type
#define STR_PART_BOUNDARY "123456789000000000000987654321"
#define STR_STREAM_CONTENT_TYPE "multipart/x-mixed-replace;boundary=123456789000000000000987654321"
#define STR_STREAM_BOUNDARY "\r\n--123456789000000000000987654321\r\n"
#define STR_STREAM_PART "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"

// Camera pins for ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


// Bluetooth Joystick configuration
#define JOYSTICK_DEVICE_NAME "ExpressLRS Joystick"
#define BLE_SCAN_TIME 5        // Время сканирования BLE устройств в секундах
#define BLE_RESCAN_INTERVAL 10000 // Период повторного сканирования, если устройство не найдено (в мс)

// Joystick calibration parameters
#define LEFT_JOYSTICK_X_MIN 19
#define LEFT_JOYSTICK_X_MAX 32767
#define LEFT_JOYSTICK_X_CENTER 16693

#define LEFT_JOYSTICK_Y_MIN 19
#define LEFT_JOYSTICK_Y_MAX 31527
#define LEFT_JOYSTICK_Y_CENTER 16693