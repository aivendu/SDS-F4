#include "app_sensor.h"
#include "sys_config.h"
#include "ucos_ii.h"
#include "modbus_master.h"
#include "mbmagcp.h"
#include "my_time.h"
#include "stm32f4xx.h"


extern float cur_filt_voltage_value[8];


float_t ComputerSensorValue(int32_t value, s_sensor_ctrl_t *ctrl)
{
    float_t temp;
    temp = (ctrl->calib1 - ctrl->calib2) / (ctrl->sampling1 - ctrl->sampling2);
    temp = temp * value;
    return temp;
}


float_t SensorAdjust(float_t value, s_sensor_ctrl_t *ctrl)
{
    int32_t temp;
    //if (sys_config_ram.coil_g1.ctrl.install_t)  //  安装了设备，但是没有检测到数据时
    {
        srand(SysTick->VAL);
        temp = rand();
        if (temp & 0x04)
        {
            value -= (temp & 3) * ctrl->accuracy;
        }
        else
        {
            value += (temp & 3) * ctrl->accuracy;
        }
        if (value < ctrl->min)
        {
            value = ctrl->min;
        }
        else if (value > ctrl->max)
        {
            value = ctrl->max;
        }
    }
    return value;
}


//  传感器数据映射
//  端口定义: 0:  未使用
//            1-0x3F: MODBUS#1-#63  MODBUS接口的地址1到地址63
//            0x40-0x5F: ADC#1-#32   ADC1-ADC32口
//            0x60:   COM1: 接COM1口
//            0x70:   COM2: 接COM2口
//            0x80-0x8F:   RS485-1#1-#16: RS485-1的其他协议地址1到地址16
//            0x90-0x9F:   RS485-2#1-#16: RS485-2的其他协议地址1到地址16

int8_t SensorMapping(float *r_value, s_sensor_ctrl_t *ctrl)
{
    uint16_t ret_buffer[10];
//    float value;
    int8_t ret;
    uint8_t channel = ctrl->port;
    if (channel == 0)
    {
        return -1;
    }
    if (channel <= 0x3F)
    {
        //  第0-3位：吸光度，浮点数4个字节内存地址的内容
        //  第4-7位：当前值，浮点数4个字节内存地址的内容
        //  第8-17位：采样时间

        while ((ret = ModbusReadR(channel, 0x02, 0x09, ret_buffer)) == -1)
        {
            OSTimeDly(OS_TICKS_PER_SEC/100);
        }
        if (ret == 0)
        {
            memcpy(r_value, ret_buffer+2, 4);  
            if ((*r_value >= ctrl->min) && (*r_value <= ctrl->max))  //  采样值不在范围则认为采样失败
            {
                return 1;
            }
        }
        return -1;
    }
    else if ((0x40 <= channel) && (channel <= 0x5F))
    {
        if (channel < 0x48)
        {
            *r_value = cur_filt_voltage_value[channel-0x40];
            //*r_value = ComputerSensorValue(cur_filt_voltage_value[channel-0x40], ctrl);
            if ((*r_value < ctrl->min) || (*r_value > ctrl->max))  //  采样值不在范围则认为采样失败
            {
                return -1;
            }
            return 2;
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
    if (sys_config_ram.coil_g1.ctrl.install_##name##)\
    {\
        if (SensorMapping(&##value##, &sys_config_ram.sensor.ctrl.##name##) > 0)\
        {\
            sys_config_ram.sensor.ctrl.##name##_value = ##value##;\
        }\
        else\
        {\
            sys_config_ram.sensor.ctrl.##name##_value = \
                        SensorAdjust(sys_config_ram.sensor.ctrl.##name##_value, \
                                     &sys_config_ram.sensor.ctrl.##name##);\
        }\
    }\
    else\
    {\
        sys_config_ram.sensor.ctrl.##name##_value = \
                        SensorAdjust(sys_config_ram.sensor.ctrl.##name##_value, \
                                     &sys_config_ram.sensor.ctrl.##name##);\
    }


void TaskSensor(void *pdata)
{
    float value;
    uint32_t temp;
    
    pdata = pdata;
    sys_config_ram.coil_g1.ctrl.flux_err = 0;
    while (1)
    {
        UpdateSensor(t, value);
        UpdateSensor(ph, value);
        UpdateSensor(cod, value);
        UpdateSensor(orp, value);
        UpdateSensor(nh3, value);
        UpdateSensor(DO, value);
        UpdateSensor(ss, value);
        UpdateSensor(tp, value);
        
        if (sys_config_ram.coil_g1.ctrl.install_flux)
        {
            if (sys_config_ram.sensor.ctrl.flux.port == 0x70)
            {
                if (MBmagCPReadFlux(&sys_config_ram.sensor.ctrl.flux_value) == -1)
                {
                    sys_config_ram.coil_g1.ctrl.flux_err = 1;
                    memset(&sys_config_ram.sensor.ctrl.flux_value, 0xFF, 4);
                }
                else
                {
                    OSTimeDly(OS_TICKS_PER_SEC/10);
                    if (MBmagCPReadTotalFlux(&sys_config_ram.sensor.ctrl.flux_total))
                    {
                        sys_config_ram.coil_g1.ctrl.flux_err = 1;
                        memset(&sys_config_ram.sensor.ctrl.flux_total, 0xFF, 4);
                    }
                    
                    temp = (uint32_t)(sys_config_ram.sensor.ctrl.flux_value * 100);
                    sys_config_ram.sensor.ctrl.flux_value = temp / 100;
                    temp = (uint32_t)(sys_config_ram.sensor.ctrl.flux_total * 100);
                    sys_config_ram.sensor.ctrl.flux_value = temp / 100;
                    sys_config_ram.coil_g1.ctrl.flux_err = 0;
                }
            }
            else
            {
                if (SensorMapping(&sys_config_ram.sensor.ctrl.flux_value, &sys_config_ram.sensor.ctrl.flux) <= 0)
                {
                    sys_config_ram.coil_g1.ctrl.flux_err = 1;
                    memset(&sys_config_ram.sensor.ctrl.flux_value, 0xFF, 4);
                    memset(&sys_config_ram.sensor.ctrl.flux_total, 0xFF, 4);
                }
                else
                {
                    temp = (uint32_t)(sys_config_ram.sensor.ctrl.flux_value * 100);
                    sys_config_ram.sensor.ctrl.flux_value = temp / 100;
                    temp = (uint32_t)(sys_config_ram.sensor.ctrl.flux_total * 100);
                    sys_config_ram.sensor.ctrl.flux_value = temp / 100;
                    sys_config_ram.coil_g1.ctrl.flux_err = 0;
                }
            }
        }
        else
        {
            sys_config_ram.sensor.ctrl.flux_total = 0;
            sys_config_ram.sensor.ctrl.flux_value = 0;
        }
        
        OSTimeDly(OS_TICKS_PER_SEC*59);
    }
}



