# ESP32-CAM Robot Control with Live Video Streaming

+Русская версия: [readme.ru.md](readme.ru.md)

## Overview

This project implements a remote-controlled robot using an ESP32-CAM board. It features real-time video streaming from the ESP32-CAM and allows for bidirectional control of two motors. The robot can be controlled via a web interface using either a joystick or sliders for motor input. Additionally, the web interface provides controls for adjusting video quality and toggling the on-board LED. Task usage statistics are also collected to monitor system performance.

## Functionality

*   **Live Video Streaming:** Streams video from the ESP32-CAM to a web browser in real-time.
*   **Dual Motor Control:** Controls two motors using an L298N motor driver, enabling forward/backward movement and turning.
*   **Web-Based Control Interface:** Provides a user-friendly web interface for controlling the robot from any device with a web browser.
*   **Joystick and Slider Control Modes:** Offers two control modes: a virtual joystick for intuitive control and sliders for individual motor speed adjustment.
*   **Adjustable Video Quality:** Allows switching between multiple video resolutions (QQVGA, QCIF, HQVGA, QVGA, CIF, HVGA, VGA, SVGA, XGA, HD, SXGA, UXGA) to optimize for bandwidth or clarity.
*   **On-board LED Control:** Controls the brightness of the ESP32-CAM's on-board LED with precise adjustment from 0-100%.
*   **Bluetooth Controller Support:** Connect a physical Bluetooth joystick (like ExpressLRS Joystick) for more precise control.
*   **Task Usage Statistics:** Monitors and displays the CPU usage of different tasks running on the ESP32, aiding in performance analysis and optimization.
*   **WiFi Connectivity:** Connects to a WiFi network for remote control and streaming.
*   **Embedded Web Interface:** The HTML interface is embedded directly into the firmware as a C header file during build process, eliminating the need for a separate filesystem.

## How It Works

The project is built using the Arduino framework for ESP32 and leverages FreeRTOS for multitasking. Here's a breakdown of the key components and their interactions:

1.  **Camera Initialization (`camera_config.h`, `src/main.cpp`):**
    *   Initializes the ESP32-CAM module using the `esp_camera.h` library.
    *   Configures camera parameters such as resolution (`FRAMESIZE_QVGA`, `FRAMESIZE_VGA`), JPEG quality, pixel format, and frame buffer count, defined in `config.h` (startLine: 10, endLine: 14) and `camera_config.h` (startLine: 8, endLine: 12).
    *   Sets up the necessary GPIO pins for camera communication as defined in `config.h` (startLine: 42, endLine: 57).

2.  **Motor Control (`include/motor_control.h`, `src/motor_control.cpp`):**
    *   Defines motor control pins for the L298N driver (startLine: 6, endLine: 9) and PWM channel configurations (startLine: 12, endLine: 15) in `motor_control.h`.
    *   Implements functions to initialize motors (`initMotors`), set motor speeds (`setMotorSpeed`), and process control inputs from joystick (`processJoystickControl`) and sliders (`processSlidersControl`).
    *   Includes logic for joystick dead zones (startLine: 20), minimum motor power (startLine: 21), and turning thresholds (startLine: 23) to provide responsive and nuanced control.
    *   Uses `ledcWrite` to control motor speed via PWM.

3.  **WiFi Management (`include/wifi_manager.h`, `src/wifi_manager.cpp`):**
    *   Handles WiFi connection and reconnection.
    *   Reads WiFi SSID and password from environment variables defined in a `.env` file (startLine: 6, endLine: 7).
    *   Provides functions to initialize WiFi (`initWiFi`) and ensure a persistent connection (`ensureWiFiConnection`).

4.  **HTTP Server (`include/http_server_task.h`, `src/http_server_task.cpp`):**
    *   Runs as a FreeRTOS task (`TaskHttpServer`) on core 1.
    *   Uses the `WebServer.h` library to handle HTTP requests.
    *   Serves the web interface HTML from the embedded constant array `INDEX_HTML` in `html_content.h`.
    *   Handles endpoints for:
        *   `/` : Serves the main HTML page from the embedded array.
        *   `/led?brightness={0-100}` : Controls the LED brightness with precise control.
        *   `/quality?resolution={RESOLUTION}` : Changes the video resolution (e.g., QVGA, VGA, HD).
        *   `/control` (POST): Receives motor control commands in JSON format from the web interface for both joystick and slider modes.
        *   `/stream` : Starts the video stream.
        *   `/stopstream` : Stops the video stream.
        *   `/bt?state={on|off}` : Enables or disables Bluetooth controller support.
        *   `/bt/status` : Returns the current Bluetooth connection status.

