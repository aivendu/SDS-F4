#include "ls595.h"
#include "cj01_gpio.h"
#include "stm32f4xx_hal.h"

static uint16_t ls595_pin_status;

/**  @breif  设置595的输出值
  *  @param  pin:  需要设置的管脚  @ref ls595_pin
  *  @param  state:  输出的状态
  *          @arg  LS595_RESET:   设置为低电平
  *          @arg  LS595_SET:     设置为高电平
  *          @arg  LS595_REALTIME_OUT:  立即输出到物理引脚
  */
void LS595OutPut(uint16_t pin, uint8_t state)
{
    if ((state & LS595_SET) == LS595_SET)
    {
        ls595_pin_status |= (1 << pin);
    }
    else
    {
        ls595_pin_status &= ~(1 << pin);
    }
    if ((state & LS595_REALTIME_OUT) == LS595_REALTIME_OUT)
    {
        LS595Update(0);
    }
}

/**  @breif  更新595输出，支持两片595级联
  *  @param  value: 需要输出的值，如果第二片595有输出高电平，需要把IO的电源脚使能，
  *  每位对应595的一个引脚，引脚定义参考 @ref ls595_pin
  */
void LS595Update(uint16_t * value)
{
    int i;
    if (value)
    {
        ls595_pin_status = *value;
    }
    
    if (ls595_pin_status & 0xFF00)
    {
        ls595_pin_status |= (1 << LS595_PWR_IO_EN);
        __set_PRIMASK(1);
        GPIO_WriteBit(LS595_LCLK_GPIO_Port, LS595_LCLK_Pin, GPIO_PIN_RESET);
        for (i=0; i<16; i++)
        {
            GPIO_WriteBit(LS595_SCLK_GPIO_Port, LS595_SCLK_Pin, GPIO_PIN_RESET);
            if ((ls595_pin_status<<i) & 0x8000)  //  先处理高位
            {
                //  输出 高电平
                GPIO_WriteBit(LS595_SER_GPIO_Port, LS595_SER_Pin, GPIO_PIN_SET);
            }
            else
            {
                //  输出 低电平
                GPIO_WriteBit(LS595_SER_GPIO_Port, LS595_SER_Pin, GPIO_PIN_RESET);
            }
            GPIO_WriteBit(LS595_SCLK_GPIO_Port, LS595_SCLK_Pin, GPIO_PIN_SET);  //  上升沿时数据移位
        }
        GPIO_WriteBit(LS595_LCLK_GPIO_Port, LS595_LCLK_Pin, GPIO_PIN_SET);  //  上升沿时数据输出
        for (i=0; i<10; i++);  //  延时
    }
    GPIO_WriteBit(LS595_LCLK_GPIO_Port, LS595_LCLK_Pin, GPIO_PIN_RESET);
    for (i=0; i<16; i++)
    {
        GPIO_WriteBit(LS595_SCLK_GPIO_Port, LS595_SCLK_Pin, GPIO_PIN_RESET);
        if ((ls595_pin_status<<i) & 0x8000)  //  先处理高位
        {
            //  输出 高电平
            GPIO_WriteBit(LS595_SER_GPIO_Port, LS595_SER_Pin, GPIO_PIN_SET);
        }
        else
        {
            //  输出 低电平
            GPIO_WriteBit(LS595_SER_GPIO_Port, LS595_SER_Pin, GPIO_PIN_RESET);
        }
        GPIO_WriteBit(LS595_SCLK_GPIO_Port, LS595_SCLK_Pin, GPIO_PIN_SET);  //  上升沿时数据移位
    }
    GPIO_WriteBit(LS595_LCLK_GPIO_Port, LS595_LCLK_Pin, GPIO_PIN_SET);  //  上升沿时数据输出
    __set_PRIMASK(0);
}



