// Camera configuration
/**
 * FRAMESIZE_QVGA - 320x240
 * FRAMESIZE_VGA - 640x480
 * FRAMESIZE_SVGA - 800x600
 * FRAMESIZE_XGA - 1024x768
 * FRAMESIZE_SXGA - 1280x1024
 * FRAMESIZE_UXGA - 1600x1200
 */
#define CAMERA_FRAMESIZE FRAMESIZE_SVGA
#define CAMERA_JPEG_QUALITY 12 // 0-63 (lower means higher quality)
#define CAMERA_XCLK_FREQ 20000000 // 20MHz
#define CAMERA_PIXEL_FORMAT PIXFORMAT_JPEG
#define CAMERA_FB_COUNT 2 // Frame buffer count

#define STR_HDR_BUF_LEN 64
#define STR_MAX_FRAME_SIZE 32 * 1024 // Уменьшаем максимальный размер кадра до 32KB
#define STR_STREAM_TASK_STACK_SIZE 4096
#define STR_STREAM_TASK_PRIORITY 1
#define STR_STREAM_TASK_CORE 1
#define STR_STREAM_DELAY_MS 10 // Уменьшаем задержку между кадрами

// Stream content type
#define STR_PART_BOUNDARY "123456789000000000000987654321"
#define STR_STREAM_CONTENT_TYPE "multipart/x-mixed-replace;boundary=123456789000000000000987654321"
#define STR_STREAM_BOUNDARY "\r\n--123456789000000000000987654321\r\n"
#define STR_STREAM_PART "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"


// Константы для led
#define LED_PIN 4
#define LED_CHANNEL 0 // Канал LEDC (0-15)
#define LED_RESOLUTION 12 // Разрешение (1-14 бит)
#define LED_FREQUENCY 10000 // Частота PWM в Гц


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