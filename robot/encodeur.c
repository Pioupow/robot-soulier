#include "xc.h"
#include "encodeur.h"

volatile uint16_t _encFrontLeftA = 0;
volatile uint16_t _encFrontRightA = 0;
volatile uint16_t _encRearLeftA = 0;
volatile uint16_t _encRearRightA = 0;

const volatile uint16_t * const ENCODEUR_CNT_FL = &_encFrontLeftA;
const volatile uint16_t * const ENCODEUR_CNT_FR = &_encFrontRightA;
const volatile uint16_t * const ENCODEUR_CNT_RL = &_encRearLeftA;
const volatile uint16_t * const ENCODEUR_CNT_RR = &_encRearRightA;

void encodeur_init(void)
{
    TRISBbits.TRISB2 = 1; // Input
    IOCPBbits.IOCPB2 = 1; // Front montant
    
    TRISBbits.TRISB13 = 1;
    IOCPBbits.IOCPB13 = 1;
    
    TRISAbits.TRISA8 = 1;
    IOCPAbits.IOCPA8 = 1;
    
    TRISCbits.TRISC6 = 1;
    IOCPCbits.IOCPC6 = 1;
    
    PADCONbits.IOCON = 1;
    IOCSTATbits.IOCPAF = 0;
    IOCSTATbits.IOCPBF = 0;
    IOCSTATbits.IOCPCF = 0;

    IEC1bits.IOCIE = 1;
    IFS1bits.IOCIF = 0;
}

void encodeur_reset()
{
    IEC1bits.IOCIE = 0;
    _encFrontLeftA  = 0;
    _encFrontRightA = 0;
    _encRearLeftA   = 0;
    _encRearRightA  = 0;
    IEC1bits.IOCIE = 1;
}

void __attribute__((interrupt, no_auto_psv)) _IOCInterrupt(void)
{
    IFS1bits.IOCIF = 0;
    
    if(IOCFBbits.IOCFB2)
    {
        _encFrontLeftA++;
        IOCFBbits.IOCFB2 = 0;
    }
    
    if(IOCFBbits.IOCFB13)
    {
        _encFrontRightA++;
        IOCFBbits.IOCFB13 = 0; 
    }
    
    if(IOCFAbits.IOCFA8)
    {
        _encRearLeftA++;
        IOCFAbits.IOCFA8 = 0; 
    }
    
    if(IOCFCbits.IOCFC6)
    {
        _encRearRightA++;
        IOCFCbits.IOCFC6 = 0; 
    }
}