5.  **Video Streaming (`include/stream_task.h`, `src/stream_task.cpp`, `include/stream_constants.h`):**
    *   Runs as a FreeRTOS task (`streamTask`) on core 0.
    *   Captures frames from the ESP32-CAM using `esp_camera_fb_get()`.
    *   Encodes frames as MJPEG and streams them to the connected client via HTTP multipart responses.
    *   Uses constants defined in `stream_constants.h` (startLine: 7, endLine: 28) for stream configuration like buffer sizes, task priority, and content types.
    *   Implements functions to start (`handleStartStream`), stop (`handleStopStream`), and manage the stream.

6.  **Web Interface Generation (`scripts/pre_build_script.py`):**
    *   During build process, the HTML file from `data/html/index.html` is converted into a C header file.
    *   The HTML content is compressed using gzip to significantly reduce memory usage.
    *   The compressed data is stored as a byte array in `include/html_content.h`.
    *   When the browser requests the HTML, it is served with appropriate headers (`Content-Encoding: gzip`), so the browser automatically decompresses it.
    *   This approach improves load time, reduces bandwidth usage, and minimizes ESP32 memory consumption.
    *   Configuration for this process is specified in `platformio.ini` with `custom_html_source` and `custom_html_header` variables.

7.  **Bluetooth Control (`include/bluetooth_task.h`, `src/bluetooth_task.cpp`):**
    *   Implements Bluetooth Low Energy (BLE) support using the NimBLE library.
    *   Scans for and connects to specific Bluetooth controllers (e.g., ExpressLRS Joystick).
    *   Processes joystick input from the Bluetooth controller and translates it to motor commands.
    *   Provides status information and connection management via the web interface.

8.  **Task Statistics (`include/task_stats.h`, `src/log_task.cpp`):**
    *   Collects and displays runtime statistics for FreeRTOS tasks.
    *   The `TaskLog` (startLine: 6) task runs on core 1 and periodically prints task runtime statistics using `vTaskGetRunTimeStats` (startLine: 18) to the serial monitor.
    *   This helps in monitoring CPU usage and identifying potential performance bottlenecks.

9.  **Configuration Files (`config.h`, `.env`, `platformio.ini`, `sdkconfig.defaults`):**
    *   `config.h`: Defines hardware configurations like camera settings, LED pin, motor control parameters, and stream settings.
    *   `.env`: Stores sensitive information like WiFi SSID and password as environment variables, which are loaded during the build process.
    *   `platformio.ini`: Configuration file for PlatformIO, defining the platform, board, framework, build flags, libraries, and HTML-to-header conversion.
    *   `sdkconfig.defaults`:  ESP-IDF configuration defaults, enabling FreeRTOS runtime statistics (startLine: 2, endLine: 5).

## Installation

Follow these steps to set up and run the project:

### Prerequisites

