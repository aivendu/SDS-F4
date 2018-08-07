#include "cj01_simulate_IIC.h"


//延时nus
//nus为要延时的us数.		    								   
void delay_us(uint32_t nus)
{		
    uint16_t i=0;     
    while(nus--)    
    {        
        i=200;  //自己定义       
        while(i--)  ;
    } 
}
/******************************************************************
** 函数名称:   IIC_Init
** 功能描述:   初始化IIC的IO口
** 输入:	   无
**
** 输出:	   无
** 全局变量:
** 调用模块: 
** 备注:
** 作者:	   彭俊
** 日期:	   20170321
******************************************************************/
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_PIN_6|GPIO_PIN_7); 
}

/******************************************************************
** 函数名称:   IIC_Start
** 功能描述:   产生IIC起始信号
** 输入:	   无
**
** 输出:	   无
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   彭俊
** 日期:	   20170321
******************************************************************/
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	I2cSetSDA();	  	  
	I2cSetSCL();
	delay_us(4);
 	I2cResetSDA();//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	I2cResetSCL();//钳住I2C总线，准备发送或接收数据 
}	  
/******************************************************************
** 函数名称:   IIC_Stop
** 功能描述:   产生IIC停止信号
** 输入:	   无
**
** 输出:	   无
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   彭俊
** 日期:	   20170321
******************************************************************/
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	I2cResetSCL();
	I2cResetSDA();//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	I2cSetSCL(); 
	I2cSetSDA();//发送I2C总线结束信号
	delay_us(4);							   	
}
/******************************************************************
** 函数名称:   IIC_Wait_Ack
** 功能描述:   等待应答信号到来
** 输入:	   无
**
** 输出:	   1，接收应答失败，0，接收应答成功
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   彭俊
** 日期:	   20170321
******************************************************************/
uint8_t IIC_Wait_Ack(void)
{
	uint32_t ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	I2cSetSDA();delay_us(1);	   
	I2cSetSCL();delay_us(1);	 
	while(I2cReadSDA())
	{
		ucErrTime++;
		if(ucErrTime>2500)
		{
			IIC_Stop();
			return 1;
		}
	}
	I2cResetSCL();//时钟输出0 	   
	return 0;  
} 
/******************************************************************
** 函数名称:   IIC_Ack
** 功能描述:   产生ACK应答
** 输入:	   无
**
** 输出:	   无
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   彭俊
** 日期:	   20170321
******************************************************************/
void IIC_Ack(void)
{
	I2cResetSCL();
	SDA_OUT();
	I2cResetSDA();
	delay_us(2);
	I2cSetSCL();
	delay_us(2);
	I2cResetSCL();
}
/******************************************************************
** 函数名称:   IIC_NAck
** 功能描述:   不产生ACK应答	
** 输入:	   无
**
** 输出:	   无
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   彭俊
** 日期:	   20170321
******************************************************************/  
void IIC_NAck(void)
{
	I2cResetSCL();
	SDA_OUT();
	I2cSetSDA();
	delay_us(2);
	I2cSetSCL();
	delay_us(2);
	I2cResetSCL();
}			
/******************************************************************
** 函数名称:   IIC_Send_Byte
** 功能描述:   IIC发送一个字节
** 输入:	   无
**
** 输出:	   1，有应答，0，无应答	
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   彭俊
** 日期:	   20170321
******************************************************************/		  
void IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	SDA_OUT(); 	    
    I2cResetSCL();//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        //IIC_SDA=(txd&0x80)>>7;
		if((txd&0x80)>>7)
			I2cSetSDA();
		else
			I2cResetSDA();
		txd<<=1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		I2cSetSCL();
		delay_us(2); 
		I2cResetSCL();	
		delay_us(2);
    }	 
} 	 
/******************************************************************
** 函数名称:   IIC_Read_Byte
** 功能描述:   读1个字节
** 输入:	   ack=1时，发送ACK，ack=0，发送nACK 
**
** 输出:	   
** 全局变量:
** 调用模块:
** 备注:
** 作者:	   彭俊
** 日期:	   20170321
******************************************************************/
uint8_t IIC_Read_Byte(uint8_t ack)
{
	uint8_t i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        I2cResetSCL(); 
        delay_us(2);
		I2cSetSCL();
        receive<<=1;
        if(I2cReadSDA())receive++;
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}




