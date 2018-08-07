#ifndef _GPIO_H_
#define _GPIO_H_

#define  KEY0      PE4
#define  KEY1      PE3
#define  KEY2      PE2
#define  WK_UP     PA0

#include "stdint.h"
#include "os_includes.h"


#define YW_IN1_Pin GPIO_PIN_2
#define YW_IN1_GPIO_Port GPIOE
#define YW_IN2_Pin GPIO_PIN_3
#define YW_IN2_GPIO_Port GPIOE
#define YW_IN3_Pin GPIO_PIN_4
#define YW_IN3_GPIO_Port GPIOE
#define YW_IN4_Pin GPIO_PIN_5
#define YW_IN4_GPIO_Port GPIOE
#define YW_IN5_Pin GPIO_PIN_6
#define YW_IN5_GPIO_Port GPIOE
#define YW_IN6_Pin GPIO_PIN_6
#define YW_IN6_GPIO_Port GPIOF
#define YW_IN7_Pin GPIO_PIN_7
#define YW_IN7_GPIO_Port GPIOF
#define YW_IN8_Pin GPIO_PIN_8
#define YW_IN8_GPIO_Port GPIOF
#define F1_PC12_Pin GPIO_PIN_9
#define F1_PC12_GPIO_Port GPIOF
#define SIM_PWRKEY_Pin GPIO_PIN_0
#define SIM_PWRKEY_GPIO_Port GPIOC
#define SIM_WWAN_Pin GPIO_PIN_2
#define SIM_WWAN_GPIO_Port GPIOC
#define F1_PD2_Pin GPIO_PIN_0
#define F1_PD2_GPIO_Port GPIOA
#define ADC_CUR_IN1_Pin GPIO_PIN_3
#define ADC_CUR_IN1_GPIO_Port GPIOA
#define ADC_CUR_IN2_Pin GPIO_PIN_4
#define ADC_CUR_IN2_GPIO_Port GPIOA
#define ADC_CUR_IN3_Pin GPIO_PIN_5
#define ADC_CUR_IN3_GPIO_Port GPIOA
#define ADC_CUR_IN4_Pin GPIO_PIN_6
#define ADC_CUR_IN4_GPIO_Port GPIOA
#define F1_SPI2_NSS_Pin GPIO_PIN_12
#define F1_SPI2_NSS_GPIO_Port GPIOB
#define W5500_INTn_Pin GPIO_PIN_6
#define W5500_INTn_GPIO_Port GPIOG
#define W5500_RSTn_Pin GPIO_PIN_7
#define W5500_RSTn_GPIO_Port GPIOG
#define W5500_DUPLED_Pin GPIO_PIN_8
#define W5500_DUPLED_GPIO_Port GPIOG
#define RUN_STU_Pin GPIO_PIN_8
#define RUN_STU_GPIO_Port GPIOA
#define ETH_RESET_Pin GPIO_PIN_3
#define ETH_RESET_GPIO_Port GPIOD
#define W5500_SPDLED_Pin GPIO_PIN_9
#define W5500_SPDLED_GPIO_Port GPIOG
#define W5500_SCSn_Pin GPIO_PIN_12
#define W5500_SCSn_GPIO_Port GPIOG
#define SDIO_CD_Pin GPIO_PIN_15
#define SDIO_CD_GPIO_Port GPIOG


#define GPIO_OUT            1                   //GPIO口为输入出
#define GPIO_IN             0                   //GPIO口为输入口

#define GPIO_OUT_H          1                   //GPIO口输出为高电平
#define GPIO_OUT_L          0                   //GPIO口输出为低电平

#if 1
extern void GPIODebounce(void);
extern void GPIOInit(void);
extern int8_t GPIOIoctl(int32_t port, uint32_t cmd, va_list args);
extern int8_t GPIOOpen(int32_t port, const void *config, uint8_t len);
extern int8_t GPIOClose(int32_t port);
extern int32_t GPIORead(int32_t port, void *buf, uint32_t buf_len);
extern int8_t GPIOWrite(int32_t port, void *buf, uint32_t buf_len);
#endif


#endif


