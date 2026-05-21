#include "xc.h"

#pragma config JTAGEN = OFF
#pragma config ICS = PGD1
#pragma config FWDTEN = OFF
#pragma config FNOSC = FRC
#pragma config POSCMD = NONE

#include "uart.h"
#include "moteur.h"
#include "encodeur.h"
#include <stdlib.h>
#include <math.h>

#define PI                  3.14159f
#define DIAMETRE_CM         6.7f
#define PERIMETRE_CM        (PI * DIAMETRE_CM)

#define TICKS_PAR_TOUR      262

#define CM_PAR_TICK         (PERIMETRE_CM / TICKS_PAR_TOUR)

#define FREQ                    5000

#define PRL                     (4000000UL / (FREQ) - 1)
#define RISE                    0
#define FALL                    (PRL / 2)
#define ROBOT_STOP              (FALL)
#define FORWARD_SPEED_GAIN      1.2f
#define REVERSE_SPEED_GAIN      0.8f
#define ROBOT_FORWARD_SPEED     (uint16_t)(FALL * FORWARD_SPEED_GAIN)
#define ROBOT_REVERSE_SPEED     (uint16_t)(FALL * REVERSE_SPEED_GAIN)
#define ROBOT_ROTATE_IN_SPEED   (FALL + (FALL / 2))
#define ROBOT_ROTATE_OU_SPEED   (FALL - (FALL / 2))


struct MCCP_Config configMoteur =
{
    .Con1L.CCSEL    = 0,        // Output Compare
    .Con1L.MOD      = 5,        // Dual Edge Compare mode, buffered
    .Con1H.SYNC     = 2,        // MCCP1 Sync Out
    .Con2H.OCAEN    = 1,        // OCxA (RB7)
    .Con2H.OCBEN    = 1,        // OCxB (RB8)
    .Con3H.OUTM     = 2,        // Half-Bridge Output
    .Con3H.POLACE   = 0,        // Active-High
    .Prl            = PRL,
    .Rise           = RISE,
    .Fall           = FALL,      // Duty
    .Con1L.CCPON    = 1,
};
struct MCCP_Config configMoteurFL;
struct MCCP_Config configMoteurFR;
struct MCCP_Config configMoteurRL;
struct MCCP_Config configMoteurRR;

enum Rx_State
{
    WAIT_START,
    WAIT_NB_POINTS,
    DECODE_POINTS,
    VERIFY_CHECKSUM
};
enum Rx_State currentState = WAIT_START;

struct Coordonnes
{
    int8_t x;
    int8_t y;
};

#define MAX_POINTS      24

struct Trajectoire
{
    struct Coordonnes points[MAX_POINTS];
    uint8_t nbPoints;
    uint8_t currentPoint;
    uint8_t newPoint;
    uint8_t checksum;
};
struct Trajectoire trajectoire = {0}; 

enum Navigation_State
{
    IDLE,
    MOVE_Y,
    PAUSE,
    ROTATE,
    MOVE_X,
    DONE
} ;
enum Navigation_State navigationState = IDLE;
enum Navigation_State nextNavigationState = IDLE;

uint8_t pauseCnt = 0;

uint8_t pointCnt = 0;
uint8_t navigateReady = 0;

#define DISTANCE_ROTATION   27.0f   // CM

enum Rotation_Side
{
    ROTATE_LEFT = -1,
    ROTATE_RIGHT = 1
};

void init_hardware();
void init_moteurs();
void init_timer1();

void navigate();
void decode_rx_data(uint8_t data);

void robot_stop();
void robot_forward();
void robot_reverse();
void robot_rotate(enum Rotation_Side side);

int main(void) 
{
    init_hardware();
    init_timer1();
    init_moteurs();
    encodeur_init();
    uart_init();

    while(1)
    {
        if(uart_rx_available())
        {
           decode_rx_data(uart_rx());
        }
        
        if(navigateReady || navigationState != IDLE)
        {
            navigate();
        }
    }
    
    return 0;
}

// 200ms
void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    
    if(navigationState == PAUSE)
    {
        pauseCnt++;
    }
}

