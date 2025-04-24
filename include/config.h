// Feature toggles - Set to 0 to disable the feature, 1 to enable
#define FEATURE_BLUETOOTH_ENABLED 0
#define FEATURE_LED_CONTROL_ENABLED 1
#define FEATURE_TASK_STATS_ENABLED 0

// Motor Control configuration (L298N / L293D / etc.) - PWM control
#define MOTOR_LEFT_IN1 12
#define MOTOR_LEFT_IN2 13
#define MOTOR_RIGHT_IN1 15
#define MOTOR_RIGHT_IN2 14

// Motor PWM configuration
#define MOTOR_PWM_CHANNEL_LEFT1 2
#define MOTOR_PWM_CHANNEL_LEFT2 3
#define MOTOR_PWM_CHANNEL_RIGHT1 4
#define MOTOR_PWM_CHANNEL_RIGHT2 5
#define MOTOR_PWM_FREQ 10000
#define MOTOR_PWM_RESOLUTION 8     // 8-bit resolution (0-255)

// Motor control parameters
#define MOTOR_DEADZONE 0.15f       // Stick values less than this are ignored
#define MOTOR_MIN_POWER 30         // Minimum PWM value to start movement
#define MOTOR_MAX_POWER 255        // Maximum PWM value
#define MOTOR_TURN_THRESHOLD 0.2f  // Threshold for enabling in-place rotation


// Constants for LED
#define LED_PIN 4
#define LED_CHANNEL 1 // LEDC channel (0-15)
#define LED_RESOLUTION 12 // Resolution (1-14 bits)
#define LED_FREQUENCY 10000 // PWM frequency in Hz


// Camera configuration
#define CAMERA_XCLK_FREQ 20000000 // 20MHz
#define CAMERA_PIXEL_FORMAT PIXFORMAT_JPEG
#define CAMERA_FB_COUNT 2 // Frame buffer count

#define STR_HDR_BUF_LEN 64
#define STR_MAX_FRAME_SIZE 64 * 1024 // Increased maximum frame size to 64KB
#define STR_STREAM_TASK_STACK_SIZE 8192 // Increased stack size
#define STR_STREAM_TASK_PRIORITY 2
#define STR_STREAM_TASK_CORE 0 // Moved to core 0
#define STR_STREAM_DELAY_MS 20 // Small delay for stability

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
#define BLE_SCAN_TIME 5        // BLE device scanning time in seconds
#define BLE_RESCAN_INTERVAL 10000 // Rescanning period if device not found (in ms)

// Joystick calibration parameters
#define LEFT_JOYSTICK_X_MIN 19
#define LEFT_JOYSTICK_X_MAX 32767
#define LEFT_JOYSTICK_X_CENTER 16693

#define LEFT_JOYSTICK_Y_MIN 19
#define LEFT_JOYSTICK_Y_MAX 31527
#define LEFT_JOYSTICK_Y_CENTER 16693