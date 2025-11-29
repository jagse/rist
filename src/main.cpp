#include <M5Unified.h>
#include <time.h>

#if __has_include(<WiFi.h>)
 #include <WiFi.h>
#endif

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;        // Berlin is UTC+1
const int daylightOffset_sec = 3600;    // Daylight saving time

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

    // Connect to WiFi
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
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

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
        M5.Display.println("Using RTC time");
    }

    delay(2000);
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

    static bool displayOn = true;
    static unsigned long buttonPressTime = 0;
    static bool beepEnabled = true;
    static bool beepStatusChanged = true;

    // Toggle beep with power button (BtnPWR - top button)
    if (M5.BtnPWR.wasPressed()) {
        beepEnabled = !beepEnabled;
        beepStatusChanged = true;
        if (beepEnabled) {
            M5.Speaker.tone(1000, 100);
        }
    }

    // Toggle display on/off with main button (BtnA)
    if (M5.BtnA.wasPressed()) {
        if (beepEnabled) {
            M5.Speaker.tone(1000, 100);
        }
        displayOn = !displayOn;
        buttonPressTime = millis();
        if (displayOn) {
            M5.Display.wakeup();
            M5.Display.setBrightness(128);
        } else {
            M5.Display.sleep();
        }

        // Flash LED in current hour color
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            uint32_t hourColor = hourColors[timeinfo.tm_hour];
            M5.Power.setLed((hourColor >> 16) & 0xFF);
        }
    }

    // Beep on side button (BtnB)
    if (M5.BtnB.wasPressed()) {
        if (beepEnabled) {
            M5.Speaker.tone(1000, 100);
        }
    }

    // Turn off LED after 500ms button flash
    if (buttonPressTime > 0 && millis() - buttonPressTime > 500) {
        M5.Power.setLed(0);
        buttonPressTime = 0;
    }

    if (!displayOn) {
        delay(100);
        return;
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
    static bool needsFullRedraw = true;

    // Only redraw when time changes
    if (timeinfo.tm_sec == lastSecond && !needsFullRedraw) {
        delay(100);
        return;
    }

    if (needsFullRedraw) {
        M5.Display.clear(TFT_BLACK);
        needsFullRedraw = false;
    }

    // Get current hour color
    uint32_t hourColor = hourColors[timeinfo.tm_hour];
    uint16_t displayColor = M5.Display.color565(
        (hourColor >> 16) & 0xFF,
        (hourColor >> 8) & 0xFF,
        hourColor & 0xFF
    );


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

    // Display date (only update on day change)
    if (timeinfo.tm_mday != lastDay) {
        M5.Display.fillRect(40, 90, 200, 16, TFT_BLACK);
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.setCursor(40, 90);
        M5.Display.printf("%02d.%02d.%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    }

    // WiFi status icon (bottom left) - update every 5 seconds
    static int lastWifiUpdate = -1;
    if (timeinfo.tm_sec % 5 == 0 && timeinfo.tm_sec != lastWifiUpdate) {
        M5.Display.fillRect(10, 110, 220, 16, TFT_BLACK);
        M5.Display.setTextSize(2);
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
        lastWifiUpdate = timeinfo.tm_sec;
    }

    // Flash LED at the start of each hour (only if not from button press)
    static int lastLedHour = -1;
    if (buttonPressTime == 0) {
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
