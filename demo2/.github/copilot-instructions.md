# EEZ Studio LVGL Project Guidelines

This is a **baseline EEZ Studio project** for exploring and learning EEZ Studio development with ESP32 CYD (Cheap Yellow Display). This project serves as a working template with all integration issues resolved - perfect for experimentation and learning.

## Project Purpose

**Learning Baseline**: This project demonstrates a fully working EEZ Studio + LVGL + ESP32 CYD integration with all common issues resolved:
- ✅ LVGL 9.x API compatibility 
- ✅ ESP32 CYD hardware configuration
- ✅ Color format correction (RGB565 with proper channel order)
- ✅ Display timing and refresh handling
- ✅ TFT_eSPI driver integration

Use this as your starting point for EEZ Studio experimentation!

## Architecture Overview

- **EEZ Studio Project**: `demo1.eez-project` contains the visual UI design and build configuration
- **Generated UI Code**: All files in `src/ui/` are auto-generated from the .eez-project file
- **Platform**: ESP32 with Arduino framework + LVGL 9.3 graphics library
- **Display**: ESP32 CYD - 320x240 ILI9341 display with RGB color format
- **Display Driver**: TFT_eSPI library with hardware-specific configuration

## Code Generation Pattern

EEZ Studio uses template-based code generation with special markers:
```c
//${eez-studio MARKER_NAME}  // Content is injected here during build
```

Key markers in templates:
- `LVGL_SCREENS_DECL/DEF` - Screen declarations and definitions
- `LVGL_ACTIONS_DECL` - Event handler declarations  
- `LVGL_VARS_DECL` - Variable declarations
- `LVGL_STYLES_DECL/DEF` - Style declarations and definitions
- `LVGL_IMAGES_DECL/DEF` - Image asset declarations and definitions

## Critical DO NOTs

❌ **Never manually edit files in `src/ui/`** - They are regenerated on every build
❌ **Don't modify generated headers** (`screens.h`, `styles.h`, `images.h`, etc.)
❌ **Don't change the .eez-project file structure** without understanding the JSON schema

## Development Workflow

### Making UI Changes
1. Open `demo1.eez-project` in EEZ Studio
2. Design UI visually in the studio
3. Build project to regenerate `src/ui/` files
4. Include generated files in your main application

### Main Application Integration
- Call `ui_init()` in setup to initialize the UI system
- Call `ui_tick()` in main loop for UI updates
- Use `loadScreen(SCREEN_ID_*)` to switch between screens

### Screen Management
Generated screens follow this pattern:
```c
// In screens.h
enum ScreensEnum { SCREEN_ID_MAIN = 1 };
objects_t objects;  // Global UI object references

// In screens.c  
void create_screen_main();    // Creates screen objects
void tick_screen_main();     // Per-frame updates
```

## Build System

- **PlatformIO** project with `platformio.ini` configuration
- **Dependencies**: LVGL 9.3.0, TFT_eSPI 2.5.43
- **Target**: ESP32 with huge_app.csv partition scheme
- **Build Commands**: `pio run` (build), `pio run -t upload` (flash)

## Hardware Configuration (ESP32 CYD)

### TFT_eSPI Configuration
- **Method**: Include file approach using `include/Setup_ESP32_2432S028R_ILI9341.h`
- **Display**: ILI9341 320x240 with standard ESP32 SPI pins
- **Configuration**: Hardware-specific setup file included via `-include` directive
- **Color Format**: RGB (set in EEZ Studio project settings)

### Key Settings in platformio.ini:
```ini
build_flags =
    -D USER_SETUP_LOADED=1
    -include include/Setup_ESP32_2432S028R_ILI9341.h
```

### LVGL Integration:
- **Timer**: Must call `lv_tick_inc(5)` in main loop for proper timing
- **Display Buffer**: Uses partial rendering with 10-line buffer
- **Flush Callback**: Implements TFT_eSPI integration via `my_disp_flush()`

## File Structure Rules

```
demo1.eez-project          # EEZ Studio project file (edit in studio)
platformio.ini             # PlatformIO config (editable)
include/
  lv_conf.h                # LVGL configuration (editable)
  Setup_ESP32_2432S028R_ILI9341.h  # TFT_eSPI hardware config (editable)
src/
  main.cpp                 # Your application code (editable)
  ui/                      # Generated UI code (DO NOT EDIT)
    *.h, *.c              # All auto-generated from .eez-project
```

## Display Configuration Notes

### Color Format Considerations:
- **EEZ Studio**: Set colorFormat to "RGB" in project settings
- **LVGL**: May need `LV_COLOR_16_SWAP` adjustment in `lv_conf.h`
- **TFT_eSPI**: Hardware-specific color order in setup file

### Resolution Settings:
- **EEZ Studio**: Configure for 320x240 in project displayWidth/displayHeight
- **LVGL**: Screen objects auto-generated with correct dimensions
- **TFT_eSPI**: Hardware setup file defines TFT_WIDTH/TFT_HEIGHT

## Integration Points

### Adding Custom Logic
- Modify `main.cpp` to integrate UI with your application
- Use generated screen objects via `objects.main` etc.
- Handle UI events through generated action functions

### Working with LVGL Objects
Generated UI creates LVGL objects accessible via:
```c
extern objects_t objects;
lv_obj_t *main_screen = objects.main;
```

## Debugging

- Use PlatformIO Serial Monitor (`pio device monitor`)
- Monitor speed: 115200 baud (configured in platformio.ini)
- LVGL debug output available through standard LVGL configuration

## Critical LVGL Integration Requirements

### Main Loop Structure:
```c
void loop() {
    lv_tick_inc(5);       // CRITICAL: Tell LVGL time has passed
    lv_timer_handler();   // Process LVGL tasks
    ui_tick();           // Update EEZ Studio UI
    delay(5);            // Small delay for timing
}
```

### Display Initialization (LVGL 9.x):
```c
lv_display_t *display = lv_display_create(320, 240);
lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
lv_display_set_flush_cb(display, my_disp_flush);
```

## Color Configuration (SOLVED)

This baseline project has **color format issues resolved**:

### Working Configuration:
- **TFT_eSPI**: `TFT_RGB_ORDER TFT_RGB` (in Setup_ESP32_2432S028R_ILI9341.h)
- **LVGL**: `LV_COLOR_16_SWAP 0` (in lv_conf.h)
- **EEZ Studio**: `colorFormat: "RGB"` (in .eez-project)

### Expected Results:
- Colors display correctly (blue appears blue, not yellow/green)
- 16-bit RGB565 color depth provides 65K colors (vs 16M on monitor)
- Slight color differences from EEZ Studio preview are normal

### Color Troubleshooting Notes:
If colors appear wrong, check these combinations:
1. TFT_eSPI: TFT_RGB vs TFT_BGR 
2. LVGL: LV_COLOR_16_SWAP 0 vs 1
3. EEZ Studio: RGB vs BGR colorFormat

## Two-Mode Architecture

The generated code supports two modes:
- **Standalone LVGL** (`#if !defined(EEZ_FOR_LVGL)`) - Direct LVGL integration
- **EEZ Framework** (`#if defined(EEZ_FOR_LVGL)`) - Full EEZ runtime with Flow support

This project uses standalone LVGL mode.