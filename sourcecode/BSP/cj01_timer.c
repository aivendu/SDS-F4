#include	"..\bsp_includes.h"
#include    "cj01_timer.h"
#include    "cj01_IRQ_priority.h" 

/*****************************************************************************************
** 函数名称 ：	TIMER3_Init
** 函数功能 ：  通用定时器3中断初始化，定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us，Ft=定时器工作频率,单位:Mhz
** 输    入 ：	uint16_t arr		自动重装值。
** 输    入 ：	uint16_t psc		时钟预分频数
** 输    出 ：	void
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
void TIMER3_Init(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  ///使能TIM3时钟

    TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;  //定时器分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);//初始化TIM3

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); //允许定时器3更新中断
    TIM_Cmd(TIM3, ENABLE); //使能定时器3

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //定时器3中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PREEMPTION_PRIORITY_0; //抢占优先级1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = SUB_PRIORITY_TIMER3; //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
}




/*****************************************************************************************
** 函数名称 ：	TIM3_IRQHandler
** 函数功能 ：  定时器3中断服务函数
** 输    入 ：	void
** 输    出 ：	void
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
void TIM3_IRQHandler(void)
{
    static uint32_t flag;
    static uint32_t second_flag;
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) //溢出中断
    {
        PS2Timer();
        if (flag < 200)
        {
            flag++;
        }
        else
        {
            flag = 0;//1秒钟
            second_flag++;
        }
        if (second_flag < 10)//10秒钟
        {
            flag++;
        }
        else
        {
            second_flag = 0;
        }
    }
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除中断标志位
}
