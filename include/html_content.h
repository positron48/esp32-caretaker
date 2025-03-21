#pragma once

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>
    <meta name="mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
    <meta name="format-detection" content="telephone=no">
    <meta name="msapplication-tap-highlight" content="no">
    <meta name="HandheldFriendly" content="true">
    <title>ESP32-CAM caretaker!</title>
    <style>
        :root {
            --primary-color: #4CAF50;
            --danger-color: #f44336;
            --control-bg: rgba(0, 0, 0, 0.5);
            --text-color: #ffffff;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            -webkit-tap-highlight-color: transparent;
            touch-action: none !important;
            -ms-touch-action: none !important;
            -webkit-touch-callout: none !important;
            -webkit-user-select: none !important;
            -moz-user-select: none !important;
            -ms-user-select: none !important;
            user-select: none !important;
            -webkit-overflow-scrolling: none !important;
        }

        body { 
            font-family: Arial, sans-serif;
            background-color: #000;
            min-height: 100vh;
            overflow: hidden;
            color: var(--text-color);
            touch-action: none !important;
            -ms-touch-action: none !important;
            overscroll-behavior: none !important;
            position: fixed;
            width: 100%;
            height: 100%;
        }

        .container {
            position: relative;
            width: 100vw;
            height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            touch-action: none;
        }

        .video-container {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            z-index: 1;
            background: #000;
        }

        .stream-placeholder {
            width: 100%;
            height: 100%;
            display: flex;
            align-items: center;
            justify-content: center;
            color: #666;
            font-size: 1.2em;
        }

        #stream {
            width: 100%;
            height: 100%;
            object-fit: contain;
            display: none;
        }

        .controls-overlay {
            position: absolute;
            z-index: 2;
            width: 100%;
            height: 100%;
            pointer-events: none;
        }

        .controls-top {
            position: absolute;
            top: 20px;
            left: 20px;
            display: flex;
            align-items: center;
            gap: 16px;
            pointer-events: all;
            background: var(--control-bg);
            padding: 8px;
            border-radius: 8px;
            backdrop-filter: blur(5px);
        }

        .controls-right {
            position: absolute;
            right: 20px;
            top: 50%;
            transform: translateY(-50%);
            pointer-events: all;
            display: flex;
            flex-direction: column;
            gap: 16px;
        }

        .control-mode-switch {
            position: absolute;
            top: 20px;
            right: 20px;
            z-index: 10;
            pointer-events: all;
            background: var(--control-bg);
            padding: 8px;
            border-radius: 8px;
            backdrop-filter: blur(5px);
        }

        .control-mode-switch .button {
            min-width: 44px;
            font-size: 20px;
            padding: 8px;
            line-height: 1;
        }

        .joystick-container {
            width: 200px;
            height: 200px;
            background: var(--control-bg);
            border-radius: 8px;
            padding: 20px;
            backdrop-filter: blur(5px);
            display: none;
        }

        .joystick-container.active {
            display: block;
        }

        #joystick {
            width: 100%;
            height: 100%;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 50%;
            position: relative;
            touch-action: none;
        }

        #stick {
            width: 40%;
            height: 40%;
            background: rgba(255, 255, 255, 0.8);
            border-radius: 50%;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            cursor: pointer;
        }

        .sliders-container {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            pointer-events: none;
            display: none;
        }

        .sliders-container.active {
            display: block;
        }

        .slider-vertical {
            position: absolute;
            width: 100px;
            height: calc(100vh - 110px); /* 100px сверху + 50px снизу */
            background: rgba(255, 255, 255, 0.1);
            border-radius: 50px;
            padding: 10px;
            box-sizing: border-box;
            pointer-events: all;
            top: 80px; /* Отступ сверху */
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .slider-left {
            left: 40px;
        }

        .slider-right {
            right: 40px;
        }

        .slider-thumb {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            width: 80px;
            height: 80px;
            background: rgba(255, 255, 255, 0.8);
            border-radius: 50%;
            cursor: pointer;
        }

        .button {
            padding: 8px 16px;
            border: none;
            border-radius: 6px;
            cursor: pointer;
            font-size: 14px;
            background: rgba(255, 255, 255, 0.1);
            color: var(--text-color);
            transition: all 0.3s;
            white-space: nowrap;
            min-width: 80px;
            text-align: center;
            touch-action: manipulation;
            user-select: none;
            -webkit-user-select: none;
            opacity: 0.7;
        }

        .button:active {
            background: rgba(255, 255, 255, 0.3);
            opacity: 1;
        }

        .button.active {
            background: rgba(255, 255, 255, 0.2);
            opacity: 1;
        }

        .button.led-off {
            opacity: 0.4;
            text-decoration: line-through;
        }

        .button.led-mid {
            opacity: 0.7;
        }

        .button.led-high {
            opacity: 1;
            text-shadow: 0 0 8px rgba(255, 255, 255, 0.5);
        }

        .status {
            position: absolute;
            bottom: 20px;
            left: 50%;
            transform: translateX(-50%);
            padding: 10px 20px;
            border-radius: 8px;
            display: none;
            z-index: 100;
            background: var(--control-bg);
            backdrop-filter: blur(5px);
            pointer-events: all;
        }

        .status.error {
            background: rgba(244, 67, 54, 0.8);
        }

        .status.success {
            background: rgba(76, 175, 80, 0.8);
        }

        .bt-status {
            display: none;
            color: #fff;
            font-size: 14px;
            padding: 4px 8px;
            background: rgba(0, 0, 0, 0.5);
            border-radius: 4px;
            margin-left: 10px;
        }
        
        .bt-status.active {
            display: block;
        }
        
        .bt-active {
            background: rgba(76, 175, 80, 0.2);
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="video-container">
            <div class="stream-placeholder" id="stream-placeholder">
                No video stream
            </div>
            <img id="stream">
        </div>
        
        <div class="controls-overlay">
            <div class="controls-top">
                <button class="button" id="stream-toggle">Start Stream</button>
                <button class="button" id="quality-toggle">Quality: SD</button>
                <button class="button led-off" id="led-button">LED Off</button>
                <button class="button active" id="control-mode">Joystick</button>
                <button class="button" id="bt-toggle">Bluetooth</button>
                <span class="bt-status">Bluetooth: Выключен</span>
            </div>

            <div class="control-mode-switch">
                <button class="button" id="fullscreen-button">⛶</button>
            </div>
            
            <div class="controls-right">
                <div class="joystick-container active" id="joystick-control">
                    <div id="joystick">
                        <div id="stick"></div>
                    </div>
                </div>
            </div>

            <div class="sliders-container" id="sliders-control">
                <div class="slider-vertical slider-left">
                    <div class="slider-thumb" id="left-thumb"></div>
                </div>
                <div class="slider-vertical slider-right">
                    <div class="slider-thumb" id="right-thumb"></div>
                </div>
            </div>

            <div id="status" class="status"></div>
        </div>
    </div>

    <script>
        const streamImg = document.getElementById('stream');
        const statusDiv = document.getElementById('status');
        const streamPlaceholder = document.getElementById('stream-placeholder');
        const streamToggle = document.getElementById('stream-toggle');
        const qualityButton = document.getElementById('quality-toggle');
        const ledButton = document.getElementById('led-button');
        const controlMode = document.getElementById('control-mode');
        const joystickControl = document.getElementById('joystick-control');
        const slidersControl = document.getElementById('sliders-control');
        const leftThumb = document.getElementById('left-thumb');
        const rightThumb = document.getElementById('right-thumb');
        const joystick = document.getElementById('joystick');
        const stick = document.getElementById('stick');
        const fullscreenButton = document.getElementById('fullscreen-button');
        const btStatus = document.querySelector('.bt-status');

        let isStoppingStream = false;
        let isStreaming = false;
        let isHD = false;
        let isLedOn = 0; // 0 - off, 1 - mid, 2 - high
        let isJoystickMode = true;
        let isDragging = false;
        let currentX = 0;
        let currentY = 0;
        let lastJoystickSendTime = 0;
        let pendingJoystickSend = null;
        let activeThumb = null;
        let startY = 0;
        let startTop = 0;
        let lastSendTime = 0;
        let pendingSend = null;
        let activeTouches = new Map(); // Хранит активные тачи для каждого слайдера

        const THROTTLE_MS = 400; // Common throttle time for both joystick and sliders

        let btPollingInterval = 2000; // 2 секунды по умолчанию
        let btConnected = false; // Флаг состояния подключения
        let isBtActive = false;  // Флаг активности Bluetooth

        function showStatus(message, isError = false) {
            statusDiv.textContent = message;
            statusDiv.style.display = 'block';
            statusDiv.className = 'status ' + (isError ? 'error' : 'success');
            setTimeout(() => {
                statusDiv.style.display = 'none';
            }, 3000);
        }

        function toggleStream() {
            if (!isStreaming) {
                startStream();
            } else {
                stopStream();
            }
        }

        function startStream() {
            isStoppingStream = false;
            isStreaming = true;
            streamImg.src = '/stream';
            streamImg.style.display = 'block';
            streamPlaceholder.style.display = 'none';
            streamToggle.textContent = 'Stop Stream';
            streamToggle.classList.add('active');
            showStatus('Starting stream...');

            streamImg.onerror = function() {
                if (!isStoppingStream) {
                    showStatus('Failed to start stream', true);
                    streamImg.style.display = 'none';
                    streamPlaceholder.style.display = 'flex';
                    isStreaming = false;
                    streamToggle.textContent = 'Start Stream';
                    streamToggle.classList.remove('active');
                }
            };
        }

        function stopStream() {
            isStoppingStream = true;
            isStreaming = false;
            fetch('/stopstream')
                .then(() => {
                    streamImg.style.display = 'none';
                    streamImg.src = '';
                    streamPlaceholder.style.display = 'flex';
                    streamToggle.textContent = 'Start Stream';
                    streamToggle.classList.remove('active');
                    showStatus('Stream stopped');
                })
                .catch(error => {
                    showStatus('Failed to stop stream: ' + error, true);
                });
        }

        function toggleQuality() {
            isHD = !isHD;
            const quality = isHD ? 'HD' : 'SD';
            qualityButton.textContent = 'Quality: ' + quality;
            qualityButton.classList.toggle('active', isHD);
            
            fetch('/quality?mode=' + quality)
                .then(response => response.text())
                .then(result => {
                    showStatus('Quality changed to ' + quality);
                })
                .catch(error => {
                    showStatus('Failed to change quality: ' + error, true);
                });
        }

        function toggleLED() {
            isLedOn = (isLedOn + 1) % 3;
            const states = ['off', 'mid', 'high'];
            const labels = ['LED Off', 'LED 50%', 'LED 100%'];
            const state = states[isLedOn];
            
            ledButton.className = 'button led-' + state;
            ledButton.textContent = labels[isLedOn];
            
            fetch('/led?state=' + state)
                .then(response => response.text())
                .then(result => {
                    showStatus('LED turned ' + state);
                })
                .catch(error => {
                    showStatus('Failed to control LED: ' + error, true);
                });
        }

        // Joystick control
        function sendJoystickData(x, y, force = false) {
            // Не отправляем, если BT активен
            if (isBtActive) return;
            
            const now = Date.now();
            
            // Clear any pending timeout
            if (pendingJoystickSend) {
                clearTimeout(pendingJoystickSend.timeout);
            }

            // If forced or enough time has passed, send immediately
            if (force || now - lastJoystickSendTime >= THROTTLE_MS) {
                fetch('/control', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({ 
                        mode: 'joystick',
                        x: x,
                        y: y
                    })
                }).catch(error => {
                    console.error('Failed to send control data:', error);
                });
                lastJoystickSendTime = now;
                pendingJoystickSend = null;
            } else {
                // Schedule to send at next available time
                pendingJoystickSend = {
                    data: { x, y },
                    timeout: setTimeout(() => {
                        sendJoystickData(x, y, true);
                    }, THROTTLE_MS - (now - lastJoystickSendTime))
                };
            }
        }

        function handleJoystickMove(event) {
            if (!isDragging) return;

            const rect = joystick.getBoundingClientRect();
            const centerX = rect.width / 2;
            const centerY = rect.height / 2;
            
            // Get touch or mouse position
            const clientX = event.clientX || event.touches[0].clientX;
            const clientY = event.clientY || event.touches[0].clientY;

            // Calculate relative position from center
            let x = clientX - rect.left - centerX;
            let y = clientY - rect.top - centerY;

            // Limit to square boundary
            const maxOffset = rect.width / 2 - stick.offsetWidth / 2;
            x = Math.max(-maxOffset, Math.min(maxOffset, x));
            y = Math.max(-maxOffset, Math.min(maxOffset, y));

            // Update stick position
            stick.style.transform = `translate(calc(-50% + ${x}px), calc(-50% + ${y}px))`;

            // Calculate normalized values (-1 to 1)
            const normalizedX = x / maxOffset;
            const normalizedY = -y / maxOffset; // Invert Y for traditional control scheme

            // Only send if values changed significantly
            if (Math.abs(normalizedX - currentX) > 0.1 || Math.abs(normalizedY - currentY) > 0.1) {
                currentX = normalizedX;
                currentY = normalizedY;
                sendJoystickData(normalizedX, normalizedY);
            }
        }

        function resetJoystick(sendData = false) {
            isDragging = false;
            stick.style.transform = 'translate(-50%, -50%)';
            currentX = 0;
            currentY = 0;
            // Отправляем данные только если это явно запрошено
            if (sendData && isJoystickMode && !isBtActive) {
                sendJoystickData(0, 0, true);
            }
        }

        // Mouse events for joystick
        stick.addEventListener('mousedown', (e) => {
            e.preventDefault();
            isDragging = true;
        });
        
        document.addEventListener('mousemove', (e) => {
            e.preventDefault();
            handleJoystickMove(e);
        });
        
        document.addEventListener('mouseup', (e) => {
            e.preventDefault();
            resetJoystick(true);
        });

        // Touch events for joystick
        stick.addEventListener('touchstart', (e) => {
            e.preventDefault();
            isDragging = true;
            handleJoystickMove(e.touches[0]);
        }, { passive: false });
        
        document.addEventListener('touchmove', (e) => {
            e.preventDefault();
            if (isDragging && e.touches.length > 0) {
                handleJoystickMove(e.touches[0]);
            }
        }, { passive: false });
        
        document.addEventListener('touchend', (e) => {
            e.preventDefault();
            resetJoystick(true);
        }, { passive: false });

        document.addEventListener('touchcancel', (e) => {
            e.preventDefault();
            resetJoystick(true);
        }, { passive: false });

        // Обработчики для кнопок (только один способ обработки)
        streamToggle.addEventListener('click', toggleStream);
        qualityButton.addEventListener('click', toggleQuality);
        ledButton.addEventListener('click', toggleLED);
        
        // Обработчик кнопки режима управления
        controlMode.addEventListener('click', () => {
            // Меняем режим
            isJoystickMode = !isJoystickMode;
            const mode = isJoystickMode ? 'joystick' : 'sliders';
            
            // Обновляем текст на кнопке
            controlMode.textContent = isJoystickMode ? 'Joystick' : 'Sliders';
            
            // Обновляем классы активности контейнеров
            if (isJoystickMode) {
                joystickControl.classList.add('active');
                slidersControl.classList.remove('active');
            } else {
                joystickControl.classList.remove('active');
                slidersControl.classList.add('active');
            }
            
            // Управляем отображением элементов управления напрямую
            const joystickContainer = document.querySelector('.joystick-container');
            const slidersContainer = document.querySelector('.sliders-container');
            
            if (isBtActive) {
                // Если BT активен, скрываем все элементы управления
                if (joystickContainer) joystickContainer.style.display = 'none';
                if (slidersContainer) slidersContainer.style.display = 'none';
            } else {
                // Иначе показываем соответствующие элементы
                if (isJoystickMode) {
                    if (joystickContainer) joystickContainer.style.display = 'block';
                    if (slidersContainer) slidersContainer.style.display = 'none';
                } else {
                    if (joystickContainer) joystickContainer.style.display = 'none';
                    if (slidersContainer) slidersContainer.style.display = 'block';
                }
            }
            
            // Сбрасываем визуальные элементы БЕЗ отправки запросов
            if (isJoystickMode) {
                // Сбрасываем джойстик без отправки данных
                isDragging = false;
                stick.style.transform = 'translate(-50%, -50%)';
                currentX = 0;
                currentY = 0;
            } else {
                // Сбрасываем слайдеры без отправки данных
                leftThumb.style.top = '50%';
                rightThumb.style.top = '50%';
            }
            
            // Отправляем ТОЛЬКО ОДИН запрос на переключение режима
            fetch('/control', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ mode })
            }).then(response => {
                if (isBtActive && response.ok) {
                    setTimeout(checkBluetoothStatus, 500);
                }
            }).catch(error => {
                console.error('Failed to change control mode:', error);
            });
        });

        // Добавляем обработку тач-событий для кнопок
        [streamToggle, qualityButton, ledButton, controlMode, fullscreenButton].forEach(button => {
            button.addEventListener('touchstart', (e) => {
                e.preventDefault();
                button.click(); // Эмулируем обычный клик
            }, { passive: false });
        });

        // Предотвращаем скролл/зум на мобильных устройствах
        document.addEventListener('gesturestart', (e) => {
            e.preventDefault();
        }, { passive: false });

        document.addEventListener('gesturechange', (e) => {
            e.preventDefault();
        }, { passive: false });

        document.addEventListener('gestureend', (e) => {
            e.preventDefault();
        }, { passive: false });

        // Sliders control
        function sendSliderData(data, force = false) {
            // Не отправляем, если BT активен
            if (isBtActive) return;
            
            const now = Date.now();
            
            // Clear any pending timeout
            if (pendingSend) {
                clearTimeout(pendingSend.timeout);
            }

            // If forced or enough time has passed, send immediately
            if (force || now - lastSendTime >= THROTTLE_MS) {
                fetch('/control', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(data)
                }).catch(error => {
                    console.error('Failed to send sliders data:', error);
                });
                lastSendTime = now;
                pendingSend = null;
            } else {
                // Schedule to send at next available time
                pendingSend = {
                    data: data,
                    timeout: setTimeout(() => {
                        sendSliderData(data, true);
                    }, THROTTLE_MS - (now - lastSendTime))
                };
            }
        }

        function startDragging(e, thumb) {
            e.preventDefault();
            e.stopPropagation();
            
            // Обрабатываем все новые касания
            if (e.touches) {
                Array.from(e.changedTouches).forEach(touch => {
                    const rect = thumb.parentElement.getBoundingClientRect();
                    activeTouches.set(touch.identifier, {
                        thumb: thumb,
                        startY: touch.clientY,
                        rect: rect,
                        startTop: (parseFloat(thumb.style.top) || 50) / 100 * rect.height
                    });
                });
            } else {
                activeTouches.set('mouse', {
                    thumb: thumb,
                    startY: e.clientY,
                    rect: thumb.parentElement.getBoundingClientRect(),
                    startTop: (parseFloat(thumb.style.top) || 50) / 100 * thumb.parentElement.getBoundingClientRect().height
                });
            }
        }

        function handleDragging(e) {
            e.preventDefault();
            e.stopPropagation();

            // Обработка всех активных касаний
            if (e.touches) {
                Array.from(e.touches).forEach(touch => {
                    const touchData = activeTouches.get(touch.identifier);
                    if (touchData) {
                        updateThumbPosition(touch.clientY, touchData);
                    }
                });
            } else if (activeTouches.has('mouse')) {
                const touchData = activeTouches.get('mouse');
                updateThumbPosition(e.clientY, touchData);
            }
        }

        function updateThumbPosition(clientY, touchData) {
            const { thumb, startY, rect, startTop } = touchData;
            const deltaY = clientY - startY;
            
            // Вычисляем новую позицию
            let newTop = startTop + deltaY;
            
            // Ограничиваем движение
            const thumbSize = thumb.offsetHeight;
            const minTop = thumbSize / 2;
            const maxTop = rect.height - thumbSize / 2;
            newTop = Math.max(minTop, Math.min(maxTop, newTop));
            
            // Обновляем позицию в процентах
            thumb.style.top = (newTop / rect.height * 100) + '%';
            
            // Вычисляем значение для отправки
            const normalizedPosition = (newTop - minTop) / (maxTop - minTop);
            const value = -(normalizedPosition * 2 - 1);
            
            // Отправляем данные
            const data = {
                mode: 'sliders',
                left: thumb === leftThumb ? value : leftValue(),
                right: thumb === rightThumb ? value : rightValue()
            };
            
            sendSliderData(data);
        }

        function stopDragging(e) {
            e.preventDefault();
            e.stopPropagation();
            
            let releasedThumbs = new Set();
            
            if (e.changedTouches) {
                Array.from(e.changedTouches).forEach(touch => {
                    const touchData = activeTouches.get(touch.identifier);
                    if (touchData) {
                        releasedThumbs.add(touchData.thumb);
                        activeTouches.delete(touch.identifier);
                    }
                });
            } else {
                const touchData = activeTouches.get('mouse');
                if (touchData) {
                    releasedThumbs.add(touchData.thumb);
                    activeTouches.delete('mouse');
                }
            }
            
            // Сбрасываем позиции отпущенных слайдеров
            releasedThumbs.forEach(thumb => {
                thumb.style.top = '50%';
            });
            
            // Отправляем последнюю позицию
            if (activeTouches.size === 0) {
                const data = {
                    mode: 'sliders',
                    left: releasedThumbs.has(leftThumb) ? 0 : leftValue(),
                    right: releasedThumbs.has(rightThumb) ? 0 : rightValue()
                };
                sendSliderData(data, true);
            }
        }

        function initSliders() {
            [leftThumb, rightThumb].forEach(thumb => {
                // Mouse events
                thumb.addEventListener('mousedown', (e) => startDragging(e, thumb));
                
                // Touch events with passive: false
                thumb.addEventListener('touchstart', (e) => startDragging(e, thumb), { passive: false });
            });

            // Global events
            document.addEventListener('mousemove', handleDragging);
            document.addEventListener('touchmove', handleDragging, { passive: false });
            document.addEventListener('mouseup', stopDragging);
            document.addEventListener('touchend', stopDragging, { passive: false });
            document.addEventListener('touchcancel', stopDragging, { passive: false });
        }

        function leftValue() {
            const rect = leftThumb.parentElement.getBoundingClientRect();
            const thumbRect = leftThumb.getBoundingClientRect();
            const thumbSize = leftThumb.offsetHeight;
            const minTop = thumbSize / 2;
            const maxTop = rect.height - thumbSize / 2;
            const currentTop = thumbRect.top + thumbSize/2 - rect.top;
            const value = Math.max(-1, Math.min(1, -((currentTop - minTop) / (maxTop - minTop) * 2 - 1)));
            return value;
        }

        function rightValue() {
            const rect = rightThumb.parentElement.getBoundingClientRect();
            const thumbRect = rightThumb.getBoundingClientRect();
            const thumbSize = rightThumb.offsetHeight;
            const minTop = thumbSize / 2;
            const maxTop = rect.height - thumbSize / 2;
            const currentTop = thumbRect.top + thumbSize/2 - rect.top;
            const value = Math.max(-1, Math.min(1, -((currentTop - minTop) / (maxTop - minTop) * 2 - 1)));
            return value;
        }

        // Добавляем обработчик для кнопки полноэкранного режима
        function toggleFullscreen() {
            if (!document.fullscreenElement) {
                document.documentElement.requestFullscreen().catch(err => {
                    showStatus('Error attempting to enable fullscreen: ' + err.message, true);
                });
            } else {
                document.exitFullscreen();
            }
        }

        fullscreenButton.addEventListener('click', toggleFullscreen);
        fullscreenButton.addEventListener('touchstart', (e) => {
            e.preventDefault();
            toggleFullscreen();
        }, { passive: false });

        // Initialize controls - не отправляем запрос при инициализации
        resetJoystick(false);
        initSliders();

        // Назначаем обработчик для кнопки включения/выключения Bluetooth
        document.getElementById('bt-toggle').addEventListener('click', toggleBluetooth);
        
        // Запускаем периодический опрос статуса Bluetooth
        checkBluetoothStatus();

        function checkBluetoothStatus() {
            fetch('/bt/status')
                .then(response => response.text())
                .then(status => {
                    const btStatusElement = document.getElementById('bt-status');
                    btStatusElement.textContent = status;
                    
                    // Проверяем статус подключения
                    const isConnected = status.includes("Готов к работе");
                    
                    // Если статус изменился
                    if (isConnected !== btConnected) {
                        btConnected = isConnected;
                        
                        // Изменяем интервал опроса в зависимости от статуса подключения
                        btPollingInterval = btConnected ? 5000 : 2000; // 5 секунд если подключено, 2 секунды если нет
                    }
                    
                    if (status.includes("Готов к работе")) {
                        btStatusElement.classList.remove('status-disconnected');
                        btStatusElement.classList.add('status-connected');
                        document.getElementById('bt-toggle').classList.add('active');
                        isBtActive = true;
                        
                        // Скрываем элементы управления напрямую
                        const joystickContainer = document.querySelector('.joystick-container');
                        const slidersContainer = document.querySelector('.sliders-container');
                        if (joystickContainer) joystickContainer.style.display = 'none';
                        if (slidersContainer) slidersContainer.style.display = 'none';
                        document.getElementById('control-mode').classList.add('active');
                    } else if (status.includes("Поиск")) {
                        btStatusElement.classList.remove('status-connected');
                        btStatusElement.classList.add('status-disconnected');
                        document.getElementById('bt-toggle').classList.add('active');
                        isBtActive = true;
                        
                        // Скрываем элементы управления напрямую
                        const joystickContainer = document.querySelector('.joystick-container');
                        const slidersContainer = document.querySelector('.sliders-container');
                        if (joystickContainer) joystickContainer.style.display = 'none';
                        if (slidersContainer) slidersContainer.style.display = 'none';
                        document.getElementById('control-mode').classList.add('active');
                    } else {
                        btStatusElement.classList.remove('status-connected');
                        btStatusElement.classList.add('status-disconnected');
                        document.getElementById('bt-toggle').classList.remove('active');
                        isBtActive = false;
                        
                        // Показываем соответствующие элементы управления напрямую
                        const joystickContainer = document.querySelector('.joystick-container');
                        const slidersContainer = document.querySelector('.sliders-container');
                        const controlMode = document.getElementById('control-mode');
                        
                        controlMode.textContent = isJoystickMode ? 'Joystick' : 'Sliders';
                        
                        if (isJoystickMode) {
                            if (joystickContainer) joystickContainer.style.display = 'block';
                            if (slidersContainer) slidersContainer.style.display = 'none';
                            joystickControl.classList.add('active');
                            slidersControl.classList.remove('active');
                        } else {
                            if (joystickContainer) joystickContainer.style.display = 'none';
                            if (slidersContainer) slidersContainer.style.display = 'block';
                            joystickControl.classList.remove('active');
                            slidersControl.classList.add('active');
                        }
                    }
                    
                    updateButtonUI();
                })
                .catch(error => {
                    console.error('Ошибка при получении статуса Bluetooth:', error);
                })
                .finally(() => {
                    // Устанавливаем следующий опрос с текущим интервалом
                    setTimeout(checkBluetoothStatus, btPollingInterval);
                });
        }

        // Функция для переключения Bluetooth
        function toggleBluetooth() {
            // Определяем текущий режим управления
            const currentMode = isJoystickMode ? 'joystick' : 'sliders';
            const action = isBtActive ? 'off' : 'on';
            
            fetch(`/bt?state=${action}&mode=${currentMode}`)
                .then(response => {
                    if (response.ok) {
                        // Устанавливаем новое состояние BT
                        isBtActive = !isBtActive;
                        
                        // Непосредственно обновляем UI элементов
                        if (isBtActive) {
                            // Скрываем элементы управления сразу при включении BT
                            const joystickContainer = document.querySelector('.joystick-container');
                            const slidersContainer = document.querySelector('.sliders-container');
                            
                            if (joystickContainer) joystickContainer.style.display = 'none';
                            if (slidersContainer) slidersContainer.style.display = 'none';
                            
                            document.getElementById('control-mode').classList.add('active');
                        } else {
                            // Показываем соответствующие элементы управления при выключении BT
                            const joystickContainer = document.querySelector('.joystick-container');
                            const slidersContainer = document.querySelector('.sliders-container');
                            const controlMode = document.getElementById('control-mode');
                            
                            controlMode.textContent = isJoystickMode ? 'Joystick' : 'Sliders';
                            
                            if (isJoystickMode) {
                                if (joystickContainer) joystickContainer.style.display = 'block';
                                if (slidersContainer) slidersContainer.style.display = 'none';
                                joystickControl.classList.add('active');
                                slidersControl.classList.remove('active');
                            } else {
                                if (joystickContainer) joystickContainer.style.display = 'none';
                                if (slidersContainer) slidersContainer.style.display = 'block';
                                joystickControl.classList.remove('active');
                                slidersControl.classList.add('active');
                            }
                        }
                        
                        // Обновляем кнопку BT
                        updateButtonUI();
                        
                        // После включения/выключения сразу проверяем статус
                        setTimeout(checkBluetoothStatus, 500);
                    }
                })
                .catch(error => {
                    console.error(`Ошибка при ${action === 'on' ? 'включении' : 'выключении'} Bluetooth:`, error);
                });
        }

        // Обновление UI кнопок в зависимости от состояния Bluetooth
        function updateButtonUI() {
            const btToggle = document.getElementById('bt-toggle');
            const btStatus = document.getElementById('bt-status');
            
            if (isBtActive) {
                btToggle.classList.add('active');
                btToggle.textContent = 'Выключить BT';
                btStatus.classList.add('active');
            } else {
                btToggle.classList.remove('active');
                btToggle.textContent = 'Включить BT';
                btStatus.classList.remove('active');
                btStatus.textContent = 'Bluetooth: Выключен';
            }
        }

        // Функция для сброса слайдеров без отправки данных
        function resetSlidersVisual() {
            const leftThumb = document.getElementById('left-thumb');
            const rightThumb = document.getElementById('right-thumb');
            leftThumb.style.top = '50%';
            rightThumb.style.top = '50%';
        }
    </script>
</body>
</html>)rawliteral";
