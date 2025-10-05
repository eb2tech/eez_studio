#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "ui/ui.h"

#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS
#define LCD_BACKLIGHT_PIN 21

// Display configuration - matches EEZ Studio project settings
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
static lv_color_t buf[screenWidth * 10]; // 10 lines buffer

TFT_eSPI tft = TFT_eSPI();

// Display flushing callback - TFT_eSPI implementation
void my_disp_flush(lv_display_t *display, const lv_area_t *area, uint8_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t*)color_p, w * h, true);
    tft.endWrite();
    
    lv_display_flush_ready(display); // Tell LVGL you are ready with the flushing
}

void setup() {
    Serial.begin(115200);
    Serial.println("EEZ Studio LVGL Demo Starting...");

    String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println(LVGL_Arduino);

    Serial.println("In setup()");
  
    // Initialize TFT display hardware
    tft.init();
    tft.setRotation(2); 
    pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
    digitalWrite(LCD_BACKLIGHT_PIN, HIGH); // Turn on backlight
    
    // Initialize LVGL
    lv_init();
    
    // Create display (LVGL 9.x API)
    lv_display_t *display = lv_display_create(screenWidth, screenHeight);
    
    // Set display buffer
    lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    // Set display flush callback
    lv_display_set_flush_cb(display, my_disp_flush);
        
    // Initialize EEZ Studio generated UI
    ui_init();
    
    Serial.println("UI initialized and ready!");
    
    // Force initial screen refresh
    lv_refr_now(display);
    
    Serial.println("Setup complete!");
}

void loop() {
    // CRITICAL: Tell LVGL how much time has passed
    lv_tick_inc(5); // 5ms matches our delay below
    
    // Handle LVGL tasks
    lv_timer_handler();
    
    // Handle EEZ Studio UI updates
    ui_tick();
    
    // Small delay to prevent watchdog issues
    delay(5);
}