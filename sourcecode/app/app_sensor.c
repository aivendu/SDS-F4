#include "app_sensor.h"
#include "sys_config.h"
#include "ucos_ii.h"
#include "modbus_master.h"
#include "mbmagcp.h"
#include "mod_time.h"


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
    else if ((0x40 <= channel) && (channel <= 0x5F))
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


#define GenerateSensorValue(name)  \
{\
    sys_config_ram.sensor.ctrl.##name##_value += \
        (sys_config_ram.sensor.ctrl.##name##.accuracy * ((clock()&0xFF)-0x80) / 0xFF);\
    if (sys_config_ram.sensor.ctrl.##name##_value > sys_config_ram.sensor.ctrl.##name##.max)\
    {\
        sys_config_ram.sensor.ctrl.##name##_value = sys_config_ram.sensor.ctrl.##name##.max;\
    }\
    else if (sys_config_ram.sensor.ctrl.##name##_value < sys_config_ram.sensor.ctrl.##name##.min)\
    {\
        sys_config_ram.sensor.ctrl.##name##_value = sys_config_ram.sensor.ctrl.##name##.min;\
    }\
}



#define UpdateSensor(name, value)  \
    if (sys_config_ram.coil_g1.ctrl.install_##name##)\
    {\
        if (SensorMapping(sys_config_ram.sensor.ctrl.##name##.port, &##value##) > 0)\
        {\
            sys_config_ram.sensor.ctrl.##name##_value = ##value##;\
        }\
    }\
    else\
    {\
        GenerateSensorValue(name);\
    }



void TaskSensor(void *pdata)
{
    float value;
    while (1)
    {
        //UpdateSensor(t, value);
        //UpdateSensor(ph, value);
        //UpdateSensor(cod, value);
        //UpdateSensor(orp, value);
        //UpdateSensor(nh3, value);
        //UpdateSensor(DO, value);
        //UpdateSensor(ss, value);
        //UpdateSensor(p, value);
        
        if (sys_config_ram.sensor.ctrl.flux.port == 0x70)
        {
            if (sys_config_ram.coil_g1.ctrl.install_flux)
            {
                MBmagCPReadFlux(&sys_config_ram.sensor.ctrl.flux_value);
                OSTimeDly(OS_TICKS_PER_SEC/10);
                MBmagCPReadTotalFlux(&sys_config_ram.sensor.ctrl.flux_total);
            }
            else
            {
                GenerateSensorValue(flux);
            }
        }
        else
        {
            UpdateSensor(flux, value);
        }
        
        OSTimeDly(OS_TICKS_PER_SEC);
    }
}



