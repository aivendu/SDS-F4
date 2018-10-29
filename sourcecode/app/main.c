/***********************************************************
** Copyright (c) 2017,   
** All rights reserved.
** 文件名称： main.c
** 摘要： 主函数
** 当前版本： 1.0, 彭俊，20170702，创建
** 历史版本：
** 说明：
***********************************************************/
#include "includes.h"
#include "mod_malloc.h"

void testSRAM(void)
{
    int32_t * p = (void *)0x68000000;
    int ewrq;
    *p = 0x12345678;
    for (ewrq=0; ewrq<1024*1024; ewrq++)
    {
        *p = ~ewrq;
        p++;
    }
    ewrq = *p;
    ewrq = ewrq;
}
#define TaskBspSize     256    
OS_STK TaskBspStk[TaskBspSize];     //任务堆栈---BSP硬件层处理任务

/*****************************************************************************************
** 函数名称 ：	main
** 函数功能 ：  主函数入口
** 输    入 ：	void			
** 输    出 ：	int  			 
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	
** 日　  期 : 	
*****************************************************************************************/
int main(void)
{
    FSMC_SRAM_Init();		//外部SRAM初始化
	mymem_init(SRAMIN);  	//初始化内部内存池
	mymem_init(SRAMEX);  	//初始化外部内存池
	mymem_init(SRAMCCM); 	//初始化CCM内存池
    OSInit();
    //testSRAM();
    memset(TaskBspStk, 0x55, sizeof(TaskBspStk));       //  用于查看堆栈越界
    OSTaskCreate(TaskBsp, (void *)0, &TaskBspStk[TaskBspSize - 1], TaskBspPrio);        //创建BSP层任务
    OSStart();
    return 0;
}
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