1.  **PlatformIO IDE:** Install PlatformIO IDE in VS Code. You can find installation instructions on the [PlatformIO website](https://platformio.org/install/ide?install=vscode).
2.  **ESP32-CAM Board:** You will need an ESP32-CAM module.
3.  **USB to Serial Converter:** To upload code to the ESP32-CAM, you'll need a USB to Serial converter (like FTDI adapter). Ensure it's compatible with 3.3V logic levels.
4.  **L298N Motor Driver (Optional):** If you want to control motors, you'll need an L298N motor driver and two DC motors.
5.  **Bluetooth Controller (Optional):** For physical control, an ExpressLRS Joystick or similar BLE joystick can be used.

### Project Setup

1.  **Clone the Repository:** Clone this project repository to your local machine.
2.  **Open in PlatformIO:** Open the cloned project folder in VS Code with PlatformIO IDE.
3.  **Configure WiFi Credentials:**
    *   Create a file named `.env` in the project root directory.
    *   Add the following lines to `.env`, replacing `YOUR_WIFI_SSID` and `YOUR_WIFI_PASSWORD` with your actual WiFi network credentials:

    ```
    WIFI_SSID="YOUR_WIFI_SSID"
    WIFI_PASSWORD="YOUR_WIFI_PASSWORD"
    ```

4.  **PlatformIO Configuration (`platformio.ini`):**
    *   Ensure the `platformio.ini` file is configured for the `esp32cam` environment.
    *   The `build_flags` in `platformio.ini` (startLine: 12) uses a bash script `generate_build_flags.sh` (startLine: 1) to inject the WiFi credentials from the `.env` file into the build process. Make sure this script is executable (`chmod +x generate_build_flags.sh`).
    *   The configuration also specifies the HTML source file and target header file for the web interface conversion.

5.  **Build the Firmware:** In PlatformIO IDE, build the project by clicking on the PlatformIO icon in the Activity Bar, then under "Project Tasks" -> "esp32cam" -> "Build".
    *   During the build process, the HTML file is automatically converted to a C header file using the Python script in `scripts/pre_build_script.py`.

### Upload Firmware

1.  **Connect ESP32-CAM:** Connect the ESP32-CAM to your USB to Serial converter for flashing.  **Important:** You might need to connect GPIO0 to GND during flashing to put the ESP32 into programming mode. Refer to ESP32-CAM documentation for specific wiring and boot mode instructions.
2.  **Upload Firmware:** In PlatformIO IDE, upload the firmware by clicking on "Upload" under "Project Tasks" -> "esp32cam".

### Wiring for Motors (Optional)

If you intend to control motors, connect the L298N motor driver to the ESP32-CAM as follows, using the pins defined in `motor_control.h` (startLine: 6, endLine: 9):

*   **ESP32-CAM Pins** | **L298N Pins**
    ------- | --------
    GPIO14 (MOTOR\_LEFT\_IN1) | IN1
    GPIO15 (MOTOR\_LEFT\_IN2) | IN2
    GPIO13 (MOTOR\_RIGHT\_IN1) | IN3
    GPIO12 (MOTOR\_RIGHT\_IN2) | IN4
    GND | GND (L298N logic side)
    5V or Vin | VCC (L298N logic side)
    ESP32-CAM Power (5V or Vin) | L298N Motor Power Supply (12V or as per your motors)
    GND | GND (L298N motor power side)
    Motor 1 Output | Motor 1
    Motor 2 Output | Motor 2

    **Note:** Ensure you provide appropriate power to the L298N for your motors, separate from the ESP32-CAM's power supply if needed.

## Configuration

You can customize various aspects of the project by modifying the configuration files:

### `config.h`

*   **Camera Settings:**
    *   `CAMERA_FRAMESIZE` (startLine: 10):  Set the video resolution (e.g., `FRAMESIZE_QVGA`, `FRAMESIZE_VGA`, `FRAMESIZE_SVGA`). Lower resolutions reduce bandwidth usage.
    *   `CAMERA_JPEG_QUALITY` (startLine: 11): Adjust JPEG quality (0-63, lower is better quality but larger size).
    *   `CAMERA_XCLK_FREQ` (startLine: 12): Camera clock frequency.
    *   `CAMERA_PIXEL_FORMAT` (startLine: 13): Pixel format, typically `PIXFORMAT_JPEG`.
    *   `CAMERA_FB_COUNT` (startLine: 14): Number of frame buffers.

*   **LED Settings:**
    *   `LED_PIN` (startLine: 31): GPIO pin connected to the LED.
    *   `LED_CHANNEL`, `LED_RESOLUTION`, `LED_FREQUENCY` (startLine: 32, endLine: 35): PWM settings for LED control.
    *   `LED_BRIGHT_OFF`, `LED_BRIGHT_MID`, `LED_BRIGHT_HIGH` (startLine: 36, endLine: 38): PWM values for different LED brightness levels.

*   **Stream Settings:**
    *   `STR_MAX_FRAME_SIZE` (startLine: 17): Maximum frame size for streaming.
    *   `STR_STREAM_TASK_STACK_SIZE` (startLine: 18): Stack size for the stream task.
    *   `STR_STREAM_TASK_PRIORITY` (startLine: 19): Priority of the stream task.
    *   `STR_STREAM_TASK_CORE` (startLine: 20): Core on which the stream task runs (0 or 1).
    *   `STR_STREAM_DELAY_MS` (startLine: 21): Delay between frames in the stream task.

### `motor_control.h`

*   **Motor Control Parameters:**
    *   `MOTOR_DEADZONE` (startLine: 20): Dead zone for joystick/slider input to ignore small stick movements.
    *   `MOTOR_MIN_POWER` (startLine: 21): Minimum PWM value to start motor movement. Adjust this if your motors require a higher starting voltage.
    *   `MOTOR_MAX_POWER` (startLine: 22): Maximum PWM value for motors (typically 255 for 8-bit resolution).
    *   `MOTOR_TURN_THRESHOLD` (startLine: 23): Joystick X-axis threshold to activate on-the-spot turning.

### `joystick_config.h`

*   **Bluetooth Joystick Parameters:**
    *   `JOYSTICK_DEVICE_NAME`: The name of the Bluetooth joystick to connect to (default is "ExpressLRS Joystick").
    *   `BLE_SCAN_TIME`: How long to scan for Bluetooth devices (in seconds).
    *   `BLE_RESCAN_INTERVAL`: How often to rescan if a device is not found (in milliseconds).
    *   Various calibration parameters for the joystick axes to match your specific controller.

### `.env`

*   **WiFi Credentials:**
    *   `WIFI_SSID`: Your WiFi network SSID.
    *   `WIFI_PASSWORD`: Your WiFi network password.

### `sdkconfig.defaults`

*   **FreeRTOS Configuration:**
    *   `CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS` (startLine: 2): Enable FreeRTOS runtime statistics collection.
    *   `CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS` (startLine: 3): Enable formatting functions for runtime statistics.
    *   `CONFIG_FREERTOS_RUN_TIME_COUNTER_TYPE` (startLine: 4): Set runtime counter type.

### Web Interface Customization

To modify the web interface:
1. Edit the HTML file in `data/html/index.html`
2. The changes will be automatically incorporated during the next build
3. The HTML content is automatically compressed with gzip during the build process
4. The compressed content is embedded directly in the firmware as a byte array
5. No need to upload filesystem images as the HTML is embedded in the firmware
6. The web server serves the compressed content with appropriate headers for browser decompression

## Usage

1.  **Power On:** Power on the ESP32-CAM board. Ensure it is connected to your WiFi network.
2.  **Get IP Address:** Open the serial monitor in PlatformIO (or Arduino IDE) and check the serial output. The ESP32-CAM will print its IP address after connecting to WiFi.
3.  **Access Web Interface:** Open a web browser on your computer or mobile device and navigate to the IP address of the ESP32-CAM.
4.  **Control the Robot:**
    *   **Start Stream:** Click the "Start Stream" button to begin video streaming.
    *   **Control Mode:** Select "Joystick" or "Sliders" control mode using the "Control Mode" button.
    *   **Joystick Control:** Use the on-screen joystick to control the robot's movement. Drag the stick to move forward, backward, left, or right.
    *   **Slider Control:** In slider mode, use the left and right vertical sliders to control the speed of the left and right motors independently.
    *   **Resolution:** Change video resolution using the dropdown in the settings panel. Multiple resolutions are available from QQVGA to UXGA.
    *   **LED Control:** Adjust the on-board LED brightness using the slider in the settings panel (0-100%).
    *   **Bluetooth Control:** Enable Bluetooth control by clicking the "Bluetooth" button. The interface will show connection status and automatically try to connect to the configured Bluetooth joystick.
    *   **Fullscreen:** Enter or exit fullscreen mode using the "⛶" button.

## Customization

This project provides a solid foundation for further development and customization. Here are some ideas:

*   **Add Sensors:** Integrate sensors like ultrasonic sensors for obstacle avoidance, line sensors for line following, or IMU for orientation data.
*   **Implement More Complex Control Algorithms:** Develop more sophisticated control algorithms for autonomous navigation, path planning, or object tracking.
*   **Enhance Web Interface:** Improve the web interface with additional features, such as displaying sensor data, adding more control options, or customizing the UI.
*   **Telemetry Data:** Send telemetry data (e.g., battery voltage, motor current, sensor readings) back to the web interface for monitoring and diagnostics.
*   **OTA Updates:** Implement Over-The-Air (OTA) firmware updates to easily update the ESP32-CAM firmware without physical access.
