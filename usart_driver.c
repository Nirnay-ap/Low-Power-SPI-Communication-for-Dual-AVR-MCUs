#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include "usart0_tx.h"

#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

// USART transmit function
static int usart0_printchar(char c, FILE *stream) {
    // Wait for transmit buffer to be empty
    while(!(USART0.STATUS & USART_DREIF_bm));
    
    // Send character
    USART0.TXDATAL = c;
    
    return 0;
}

// Standard output stream
static FILE usart0_stream = FDEV_SETUP_STREAM(usart0_printchar, NULL, _FDEV_SETUP_WRITE);

void usart_init(uint8_t mode, uint8_t parity, uint8_t stop_bits, 
                uint8_t char_size, uint32_t baud_rate) {
    
    // Set pin direction (PD4 = TX as output)
    PORTD.DIRSET = PIN4_bm;
    
    // Calculate and set baud rate
    uint16_t baud_setting = (uint16_t)USART0_BAUD_RATE(baud_rate);
    USART0.BAUD = baud_setting;
    
    // Configure USART Control A
    USART0.CTRLC = (mode << USART_CMODE_gp)       // Mode (async/sync)
                 | (parity << USART_PMODE_gp)     // Parity
                 | (stop_bits << USART_SBMODE_bp) // Stop bits
                 | (char_size << USART_CHSIZE_gp);// Character size
    
    // Enable transmitter
    USART0.CTRLB = USART_TXEN_bm;
    
    // Redirect stdout to USART
    stdout = &usart0_stream;
}

void usart0_send_char(char c) {
    // Wait for transmit buffer to be empty
    while(!(USART0.STATUS & USART_DREIF_bm));
    
    // Send character
    USART0.TXDATAL = c;
}

void usart0_send_string(const char *str) {
    while(*str) {
        usart0_send_char(*str++);
    }
}
