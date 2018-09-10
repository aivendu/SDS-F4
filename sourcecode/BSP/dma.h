/***********************************************************
** Copyright (c) 2017, 	 XXXXXXXXXXXX有限公司 
** All rights reserved.
** 文件名称： dma.h
** 摘要： 
** 当前版本： 1.0, arjun，20170825，创建
** 历史版本： 
***********************************************************/
#ifndef __DMA__
#define __DMA__
#include "string.h"
#include "stdint.h"
#include "stm32f4xx.h"


//ADC
extern uint16_t adc_data[9];
extern uint16_t dis_adc_data[8];//滤波处理后的ADC值

#define DMA2_ADC_Streamx                      DMA2_Stream0
#define DMA2_ADC_CHx                          DMA_Channel_0
#define DMA2_ADC_CHx_IRQn                     DMA2_Stream0_IRQn
#define DMA2_ADC_INT_ENABLE                   0//1：使能DMA2的ADC中断,0:禁止DMA2的ADC中断,
#define DMA_ADC_BUFSIZE                       9 

#define ADC_DR_ADDR                           ((uint32_t)0x40012308)//(uint32_t)&ADC1->DR// ADC DR 
#define ADC_MEMORY_ADDR                       (uint32_t)adc_data


void DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,uint16_t ndtr);
void DMA_AllInit(void);
void CUR_DataToVoltage(void);

#endif
