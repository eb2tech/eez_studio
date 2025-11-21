#include <Arduino.h>
#include <lvgl.h>
#include <DEV_Config.h>
#include <EPD.h>
#include <GUI_Paint.h>

// Display configuration
static const uint16_t screenWidth = EPD_7IN5_V2_WIDTH;   // 800
static const uint16_t screenHeight = EPD_7IN5_V2_HEIGHT; // 480

// Waveshare display buffers - keep these global
UBYTE *BlackImage;
UWORD Imagesize;

// LVGL draw buffer
static lv_color_t *buf1 = nullptr;
static const size_t buffer_pixels = screenWidth * 20; // 20 rows buffer

// LVGL log callback
void log_print(lv_log_level_t level, const char *buf)
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}

// Display update control
static bool display_needs_update = false;
static unsigned long last_display_update = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 3000; // 3 seconds

// LVGL flush callback - integrates with proven Waveshare library
void display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    Serial.printf("LVGL flush: x1:%d y1:%d x2:%d y2:%d\n", area->x1, area->y1, area->x2, area->y2);

    int16_t width = area->x2 - area->x1 + 1;
    int16_t height = area->y2 - area->y1 + 1;

    if (width <= 0 || height <= 0 || area->x1 < 0 || area->y1 < 0) {
        Serial.println("Invalid area dimensions!");
        lv_display_flush_ready(disp);
        return;
    }

    // Add debugging for pixel data
    Serial.printf("Processing %d x %d pixels\n", width, height);

    // Select the BlackImage buffer (same one used in working code)
    Paint_SelectImage(BlackImage);

    // Draw LVGL pixels to Waveshare buffer using proven Paint functions
    auto px_ptr = px_map;
    for (int16_t y = 0; y < height; y++) {
        for (int16_t x = 0; x < width; x++) {
            // Handle anti-aliased pixels with strict threshold for crisp text
            uint8_t lvgl_pixel = *px_ptr++;
            // Use 200 threshold instead of 128 to make anti-aliased edges render as black
            UWORD color = (lvgl_pixel < 200) ? BLACK : WHITE;
            Paint_SetPixel(area->x1 + x, area->y1 + y, color);
        }
    }

    display_needs_update = true;
    lv_display_flush_ready(disp);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("EPD_7IN5_V2 + LVGL Integration Demo");
    
    // Initialize e-paper display (proven working code)
    DEV_Module_Init();
    Serial.println("e-Paper Init and Clear...");
    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Clear();
    delay(500);

    // Create image cache (same as working code)
    Imagesize = ((EPD_7IN5_V2_WIDTH % 8 == 0) ? (EPD_7IN5_V2_WIDTH / 8) : (EPD_7IN5_V2_WIDTH / 8 + 1)) * EPD_7IN5_V2_HEIGHT;
    if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        Serial.println("Failed to apply for black memory...");
        while (1);
    }
    
    Serial.println("Paint_NewImage");
    Paint_NewImage(BlackImage, EPD_7IN5_V2_WIDTH, EPD_7IN5_V2_HEIGHT, 0, WHITE);

    // Initialize LVGL
    Serial.println("Initializing LVGL...");
    lv_init();
    lv_log_register_print_cb(log_print);

    // Allocate LVGL buffer
    size_t buffer_size_bytes = buffer_pixels * sizeof(uint8_t);
    buf1 = (lv_color_t *)lv_malloc(buffer_size_bytes);

    if (!buf1) {
        Serial.println("Failed to allocate LVGL buffer!");
        return;
    }

    Serial.printf("LVGL buffer allocated: 0x%x, size: %d bytes\n", (uint32_t)buf1, buffer_size_bytes);

    // Create LVGL display object
    lv_display_t *lvDisp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_color_format(lvDisp, LV_COLOR_FORMAT_L8); // Monochrome
    lv_display_set_flush_cb(lvDisp, display_flush_cb);
    lv_display_set_buffers(lvDisp, buf1, NULL, buffer_size_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Create simple LVGL test widgets
    lv_obj_t *screen = lv_screen_active();
    
    // Set screen background to white (important for e-paper)
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);

    // Title label - use default font first
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "LVGL + Waveshare");
    // Try default font first
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_set_style_text_opa(title, LV_OPA_COVER, 0);  // Disable transparency
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // Content label - simplified
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "Hello LVGL!");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);  // Disable transparency
    lv_obj_center(label);

    // Simple button with explicit colors
    lv_obj_t *btn = lv_btn_create(screen);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_bg_color(btn, lv_color_white(), 0);
    lv_obj_set_style_border_color(btn, lv_color_black(), 0);
    lv_obj_set_style_border_width(btn, 2, 0);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Click Me");
    lv_obj_set_style_text_color(btn_label, lv_color_black(), 0);
    lv_obj_set_style_text_opa(btn_label, LV_OPA_COVER, 0);  // Disable transparency
    lv_obj_center(btn_label);

    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println("LVGL setup complete - widgets should now be visible");
}

void loop()
{
    // Tell LVGL how much time has passed
    lv_tick_inc(100);

    // Handle LVGL tasks (this will call display_flush_cb when needed)
    lv_timer_handler();

    // Periodic display update using proven Waveshare library
    if (display_needs_update && (millis() - last_display_update > DISPLAY_UPDATE_INTERVAL)) {
        Serial.println("Updating e-paper display via LVGL integration...");
        
        // Use the same proven display function from working code
        EPD_7IN5_V2_Display(BlackImage);
        
        display_needs_update = false;
        last_display_update = millis();
        Serial.println("Display update complete");
    }

    delay(100);
}