void navigate()
{
    struct Coordonnes *p = &trajectoire.points[trajectoire.currentPoint];
    
    float distanceFL = (float)(*ENCODEUR_CNT_FL) * CM_PAR_TICK;
    float distanceFR = (float)(*ENCODEUR_CNT_FR) * CM_PAR_TICK;
    float distanceRL = (float)(*ENCODEUR_CNT_RL) * CM_PAR_TICK;
    float distanceRR = (float)(*ENCODEUR_CNT_RR) * CM_PAR_TICK;
    
    float distanceMoyenne = (distanceFL + distanceFR + distanceRL + distanceRR) / 4.0f;

    switch(navigationState)
    {
        case IDLE:
            if (navigateReady) 
            {
                navigateReady = 0; 
                encodeur_reset();
                
                navigationState = MOVE_Y;
            }
            break;

        case MOVE_Y:
            {
                float cibleY = (float)p->y;

                if (cibleY == 0.0f)
                {
                    encodeur_reset();
                    nextNavigationState = ROTATE;
                    navigationState = PAUSE;
                    break;
                }

                float distanceCible = fabsf(cibleY);

                if (distanceMoyenne < distanceCible)
                {
                    if (cibleY > 0)
                    {
                        robot_forward();
                    }
                    else
                    {
                        robot_reverse();
                    }
                }
                else
                {
                    robot_stop();
                    encodeur_reset();
                    
                    nextNavigationState = ROTATE;
                    navigationState = PAUSE;
                }
                break;
            }


        case ROTATE:
            
            if (p->x == 0)
            {
                encodeur_reset();
                nextNavigationState = MOVE_X;
                navigationState = PAUSE;
                break;
            }
            
            float distanceRotation = 0.0f;
            if (p->x > 0)
            {
                distanceRotation = (distanceFL + distanceRL) / 2.0f;
            }
            else
            {
                distanceRotation = (distanceFR + distanceRR) / 2.0f;
            }
            
            if(distanceRotation < DISTANCE_ROTATION)
            {
                if(p->x > 0)
                {
                    robot_rotate(ROTATE_LEFT);
                }
                else
                {
                    robot_rotate(ROTATE_RIGHT);
                }
            }
            else
            {
                robot_stop();
                encodeur_reset();

                nextNavigationState = MOVE_X;
                navigationState = PAUSE;
            }
            break;

        case MOVE_X:
            {
                float cibleX = fabsf((float)p->x);

                if (cibleX == 0.0f)
                {
                    encodeur_reset();
                    nextNavigationState = ROTATE;
                    navigationState = PAUSE;
                    break;
                }

                if (distanceMoyenne < cibleX)
                {
                    if (p->x > 0)
                    {
                        robot_forward();
                    }
                    else
                    {
                        robot_reverse();
                    }
                }
                else
                {
                    robot_stop();
                    encodeur_reset();

                    trajectoire.currentPoint++;

                    if (trajectoire.currentPoint >= trajectoire.nbPoints)
                    {
                        nextNavigationState = DONE;
                        navigationState = PAUSE;
                    }
                    else
                    {
                        nextNavigationState = MOVE_Y;
                        navigationState = PAUSE;
                    }
                }
                break;
            }
            
        case PAUSE:
            if (pauseCnt >= 1)
            {
                pauseCnt = 0;
                encodeur_reset();
                navigationState = nextNavigationState;
            }
            break;

        case DONE:
            uart_tx(0xBB);
            navigationState = IDLE;
            break;
    }
}

void decode_rx_data(uint8_t data)
{
    switch(currentState)
    {
        case WAIT_START:
            if(data == 0xAA)
            {
                
                navigateReady = 0;
                trajectoire.nbPoints = 0;
                trajectoire.currentPoint = 0;
                trajectoire.newPoint = 0;
                trajectoire.checksum = 0;
                pointCnt = 0;
                uart_tx(0xAA); // ACK
                currentState = WAIT_NB_POINTS;
            }
            break;

        case WAIT_NB_POINTS:
            if(data > 0 && data <= MAX_POINTS)
            {
                trajectoire.nbPoints = data;
                trajectoire.checksum ^= data;
                pointCnt = 0;
                currentState = DECODE_POINTS;
            }
            else
            {
                currentState = WAIT_START;
            }
            break;

        case DECODE_POINTS:
            trajectoire.checksum ^= data;

            switch(pointCnt)
            {
                case 0:
                    trajectoire.points[trajectoire.newPoint].x = (int8_t)data;
                    break;
                case 1:
                    trajectoire.points[trajectoire.newPoint].y = (int8_t)data;
                    break;
            }

            pointCnt++;

            if(pointCnt >= 2)
            {
                pointCnt = 0;
                trajectoire.newPoint++;

                if(trajectoire.newPoint >= trajectoire.nbPoints)
                {
                    currentState = VERIFY_CHECKSUM;
                }
            }
            break;

        case VERIFY_CHECKSUM:
            if(data == trajectoire.checksum)
            {
                trajectoire.currentPoint = 0;
                encodeur_reset();
                navigateReady = 1;
                uart_tx(0xAA);  // ACK
            }
            else
            {
                uart_tx(0xFF);  // NAK
            }
            currentState = WAIT_START;
            break;
    }
}

