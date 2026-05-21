#include "xc.h"

#ifndef MOTEUR_H
#define MOTEUR_H

#define PPS_OUT_OCM2A       16
#define PPS_OUT_OCM2B       17

#define PPS_OUT_OCM3A       18
#define PPS_OUT_OCM3B       19

#define PPS_OUT_OCM4A       20
#define PPS_OUT_OCM4B       21

enum ID_Moteur {
    MOTEUR_FL = 4,
    MOTEUR_FR = 3,
    MOTEUR_RL = 2,
    MOTEUR_RR = 1
};

struct MCCP_Config
{
    CCP1CON1LBITS Con1L;
    CCP1CON1HBITS Con1H;
    CCP1CON2LBITS Con2L; 
    CCP1CON2HBITS Con2H;
    CCP1CON3LBITS Con3L;
    CCP1CON3HBITS Con3H;
    CCP1STATBITS  Stat;
    uint16_t      Prl;
    uint16_t      Rise;
    uint16_t      Fall;
};

void moteur_init(uint8_t module, struct MCCP_Config *config);
void moteur_drive(uint8_t module, uint16_t speed);
void moteur_coast(uint8_t module);

#endif