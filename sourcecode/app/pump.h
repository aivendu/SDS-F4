#ifndef _PUMP_H_
#define _PUMP_H_

#include "stdint.h"

#define MAX_PUMP_CHANNEL   14

#define PUMP_CH_1          (1<<0 )
#define PUMP_CH_2          (1<<1 )
#define PUMP_CH_3          (1<<2 )          
#define PUMP_CH_4          (1<<3 )
#define PUMP_CH_5          (1<<4 )
#define PUMP_CH_6          (1<<5 )
#define PUMP_CH_7          (1<<6 )
#define PUMP_CH_8          (1<<7 )
#define PUMP_CH_9          (1<<8 )
#define PUMP_CH_10         (1<<9 )
#define PUMP_CH_11         (1<<10)
#define PUMP_CH_12         (1<<11)
#define PUMP_CH_13         (1<<12)
#define PUMP_CH_14         (1<<13)

extern int8_t SinglePumpCtrl(uint8_t channel, uint8_t open);

extern int8_t PumpCtrl(uint32_t channel, uint8_t open);

extern int8_t GetPumpCtrlState(uint32_t channel);

#endif

