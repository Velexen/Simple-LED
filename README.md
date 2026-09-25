# ESP-LED

A custom ESP32-based LED controller that integrates with Home Assistant over Wi-Fi. The device controls two independent LED strips and exposes them as separate light entities in Home Assistant, allowing full smart-home control.

## Purpose

The goal of this project is to build a DIY smart LED controller from scratch — including custom PCB hardware, firmware, and home automation integration — without relying on off-the-shelf solutions.

## Features

- Controls two independent LED strips
- Wi-Fi connectivity for remote control
- Home Assistant integration (appears as two distinct light entities)
- Main power switch and a strip-selector switch
- Custom 4-layer PCB design (schematics in [schematics/](schematics/))

## Hardware

The PCB was designed in Altium Designer. Design files are located in [schematics/ESP-LED/](schematics/ESP-LED/). A pinout reference sheet is available at [schematics/Pinout.xlsx](schematics/Pinout.xlsx).

Key components:
- ESP32 microcontroller
- MOSFETs for LED switching (see `MosFets.SchDoc`)
- 5V/12V power input (see `Power.SchDoc`)

## Firmware

Built with **ESP-IDF** (Espressif IoT Development Framework). Source code lives in [src/](src/).

A code rework is currently in work under the branch "code-redo"

## Getting Started

1. Install [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)
2. Clone this repo
3. Set your Wi-Fi credentials in `src/main/wifi.c`
4. Build and flash:
   ```
   idf.py build flash monitor
   ```
