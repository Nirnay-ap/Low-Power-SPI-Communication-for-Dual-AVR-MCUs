/*
 * AVR Bare Metal Series - Episode 14/15
 * HOST Device Implementation
 * 
 * Project: Low-power SPI communication with ADC reading
 * Hardware: AVR DD Curiosity Nano
 */

#define F_CPU 32768UL  // 32.768 KHz for most of the time

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

// Include custom libraries
#include "main_clock_control.h"
#include "usart0_tx.h"
#include "spi0.h"
#include "adc.h"
#include "ports.h"
#include "sleep.h"

// SPI Configuration
#define NUM_SPI_BYTES 2  // Number of bytes to send over SPI

// Window comparison results
#define WINDOW_SATISFIED     (1 << 7)  // Bit 7 = 1
#define WINDOW_NOT_SATISFIED (0 << 7)  // Bit 7 = 0

// SPI data array
uint8_t spi_data[NUM_SPI_BYTES];

// Spin lock function for delays (works with any clock speed)
void spin_lock(uint16_t time_ms) {
    // Convert ms to us and apply scaling factor
    // Scaling accounts for NOP execution time and loop overhead
    uint32_t loops = (uint32_t)time_ms * 1000UL / 10UL;
    
    for(uint32_t i = 0; i < loops; i++) {
        __asm__ __volatile__("nop");
    }
}

// State Machine Type Definition
typedef enum {
    STATE_INIT,
    STATE_SLEEP,
    STATE_READ_ADC,
    STATE_SWITCH_TO_HIGHSPEED_CLOCK,
    STATE_SEND_SPI,
    STATE_SWITCH_TO_LOWPOWER_CLOCK
} app_states_t;

// Application data structure
typedef struct {
    app_states_t state;
} app_data_t;

int main(void) {
    // Create state machine instance
    app_data_t app_data;
    app_data.state = STATE_INIT;
    
    // Main state machine loop
    while(1) {
        switch(app_data.state) {
            
            case STATE_INIT:
                // Setup low power 32.768 KHz clock
                low_power_clock_control(
                    0x01,  // Internal 32.768 KHz oscillator
                    0,     // Prescaler enable = 0
                    0x00,  // Prescaler divider (ignored)
                    0,     // Run standby = 0
                    0      // Clock out = 0
                );
                
                // Initialize ports
                port_init();
                
                // Initialize ADC
                adc_init(
                    0x00,  // VREF = VDD
                    0,     // Always on disabled
                    0,     // Run standby disabled
                    0x00,  // Single 12-bit conversion
                    0,     // Right adjust
                    0,     // Free running disabled
                    0x01,  // Init delay 16 ADC clocks
                    0,     // No sample accumulation
                    0x00,  // ADC clock prescaler /2
                    0,     // Sample delay = 0
                    2,     // Sample length = 2 ADC clocks
                    0x08,  // Positive channel AIN8 (PF2)
                    0x40,  // Negative channel = GND
                    0x03,  // Window mode = INSIDE
                    1000,  // Lower threshold
                    3000,  // Upper threshold
                    0      // Enable = done later
                );
                
                // Initialize USART (1200 baud)
                usart_init(
                    0x00,  // Async mode
                    0x00,  // No parity
                    0x00,  // 1 stop bit
                    0x00,  // 8 data bits
                    1200   // Baud rate
                );
                
                // Initialize sleep controller (power down mode)
                sleep_init(0x04, 0x01);  // Power down + sleep enable
                
                // Enable global interrupts
                sei();
                
                // Move to sleep state
                app_data.state = STATE_SLEEP;
                break;
                
            case STATE_SLEEP:
                // Put CPU to sleep
                __asm__ __volatile__("sleep");
                
                // Wake on button press (pin change interrupt)
                if(get_button_pressed_status()) {
                    clear_button_pressed_status();
                    app_data.state = STATE_SWITCH_TO_HIGHSPEED_CLOCK;
                }
                break;
                
            case STATE_SWITCH_TO_HIGHSPEED_CLOCK:
                // Switch to 4 MHz for faster processing
                main_clock_control(
                    0x00,  // OSCHF (high freq internal oscillator)
                    0x02,  // 4 MHz
                    0,     // Prescaler disabled
                    0x00,  // Prescaler div (ignored)
                    0      // Clock out disabled
                );
                
                // Wait for oscillator to stabilize
                while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_OSCHFS_bm));
                
                app_data.state = STATE_READ_ADC;
                break;
                
            case STATE_READ_ADC:
                // Enable power rails for ADC (PC3=HIGH, PC2=LOW)
                adc_enable_power_rails_before_conversion();
                
                // Enable ADC
                adc_enable();
                
                // Delay for VREF stabilization (1ms)
                _delay_ms(1);
                
                // Start ADC conversion
                adc_start_conversion();
                
                // Wait for conversion to complete
                while(!adc_is_conversion_done());
                
                // Clear SPI data array
                for(uint8_t i = 0; i < NUM_SPI_BYTES; i++) {
                    spi_data[i] = 0;
                }
                
                // Check window comparison BEFORE reading result
                // (reading result clears the window flag)
                if(adc_is_window_satisfied()) {
                    spi_data[1] = WINDOW_SATISFIED;
                } else {
                    spi_data[1] = WINDOW_NOT_SATISFIED;
                }
                
                // Read ADC result
                uint16_t adc_result = adc_get_result();
                
                // Pack data into SPI bytes
                spi_data[0] = (uint8_t)(adc_result & 0xFF);        // Low byte
                spi_data[1] |= (uint8_t)((adc_result >> 8) & 0x0F); // High nibble + window bit
                
                // Disable ADC and power rails
                adc_disable();
                adc_disable_power_rails_after_conversion();
                
                app_data.state = STATE_SEND_SPI;
                break;
                
            case STATE_SEND_SPI:
                // Initialize SPI as host
                spi_host_init();
                
                // Select client (pull SS low)
                spi_select_client();
                
                // Delay to allow client to wake and be ready
                spin_lock(4);  // 4ms delay
                
                // Send SPI data block
                spi0_write_block(spi_data, NUM_SPI_BYTES);
                
                // Deselect client (pull SS high)
                spi_deselect_client();
                
                // Disable SPI
                spi_disable();
                
                // Disable SPI pins to save power
                spi_disable_pins();
                
                app_data.state = STATE_SWITCH_TO_LOWPOWER_CLOCK;
                break;
                
            case STATE_SWITCH_TO_LOWPOWER_CLOCK:
                // Switch back to 32.768 KHz for low power
                low_power_clock_control(
                    0x01,  // Internal 32.768 KHz
                    0,     // Prescaler disabled
                    0x00,  // Prescaler div (ignored)
                    0,     // Run standby disabled
                    0      // Clock out disabled
                );
                
                // Wait for oscillator to stabilize
                while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_OSC32KS_bm));
                
                // Disable unused pins before sleep
                turn_off_unused_pins_before_sleep();
                
                app_data.state = STATE_SLEEP;
                break;
        }
    }
    
    return 0;
}
