#include "xc.h"

#ifndef UART_H
#define UART_H

#define PPS_INPUT_U1RX      26
#define PPS_OUTPUT_U1TX     3

#define TX_BUFFER_SIZE    255
#define RX_BUFFER_SIZE    255

void uart_init();

uint8_t uart_tx(uint8_t data);

uint8_t uart_rx_available();

uint8_t uart_rx();

#endif