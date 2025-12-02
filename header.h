#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

void port_init(void);

// HOST DEVICE functions
#ifdef HOST_DEVICE
uint8_t get_button_pressed_status(void);
void clear_button_pressed_status(void);
void turn_off_unused_pins_before_sleep(void);
#endif

// CLIENT DEVICE functions
#ifdef CLIENT_DEVICE
uint8_t get_client_select_flag_status(void);
void clear_client_select_flag(void);
#endif

#endif // PORTS_H


// ========================================
// spi0.h
// ========================================
#ifndef SPI0_H
#define SPI0_H

#include <stdint.h>

// HOST DEVICE functions
#ifdef HOST_DEVICE
void spi_host_init(void);
void spi_select_client(void);
void spi_deselect_client(void);
void spi0_write_block(uint8_t *data, uint8_t size);
void spi_disable(void);
void spi_disable_pins(void);
#endif

// CLIENT DEVICE functions
#ifdef CLIENT_DEVICE
void spi_client_init(void);
uint8_t get_packet_complete_status(void);
void clear_packet_complete_status(void);
#endif

#endif // SPI0_H


// ========================================
// adc.h
// ========================================
#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void adc_init(uint8_t vref, uint8_t always_on, uint8_t run_standby,
              uint8_t conv_mode, uint8_t left_adjust, uint8_t freerun,
              uint8_t init_delay, uint8_t sample_num, uint8_t presc,
              uint8_t sample_delay, uint8_t sample_len, uint8_t pos_ch,
              uint8_t neg_ch, uint8_t window_mode, uint16_t win_low,
              uint16_t win_high, uint8_t enable);

void adc_enable(void);
void adc_disable(void);
void adc_start_conversion(void);
uint8_t adc_is_conversion_done(void);
uint8_t adc_is_window_satisfied(void);
uint16_t adc_get_result(void);
void adc_enable_power_rails_before_conversion(void);
void adc_disable_power_rails_after_conversion(void);

#endif // ADC_H


// ========================================
// main_clock_control.h
// ========================================
#ifndef MAIN_CLOCK_CONTROL_H
#define MAIN_CLOCK_CONTROL_H

#include <stdint.h>

void main_clock_control(uint8_t clk_sel, uint8_t clk_freq, 
                       uint8_t presc_en, uint8_t presc_div, 
                       uint8_t clkout_en);

void low_power_clock_control(uint8_t clk_sel, uint8_t presc_en, 
                             uint8_t presc_div, uint8_t run_standby, 
                             uint8_t clkout_en);

#endif // MAIN_CLOCK_CONTROL_H


// ========================================
// sleep.h
// ========================================
#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>

void sleep_init(uint8_t sleep_mode, uint8_t sleep_en);
void sleep_disable(void);
void sleep_enable(void);

#endif // SLEEP_H


// ========================================
// usart0_tx.h
// ========================================
#ifndef USART0_TX_H
#define USART0_TX_H

#include <stdint.h>

void usart_init(uint8_t mode, uint8_t parity, uint8_t stop_bits, 
                uint8_t char_size, uint32_t baud_rate);

void usart0_send_char(char c);
void usart0_send_string(const char *str);

#endif // USART0_TX_H


Client
/*
 * AVR Bare Metal Series - Episode 14/15
 * CLIENT Device Implementation
 * 
 * Project: Low-power SPI communication with USART output
 * Hardware: AVR DD Curiosity Nano
 */

#define F_CPU 32768UL  // 32.768 KHz for most of the time

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

// Include custom libraries
#include "main_clock_control.h"
#include "ports.h"
#include "sleep.h"
#include "spi0.h"
#include "usart0_tx.h"

// SPI Configuration
#define NUM_SPI_BYTES 2  // Number of bytes to receive

// External SPI data array (defined in spi0.c)
extern uint8_t spi_data[NUM_SPI_BYTES];

// State Machine Type Definition
typedef enum {
    STATE_INIT,
    STATE_SLEEP,
    STATE_SWITCH_TO_HIGHSPEED_CLOCK,
    STATE_RECEIVE_SPI,
    STATE_SWITCH_TO_LOWPOWER_CLOCK,
    STATE_WRITE_TO_USART
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
                // Initialize ports
                port_init();
                
                // Setup low power 32.768 KHz clock
                low_power_clock_control(
                    0x01,  // Internal 32.768 KHz oscillator
                    0,     // Prescaler enable = 0
                    0x00,  // Prescaler divider (ignored)
                    0,     // Run standby = 0
                    0      // Clock out = 0
                );
                
                // Initialize USART (1200 baud)
                usart_init(
                    0x00,  // Async mode
                    0x00,  // No parity
                    0x00,  // 1 stop bit
                    0x00,  // 8 data bits
                    1200   // Baud rate
                );
                
                // Initialize SPI as client
                spi_client_init();
                
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
                
                // Wake on SPI client select (PA7 pin change interrupt)
                if(get_client_select_flag_status()) {
                    clear_client_select_flag();
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
                
                app_data.state = STATE_RECEIVE_SPI;
                break;
                
            case STATE_RECEIVE_SPI:
                // Wait for packet to be complete (received in ISR)
                while(!get_packet_complete_status());
                
                // Clear packet complete flag
                clear_packet_complete_status();
                
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
                
                app_data.state = STATE_WRITE_TO_USART;
                break;
                
            case STATE_WRITE_TO_USART:
                // Print raw SPI bytes
                printf("SPI Byte[1]: 0x%02X\r\n", spi_data[1]);
                printf("SPI Byte[0]: 0x%02X\r\n", spi_data[0]);
                
                // Reconstruct 16-bit result
                uint16_t results = ((uint16_t)spi_data[1] << 8) | spi_data[0];
                printf("Results: 0x%04X\r\n", results);
                
                // Extract window comparison (bit 15)
                uint8_t window_result = (results >> 15) & 0x01;
                printf("Window: %u\r\n", window_result);
                
                // Extract ADC result (bits 0-11)
                uint16_t adc_result = results & 0x0FFF;
                printf("ADC: %u\r\n\r\n", adc_result);
                
                // Delay to ensure print completes
                _delay_ms(100);
                
                app_data.state = STATE_SLEEP;
                break;
        }
    }
    
    return 0;
}
