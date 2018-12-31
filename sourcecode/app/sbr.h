#ifndef _SBR_H_
#define _SBR_H_
#include "stdint.h"
//  曝气泵
#define aeration_pump_1
#define aeration_pump_2
#define aeration_pump_3

//  潜污泵
#define submersible_sewage_pump_1
#define submersible_sewage_pump_2
#define submersible_sewage_pump_3

//  回流泵
#define reflex_pump_1
#define reflex_pump_2

//  出水泵
#define water_pump_1
#define water_pump_2

//  加药泵
#define Dosing_pump_1
#define Dosing_pump_2

//  提升泵
#define lifting_pump_1
#define lifting_pump_2

extern void FlowSBR(void);

#endif

