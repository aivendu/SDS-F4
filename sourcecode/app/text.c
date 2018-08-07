#include "stdint.h"
#include "ff.h"
#include "sys_config.h"
#include "ucos_ii.h"
#include "debug_log.h"
#include "cj01_hardware.h"
#include "cj01_io_api.h"
#include "chip_communication.h"
#include "stdint.h"
#include "modbus_register.h"
#include "pump.h"
#include "a2o.h"
#include "spi_uart.h"
#include "sys_task_config.h"




uint16_t relay_ctl;
void UpdateRelayCtl(void)
{
    uint16_t temp = relay_out;//relay_ctl ^ relay_out;
    uint16_t flag = relay_ctl ^ temp;
    uint8_t  relay_ctl_data[14];
    memset(relay_ctl_data, 0, sizeof(relay_ctl_data));
    if (flag)
    {
        PumpCtrl(flag & temp, 1);     //  开
        PumpCtrl(flag & (~temp), 0);  //  关
    }
}

int8_t ReadYWInput(uint8_t port)
{
    uint8_t temp;
    
    IoOpen(port, 0, 0);
    IoRead(port, &temp, 1);
    IoClose(port);
    return temp;
}


void UpdataYWInput(void)
{
    uint16_t yw_in = 0;
//    uint8_t temp;
    if (ReadYWInput(GPIO_IN_01))
    {
        yw_in |= 1<<0;
    }
    if (ReadYWInput(GPIO_IN_02))
    {
        yw_in |= 1<<1;
    }
    if (ReadYWInput(GPIO_IN_03))
    {
        yw_in |= 1<<2;
    }
    if (ReadYWInput(GPIO_IN_04))
    {
        yw_in |= 1<<3;
    }
    if (ReadYWInput(GPIO_IN_05))
    {
        yw_in |= 1<<4;
    }
    if (ReadYWInput(GPIO_IN_06))
    {
        yw_in |= 1<<5;
    }
    if (ReadYWInput(GPIO_IN_07))
    {
        yw_in |= 1<<6;
    }
    if (ReadYWInput(GPIO_IN_08))
    {
        yw_in |= 1<<7;
    }
    yw_input = yw_in;
}


uint8_t  open[14];
void TaskInit(void *pdata)
{
    s_UartStr_t com_arg = {9600, 8,0,1};
    OSTimeDly(1000);
    InitFileSystem();
    InitSpiUart(TaskSpiUartPrio);
    IoOpen(COM3, &com_arg, sizeof(s_UartStr_t));
    IoOpen(COM6, &com_arg, sizeof(s_UartStr_t));
    while (1)
    {
        //Printf_D("", "-------- system start ----------\r\n");
        //OSTimeDly(5000);
        //IoWrite(COM3, "-------- COM3 ----------\r\n", 27);
        //IoWrite(COM6, "-------- COM6 ----------\r\n", 27);
        //memset(open, 0x01, sizeof(open));
        //ChipWriteFrame(0, 0, 14, open);
        //OSTimeDly(5000);
            
        //memset(open, 0x02, sizeof(open));
        //ChipWriteFrame(0, 0, 14, open);
        //ChipReadFrame(1, 0, 14, open);
        //ChipWriteFrame(3, 0, 14, open);
        //ChipReadFrame(3, 0, 14, open);
        
        //{
        //    uint8_t len = SpiUart2Read(0, open, 14);
        //    SpiUart2Write(0, open, len);
        //}
        
        if (sys_config_ram.coil_g1.ctrl.manual)  //  是否进入手动控制
        {
            UpdateRelayCtl();
        }
        UpdataYWInput();
        if (process_technology_type == 0)
        {
            FlowA2O();
        }
        OSTimeDly(10);
        
        if (coil_group_1.ctrl.save)
        {
            coil_group_1.ctrl.save = 0;
            SaveSysConfig(sys_config_ram);
        }
        if (coil_group_1.ctrl.calib)
        {
            coil_group_1.ctrl.calib = 0;
            SaveSensorCfg();
        }
    }
}

#include "sys_task_config.h"
#define TaskInitSize        1024
OS_STK TaskInitStk[TaskInitSize]; 
void AppInit(void)
{
    memset(TaskInitStk, 0x55, sizeof(TaskInitStk));
    OSTaskCreate(TaskInit, (void *)0, &TaskInitStk[TaskInitSize - 1], TaskInitPrio);
}

