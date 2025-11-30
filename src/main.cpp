#include <M5Unified.h>
#include <time.h>
#include <esp_sleep.h>

#if __has_include(<WiFi.h>)
 #include <WiFi.h>
#endif

const char* ntpServer = "pool.ntp.org";
const char* timeZone = "CET-1CEST,M3.5.0,M10.5.0/3";  // Berlin timezone with automatic DST

const char* ssid = "jagse";
const char* password = "UM2p-m2HZ-v4mo-4ttL";

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);

    // Set landscape orientation
    M5.Display.setRotation(1);
    M5.Display.clear(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextSize(2);

    delay(1000);
    M5.Display.clear(TFT_BLACK);
}

// Color palette for each hour (24 unique colors)
const uint32_t hourColors[24] = {
    0xFF0000, // 00:00 - Red
    0xFF4500, // 01:00 - Orange Red
    0xFF8C00, // 02:00 - Dark Orange
    0xFFD700, // 03:00 - Gold
    0xFFFF00, // 04:00 - Yellow
    0x9ACD32, // 05:00 - Yellow Green
    0x00FF00, // 06:00 - Green
    0x00FF7F, // 07:00 - Spring Green
    0x00FFFF, // 08:00 - Cyan
    0x1E90FF, // 09:00 - Dodger Blue
    0x0000FF, // 10:00 - Blue
    0x4169E1, // 11:00 - Royal Blue
    0x8A2BE2, // 12:00 - Blue Violet
    0x9370DB, // 13:00 - Medium Purple
    0xFF00FF, // 14:00 - Magenta
    0xFF1493, // 15:00 - Deep Pink
    0xFF69B4, // 16:00 - Hot Pink
    0xFFC0CB, // 17:00 - Pink
    0xFFFFFF, // 18:00 - White
    0xC0C0C0, // 19:00 - Silver
    0x808080, // 20:00 - Gray
    0xFFA500, // 21:00 - Orange
    0xFF6347, // 22:00 - Tomato
    0xDC143C  // 23:00 - Crimson
};

const char* daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// Menu system
enum MenuState {
    MENU_HIDDEN,
    MENU_VISIBLE
};

const int MENU_ITEMS_COUNT = 1;
const char* menuItems[] = {
    "Connect to WiFi"
};

int selectedMenuItem = 0;

