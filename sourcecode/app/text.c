#include "stdint.h"
#include "ff.h"
#include "sys_config.h"
#include "ucos_ii.h"
#include "my_debug.h"
#include "cj01_hardware.h"
#include "cj01_io_api.h"
#include "chip_communication.h"
#include "stdint.h"
#include "modbus_register.h"
#include "pump.h"
#include "a2o.h"
#include "spi_uart.h"
#include "sys_task_config.h"
#include "modbus_master.h"
#include "app_sensor.h"
#include "sys_timer.h"
#include "gprs.h"
#include "server_comm.h"
#include "mini_pcie.h"



uint16_t relay_ctl;
void UpdateRelayCtl(void)
{
    uint16_t temp = relay_out;//relay_ctl ^ relay_out;
    uint16_t flag = relay_ctl ^ temp;
    uint8_t  relay_ctl_data[14];
    memset(relay_ctl_data, 0, sizeof(relay_ctl_data));
    if (flag)
    {
        relay_ctl = temp;
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


void SIM7600Test(void)
{
    uint8_t state = 0;
    int32_t len = 0;
    uint8_t buffer[128];
    s_UartStr_t com_arg = {38400, 8,0,1};
    OSTimeDly(OS_TICKS_PER_SEC * 2);
    IoOpen(COM1, &com_arg, sizeof(s_UartStr_t));
    IoOpen(MINI_PCIE, &com_arg, sizeof(s_UartStr_t));
    sprintf((char *)buffer, "123\r\n");
    IoWrite(COM1, buffer, 5);
    //while (Ioctl(MINI_PCIE, MINIPCIE_POWRE_ON) != 1) OSTimeDly(10);
    
    while (1)
    {
        len += IoRead(COM1, buffer, 128);
        if (len>0)
        {
            //IoWrite(COM1, buffer, len);
        }
        //len = IoRead(COM1, buffer, 128);
        //if (len>0)
        //{
        //    IoWrite(MINI_PCIE, buffer, len);
        //}
    }
}

int8_t channel_modbus_test;
uint8_t  open[14];
void TaskInit(void *pdata)
{
    s_UartStr_t com_arg = {9600, 8,0,1};
    OSTimeDly(1000);
    InitFileSystem();
    InitSpiUart(TaskSpiUartPrio);
    //IoOpen(COM1, &com_arg, sizeof(s_UartStr_t));
    //IoOpen(COM3, &com_arg, sizeof(s_UartStr_t));
    IoOpen(COM6, &com_arg, sizeof(s_UartStr_t));  // 初始化modbus串口
    PumpCtrl(0xFFFF, 0);
    IoOpen(WDT, (void *)10000, 0);
    
    while (1)
    {
        //SIM7600Test();
        //uint8_t temp = 0x55;
        //IoWrite(COM3, "AT\r\n",4);
        //OSTimeDly(1000);
        //Printf_D("", "-------- system start ----------\r\n");
        //OSTimeDly(5000);
        //IoWrite(COM1, "-------- COM1 ----------\r\n", 27);
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
        //continue;
        
        UpdateRelayCtl();
        UpdataYWInput();
        switch (process_technology_type)
        {
            case 1:
                FlowA2O();
                break;
        }
        OSTimeDly(10);
        
        if (coil_group_1.ctrl.save)
        {
            coil_group_1.ctrl.save = 0;
            SaveSysConfig(sys_config_ram);
        }
        if (coil_group_1.ctrl.back)
        {
            InitFileSystem();
            coil_group_1.ctrl.back = 0;
        }
        if (coil_group_1.ctrl.logout)
        {
            memset(permission, 0, sizeof(permission));
            coil_group_1.ctrl.logout = 0;
        }
        if (coil_group_1.ctrl.calib)
        {
            coil_group_1.ctrl.calib = 0;
            SaveSensorCfg();
        }
        if (coil_group_1.ctrl.init)
        {
            coil_group_1.ctrl.init = 0;
            sys_config_ram = sys_config_rom;
            SaveSysConfig(sys_config_ram);
        }
        if (coil_group_1.ctrl.reboot)
        {
            coil_group_1.ctrl.reboot = 0;
            sys_config_ram.reg_group_1.technology_type = 0;
            //SaveSensorCfg();
            SaveSystemCfg(0, (uint8_t *)&sys_config_ram, sizeof(sys_config_ram));
            OSTimeDly(OS_TICKS_PER_SEC/2);
            Reset();
        }
        IoWrite(WDT, 0, 0);
    }
}


#define  DefineTaskStk(name, size)    \
      OS_STK Task##name##Stk[size];\
      OS_STK *const Task##name##StkTop=&Task##name##Stk[##size## - 1];


#include "sys_task_config.h"

DefineTaskStk(Init, 1024);
DefineTaskStk(Sensor, 1024);
DefineTaskStk(Sim, 1024);
DefineTaskStk(ServerComm, 1024);

void AppInit(void)
{
    OSTaskCreate(TaskInit, (void *)0, TaskInitStkTop, TaskInitPrio);
    OSTaskCreate(TaskSensor, (void *)0, TaskSensorStkTop, TaskSensorPrio);
    OSTaskCreate(TaskSim, (void *)0, TaskSimStkTop, TaskSimPrio);
    OSTaskCreate(TaskServerComm, (void *)0, TaskServerCommStkTop, TaskServerCommPrio);
}

