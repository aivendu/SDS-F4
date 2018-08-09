#ifndef __SIMULATE_IIC_H
#define __SIMULATE_IIC_H

#include "stm32f4xx.h"
#include "sys_config.h"


//IO方向设置
#define SetGPIODir(a,b)  {   \
      GPIOB->MODER  &= ~(GPIO_MODER_MODER0 << ((a) * 2));\
      GPIOB->MODER |= (((uint32_t)b) << ((a) * 2));\
}

#define SDA_IN()   SetGPIODir(7, GPIO_Mode_IN)
#define SDA_OUT()  SetGPIODir(7, GPIO_Mode_OUT)

//IO操作函数	 
#define I2cSetSCL()    GPIO_SetBits(GPIOB, GPIO_PIN_6) //SCL
#define I2cResetSCL()  GPIO_ResetBits(GPIOB, GPIO_PIN_6) //SCL
#define I2cSetSDA()    GPIO_SetBits(GPIOB, GPIO_PIN_7)   //SDA
#define I2cResetSDA()  GPIO_ResetBits(GPIOB, GPIO_PIN_7)   //SDA	 
#define I2cReadSDA()   (GPIO_ReadInputData(GPIOB) & GPIO_PIN_7)  //输入SDA 

//IIC所有操作函接口
extern void IIC_Init(void);                //初始化IIC的IO
extern void IIC_Start(void);				//发送IIC开始信号
extern void IIC_Stop(void);	  			//发送IIC停止信号
extern void IIC_Send_Byte(uint8_t txd);			//IIC发送一个字节
extern uint8_t IIC_Read_Byte(uint8_t ack);//IIC读取一个字节
extern uint8_t IIC_Wait_Ack(void); 				//IIC等待ACK信号
extern void IIC_Ack(void);					//IIC发送ACK信号
extern void IIC_NAck(void);				//IIC不发送ACK信号

#endif

















