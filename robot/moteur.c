#include "moteur.h"

struct MCCP_Registers
{
    volatile uint16_t *CON1L;
    volatile uint16_t *CON1H;
    volatile uint16_t *CON2L;
    volatile uint16_t *CON2H;
    volatile uint16_t *CON3L;
    volatile uint16_t *CON3H;
    volatile uint16_t *STAT;
    volatile uint16_t *PRL;
    volatile uint16_t *RA;
    volatile uint16_t *RB;
};

struct MCCP_Registers _modules[] = 
{
    {0}, // Offset, index 0
    {&CCP1CON1L, &CCP1CON1H, &CCP1CON2L, &CCP1CON2H, &CCP1CON3L, &CCP1CON3H, &CCP1STATL, &CCP1PRL, &CCP1RA, &CCP1RB},
    {&CCP2CON1L, &CCP2CON1H, &CCP2CON2L, &CCP2CON2H, 0         , &CCP2CON3H, &CCP2STATL, &CCP2PRL, &CCP2RA, &CCP2RB},
    {&CCP3CON1L, &CCP3CON1H, &CCP3CON2L, &CCP3CON2H, 0         , &CCP3CON3H, &CCP3STATL, &CCP3PRL, &CCP3RA, &CCP3RB},
    {&CCP4CON1L, &CCP4CON1H, &CCP4CON2L, &CCP4CON2H, 0         , &CCP4CON3H, &CCP4STATL, &CCP4PRL, &CCP4RA, &CCP4RB}
};

static void _ccp_init(uint8_t module, struct MCCP_Config *config)
{
    if(module == 0 || module > 4) return;
    
    struct MCCP_Registers *reg = &_modules[module];
    
    *reg->CON1L = 0;
    
    *reg->CON1L = *(uint16_t *)&config->Con1L & ~(1 << 15);
    *reg->CON1H = *(uint16_t *)&config->Con1H;
    *reg->CON2L = *(uint16_t *)&config->Con2L;
    *reg->CON2H = *(uint16_t *)&config->Con2H;
    *reg->CON3L = *(uint16_t *)&config->Con3L;
    *reg->CON3H = *(uint16_t *)&config->Con3H;
    *reg->STAT  = *(uint16_t *)&config->Stat;
    *reg->PRL   = config->Prl;
    *reg->RA    = config->Rise;
    *reg->RB    = config->Fall;
    
    if(config->Con1L.CCPON) *reg->CON1L |= (1 << 15);
}

void moteur_init(uint8_t module, struct MCCP_Config *config)
{
    _ccp_init( module, config);
}

void moteur_drive(uint8_t module, uint16_t speed)
{
    if(module == 0 || module > 4) return;
    
    struct MCCP_Registers *reg = &_modules[module];
    
    *reg->RB = speed;
    
    if(!(*reg->CON1L & (1 << 15)))
    {
        *reg->STAT  = 0;
        *reg->CON1L |= (1 << 15);
    }
}

void moteur_coast(uint8_t module)
{
    if(module == 0 || module > 4) return;
    
    struct MCCP_Registers *reg = &_modules[module];
    
    *reg->CON1L &= ~(1 << 15);   // CCPON = 0
}