#include "mini_pcie.h"
#include "cj01_gpio.h"
#include "cj01_usart.h"
#include "cj01_io_api.h"
#include "stm32f4xx_gpio.h"
#include "mod_time.h"

#define SetSIM_PWRKEY()   GPIO_SetBits(SIM_PWRKEY_GPIO_Port, SIM_PWRKEY_Pin)
#define ReSetSIM_PWRKEY() GPIO_ResetBits(SIM_PWRKEY_GPIO_Port, SIM_PWRKEY_Pin)
#define GetSIM_PWRKEY()   GPIO_ReadOutputDataBit(SIM_PWRKEY_GPIO_Port, SIM_PWRKEY_Pin)
#define GetSIM_WWAN()     GPIO_ReadInputDataBit(SIM_WWAN_GPIO_Port, SIM_WWAN_Pin)

static uint32_t powerkey_high_time, powerkey_low_time;
static uint8_t minipci_sim_status;
static uint16_t SimcommPowerOn(char *data)
{
    if (minipci_sim_status >= 1)
    {
        powerkey_low_time = clock();
        //ReSetSIM_PWRKEY();
        //Printf_D("gprs", "power on successufully!\r\n");
        return 1;
    }
    else
    {
        if (GetSIM_PWRKEY() == 1)
        {
            if (ComputeTickTime(powerkey_high_time) > 1000)
            {
                powerkey_low_time = clock();
                ReSetSIM_PWRKEY();
            }
        }
        else
        {
            if (ComputeTickTime(powerkey_low_time) > 30000)
            {
                powerkey_high_time = clock();
                SetSIM_PWRKEY();
                //Printf_D("gprs", "power on timeout!\r\n");
            }
        }
    }
    return 0;
}

static uint16_t SimcommPowerOff(char *data)
{
    if (minipci_sim_status == 0)
    {
        powerkey_low_time = clock();
        ReSetSIM_PWRKEY();
        //Printf_D("gprs", "power off successfully!\r\n");
        return 1;
    }
    else
    {
        if (GetSIM_PWRKEY() == 1)
        {
            if (ComputeTickTime(powerkey_high_time) > 1000)
            {
                powerkey_low_time = clock();
                ReSetSIM_PWRKEY();
            }
        }
        else
        {
            if (ComputeTickTime(powerkey_low_time) > 30000)
            {
                powerkey_high_time = clock();
                SetSIM_PWRKEY();
                //Printf_D("gprs", "power off timeout!\r\n");
            }
        }
    }
    return 0;
}

uint32_t sim_wwan_high_time, sim_wwan_low_time;
void CheckWWANState(void)
{
    uint32_t time_interval;
    if (GetSIM_WWAN() == 0)
    {
        if (sim_wwan_high_time == 0)  sim_wwan_high_time = clock();
        time_interval = ComputeTickTime(sim_wwan_high_time);
        if (time_interval >= 900)
        {
            minipci_sim_status = 3;
        }
        else
        {
            time_interval = ComputeTickTime(sim_wwan_low_time);
            if (time_interval >= 700)
            {
                minipci_sim_status = 1;
            }
            else if (time_interval >= 180)
            {
                minipci_sim_status = 2;
            }
        }
        sim_wwan_low_time = 0;
    }
    else
    {
        if (sim_wwan_low_time == 0)  sim_wwan_low_time = clock();
        time_interval = ComputeTickTime(sim_wwan_low_time);
        if (time_interval >= 900)
        {
            minipci_sim_status = 0;
        }
        else
        {
            time_interval = ComputeTickTime(sim_wwan_high_time);
            if (time_interval >= 750)
            {
                minipci_sim_status = 1;
            }
            else if (time_interval >= 180)
            {
                minipci_sim_status = 2;
            }
        }
        sim_wwan_high_time = 0;
    }
}


int8_t  MiniPcieOpen(int32_t port, const void *config, uint8_t len)
{
    return UartOpen(COM3, config, len);
}

int8_t  MiniPcieClose(int32_t port)
{
    return 1;
}

int32_t MiniPcieRead(int32_t port, void *buf, uint32_t buf_len)
{
    return UartRead(COM3, buf, buf_len);
}

int8_t MiniPcieWrite(int32_t port, void *buf, uint32_t buf_len)
{
    return UartWrite(COM3, buf, buf_len);
}

int8_t  MiniPcieIoctl(int32_t port, uint32_t cmd, va_list args)
{
    uint8_t *data = va_arg(args, uint8_t*);
    uint32_t len  = va_arg(args, uint32_t);
    uint32_t i;
    switch (cmd)
    {
        case MINIPCIE_POWRE_ON:
            return SimcommPowerOn(0);
            //break;
        case MINIPCIE_POWRE_OFF:
            return SimcommPowerOff(0);
            //break;
        case MINIPCIE_WWAN_STATE:
            return minipci_sim_status;
            //break;
        case MINIPCIE_SENDDATA_REALTIME:
            for (i=0; i<len; i++)
            {
                if(USART_GetFlagStatus(USART3, USART_FLAG_TXE) != RESET)                
                {
                    //UART1发送保持寄存器空
                    USART_SendData(USART3, data[i]); //通过外设 USART 发送单个数据 
                 }
            }
        default:
            break;
    }
    return -1;
}












