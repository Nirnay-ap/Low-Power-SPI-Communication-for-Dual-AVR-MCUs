AVR Bare Metal SPI Communication Project
Overview
This project implements ultra-low-power communication between two AVR DD microcontrollers using SPI protocol. The HOST device reads an ADC sensor and transmits data to the CLIENT device, which outputs the results via USART.
Key Features:

Ultra-low power consumption (~1.5µA sleep current)
State machine-based architecture
Dynamic clock switching (32.768 kHz ↔ 4 MHz)
Window comparison ADC
Interrupt-driven SPI communication


Hardware Requirements
Both Devices:

2x AVR DD Curiosity Nano Development Boards
USB cables for programming and power
Logic analyzer (optional, for debugging)

HOST Device Additional:

Analog sensor (connected to PF2)
Button switch (built-in on PF6)

Wiring Diagram:
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

Software Architecture
HOST State Machine:

INIT → Initialize peripherals
SLEEP → Power-down mode (~1.5µA)
SWITCH_TO_HIGHSPEED → 4 MHz clock
READ_ADC → Sample sensor with window comparison
SEND_SPI → Transmit 2 bytes to CLIENT
SWITCH_TO_LOWPOWER → 32.768 kHz clock
SLEEP → Return to power-down

CLIENT State Machine:

INIT → Initialize peripherals
SLEEP → Power-down mode (~2µA)
SWITCH_TO_HIGHSPEED → 4 MHz clock
RECEIVE_SPI → Collect 2 bytes from HOST
SWITCH_TO_LOWPOWER → 32.768 kHz clock
WRITE_TO_USART → Output formatted data
SLEEP → Return to power-down


SPI Data Packet Format
2-byte packet structure:
Byte 1 (High):  [W][A11][A10][A9][A8][A7][A6][A5]
Byte 0 (Low):   [A4][A3][A2][A1][A0][X][X][X]

Where:
  W   = Window comparison result (1=satisfied, 0=not satisfied)
  A11-A0 = 12-bit ADC result
  X   = Unused bits

Power Consumption
DeviceStateCurrentClockHOSTSleep~1.5µA32.768 kHzHOSTADC~160µA4 MHzHOSTSPI TX~1.3mA4 MHzCLIENTSleep~2µA32.768 kHzCLIENTSPI RX + USART~1.1mA4 MHz

Building the Project
Using MPLAB X IDE:

Create two separate projects (HOST and CLIENT)
Add source and header files to each project
Set device to AVR128DD32 (or your specific AVR DD variant)
Configure compiler:

HOST: Add -DHOST_DEVICE to compiler flags
CLIENT: Add -DCLIENT_DEVICE to compiler flags


Build and program each device

Using Command Line (avr-gcc):
For HOST:
bashavr-gcc -mmcu=avr128dd32 -DF_CPU=32768UL -DHOST_DEVICE \
  -Os -Wall -o host.elf \
  host_main.c ports.c spi0.c adc.c main_clock_control.c \
  sleep.c usart0_tx.c

avr-objcopy -O ihex -R .eeprom host.elf host.hex
avrdude -c atmelice_updi -p avr128dd32 -U flash:w:host.hex
For CLIENT:
bashavr-gcc -mmcu=avr128dd32 -DF_CPU=32768UL -DCLIENT_DEVICE \
  -Os -Wall -o client.elf \
  client_main.c ports.c spi0.c main_clock_control.c \
  sleep.c usart0_tx.c

avr-objcopy -O ihex -R .eeprom client.elf client.hex
avrdude -c atmelice_updi -p avr128dd32 -U flash:w:client.hex

Testing

Program both devices with their respective firmware
Connect USART from CLIENT to terminal (1200 baud, 8N1)
Power both devices (3.3V recommended)
Press button on HOST device
Observe output on CLIENT's serial terminal

Expected Output:
SPI Byte[1]: 0x8A
SPI Byte[0]: 0xBC
Results: 0x8ABC
Window: 1
ADC: 2748

Troubleshooting
Problem: CLIENT not receiving data

Check wiring: Verify SPI connections (especially GND)
Check SPI clock: Should be 250 kHz (not 1 MHz)
Add delay: Increase spin_lock(4) delay on HOST

Problem: High sleep current

Disable unused peripherals: Ensure ADC/SPI disabled before sleep
Check floating pins: All unused pins should have pull-ups enabled
Disconnect debugger: Logic analyzers add significant current

Problem: USART not working

Check baud rate: Must match 1200 baud
Check terminal settings: 8 data bits, no parity, 1 stop bit
Verify F_CPU: Must be defined correctly for USART calculations


Key Gotchas (From Video)
1. F_CPU Redefinition Issues
The compiler may optimize out multiple F_CPU definitions when switching clocks. Solution: Use spin_lock() function instead of _delay_ms().
2. SPI Clock Too Fast
Using 1 MHz SPI clock (from episode 13) doesn't give CLIENT enough time to process interrupts. Solution: Reduced to 250 kHz (prescaler /16).
3. Window Flag Cleared on Read
Reading ADC result clears the window comparison flag. Solution: Always check adc_is_window_satisfied() BEFORE calling adc_get_result().

File Structure
project/
├── host/
│   ├── main.c              (HOST state machine)
│   ├── ports.c/h           (GPIO + button interrupt)
│   ├── spi0.c/h            (SPI host mode)
│   ├── adc.c/h             (ADC with window compare)
│   ├── main_clock_control.c/h
│   ├── sleep.c/h
│   └── usart0_tx.c/h       (optional for debugging)
│
└── client/
    ├── main.c              (CLIENT state machine)
    ├── ports.c/h           (GPIO + SS interrupt)
    ├── spi0.c/h            (SPI client mode)
    ├── main_clock_control.c/h
    ├── sleep.c/h
    └── usart0_tx.c/h       (serial output)
