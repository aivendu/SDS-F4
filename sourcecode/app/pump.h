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


typedef enum {
    PUMP_ST_INIT,  //  初始化状态, 未检测，未控制
    PUMP_ST_OFF,   //  泵已经正常关闭
    PUMP_ST_ON,    //  泵已经正常打开
    PUMP_ST_WAIT,  //  泵正在等待打开
    PUMP_ST_SHORT, //  泵短路
    PUMP_ST_OPEN,  //  泵开路
    PUMP_ST_NOINIT, //  泵没有启用
} e_pump_state_t;
#define  IsPumpFault(state)    ((state == PUMP_ST_SHORT) || (state == PUMP_ST_OPEN) || (state == PUMP_ST_NOINIT))

extern int8_t PumpCtrlByTech(uint8_t channel, uint8_t open);

extern int8_t PumpCtrl(uint32_t channel, uint8_t open);

extern int8_t PairOfPumpCtrlByTech(uint8_t primary, uint8_t standby, uint8_t both);

extern void UpdateRelayCtl(void);

extern int8_t GetPumpCtrlState(uint32_t channel);

extern e_pump_state_t GetPumpState(uint32_t channel);

extern int8_t ClearPump(void);
#endif

