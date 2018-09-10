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
#include "modbus_master.h"




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


extern float cur_filt_voltage_value[8];
//  传感器数据映射
//  端口定义: 0:  未使用
//            1-0x3F: MODBUS#1-#63  MODBUS接口的地址1到地址63
//            0x40-0x5F: ADC#1-#32   ADC1-ADC32口
//            0x60:   COM1: 接COM1口
//            0x70:   COM2: 接COM2口
//            0x80-0x8F:   RS485-1#1-#16: RS485-1的其他协议地址1到地址16
//            0x90-0x9F:   RS485-2#1-#16: RS485-2的其他协议地址1到地址16

int8_t SensorMapping(uint8_t channel, float *r_value)
{
    uint16_t ret_buffer[10];
//    float value;
    int8_t ret;
    if (channel == 0)
    {
        return -1;
    }
    if (channel <= 0x3F)
    {
        while ((ret = ModbusReadR(channel, 0x02, 0x09, ret_buffer)) == -1)
        {
            OSTimeDly(OS_TICKS_PER_SEC/100);
        }
        if (ret == 0)
        {
            memcpy(r_value, ret_buffer, 4);
            return 1;
        }
        return -1;
    }
    else if ((0x40 < channel) && (channel <= 0x5F))
    {
        if (channel < 0x48)
        {
            *r_value = cur_filt_voltage_value[channel-0x40];
            return 1;
        }
    }
    else if (channel == 0x60)
    {
        
    }
    else if (channel == 0x70)
    {
        
    }
    else if ((0x80 <= channel) && (channel <= 0x8F))
    {
        
    }
    else if ((0x80 <= channel) && (channel <= 0x8F))
    {
        
    }
    return -1;
}

#define UpdateSensor(name, value)  \
    if (SensorMapping(sys_config_ram.sensor.ctrl.##name##.port, &##value##) > 0)\
    {\
        sys_config_ram.sensor.ctrl.##name##_value = ##value##;\
    }

void UpdateSensors(void)
{
    float value;
    UpdateSensor(t, value);
    UpdateSensor(ph, value);
    UpdateSensor(cod, value);
    UpdateSensor(orp, value);
    UpdateSensor(nh3, value);
    UpdateSensor(DO, value);
    UpdateSensor(ss, value);
    UpdateSensor(p, value);
    UpdateSensor(flux, value);
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
    IoOpen(COM3, &com_arg, sizeof(s_UartStr_t));
    IoOpen(COM6, &com_arg, sizeof(s_UartStr_t));
    while (1)
    {
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
        UpdateSensors();
        //{
        //    float value;
        //    SensorMapping(channel_modbus_test, &value);
        //}
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

