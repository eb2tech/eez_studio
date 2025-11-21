# ESP32 E-Paper Display Project Instructions

## PlatformIO Library Management

### Automatic Library Inclusion
- **PlatformIO automatically includes libraries from the `./lib` folder**
- No need to manually add `-I lib/waveshare` or `-I lib/waveshare/utility` to build_flags
- Libraries placed in `./lib` are automatically discovered and their include paths added
- This applies to both header files and source files in subdirectories

### Current Library Structure
```
lib/
└── waveshare/
    ├── DEV_Config.cpp
    ├── DEV_Config.h
    ├── EPD.h
    ├── GUI_Paint.cpp
    ├── GUI_Paint.h
    └── utility/
        ├── EPD_7in5_V2.cpp
        ├── EPD_7in5_V2.h
        └── [other display drivers...]
```

### Usage in Code
- Include files directly: `#include "DEV_Config.h"`
- Include utility files: `#include "utility/EPD_7in5_V2.h"`
- No path prefix needed since PlatformIO handles the include paths automatically

## LVGL Integration Notes

### Anti-Aliasing Configuration
- E-paper displays require crisp, non-anti-aliased text
- Build flags include:
  - `-D LV_FONT_SUBPX=0` - Disable subpixel rendering
  - `-D LV_ANTIALIAS=0` - Disable anti-aliasing
- Use `LV_OPA_COVER` style for solid text rendering
- Pixel threshold of 200 (instead of 128) captures anti-aliased edges as black

### Display Integration
- Uses native Waveshare library for proven hardware compatibility
- LVGL flush callback draws to Waveshare's `BlackImage` buffer
- Periodic display updates using `EPD_7IN5_V2_Display(BlackImage)`

## Hardware Configuration
- **Display**: Waveshare 7.5" e-Paper HAT (B) - EPD_7IN5_V2 (Black/White/Red capable)
- **Driver Board**: Waveshare ESP32 e-Paper Driver Board Rev 3
- **Pin Mapping**: CS=5, DC=17, RST=16, BUSY=4

## Build Configuration
- Platform: ESP32 (espressif32)
- Framework: Arduino
- Partition Scheme: huge_app.csv (for larger applications)
- LVGL Version: 9.3.0 (stable version, avoid 9.4 due to compatibility issues)