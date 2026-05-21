#include "uart.h"
#include "string.h"

#define FCY                 4000000UL
#define BAUDRATE            250000UL
#define BRGVAL              (FCY / (16 * BAUDRATE) - 1)

volatile uint8_t _txBuffer[TX_BUFFER_SIZE];
volatile uint16_t _txBufferHead = 0;
volatile uint16_t _txBufferTail = 0;
volatile uint16_t _txBufferCnt = 0;

volatile uint8_t _rxBuffer[RX_BUFFER_SIZE];
volatile uint16_t _rxBufferHead = 0;
volatile uint16_t _rxBufferTail = 0;

void uart_init() {
    U1BRG = BRGVAL;

    U1MODEbits.STSEL = 0;
    U1MODEbits.PDSEL = 0;
    U1MODEbits.BRGH = 0;

    U1STAbits.UTXISEL1 = 0;
    U1STAbits.UTXISEL0 = 0;
    U1STAbits.URXISEL = 0;

    IEC0bits.U1RXIE = 1;

    U1MODEbits.UARTEN = 1; // Enable UART

    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;

    IFS0bits.U1TXIF = 0;
    IFS0bits.U1RXIF = 0;
}

uint8_t uart_tx(uint8_t data) 
{
    
    if (_txBufferCnt < TX_BUFFER_SIZE) 
    {
        _txBuffer[_txBufferHead++] = data;

        if (_txBufferHead >= TX_BUFFER_SIZE) 
        {
            _txBufferHead = 0;
        }

        IEC0bits.U1TXIE = 0;
        _txBufferCnt++;
        IEC0bits.U1TXIE = 1;

        IFS0bits.U1TXIF = 1;
        return 1;
    }

    IFS0bits.U1TXIF = 1;
    return 0;
}

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) 
{
    IFS0bits.U1TXIF = 0;

    if (_txBufferCnt) 
    {
        U1TXREG = _txBuffer[_txBufferTail++];

        if (_txBufferTail >= TX_BUFFER_SIZE) 
        {
            _txBufferTail = 0;
        }

        _txBufferCnt--;
    } 
    else 
    {
        IEC0bits.U1TXIE = 0;
    }
}

/*
 ----------------------------------------
 ----------------------------------------
 */

uint8_t uart_rx_available() 
{
    return _rxBufferHead != _rxBufferTail;
}

uint8_t uart_rx() {
    uint8_t data = 0;

    if (_rxBufferHead != _rxBufferTail) 
    {
        data = _rxBuffer[_rxBufferTail];

        IEC0bits.U1RXIE = 0;
        _rxBufferTail++;
        IEC0bits.U1RXIE = 1;

        if (_rxBufferTail >= RX_BUFFER_SIZE) 
        {
            _rxBufferTail = 0;
        }
    }

    return data;
}

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;

    _rxBuffer[_rxBufferHead++] = U1RXREG;

    if (_rxBufferHead >= RX_BUFFER_SIZE) 
    {
        _rxBufferHead = 0;
    }

    // Priorise les nouvelles données
    if (_rxBufferHead == _rxBufferTail) 
    {
        _rxBufferTail++;
        if (_rxBufferTail >= RX_BUFFER_SIZE) 
        {
            _rxBufferTail = 0;
        }
    }
}