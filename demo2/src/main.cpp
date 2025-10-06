#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
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
static lv_color_t buf1[screenWidth * 40]; // 40 lines buffer for faster refresh
static lv_color_t buf2[screenWidth * 40]; // Second buffer for double buffering

TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSpi = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240, touchScreenMaximumY = 3800;

// LVGL log callback
void my_log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Display flushing callback - TFT_eSPI implementation
void my_disp_flush(lv_display_t *display, const lv_area_t *area, uint8_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushPixels((uint16_t *)color_p, w * h);
    tft.endWrite();

    lv_display_flush_ready(display); // Tell LVGL you are ready with the flushing
}

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    //Some very basic auto calibration so it doesn't go out of range
    if (p.x < touchScreenMinimumX) touchScreenMinimumX = p.x;
    if (p.x > touchScreenMaximumX) touchScreenMaximumX = p.x;
    if (p.y < touchScreenMinimumY) touchScreenMinimumY = p.y;
    if (p.y > touchScreenMaximumY) touchScreenMaximumY = p.y;
    //Map this to the pixel position
    data->point.x = map(p.x, touchScreenMinimumX, touchScreenMaximumX, 1, screenWidth); /* Touchscreen X calibration */
    data->point.y = map(p.y, touchScreenMinimumY, touchScreenMaximumY, 1, screenHeight); /* Touchscreen Y calibration */
    data->state = LV_INDEV_STATE_PRESSED;

    Serial.print("Touch x ");
    Serial.print(data->point.x);
    Serial.print(" y ");
    Serial.println(data->point.y);
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
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

    //Initialize the touchscreen
    touchscreenSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); // Start second SPI bus for touchscreen
    touchscreen.begin(touchscreenSpi);                                         // Touchscreen init
    touchscreen.setRotation(1);                                                // Inverted landscape orientation to match screen

    // Initialize LVGL
    lv_init();
    lv_log_register_print_cb(my_log_print);
    
    // Create display (LVGL 9.x API)
    lv_display_t *display = lv_display_create(screenWidth, screenHeight);
    
    // Set display buffer with double buffering for smoother refresh
    lv_display_set_buffers(display, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    // Set display flush callback
    lv_display_set_flush_cb(display, my_disp_flush);
        
    // Set up touch input device
    lv_indev_t *indev_touchpad = lv_indev_create();
    lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_touchpad, my_touchpad_read);

    // Initialize EEZ Studio generated UI
    ui_init();
    
    Serial.println("UI initialized and ready!");
    
    // Force initial screen refresh
    lv_refr_now(display);
    
    Serial.println("Setup complete!");
}

void loop() {
    // CRITICAL: Tell LVGL how much time has passed
    lv_tick_inc(3); // 3ms matches our delay below for faster refresh
    
    // Handle LVGL tasks
    lv_timer_handler();
    
    // Handle EEZ Studio UI updates
    ui_tick();
    
    // Small delay to prevent watchdog issues - reduced for faster refresh
    delay(3);
}