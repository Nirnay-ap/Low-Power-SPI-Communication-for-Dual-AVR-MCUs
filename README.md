✨ AVR Bare-Metal SPI Communication Project

Ultra-Low-Power Host–Client Communication using AVR DD Series

🚀 Overview

This project implements ultra-low-power SPI communication between two AVR DD microcontrollers.
The HOST reads ADC sensor data and sends it to the CLIENT, which prints the result via USART.

🔧 Features

⚡ Ultra-low power (~1.5 µA sleep current)

🧠 State-machine architecture

🔄 Dynamic clock switching (32.768 kHz ↔ 4 MHz)

🎯 ADC Window Comparison

🔗 Interrupt-driven SPI communication

🛠 Hardware Requirements
📌 Boards

2× AVR DD Curiosity Nano

USB cables

Optional: Logic analyzer

📌 HOST Extras

Analog sensor → PF2

Built-in button → PF6

📡 Wiring Diagram
HOST (PA4-PA7)  ←→  CLIENT (PA4-PA7)
    PA4 (MOSI)  →   PA4 (MOSI)
    PA5 (MISO)  ←   PA5 (MISO)
    PA6 (SCK)   →   PA6 (SCK)
    PA7 (SS)    →   PA7 (SS)
    GND         ←→  GND

HOST Sensor:
    PC3         →   Sensor VCC
    PC2         →   Sensor GND
    PF2         ←   Sensor Analog Out

🧩 Software Architecture
🟦 HOST State Machine

INIT

SLEEP

SWITCH_TO_HIGHSPEED

READ_ADC

SEND_SPI

SWITCH_TO_LOWPOWER

SLEEP

🟩 CLIENT State Machine

INIT

SLEEP

SWITCH_TO_HIGHSPEED

RECEIVE_SPI

SWITCH_TO_LOWPOWER

WRITE_TO_USART

SLEEP

📦 SPI Packet Format
Byte 1: [W][A11][A10][A9][A8][A7][A6][A5]
Byte 0: [A4][A3][A2][A1][A0][X][X][X]


W = Window Compare Flag

A11–A0 = ADC Value

X = Reserved

🔋 Power Consumption Summary
Device	State	Current	Clock
HOST	Sleep	1.5 µA	32.768 kHz
HOST	ADC	160 µA	4 MHz
HOST	SPI TX	1.3 mA	4 MHz
CLIENT	Sleep	2 µA	32.768 kHz
CLIENT	SPI RX + USART	1.1 mA	4 MHz
🧱 Building the Project
📌 Using MPLAB X

Create two projects: HOST & CLIENT

Add source files

Set device: AVR128DD32

Compiler flags:

HOST → -DHOST_DEVICE

CLIENT → -DCLIENT_DEVICE

📌 Using avr-gcc (Command Line)

HOST

avr-gcc -mmcu=avr128dd32 -DF_CPU=32768UL -DHOST_DEVICE \
  -Os -Wall -o host.elf \
  host_main.c ports.c spi0.c adc.c main_clock_control.c \
  sleep.c usart0_tx.c


CLIENT

avr-gcc -mmcu=avr128dd32 -DF_CPU=32768UL -DCLIENT_DEVICE \
  -Os -Wall -o client.elf \
  client_main.c ports.c spi0.c main_clock_control.c \
  sleep.c usart0_tx.c

🧪 Testing Procedure

Flash both devices

Open serial monitor on CLIENT (1200 baud, 8N1)

Power devices

Press button on HOST

Observe output

Example Output
SPI Byte[1]: 0x8A
SPI Byte[0]: 0xBC
Results: 0x8ABC
Window: 1
ADC: 2748

🐞 Troubleshooting Tips
❌ CLIENT not receiving data

Check SPI wiring

Lower SPI speed (250 kHz)

Add delay (spin_lock(4))

❌ High sleep current

Disable unused peripherals

Enable pull-ups on unused pins

Disconnect debugger

❌ USART issues

Check baud rate (1200)

Verify F_CPU definitions

Ensure correct terminal settings

⚠️ Common Gotchas

F_CPU redefine removed by optimizer → use spin_lock()

1 MHz SPI too fast → use 250 kHz

ADC window flag clears on read → check before reading

📂 File Structure
project/
├── host/
│   ├── main.c
│   ├── ports.c/h
│   ├── spi0.c/h
│   ├── adc.c/h
│   ├── main_clock_control.c/h
│   ├── sleep.c/h
│   └── usart0_tx.c/h
│
└── client/
    ├── main.c
    ├── ports.c/h
    ├── spi0.c/h
    ├── main_clock_control.c/h
    ├── sleep.c/h
    └── usart0_tx.c/h
