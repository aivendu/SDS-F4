/*****************************************************************
** Copyright (c) 2017, 	 XXXXXXXXXXXX有限公司 
** All rights reserved.
** 文件名称： adc.c
** 摘要：     ADC转换输出函数
** 当前版本： 1.0, arjun，20180103，创建
** 历史版本： 
******************************************************************/
#include "adc.h"
#include "cj01_gpio.h"



/******************************************************************
** 函数名称:   ADC_AllInit
** 功能描述:   ADC初始化函数
** 输入:	   无
**
** 输出:	   无
** 全局变量:
** 调用模块: 
** 备注:
** 作者:	   arjun
** 日期:	   20180103
******************************************************************/
void ADC_AllInit(void)
{
    ADC_Configuration();  
}
/******************************************************************
** 函数名称:   ADC_Configuration
** 功能描述:   ADC配置函数
** 输入:	   无
**
** 输出:	   无
** 全局变量:
** 调用模块: 
** 备注:
** 作者:	   arjun
** 日期:	   20180103
******************************************************************/
void ADC_Configuration(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    ADC_CommonInitTypeDef   ADC_CommonInitStructure;
    ADC_InitTypeDef         ADC_InitStructure;

    //RCC_ADCCLKConfig(RCC_PCLK2_Div6); //设置ADC时钟分频因子为6(72M/6=12M),ADC最大工作频率为14M
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOA\B\C时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | RCC_APB2Periph_ADC3, ENABLE); //使能ADC1时钟，使能ADC2时钟，使能ADC3时钟
    
    //PA口
	GPIO_InitStructure.GPIO_Pin =  ADC_CUR_IN1_Pin | ADC_CUR_IN2_Pin | ADC_CUR_IN3_Pin | ADC_CUR_IN4_Pin;//电流采样口
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; //工作模式为模拟输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    //PB口
	GPIO_InitStructure.GPIO_Pin = ADC_CUR_IN7_Pin | ADC_CUR_IN8_Pin; //电流采样口
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
    
    //PC口
	GPIO_InitStructure.GPIO_Pin = ADC_CUR_IN5_Pin | ADC_CUR_IN6_Pin; //电流采样口
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC,&GPIO_InitStructure);

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | RCC_APB2Periph_ADC2,ENABLE);	  //ADC1、ADC2、ADC3复位
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | RCC_APB2Periph_ADC3,DISABLE);  //复位结束
     
    ADC_CommonInitStructure.ADC_Mode = ADC_TripleMode_RegSimult;//三重交替模式，同步模式   
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;//两个采样阶段之间的延迟20个时钟,仅适用于双重或三重交错模式
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;//DMA直接访问禁止使能
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;//预分频8分频。ADCCLK = PCLK2/8 = 84/8 = 10.5Mhz, ADC时钟最好不要超过36Mhz 
    ADC_CommonInit(&ADC_CommonInitStructure);//初始化

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;//扫描模式

    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发      
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;//外部触发通道，本例子使用软件触发，此值随便赋值即可
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐    
    
    ADC_InitStructure.ADC_NbrOfConversion = 3;
    ADC_Init(ADC1, &ADC_InitStructure);//ADC1初始化  
    //配置ADC通道转换顺序为1，第一个转换，采样周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_480Cycles);//ADC1CH1
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 2, ADC_SampleTime_480Cycles);//ADC1CH2
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 3, ADC_SampleTime_480Cycles);//ADC1CH3
    
    ADC_Init(ADC2, &ADC_InitStructure);//ADC2初始化
    ADC_RegularChannelConfig(ADC2, ADC_Channel_6, 1, ADC_SampleTime_480Cycles);//ADC2CH1
    ADC_RegularChannelConfig(ADC2, ADC_Channel_8, 2, ADC_SampleTime_480Cycles);//ADC2CH2
    ADC_RegularChannelConfig(ADC2, ADC_Channel_9, 3, ADC_SampleTime_480Cycles);//ADC2CH3
    
    ADC_Init(ADC3, &ADC_InitStructure);//ADC3初始化    
    ADC_RegularChannelConfig(ADC3, ADC_Channel_10,  1, ADC_SampleTime_480Cycles);//ADC3CH1
    ADC_RegularChannelConfig(ADC3, ADC_Channel_13,  2, ADC_SampleTime_480Cycles);//ADC3CH2
    ADC_RegularChannelConfig(ADC3, ADC_Channel_13,  3, ADC_SampleTime_480Cycles);//ADC3CH3
 

    
    ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);//使能DMA请求在最后一次传输完成（多通道ADC模式）
    ADC_DMACmd(ADC1, ENABLE);//使能ADC——DMA
    
    ADC_Cmd(ADC1, ENABLE);//开启ADC1转换器
    ADC_Cmd(ADC2, ENABLE);//开启ADC1转换器
    ADC_Cmd(ADC3, ENABLE);//开启ADC3转换器
    ADC_SoftwareStartConv(ADC1);
}

/******************************************************************
** 函数名称:   ADC_GetSamplingValue
** 功能描述:   ADC获取采样AD值
** 输入:	   ADCx : ADC1~ADC3
**             channel : 0~16取值范围为：ADC_Channel_0~ADC_Channel_16
**
** 输出:	   转换结果
** 全局变量:
** 调用模块: 
** 备注:
** 作者:	   arjun
** 日期:	   20180103
******************************************************************/
uint16_t ADC_GetSamplingValue(ADC_TypeDef* ADCx, uint8_t channel)   
{
	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADCx, channel, 1, ADC_SampleTime_480Cycles);	//ADCx,ADC通道,239.5个周期,提高采样时间可以提高精确度			    
  
	ADC_SoftwareStartConv(ADCx);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADCx);	//返回最近一次ADC1规则组的转换结果
}


/******************************************************************
** 函数名称:   ADC_GetAverageValue
** 功能描述:   ADC获取多次采样AD值，并计算平均值
** 输入:	   ADCx : ADC1~ADC3
**             channel : 0~16取值范围为：ADC_Channel_0~ADC_Channel_16
**             times: 取采样的次数
** 输出:	   通道channel的times次转换结果平均值
** 全局变量:
** 调用模块: 
** 备注:
** 作者:	   arjun
** 日期:	   20180103
******************************************************************/
uint16_t ADC_GetAverageValue(ADC_TypeDef* ADCx, uint8_t channel, uint8_t times)
{
	uint32_t temp_val = 0;
	uint8_t i = 0;
	for(i = 0; i < times; i++)
	{
		temp_val += ADC_GetSamplingValue(ADCx, channel);
		//CountDelayMs(5);
	}
	return temp_val/times;
}
/******************************************************************
** 函数名称:   ADC_DataToVoltage
** 功能描述:   ADC的采样数据值转电压
** 输入:	   
**             value : 采样的ADC值
** 输出:	   根据参考电压转换后的电压值, 单位mA
** 全局变量:
** 调用模块: 
** 备注:
** 作者:	   arjun
** 日期:	   20180103
******************************************************************/
float ADC_DataToVoltage(uint16_t value)
{
    float value_temp = 0;
    if(ADC_RESOLU == 12)//分辨率12位
    {
        value_temp = (float)(value * ADC_VREF) / 4095;
    }
    else if(ADC_RESOLU == 10)//分辨率10位
    {
        value_temp = (float)(value * ADC_VREF) / 1023;
    }
    else if(ADC_RESOLU == 8)//分辨率8位
    {
        value_temp = (float)(value * ADC_VREF) / 255;
    }
    return value_temp;
}


/*********************************************************************************************************
**                            End Of File
**********************************************************************************************************/
