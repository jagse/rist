# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a PlatformIO-based embedded systems project targeting the **M5Stack StickC-Plus2** development board (ESP32-S3-based). The project uses the Arduino framework for development.

## Build and Development Commands

PlatformIO commands are used for all build operations:

- **Build the project**: `pio run` or `platformio run`
- **Upload to device**: `pio run --target upload`
- **Clean build files**: `pio run --target clean`
- **Run tests**: `pio test`
- **Monitor serial output**: `pio device monitor`
- **Build and upload**: `pio run --target upload && pio device monitor`

## Architecture

### Project Structure

- **src/main.cpp**: Main application entry point containing `setup()` and `loop()` functions following Arduino conventions
- **lib/**: Project-specific libraries (compiled to static libraries and linked)
  - Each library should be in its own subdirectory: `lib/LibraryName/`
- **include/**: Project header files (.h files)
- **test/**: Test files for PlatformIO unit testing
- **platformio.ini**: Build configuration and target specification

### Key Conventions

- **Arduino Framework**: Code follows Arduino conventions with `setup()` running once at startup and `loop()` running continuously
- **Target Device**: M5Stack StickC-Plus2 (ESP32-S3-based board with built-in display, buttons, IMU, and battery)
- **Platform**: espressif32 (ESP32 microcontrollers)