void robot_stop()
{
    moteur_drive(MOTEUR_FL, ROBOT_STOP);
    moteur_drive(MOTEUR_FR, ROBOT_STOP);
    moteur_drive(MOTEUR_RL, ROBOT_STOP);
    moteur_drive(MOTEUR_RR, ROBOT_STOP);
}

void robot_forward()
{
    moteur_drive(MOTEUR_FL, ROBOT_FORWARD_SPEED);
    moteur_drive(MOTEUR_FR, ROBOT_FORWARD_SPEED);
    moteur_drive(MOTEUR_RL, ROBOT_FORWARD_SPEED);
    moteur_drive(MOTEUR_RR, ROBOT_FORWARD_SPEED);
}

void robot_reverse()
{
    moteur_drive(MOTEUR_FL, ROBOT_REVERSE_SPEED);
    moteur_drive(MOTEUR_FR, ROBOT_REVERSE_SPEED);
    moteur_drive(MOTEUR_RL, ROBOT_REVERSE_SPEED);
    moteur_drive(MOTEUR_RR, ROBOT_REVERSE_SPEED);
}

void robot_rotate(enum Rotation_Side side)
{
    if(side == ROTATE_LEFT)
    {
        moteur_drive(MOTEUR_FL, ROBOT_ROTATE_IN_SPEED); moteur_drive(MOTEUR_RL, ROBOT_ROTATE_IN_SPEED);
        moteur_drive(MOTEUR_FR, ROBOT_ROTATE_OU_SPEED); moteur_drive(MOTEUR_RR, ROBOT_ROTATE_OU_SPEED);
    }
    else if(side == ROTATE_RIGHT)
    {
        moteur_drive(MOTEUR_FL, ROBOT_ROTATE_OU_SPEED); moteur_drive(MOTEUR_RL, ROBOT_ROTATE_OU_SPEED);
        moteur_drive(MOTEUR_FR, ROBOT_ROTATE_IN_SPEED); moteur_drive(MOTEUR_RR, ROBOT_ROTATE_IN_SPEED);
    }
}

void init_moteurs()
{
    configMoteurFL = configMoteur;
    configMoteurFR = configMoteur;
    configMoteurRL = configMoteur;
    configMoteurRR = configMoteur;

    configMoteurRR.Con1H.SYNC = 0;   // Self-sync sur CCP1PRL 
    
    moteur_init(MOTEUR_FL, &configMoteurFL);
    moteur_init(MOTEUR_FR, &configMoteurFR);
    moteur_init(MOTEUR_RL, &configMoteurRL);
    moteur_init(MOTEUR_RR, &configMoteurRR); // ***
}

void init_timer1()
{
    T1CON = 0;              // Reset
    T1CONbits.TCKPS = 2;    // 64
    IFS0bits.T1IF = 0;      // Clear Flag
    IEC0bits.T1IE = 1;      // Enable Interrupt
    T1CONbits.TON = 1;      // Enable Timer 1
    IPC0bits.T1IP = 7;      // Priority level 7
    
    PR1 = 25000 -1;         // 200ms
}

void init_hardware()
{
    TRISA = 0;  // Output
    LATA = 0;   // LOW
    ANSA = 0;   // Digital
    
    TRISB = 0;  // Output
    LATB = 0;   // LOW
    ANSB = 0;   // Digital
    
    TRISC = 0;  // Output
    LATC = 0;   // LOW
    ANSC = 0;   // Digital
    
    // UART RX INPUT
    TRISCbits.TRISC7 = 1;
    
    __builtin_write_OSCCONL(OSCCON & ~0x40);
    
    // UART
    RPINR18bits.U1RXR = 23;             // RC7
    RPOR12bits.RP24R = PPS_OUTPUT_U1TX; // RC8
    
    // MOTEURS
    RPOR10bits.RP21R = PPS_OUT_OCM2A;   // RC5
    RPOR10bits.RP20R = PPS_OUT_OCM2B;   // RC4
    
    RPOR7bits.RP14R = PPS_OUT_OCM3A;    // RB14
    RPOR7bits.RP15R = PPS_OUT_OCM3B;    // RB15
    
    RPOR13bits.RP26R = PPS_OUT_OCM4A;   // RA0
    RPOR13bits.RP27R = PPS_OUT_OCM4B;   // RA1
    
    __builtin_write_OSCCONL(OSCCON | 0x40);
}