void connectToWiFi() {
    M5.Display.clear(TFT_BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.printf("SSID: %s\n", ssid);
    M5.Display.println("Connecting...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        M5.Display.print(".");
        attempts++;

        if (attempts % 20 == 0) {
            M5.Display.println();
        }
    }

    M5.Display.println();

    if (WiFi.status() == WL_CONNECTED) {
        M5.Display.println("WiFi Connected!");
        M5.Display.printf("IP: %s\n", WiFi.localIP().toString().c_str());

        // Sync time with NTP
        configTzTime(timeZone, ntpServer);

        // Wait for NTP sync
        M5.Display.println("Syncing time...");
        struct tm timeinfo;
        int syncAttempts = 0;
        while (!getLocalTime(&timeinfo) && syncAttempts < 10) {
            delay(500);
            syncAttempts++;
        }

        if (getLocalTime(&timeinfo)) {
            // Update RTC with NTP time
            auto dt = m5::rtc_datetime_t();
            dt.date.year = timeinfo.tm_year + 1900;
            dt.date.month = timeinfo.tm_mon + 1;
            dt.date.date = timeinfo.tm_mday;
            dt.date.weekDay = timeinfo.tm_wday;
            dt.time.hours = timeinfo.tm_hour;
            dt.time.minutes = timeinfo.tm_min;
            dt.time.seconds = timeinfo.tm_sec;
            M5.Rtc.setDateTime(dt);
            M5.Display.println("RTC updated!");
        }
    } else {
        M5.Display.println("WiFi Failed!");
    }

    delay(2000);
    M5.Display.clear(TFT_BLACK);
}

void drawMenu() {
    M5.Display.clear(TFT_BLACK);
    M5.Display.setTextSize(2);

    // Draw title
    M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Display.setCursor(10, 10);
    M5.Display.println("MENU");

    // Draw menu items
    for (int i = 0; i < MENU_ITEMS_COUNT; i++) {
        int y = 40 + (i * 25);

        if (i == selectedMenuItem) {
            // Highlight selected item
            M5.Display.fillRect(5, y - 2, 230, 20, TFT_DARKGREY);
            M5.Display.setTextColor(TFT_YELLOW, TFT_DARKGREY);
        } else {
            M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        }

        M5.Display.setCursor(10, y);
        M5.Display.println(menuItems[i]);
    }

    // Instructions
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    M5.Display.setCursor(10, 110);
    M5.Display.println("PWR: Select  BtnB: Back");
}

void drawBatteryIcon(int x, int y, int level) {
    // Draw battery outline (larger)
    M5.Display.drawRect(x, y, 26, 14, TFT_WHITE);
    M5.Display.fillRect(x + 26, y + 4, 3, 6, TFT_WHITE); // battery tip

    // Fill battery based on level
    int fillWidth = (level * 24) / 100;
    uint16_t fillColor;
    if (level > 60) {
        fillColor = TFT_DARKGREEN;
    } else if (level > 20) {
        fillColor = TFT_ORANGE;
    } else {
        fillColor = TFT_RED;
    }

    if (fillWidth > 0) {
        M5.Display.fillRect(x + 1, y + 1, fillWidth, 12, fillColor);
    }
}

void drawBellIcon(int x, int y, uint16_t color) {
    // Draw bell body (larger trapezoid shape)
    M5.Display.drawLine(x + 7, y, x + 2, y + 9, color);
    M5.Display.drawLine(x + 7, y, x + 12, y + 9, color);
    M5.Display.drawLine(x + 2, y + 9, x + 12, y + 9, color);

    // Fill bell
    for (int i = 0; i < 9; i++) {
        int width = 3 + (i * 7) / 9;
        int startX = x + 7 - width / 2;
        M5.Display.drawLine(startX, y + i, startX + width, y + i, color);
    }

    // Bell bottom rim
    M5.Display.drawLine(x + 1, y + 10, x + 13, y + 10, color);

    // Bell clapper
    M5.Display.fillCircle(x + 7, y + 13, 2, color);
}

void loop() {
    M5.update();

    static unsigned long ledFlashTime = 0;
    static unsigned long btnAHoldStartTime = 0;
    static bool beepEnabled = true;
    static bool beepStatusChanged = true;
    static MenuState menuState = MENU_HIDDEN;
    static bool needsFullRedraw = true;

    // Show menu with power button (BtnPWR - top button)
    if (M5.BtnPWR.wasPressed()) {
        if (menuState == MENU_HIDDEN) {
            // Show menu
            menuState = MENU_VISIBLE;
            selectedMenuItem = 0;
            drawMenu();
            if (beepEnabled) {
                M5.Speaker.tone(1000, 100);
            }
        } else {
            // Execute selected menu item
            if (beepEnabled) {
                M5.Speaker.tone(1200, 100);
            }

            if (selectedMenuItem == 0) {
                // Connect to WiFi
                connectToWiFi();
            }

            // Return to main screen
            menuState = MENU_HIDDEN;
            needsFullRedraw = true;
            beepStatusChanged = true;
        }
    }

    // Enter deep sleep when main button (BtnA) held for 5 seconds
    if (M5.BtnA.isPressed()) {
        if (btnAHoldStartTime == 0) {
            btnAHoldStartTime = millis();
            if (beepEnabled) {
                M5.Speaker.tone(1000, 100);
            }
        }

        unsigned long holdDuration = millis() - btnAHoldStartTime;

        // Visual feedback: flash LED while holding
        if (holdDuration < 5000) {
            if (holdDuration % 1000 < 500) {
                struct tm timeinfo;
                if (getLocalTime(&timeinfo)) {
                    uint32_t hourColor = hourColors[timeinfo.tm_hour];
                    M5.Power.setLed((hourColor >> 16) & 0xFF);
                }
            } else {
                M5.Power.setLed(0);
            }
        }

        // After 5 seconds, enter deep sleep
        if (holdDuration >= 5000) {
            M5.Power.setLed(0);
            if (beepEnabled) {
                M5.Speaker.tone(1500, 200);
                delay(250);
            }

            // Configure wake-up on power button (GPIO35 for StickC Plus2)
            esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);  // Wake on LOW

            // Enter deep sleep
            esp_deep_sleep_start();
        }
    } else if (btnAHoldStartTime > 0) {
        // Button released before 5 seconds
        btnAHoldStartTime = 0;
        M5.Power.setLed(0);
    }

    // Back/cancel with side button (BtnB)
    if (M5.BtnB.wasPressed()) {
        if (menuState == MENU_VISIBLE) {
            // Exit menu
            menuState = MENU_HIDDEN;
            needsFullRedraw = true;
            beepStatusChanged = true;
            if (beepEnabled) {
                M5.Speaker.tone(800, 100);
            }
        } else {
            if (beepEnabled) {
                M5.Speaker.tone(1000, 100);
            }
        }
    }

    // If menu is visible, don't update the clock display
    if (menuState == MENU_VISIBLE) {
        delay(100);
        return;
    }

    // Turn off LED after 500ms flash
    if (ledFlashTime > 0 && millis() - ledFlashTime > 500) {
        M5.Power.setLed(0);
        ledFlashTime = 0;
    }

    // Get time from RTC
    auto dt = M5.Rtc.getDateTime();
    struct tm timeinfo;
    timeinfo.tm_year = dt.date.year - 1900;
    timeinfo.tm_mon = dt.date.month - 1;
    timeinfo.tm_mday = dt.date.date;
    timeinfo.tm_wday = dt.date.weekDay;
    timeinfo.tm_hour = dt.time.hours;
    timeinfo.tm_min = dt.time.minutes;
    timeinfo.tm_sec = dt.time.seconds;

    static int lastSecond = -1;
    static int lastMinute = -1;
    static int lastHour = -1;
    static int lastDay = -1;
    static int lastBatteryMinute = -1;
    static int lastWifiUpdate = -1;

    // Only redraw when time changes
    if (timeinfo.tm_sec == lastSecond && !needsFullRedraw) {
        delay(100);
        return;
    }

    if (needsFullRedraw) {
        M5.Display.clear(TFT_BLACK);
    }

    // Get current hour color
    uint32_t hourColor = hourColors[timeinfo.tm_hour];
    uint16_t displayColor = M5.Display.color565(
        (hourColor >> 16) & 0xFF,
        (hourColor >> 8) & 0xFF,
        hourColor & 0xFF
    );


    // Update all display elements immediately when needsFullRedraw is true
    if (needsFullRedraw) {
        // Day of week at top
        M5.Display.fillRect(10, 8, 80, 20, TFT_BLACK);
        M5.Display.setTextSize(3);
        M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
        M5.Display.setCursor(10, 8);
        M5.Display.printf("%s", daysOfWeek[timeinfo.tm_wday]);
        lastDay = timeinfo.tm_mday;

        // Beep status icon
        M5.Display.fillRect(165, 8, 20, 20, TFT_BLACK);
        if (beepEnabled) {
            drawBellIcon(167, 9, TFT_YELLOW);
        } else {
            drawBellIcon(167, 9, TFT_DARKGREY);
        }
        beepStatusChanged = false;

        // Battery icon
        int batteryLevel = M5.Power.getBatteryLevel();
        M5.Display.fillRect(190, 8, 35, 20, TFT_BLACK);
        drawBatteryIcon(195, 10, batteryLevel);
        lastBatteryMinute = timeinfo.tm_min;

        // Time display
        M5.Display.fillRect(10, 50, 230, 26, TFT_BLACK);
        M5.Display.setTextSize(3);

        char timeStr[12];
        sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        // Shadow effect
        M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
        M5.Display.setCursor(17, 57);
        M5.Display.print(timeStr);

        // Main time display with hour color
        M5.Display.setTextColor(displayColor, TFT_BLACK);
        M5.Display.setCursor(15, 55);
        M5.Display.print(timeStr);

        // WiFi status and Date on same line
        M5.Display.fillRect(10, 110, 220, 16, TFT_BLACK);
        M5.Display.setTextSize(2);

        // WiFi status (left side)
        if (WiFi.status() == WL_CONNECTED) {
            int rssi = WiFi.RSSI();
            M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
            M5.Display.setCursor(10, 110);
            M5.Display.printf("WiFi:%ddBm", rssi);
        } else {
            M5.Display.setTextColor(TFT_RED, TFT_BLACK);
            M5.Display.setCursor(10, 110);
            M5.Display.print("WiFi:OFF");
        }

        // Date (right side)
        char dateStr[16];
        sprintf(dateStr, "%02d.%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1);
        int dateWidth = strlen(dateStr) * 12; // Each char is ~12 pixels wide at size 2
        int dateX = 240 - dateWidth - 10; // Screen width 240 - text width - padding 10
        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.setCursor(dateX, 110);
        M5.Display.print(dateStr);

        lastSecond = timeinfo.tm_sec;
        lastMinute = timeinfo.tm_min;
        lastHour = timeinfo.tm_hour;
        lastWifiUpdate = timeinfo.tm_sec;
        needsFullRedraw = false;
    }

    // Day of week at top (only update on day change)
    if (timeinfo.tm_mday != lastDay) {
        M5.Display.fillRect(10, 8, 80, 20, TFT_BLACK);
        M5.Display.setTextSize(3);
        M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
        M5.Display.setCursor(10, 8);
        M5.Display.printf("%s", daysOfWeek[timeinfo.tm_wday]);
        lastDay = timeinfo.tm_mday;
    }

    // Beep status icon (top, left of battery) - update when changed
    if (beepStatusChanged) {
        M5.Display.fillRect(165, 8, 20, 20, TFT_BLACK);
        if (beepEnabled) {
            drawBellIcon(167, 9, TFT_YELLOW);
        } else {
            drawBellIcon(167, 9, TFT_DARKGREY);
        }
        beepStatusChanged = false;
    }

    // Battery icon (top right) - update every minute
    if (timeinfo.tm_min != lastBatteryMinute) {
        int batteryLevel = M5.Power.getBatteryLevel();
        M5.Display.fillRect(190, 8, 35, 20, TFT_BLACK);
        drawBatteryIcon(195, 10, batteryLevel);
        lastBatteryMinute = timeinfo.tm_min;
    }

    // Display time with color and glow effect (only update on second change)
    if (timeinfo.tm_sec != lastSecond) {
        M5.Display.fillRect(10, 50, 230, 26, TFT_BLACK);
        M5.Display.setTextSize(3);

        char timeStr[12];
        sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        // Shadow effect (draw offset in dark gray)
        M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
        M5.Display.setCursor(17, 57);
        M5.Display.print(timeStr);

        // Main time display with hour color
        M5.Display.setTextColor(displayColor, TFT_BLACK);
        M5.Display.setCursor(15, 55);
        M5.Display.print(timeStr);

        lastMinute = timeinfo.tm_min;
        lastHour = timeinfo.tm_hour;
    }

    // WiFi status and Date (bottom line) - update every 5 seconds
    if (timeinfo.tm_sec % 5 == 0 && timeinfo.tm_sec != lastWifiUpdate) {
        M5.Display.fillRect(10, 110, 220, 16, TFT_BLACK);
        M5.Display.setTextSize(2);

        // WiFi status (left side)
        if (WiFi.status() == WL_CONNECTED) {
            int rssi = WiFi.RSSI();
            M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
            M5.Display.setCursor(10, 110);
            M5.Display.printf("WiFi:%ddBm", rssi);
        } else {
            M5.Display.setTextColor(TFT_RED, TFT_BLACK);
            M5.Display.setCursor(10, 110);
            M5.Display.print("WiFi:OFF");
        }

        // Date (right side)
        char dateStr[16];
        sprintf(dateStr, "%02d.%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1);
        int dateWidth = strlen(dateStr) * 12; // Each char is ~12 pixels wide at size 2
        int dateX = 240 - dateWidth - 10; // Screen width 240 - text width - padding 10
        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.setCursor(dateX, 110);
        M5.Display.print(dateStr);

        lastWifiUpdate = timeinfo.tm_sec;
    }

    // Flash LED at the start of each hour (only if not holding button)
    static int lastLedHour = -1;
    if (btnAHoldStartTime == 0) {
        if (timeinfo.tm_hour != lastLedHour && timeinfo.tm_min == 0 && timeinfo.tm_sec < 10) {
            // Flash the LED for the first 10 seconds of each hour
            M5.Power.setLed((hourColor >> 16) & 0xFF); // Use red component for brightness
            lastLedHour = timeinfo.tm_hour;
        } else if (timeinfo.tm_min > 0 || timeinfo.tm_sec >= 10) {
            M5.Power.setLed(0); // Turn off LED
            if (timeinfo.tm_min > 0) lastLedHour = -1;
        }
    }

    lastSecond = timeinfo.tm_sec;
    delay(100);
}
