#ifndef _A2O_H_
#define _A2O_H_
#include "stdint.h"


//  曝气泵
#define a2o_aeration_pump_1
#define a2o_aeration_pump_2
#define a2o_aeration_pump_3

//  潜污泵
#define a2o_submersible_sewage_pump_1
#define a2o_submersible_sewage_pump_2
#define a2o_submersible_sewage_pump_3

//  回流泵
#define a2o_reflex_pump_1
#define a2o_reflex_pump_2

//  出水泵
#define a2o_water_pump_1
#define a2o_water_pump_2

//  加药泵
#define a2o_Dosing_pump_1
#define a2o_Dosing_pump_2

//  提升泵
#define a2o_lifting_pump_1
#define a2o_lifting_pump_2


typedef struct s_subm_open
{
    struct s_subm_open * next;
    uint32_t start;  //  单位S
    uint32_t time : 28;  //  单位S
    uint32_t type : 4;   // 0--结束时间; 1--起始时间; 
                         // 2--起始持续时间; 3--结束持续时间; 4--时间标志
} s_subm_open_t;

extern void AddSubmOpenTimestamp(s_subm_open_t *curr);
extern s_subm_open_t *GetSubmOpenTimestamp(void);

extern void FlowA2O(void);


#endif

