/*****************************************************************
** Copyright (c) 2017, 	 XXXXXXXXXXXX有限公司 
** All rights reserved.
** 文件名称： dma.c
** 摘要：     dma——初始化
** 当前版本： 1.0, arjun，20170825，创建
** 历史版本： 
******************************************************************/
#include "dma.h"
#include "misc.h" 
#include "adc.h" 


float cur_filt_voltage_value[8];//电流滤波后的转的电压值

uint16_t adc_data[9];//当前值，因为采样的是9个通道的AD值

uint16_t dis_adc_data[8];//滤波处理后的ADC值

uint8_t *DMA_SendBuff = NULL;//发送的缓冲区
uint8_t *DMA_RecBuff = NULL;//接收的缓冲区
/******************************************************************
** 函数名称:   DMA_Configuration_ADC
** 功能描述:   DMA配置函数
** 输入:	   
** 输出:	   无
** 全局变量:
** 调用模块:            
** 备注:       
**             
** 作者:	   arjun
** 日期:	   20180118
******************************************************************/
void DMA_Configuration_ADC()
{  
    DMA_InitTypeDef  DMA_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能 
    
    DMA_DeInit(DMA2_Stream0);//初始化DMA的寄存器为默认值
    while (DMA_GetCmdStatus(DMA2_Stream0) != DISABLE)//等待DMA可配置
    {        
    }        
  
    //配置 DMA Stream
    DMA_InitStructure.DMA_Channel = DMA2_ADC_CHx;  //通道选择
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC_DR_ADDR;//DMA外设地址
    DMA_InitStructure.DMA_Memory0BaseAddr = ADC_MEMORY_ADDR;//DMA 存储器0地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//外设到存储器模式
    DMA_InitStructure.DMA_BufferSize = DMA_ADC_BUFSIZE;//数据传输量 
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设数据长度:2 BYTE
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//存储器数据长度:2 BYTE
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;// Normal使用普通模式 ,Circular循环传输
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//中优先级
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;//禁止DMA FIFO，使用直连模式         
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;//FIFO的大小
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);//初始化DMA Stream
    DMA_Cmd(DMA2_Stream0, ENABLE);//使能DMA
    
#if DMA2_ADC_INT_ENABLE//使能RX0中断    
    DMA_ITConfig(DMA2_ADC_Streamx, DMA_IT_TC, ENABLE);//DMA中断配置
#endif

//#if DMA2_ADC_INT_ENABLE//使能ADC中断
//    //使能DMA2的ADC中断
//    NVIC_InitStructure.NVIC_IRQChannel = DMA2_ADC_CHx_IRQn;   //ADC connect to channel 0 of DMA2  
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //选择主中断分组为2 
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 次优先级为3
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
//    NVIC_Init(&NVIC_InitStructure); 
//#endif
} 

/******************************************************************
** 函数名称:   DMA_AllInit
** 功能描述:   DMA初始化函数
** 输入:	   无
**
** 输出:	   无
** 全局变量:
** 调用模块: 
** 备注:
** 作者:	   arjun
** 日期:	   20170829
******************************************************************/
void DMA_AllInit()
{
    DMA_Configuration_ADC();
}

/******************************************************************
** 函数名称:   DMA_Enable
** 功能描述:   DMA使能函数
** 输入:	   -DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7 
**             -ndtr:数据传输量  
** 输出:	   无
** 全局变量:
** 调用模块:   
** 备注:       开启一次DMA传输
** 作者:	   arjun
** 日期:	   20170825
******************************************************************/
void DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx, uint16_t ndtr)
{ 
	DMA_Cmd(DMA_Streamx, DISABLE);                     //关闭DMA传输 	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE)   //确保DMA可以被设置
    {        
    }        	
	DMA_SetCurrDataCounter(DMA_Streamx, ndtr);         //数据传输量   
	DMA_Cmd(DMA_Streamx, ENABLE);                      //开启DMA传输
}

/******************************************************************
** 函数名称:   DMA2_Stream0_IRQHandler
** 功能描述:   DMA2 数据流0中断的处理函数
** 输入:	   无
**
** 输出:	   无
** 全局变量:
** 调用模块: 
** 备注:
** 作者:	   arjun
** 日期:	   20180202
******************************************************************/
#define        ADC_FILT           1//滤波系数
void DMA2_Stream0_IRQHandler(void)  
{
    uint8_t i = 0;
    if (DMA_GetFlagStatus(DMA2_ADC_Streamx, DMA_FLAG_TCIF0) != RESET)//等待DMA2_Steam0传输完成
    {        
        DMA_ClearFlag(DMA2_ADC_Streamx, DMA_FLAG_TCIF0);//清数据流传输完成中断标志
        for(i = 0; i < 8; i++)
        {
            if(dis_adc_data[i] == 0)
            {
                dis_adc_data[i] = adc_data[i];
            }
            else
            {
                dis_adc_data[i] = (ADC_FILT * adc_data[i]) + ((10 - ADC_FILT) * dis_adc_data[i]);
                dis_adc_data[i] = dis_adc_data[i] / 10;
            }
        }    
    }
}

/******************************************************************
** 函数名称:   CUR_DataToVoltage
** 功能描述:   4-20ma采样滤波处理
** 输入:	   
**             
** 输出:	   
** 全局变量:
** 调用模块: 
** 备注:       
** 作者:	   arjun
** 日期:	   20180103
******************************************************************/
#define        ADC_FILT1           1//滤波系数
#define        ADC_NUM             10//滤波系数
#define        SAMPLING_RESISTANCE 150

void CUR_DataToVoltage(void)
{
    uint8_t i = 0;
    uint8_t j = 0; 
    uint16_t adc_temp[8];

    memset(adc_temp, 0, sizeof(adc_temp));//清数组    
    
    for(i = 0; i < 8; i++)
    {
        if(dis_adc_data[i] == 0)
        {
            dis_adc_data[i] = adc_data[i];
        }
        else
        {
            dis_adc_data[i] = (ADC_FILT1 * adc_data[i]) + ((ADC_NUM - ADC_FILT1) * dis_adc_data[i]);
            dis_adc_data[i] = dis_adc_data[i] / ADC_NUM;
        }
    }
    adc_temp[0] = dis_adc_data[0];
    adc_temp[1] = dis_adc_data[3];
    adc_temp[2] = dis_adc_data[6];
    adc_temp[3] = dis_adc_data[1];
    adc_temp[4] = dis_adc_data[5];
    adc_temp[5] = dis_adc_data[2];
    adc_temp[6] = dis_adc_data[4];
    adc_temp[7] = dis_adc_data[7];
    for(j = 0; j < 8; j++)
    {
        cur_filt_voltage_value[j] = ADC_DataToVoltage(adc_temp[j]) / SAMPLING_RESISTANCE;
    }
}
/*********************************************************************************************************
**                            End Of File
**********************************************************************************************************/
