/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "cj01_gpio.h"
/* USER CODE BEGIN 0 */
#include "ls595.h"
#include "cj01_io_api.h"
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
        * Free pins are configured automatically as Analog (this feature is enabled through 
        * the Code Generation settings)
*/
void GPIOInit(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);


  /*Configure GPIO pin Output Level */
  GPIO_WriteBit(F1_PC12_GPIO_Port, F1_PC12_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  GPIO_WriteBit(SIM_PWRKEY_GPIO_Port, SIM_PWRKEY_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  GPIO_WriteBit(F1_SPI2_NSS_GPIO_Port, F1_SPI2_NSS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  GPIO_WriteBit(GPIOG, W5500_RSTn_Pin|W5500_SCSn_Pin|SDIO_CD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  GPIO_WriteBit(RUN_STU_GPIO_Port, RUN_STU_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  GPIO_WriteBit(ETH_RESET_GPIO_Port, ETH_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PEPin PEPin PEPin PEPin 
                           PEPin */
  GPIO_InitStruct.GPIO_Pin = YW_IN1_Pin|YW_IN2_Pin|YW_IN3_Pin|YW_IN4_Pin 
                          |YW_IN5_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC13 PC3 */
  GPIO_InitStruct.GPIO_Pin = GPIO_PIN_13|GPIO_PIN_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PFPin PFPin PFPin */
  GPIO_InitStruct.GPIO_Pin = YW_IN6_Pin|YW_IN7_Pin|YW_IN8_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.GPIO_Pin = F1_PC12_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(F1_PC12_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PF10 PF11 */
  GPIO_InitStruct.GPIO_Pin = GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.GPIO_Pin = SIM_PWRKEY_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(SIM_PWRKEY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.GPIO_Pin = SIM_WWAN_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(SIM_WWAN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  //GPIO_InitStruct.GPIO_Pin = F1_PD2_Pin;
  //GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  //GPIO_InitStruct.Pull = GPIO_NOPULL;
  //GPIO_Init(F1_PD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 */
  GPIO_InitStruct.GPIO_Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.GPIO_Pin = F1_SPI2_NSS_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(F1_SPI2_NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PGPin PGPin PGPin */
  GPIO_InitStruct.GPIO_Pin = W5500_INTn_Pin|W5500_DUPLED_Pin|W5500_SPDLED_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PGPin PGPin PGPin */
  GPIO_InitStruct.GPIO_Pin = W5500_RSTn_Pin|W5500_SCSn_Pin|SDIO_CD_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.GPIO_Pin = RUN_STU_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(RUN_STU_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.GPIO_Pin = ETH_RESET_Pin;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(ETH_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PD6 PD7 */
  GPIO_InitStruct.GPIO_Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/* USER CODE BEGIN 2 */
#define FILTER_TIME       10
typedef struct s_key
{
	uint8_t   active     :4;  //  滤波后确认的值
	uint8_t   value      :4;  //  当前值
	uint8_t   edge       :1;  //  表示当前处于边沿跳变状态
	uint8_t   time       :7;  //  滤波计时
    uint32_t  port;
	int8_t  (*get_key_value)(uint32_t arg);
	int8_t  (*key_value_proc)(struct s_key *arg);
} s_key_t;

int8_t GetGPIOInput(uint32_t port)
{
    GPIO_TypeDef * gpio_port;
    uint32_t gpio_pin;
    switch (port)
    {
        case GPIO_IN_01:    gpio_port = YW_IN1_GPIO_Port;  gpio_pin = YW_IN1_Pin;  break;
        case GPIO_IN_02:    gpio_port = YW_IN2_GPIO_Port;  gpio_pin = YW_IN2_Pin;  break;
        case GPIO_IN_03:    gpio_port = YW_IN3_GPIO_Port;  gpio_pin = YW_IN3_Pin;  break;
        case GPIO_IN_04:    gpio_port = YW_IN4_GPIO_Port;  gpio_pin = YW_IN4_Pin;  break;
        case GPIO_IN_05:    gpio_port = YW_IN5_GPIO_Port;  gpio_pin = YW_IN5_Pin;  break;
        case GPIO_IN_06:    gpio_port = YW_IN6_GPIO_Port;  gpio_pin = YW_IN6_Pin;  break;
        case GPIO_IN_07:    gpio_port = YW_IN7_GPIO_Port;  gpio_pin = YW_IN7_Pin;  break;
        case GPIO_IN_08:    gpio_port = YW_IN8_GPIO_Port;  gpio_pin = YW_IN8_Pin;  break;
        //case GPIO_IN_MENU:  gpio_port = MENU_GPIO_Port;    gpio_pin = MENU_Pin;  break;
        //case SD_CD:         gpio_port = IO_IN2_GPIO_Port;  gpio_pin = ;  break;
        default : return 0;
    }
    return GPIO_ReadInputDataBit(gpio_port, gpio_pin);
}
static s_key_t gpios[10] = 
{
    {0, 0, 0, 0, GPIO_IN_01,    GetGPIOInput, 0},
    {0, 0, 0, 0, GPIO_IN_02,    GetGPIOInput, 0},
    {0, 0, 0, 0, GPIO_IN_03,    GetGPIOInput, 0},
    {0, 0, 0, 0, GPIO_IN_04,    GetGPIOInput, 0},
    {0, 0, 0, 0, GPIO_IN_05,    GetGPIOInput, 0},
    {0, 0, 0, 0, GPIO_IN_06,    GetGPIOInput, 0},
    {0, 0, 0, 0, GPIO_IN_07,    GetGPIOInput, 0},
    {0, 0, 0, 0, GPIO_IN_08,    GetGPIOInput, 0},
    //{0, 0, 0, 0, GPIO_IN_MENU,  GetGPIOInput, 0},
    {0, 0, 0, 0, 0,             GetGPIOInput, 0},
};
static int8_t GetIndexIndexPortInGpios(uint32_t port)
{
    switch (port)
    {
        case GPIO_IN_01:    return 0;  
        case GPIO_IN_02:    return 1;  
        case GPIO_IN_03:    return 2;  
        case GPIO_IN_04:    return 3;  
        case GPIO_IN_05:    return 4;  
        case GPIO_IN_06:    return 5;  
        case GPIO_IN_07:    return 6;  
        case GPIO_IN_08:    return 7;  
        //case GPIO_IN_MENU:    return 8;
        //case SD_CD:         return 9;
        default: return -1;
    }
}

static void KeyFilter(s_key_t * key, uint8_t value)
{
	if (value == key->value)
	{
		if (key->time < FILTER_TIME)
		{
			key->time++;
		}
		else if (key->time == FILTER_TIME)
		{
			key->time++;
			if (key->key_value_proc)
			{
				key->key_value_proc(key);
			}
			else if (key->active != key->value)
			{
				key->edge = 0;
				key->active = key->value;
			}
		}
	}
	else
	{
		key->edge = 1;
	    key->value = value;
	    key->time = 0;
	}
}  /* end KeyFilter */

void GPIODebounce(void)
{
    int i;
    int value;
    for (i=0; i< (sizeof(gpios)/sizeof(s_key_t)); i++)
    {
        if (gpios[i].get_key_value)
        {
            value = gpios[i].get_key_value(gpios[i].port);
        }
        KeyFilter(&gpios[i], value);
    }
}

int8_t  GPIOOpen(int32_t port, const void *config, uint8_t config_len)
{
    return 0;
}

int8_t  GPIOClose(int32_t port)
{
    return 0;
}

int32_t GPIORead(int32_t port, void *buf, uint32_t buf_len)
{
    int port_index = GetIndexIndexPortInGpios(port);
    if (port_index < 0)
    {
        return -1;
    }
    *((uint8_t *)buf) = gpios[port_index].active;
    return 1;
}

int8_t  GPIOWrite(int32_t port, void *buf, uint32_t buf_len)
{
    uint8_t value = (*(uint8_t *)buf == 1)?GPIO_PIN_SET:GPIO_PIN_RESET;
    switch (port)
    {
//        case GPIO_OUT_01: LS595OutPut(LS595_IO_OUT_01, value);  break;
//        case GPIO_OUT_02: LS595OutPut(LS595_IO_OUT_02, value);  break;
//        case GPIO_OUT_03: LS595OutPut(LS595_IO_OUT_03, value);  break;
//        case GPIO_OUT_04: LS595OutPut(LS595_IO_OUT_04, value);  break;
//        case GPIO_OUT_05: LS595OutPut(LS595_IO_OUT_05, value);  break;
//        case GPIO_OUT_06: LS595OutPut(LS595_IO_OUT_06, value);  break;
//        case GPIO_OUT_07: LS595OutPut(LS595_IO_OUT_07, value);  break;
//        case GPIO_OUT_08: LS595OutPut(LS595_IO_OUT_08, value);  break;
        case ST_LED: GPIO_WriteBit(RUN_STU_GPIO_Port, RUN_STU_Pin, (BitAction)value); break;
        default : return -1;
    }    
    return 0;
}
int8_t  GPIOIoctl(int32_t port, uint32_t cmd, va_list argp)
{
    return -1;
}


/* USER CODE END 2 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
