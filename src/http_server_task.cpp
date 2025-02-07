#include "http_server_task.h"
#include "stream_task.h"

void TaskHttpServer(void* parameter) {
    WebServer server(80);

    // Setup routes
    server.on("/", HTTP_GET, [&server]() {
        server.sendHeader("Content-Type", "text/html");
        server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        server.sendHeader("Pragma", "no-cache");
        server.sendHeader("Expires", "-1");
        
        String html = "<html><head>";
        html += "<meta charset='UTF-8'>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        html += "<title>ESP32-CAM Stream</title>";
        html += "<style>";
        html += "body { font-family: Arial, sans-serif; margin: 20px; text-align: center; background-color: #f0f0f0; }";
        html += "h1 { color: #333; margin-bottom: 30px; }";
        html += ".button-container { margin: 20px 0; }";
        html += "button { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; ";
        html += "text-align: center; text-decoration: none; display: inline-block; font-size: 16px; ";
        html += "margin: 4px 2px; cursor: pointer; border-radius: 4px; transition: all 0.3s; }";
        html += "button:hover { background-color: #45a049; transform: translateY(-2px); }";
        html += "button.stop { background-color: #f44336; }";
        html += "button.stop:hover { background-color: #da190b; }";
        html += ".stream-container { margin-top: 20px; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }";
        html += "img { max-width: 100%; height: auto; border-radius: 4px; }";
        html += "</style>";
        html += "</head><body>";
        html += "<h1>ESP32-CAM Stream Control</h1>";
        html += "<div class='button-container'>";
        html += "<button onclick='startStream()'>Start Stream</button>";
        html += "<button class='stop' onclick='stopStream()'>Stop Stream</button>";
        html += "</div>";
        html += "<div class='stream-container'>";
        html += "<img id='stream' style='display:none;'>";
        html += "</div>";
        html += "<script>";
        html += "function startStream() {";
        html += "  const img = document.getElementById('stream');";
        html += "  img.src = '/stream';";
        html += "  img.style.display = 'block';";
        html += "}";
        html += "function stopStream() {";
        html += "  const img = document.getElementById('stream');";
        html += "  fetch('/stopstream').then(() => {";
        html += "    img.style.display = 'none';";
        html += "    img.src = '';";
        html += "  });";
        html += "}";
        html += "</script>";
        html += "</body></html>";
        server.send(200, "text/html", html);
    });

    setupStreamTask(&server);

    server.begin();
    Serial.println("HTTP server started");

    while (true) {
        server.handleClient();
        // Small delay to prevent watchdog triggers
        delay(1);
    }
} 