
# ⚡AVR Bare Metal SPI Communication Project

Low-power,bare-metal AVR DD project enabling seamless SPI communication between two microcontrollers with ADC sensing and USART output

---

## 📌 Table of Contents
- <a href="#overview">Overview</a>
- <a href="#Key features">Key features</a>
- <a href="#Hardware requirements">Hardware requirements</a>
- <a href="#Wiring diagram">Wiring diagram</a>
- <a href="#Software Architecture">Software Architecture</a>
- <a href="#SPI data packet format">SPI data packet format</a>
- <a href="#Power consumption">Power consumption</a>
- <a href="#Building the project">Building the project</a>
- <a href="#Testing">Testing</a>
- <a href="#Troubleshoot">Troubleshoot</a>
- <a href="#File structure">File structure</a>


---
<h2><a class="anchor" id="overview"></a>Overview</h2>

This project implements low-power communication between two AVR DD microcontrollers using SPI protocol. The HOST device reads an ADC sensor and transmits data to the CLIENT device, which outputs the results via USART.

<h2><a class="anchor" id="Key features"></a>Key features</h2>


```
1.Ultra-low power consumption (~1.5µA sleep current)
2.State machine-based architecture
3.Dynamic clock switching (32.768 kHz ↔ 4 MHz)
4.Window comparison ADC
5.Interrupt-driven SPI communication
```

---
<h2><a class="anchor" id="Hardware requirements"></a>Hardware requirements</h2>

Both Devices:

-2x AVR DD Curiosity Nano Development Boards
-USB cables for programming and power
-Logic analyzer (optional, for debugging)

HOST Device Additional:

-Analog sensor (connected to PF2)
-Button switch (built-in on PF6)
---

<h2><a class="anchor" id="Wiring diagram"></a>Wiring diagram</h2>

```
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
```


---
<h2><a class="anchor" id="Software Architecture"></a>Software Architecture</h2>

HOST State Machine:

1.INIT → Initialize peripherals
2.SLEEP → Power-down mode (~1.5µA)
3.SWITCH_TO_HIGHSPEED → 4 MHz clock
4.READ_ADC → Sample sensor with window comparison
5.SEND_SPI → Transmit 2 bytes to CLIENT
6.SWITCH_TO_LOWPOWER → 32.768 kHz clock
7.SLEEP → Return to power-down

CLIENT State Machine:

1.INIT → Initialize peripherals
2.SLEEP → Power-down mode (~2µA)
3.SWITCH_TO_HIGHSPEED → 4 MHz clock
4.RECEIVE_SPI → Collect 2 bytes from HOST
5.SWITCH_TO_LOWPOWER → 32.768 kHz clock
6.WRITE_TO_USART → Output formatted data
7.SLEEP → Return to power-down

---
<h2><a class="anchor" id="SPI data packet format"></a>SPI data packet format</h2>

2-byte packet structure:

```
Byte 1 (High):  [W][A11][A10][A9][A8][A7][A6][A5]
Byte 0 (Low):   [A4][A3][A2][A1][A0][X][X][X]

Where:
  W   = Window comparison result (1=satisfied, 0=not satisfied)
  A11-A0 = 12-bit ADC result
  X   = Unused bits
```
---
<h2><a class="anchor" id="Power Consumption"></a>Power Consumption</h2>
## 

| Device | State            | Current | Clock      |
|--------|------------------|---------|------------|
| HOST   | Sleep            | ~1.5µA  | 32.768 kHz |
| HOST   | ADC              | ~160µA  | 4 MHz      |
| HOST   | SPI TX           | ~1.3mA  | 4 MHz      |
| CLIENT | Sleep            | ~2µA    | 32.768 kHz |
| CLIENT | SPI RX + USART   | ~1.1mA  | 4 MHz      |


---
<h2><a class="anchor" id="Building the project"></a>Building the project</h2>

Using MPLAB X IDE:

1.Create two separate projects (HOST and CLIENT)
2.Add source and header files to each project
3.Set device to AVR128DD32 (or your specific AVR DD variant)
4.Build and program each device
  Using Command Line (avr-gcc):
  For HOST:
  ```
  avr-gcc -mmcu=avr128dd32 -DF_CPU=32768UL -DHOST_DEVICE \
  -Os -Wall -o host.elf \
  host_main.c ports.c spi0.c adc.c main_clock_control.c \
  sleep.c usart0_tx.c

  avr-objcopy -O ihex -R .eeprom host.elf host.hex
  avrdude -c atmelice_updi -p avr128dd32 -U flash:w:host.hex

  ```
For CLIENT:
   ```
  avr-gcc -mmcu=avr128dd32 -DF_CPU=32768UL -DCLIENT_DEVICE \
  -Os -Wall -o client.elf \
  client_main.c ports.c spi0.c main_clock_control.c \
  sleep.c usart0_tx.c

  avr-objcopy -O ihex -R .eeprom client.elf client.hex
  avrdude -c atmelice_updi -p avr128dd32 -U flash:w:client.hex
   ```    
  ---
  
<h2><a class="Testing" id="dashboard"></a>Testing</h2>

1.Program both devices with their respective firmware
2.Connect USART from CLIENT to terminal (1200 baud, 8N1)
3.Power both devices (3.3V recommended)
4.Press button on HOST device
5.Observe output on CLIENT's serial terminal

![Vendor Performance Dashboard](<img width="1082" height="1020" alt="SPI_Rsult" src="https://github.com/user-attachments/assets/1482f12b-2a81-482e-88dc-9d7dc3edc14c" />
)

---
<h2><a class="anchor" id="Troubleshoot"></a>Troubleshoot</h2>

Problem: CLIENT not receiving data

1.Check wiring: Verify SPI connections (especially GND)
2.Check SPI clock: Should be 250 kHz (not 1 MHz)
3.Add delay: Increase spin_lock(4) delay on HOST

Problem: High sleep current

1.Disable unused peripherals: Ensure ADC/SPI disabled before sleep
2.Check floating pins: All unused pins should have pull-ups enabled
3.Disconnect debugger: Logic analyzers add significant current

Problem: USART not working

1.Check baud rate: Must match 1200 baud
2.Check terminal settings: 8 data bits, no parity, 1 stop bit
3.Verify F_CPU: Must be defined correctly for USART calculations

---
<h2><a class="anchor" id="File structure"></a>File structure</h2>

 ```
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
 ```  

---

