#include "server_comm.h"
#include "string.h"
#include "stdio.h"
#include "stm32f4xx.h"
#include "ucos_ii.h"
#include "sys_config.h"
#include "gprs_socket.h"
#include "mod_time.h"
#include "crc.h"



/**
 *
 上传数据格式（88字节）
 -------------------------------------------------
 
 23 23                         固定开始##                                   2
 
 32 30 31 38 30 39 30 34       2018/09/04（日期字符）                       8 
 
 30 37 30 28 30 39             07:08:09(时间字符)                           6
 
 30 30 38 38                   包长度字符型（88）                           4 
 
 00 0c                         站点编号（高位:站点号/256,低位：站点号%256） 2   
 
 09 02 00 00                   温度（float型）                              4
 
 4D 02 00 00                   PH（float型）                                4
 
 77 02 00 00                   orp（float型）                               4
 
 C7 00 00 00                   do （float型）                               4
 
 BA 00 00 00                   flow (float型)                               4
 
 92 8A 10 00                   泵的期望状态（int型，值：101010）            4
 
 92 8A 10 00                   泵的实际状态（int型，值：101010）            4
 
 CD CC CC CC CC 4C 3E 40       瞬时流量（double型，值：30.3）               8
 
 CD CC CC CC CC 4C 3E 40       总流量（double型， 值：30.3）                8
 
 66 66 92 41                   cod（float型，值：18.3）                     4
 
 66 66 92 41                   nh3（float型，值：18.3）                     4
 
 66 66 92 41                   p  （float型，值：18.3）                     4
 
 66 66 92 41                   ss（float型，值：18.3）                      4
 
 00 65                         液位（byte型，值：101 高液位：1，低液位0）2  2 // 1,1接通, 1，3接通为高液位;1，4接通为高夜位
 
 4E 0F                         crc校验位                                    2
 
 0D 0A                         '\r''\n' (固定结尾)                          2
 **/

uint8_t comm_data[100];
void ServerCommFramingData(void)
{
    double_t temp_db;
    uint32_t temp;
    struct tm tm_time;
    uint8_t index = 0;
    comm_data[index++] = 0x23;
    comm_data[index++] = 0x23;
    temp = time(0);
    tm_time = *localtime(&temp);
    sprintf((char *)&comm_data[index], "%04u%02u%02u%02u%02u%02u%04u",
        tm_time.tm_year, tm_time.tm_mon, tm_time.tm_mday,
        tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
        88);  //  数据长度
    index+=18;
    comm_data[index++] = (uint8_t)(sys_config_ram.reg_group_1.device_id>>8);
    comm_data[index++] = (uint8_t)(sys_config_ram.reg_group_1.device_id);
    memcpy(&temp, &sys_config_ram.sensor.ctrl.t_value, 4);
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    memcpy(&temp, &sys_config_ram.sensor.ctrl.ph_value, 4);
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    memcpy(&temp, &sys_config_ram.sensor.ctrl.orp_value, 4);
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    memcpy(&temp, &sys_config_ram.sensor.ctrl.DO_value, 4);
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    memcpy(&temp, &sys_config_ram.sensor.ctrl.flux_value, 4);  // flow是什么
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    //  泵控制状态
    temp = 0;
    if (pump_state.pump_st.pump_lift != 0)
    {
        temp = 100000;
    }
    if (pump_state.pump_st.pump_subm != 0)
    {
        temp += 10000;
    }
    if (pump_state.pump_st.pump_rflx != 0)
    {
        temp += 1000;
    }
    if (pump_state.pump_st.pump_watr != 0)
    {
        temp += 100;
    }
    if (pump_state.pump_st.pump_dosg != 0)
    {
        temp += 10;
    }
    if (pump_state.pump_st.pump_aera != 0)
    {
        temp += 1;
    }
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    //  泵实际状态
    temp = 0;
    if (pump_state.pump_st.pump_lift == 1)
    {
        temp = 100000;
    }
    if (pump_state.pump_st.pump_subm == 1)
    {
        temp += 10000;
    }
    if (pump_state.pump_st.pump_rflx == 1)
    {
        temp += 1000;
    }
    if (pump_state.pump_st.pump_watr == 1)
    {
        temp += 100;
    }
    if (pump_state.pump_st.pump_dosg == 1)
    {
        temp += 10;
    }
    if (pump_state.pump_st.pump_aera == 1)
    {
        temp += 1;
    }
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    //  瞬时流量
    temp_db = sys_config_ram.sensor.ctrl.flux_value;
    memcpy(&comm_data[index], &temp_db, 8);
    index+=8;
    //  时间流量
    temp_db = sys_config_ram.sensor.ctrl.flux_total;
    memcpy(&comm_data[index], &temp_db, 8);
    index+=8;
    
    memcpy(&temp, &sys_config_ram.sensor.ctrl.cod_value, 4);
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    memcpy(&temp, &sys_config_ram.sensor.ctrl.nh3_value, 4);
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    memcpy(&temp, &sys_config_ram.sensor.ctrl.p_value, 4);
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    memcpy(&temp, &sys_config_ram.sensor.ctrl.ss_value, 4);
    memcpy(&comm_data[index], &temp, 4);
    index+=4;
    if (yw_input & 0x4)    temp = 1*100;
    if (yw_input & 0x1)    temp = temp + 1*10;
    if (yw_input & 0x10)   temp = temp + 1;
    memcpy(&comm_data[index], &temp, 2);
    index+=2;
    //  CRC
    temp = CRC16(0xFFFF, comm_data, 84);
    memcpy(&comm_data[index], &temp, 2);
    index+=2;
    //  结束符
    memcpy(&comm_data[index], "\r\n", 2);
    index+=2;
}


int8_t ServerConnect(void)
{
    char ip_c[16];
    uint8_t * ip_p = (uint8_t *)&sys_config_ram.reg_group_1.server_ip;
    sprintf(ip_c, "%u.%u.%u.%u", ip_p[3], ip_p[2], ip_p[1], ip_p[0]);
    return GprsSocketConnect(ip_c, sys_config_ram.reg_group_1.server_port);
}


void TaskServerComm(void *pdata)
{
    uint8_t state = 0;
    uint32_t intevel = clock();
    pdata = pdata;
    while (1)
    {
        OSTimeDly(OS_TICKS_PER_SEC);
        switch (state)
        {
            case 0:
                sys_config_ram.coil_g1.ctrl.net = 0;
                if (ServerConnect() == 1)
                {
                    sys_config_ram.coil_g1.ctrl.net = 1;
                    state = 1;
                }
                break;
            case 1:
                if ((ComputeTickTime(intevel) > sys_config_ram.reg_group_1.linkup_tmo*OS_TICKS_PER_SEC) || 
                    (sys_config_ram.coil_g1.ctrl.communication))
                {
                    sys_config_ram.coil_g1.ctrl.communication = 0;
                    ServerCommFramingData();
                    if (GprsSocketSend(comm_data, 88) <= 0)
                    {
                        state = 1;
                    }
                    intevel = clock();
                }
                break;
            default:
                state = 0;
                break;
        }
    }
}




