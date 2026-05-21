#include "xc.h"

// MOTEUR AVANT GAUCHE
// RB2 A

// MOTEUR AVANT DROITE
// RB13 A

// MOTEUR ARRIERE GAUCHE
// RA8 A

// MOTEUR ARRIERE DROITE
// RC6 A

#ifndef ENCODEUR_H
#define ENCODEUR_H

enum ID_Encodeur
{
    ENCODEUR_FL_A = 2,
    ENCODEUR_FL_B = 3,
    ENCODEUR_FR_A = 13,
    ENCODEUR_FR_B = 12,
    ENCODEUR_RL_A = 8,
    ENCODEUR_RL_B = 4,
    ENCODEUR_RR_A = 6,
    ENCODEUR_RR_B = 9
};

extern const volatile uint16_t * const ENCODEUR_CNT_FL;
extern const volatile uint16_t * const ENCODEUR_CNT_FR;
extern const volatile uint16_t * const ENCODEUR_CNT_RL;
extern const volatile uint16_t * const ENCODEUR_CNT_RR;

void encodeur_init(void);
void encodeur_reset(void);

#endif