/*****************************************************************
** Copyright (c) 2017, 	 XXXXXXXXXXXX有限公司 
** All rights reserved.
** 文件名称： adc.h
** 摘要：     
** 当前版本： 1.0, arjun，20180103，创建
** 历史版本： 
******************************************************************/
#ifndef __ADC__
#define __ADC__
#include "stdint.h"
#include "stm32f4xx.h"


#define ADC_VREF                    3300       //ADC采样的参考电压值：3.3V
#define ADC_RESOLU                  12        //ADC采样的分辨率12位或10位或8位
#define ADC_CHANEL                  14        //ADC采样的总通道数8

//#define ADC_CUR_IN1                 CUR_IN1
//#define ADC_CUR_IN2                 CUR_IN2
//#define ADC_CUR_IN3                 CUR_IN3
//#define ADC_CUR_IN4                 CUR_IN4
//#define ADC_CUR_IN5                 CUR_IN5
//#define ADC_CUR_IN6                 CUR_IN6
//#define ADC_CUR_IN7                 CUR_IN7
//#define ADC_CUR_IN8                 CUR_IN8
//#define ADC_CUR_IN9                 CUR_IN9
//#define ADC_CUR_IN10                CUR_IN10
//#define ADC_CUR_IN11                CUR_IN11
//#define ADC_CUR_IN12                CUR_IN12
//#define ADC_CUR_IN13                CUR_IN13
//#define ADC_CUR_IN14                CUR_IN14


void ADC_AllInit(void);
void ADC_Configuration(void);
uint16_t ADC_GetSamplingValue(ADC_TypeDef* ADCx, uint8_t channel);
uint16_t ADC_GetAverageValue(ADC_TypeDef* ADCx, uint8_t channel, uint8_t times);
float ADC_DataToVoltage(uint16_t value);
#endif
