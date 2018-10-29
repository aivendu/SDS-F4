
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ucos_ii.h"
#include "cj01_usart.h"
#include "stm32f4xx.h"
#include "modbus_register.h"
#include "crc.h"
#include "modbus_slave.h"
#include "modbus_core.h"


#define CRC16_MODBUS_ENABLE      1
#define CRC_DNP_ENALBE           0
#define CRC32_ENABLE             0
//#include "modbus_crc.h"

uint32_t boot_program_addr;
uint32_t app_program_addr;

//extern s_flash_parameters_t * program_parameters;


s_modbus_register_t *modbus_register;
s_modbus_record_t   *modbus_record;
s_modbus_coils_t    *modbus_coil;
uint32_t reg_number;
uint32_t record_num;
uint32_t coils_num;
uint8_t modbus_datatimeout;//接收到两个串口中断的时间间隔
uint8_t device_addr=1;
uint8_t reboot_flag;    //  485重启标志


//func_code:功能码；addr：启始地址；len:数据的长度
//匹配功能码、启始地址、数据长度是否满足协议规定
static int8_t MatchModbusRegister(uint8_t func_code, uint16_t addr, uint16_t len)
{
    uint8_t i;
	uint8_t max_i;
	uint8_t min_i;
	min_i = 0;	
	i = reg_number / 2;
	max_i = reg_number; 
    if (modbus_register == NULL)
    {
        return -1;
    }
	if((addr == 0x0020)&&(func_code == 0x05)&&(len == 0x0001))//特殊处理0x0020的写操作
	{
		return 3;	
	}
	else if((addr == 0x0020)&&(func_code == 0x02)&&(len == 0x0008))//特殊处理0x0020的连续读操作
	{
		return 2;
	}
	else if((addr == 0x0021)&&(func_code == 0x05)&&(len == 0x0001))//特殊处理0x0021的写操作
	{
		return 4;
	}

    if ((modbus_register[0].start_addr <= addr) &&
       ((modbus_register[0].start_addr + modbus_register[0].length) > addr))  //
    {
        if (modbus_register[0].cmd_support[func_code / 32] & (1 << (func_code % 32)))
        {
            if (((modbus_register[0].start_addr + modbus_register[0].length) < (addr + len)) || (len == 0))
            {
                //return -2;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            //return -1;
        }
    }
    while (i != min_i)  // 判断是否到最后三个
    {
        //  采用二分法查找
        if ((modbus_register[i].start_addr > addr) || (modbus_register[i].length == 0))
        {
            //  需要找的code在小半区
            max_i = i;
            i = (i + min_i) >> 1;
        }
        else if (((modbus_register[i].length > 0) &&
                 ((modbus_register[i].start_addr + modbus_register[i].length) <= addr)) ||
                 ((modbus_register[i].length < 0) && (modbus_register[i].start_addr < addr)))
        {
            //  需要找的code在大半区
            min_i = i;
            i = (i + max_i) >> 1;
        }
        else
        {
            if (modbus_register[i].cmd_support[func_code / 32] & (1 << (func_code % 32)))
            {
                if ((modbus_register[i].length > 0) &&
                   ((modbus_register[i].start_addr + modbus_register[i].length) >= (addr + len)))
                {
                    return i;
                }
                else if ((modbus_register[i].length < 0) && (modbus_register[i].start_addr == addr))
                {
                    return i;
                }
                else
                {
                    return -2;
                }
            }
            else
            {
                return -1;
            }
        }
    }
    return -1;
}

//func_code:功能码；addr：启始地址；len:数据的长度
//匹配功能码、启始地址、数据长度是否满足协议规定
static int8_t MatchModbusCoils(uint8_t func_code, uint16_t addr, uint16_t len)
{
    uint8_t i;
	uint8_t max_i;
	uint8_t min_i;
	min_i = 0;	
	i = COILS_NUM / 2;
	max_i = COILS_NUM; 
    if (modbus_coil == NULL)
    {
        return -1;
    }
    if ((modbus_coil[0].start_addr <= addr) &&
       ((modbus_coil[0].start_addr + modbus_coil[0].coils_num) > addr))  //
    {
        if (modbus_coil[0].cmd_support[func_code / 32] & (1 << (func_code % 32)))
        {
            if (((modbus_coil[0].start_addr + modbus_coil[0].coils_num) < (addr + len)) || (len == 0))
            {
                //return -2;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            //return -1;
        }
    }
    while (i != min_i)  // 判断是否到最后三个
    {
        //  采用二分法查找
        if ((modbus_coil[i].start_addr > addr) || (modbus_coil[i].coils_num == 0))
        {
            //  需要找的code在小半区
            max_i = i;
            i = (i + min_i) >> 1;
        }
        else if (((modbus_coil[i].coils_num > 0) &&
                 ((modbus_coil[i].start_addr + modbus_coil[i].coils_num) <= addr)) ||
                 ((modbus_coil[i].coils_num < 0) && (modbus_coil[i].start_addr < addr)))
        {
            //  需要找的code在大半区
            min_i = i;
            i = (i + max_i) >> 1;
        }
        else
        {
            if (modbus_coil[i].cmd_support[func_code / 32] & (1 << (func_code % 32)))
            {
                if ((modbus_coil[i].coils_num > 0) &&
                   ((modbus_coil[i].start_addr + modbus_coil[i].coils_num) >= (addr + len)))
                {
                    return i;
                }
                else if ((modbus_coil[i].coils_num < 0) && (modbus_coil[i].start_addr == addr))
                {
                    return i;
                }
                else
                {
                    //return -2;
                }
            }
            else
            {
                //return -1;
            }
        }
    }
    return -1;
}

const uint32_t bit_mask[33] = 
{
    0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
    0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF,
    0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
    0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF,
    0xFFFFFFFF
};
void CopyCoilFromBuffer(uint16_t start, uint16_t size, uint32_t reg, uint8_t *data)
{
    uint8_t j;
    uint8_t offset = start & 0x07;
    uint16_t start_byte = start>>3;  //  16bit 对齐
    uint16_t temp;
    if (size == 0)
    {
        *data = 0;
        return ;
    }
    for (j=0; j<((size+7)>>3); j++)
    {
        temp = ((modbus_coil[reg].coils[start_byte+j] >> offset) & bit_mask[8-offset])
                   |(((modbus_coil[reg].coils[start_byte+j+1]) & bit_mask[offset]) << (8-offset));
        data[j] = temp & 0xFF;
    }
    if (size & 0x07) {
        data[(j-1)] &= bit_mask[size&0x07];    
    }
}
void CopyCoilToBuffer(uint16_t start, uint16_t size, uint32_t reg, uint8_t *data)
{
    uint8_t j;
    uint8_t offset = start & 0x07;
    uint16_t start_byte = start>>3;  //  16bit 对齐
    uint16_t temp;
    for (j=0; j<((size)>>3); j++)
    {
        temp = data[j] + (data[(j)+1] << 8);
        modbus_coil[reg].coils[start_byte+j] &= bit_mask[offset];
        modbus_coil[reg].coils[start_byte+j] |= (temp << offset);
        if (offset)
        {
            modbus_coil[reg].coils[start_byte+j+1] &= (~bit_mask[offset]);
            modbus_coil[reg].coils[start_byte+j+1] |= (temp >> (8-offset));
        }
    }
    if (size & 0x07)
    {
        //temp = data
    }
}

//  读多个线圈
uint32_t ModbusSlave_01_Reply(uint8_t code, uint8_t *dat)
{
    uint8_t *buff, *rec_dat;
    uint16_t start = (dat[2] << 8) + dat[3];
    uint16_t size = (dat[4] << 8) + dat[5];
    int8_t i = MatchModbusCoils(code, start, size);
    if (i < 0)
    {
        return -i;
    }
    buff = 0;
    while ((buff = malloc((size >> 3) + 6)) == 0)
    {
        OSTimeDly(1);
    }
    buff[1] = ModbusLocalAddr();
    buff[2] = 0x01;
    buff[3] = (size+7) >> 3;
    buff[0] = buff[3] + 3;

    if (modbus_coil[i].func)
    {
        rec_dat = (void *)modbus_coil[i].func(code, &dat[2]);
        if (rec_dat == 0)
        {
            if (modbus_coil[i].coils)
            {
                start -= modbus_coil[i].start_addr;
                CopyCoilFromBuffer(start, size, i, &buff[4]);
            }
            else
            {
                free(buff);
                return 0xFF;
            }
        }
        else if ((uint32_t)rec_dat < 0x100)
        {
            free(buff);
            return (uint32_t)rec_dat;
        }
        else
        {
            memcpy(&buff[4], (void *)rec_dat, buff[3]);
            free(rec_dat);
        }
    }
    else if (modbus_coil[i].coils)
    {
        start -= modbus_coil[i].start_addr;
        CopyCoilFromBuffer(start, size, i, &buff[4]);
    }
    else
    {
        free(buff);
        return 0xFF;
    }
    return (uint32_t)buff;
}

//  读多个输入
uint32_t ModbusSlave_02_Reply(uint8_t code, uint8_t *dat)
{
    uint8_t *buff, *rec_dat;
    uint16_t start = (dat[2] << 8) + dat[3];
    uint16_t size = (dat[4] << 8) + dat[5];
    int8_t i = MatchModbusCoils(code, start, size);
    if (i < 0)
    {
        return -i;
    }
    buff = 0;
    while ((buff = malloc((size >> 3) + 6)) == 0)
    {
        OSTimeDly(1);
    }
    buff[1] = ModbusLocalAddr();
    buff[2] = 0x02;
    buff[3] = (size+7) >> 3;
    buff[0] = buff[3] + 3;

    if (modbus_coil[i].func)
    {
        rec_dat = (void *)modbus_coil[i].func(code, &dat[2]);
        if (rec_dat == 0)
        {
            if (modbus_coil[i].coils)
            {
                start -= modbus_coil[i].start_addr;
                CopyCoilFromBuffer(start, size, i, &buff[4]);
            }
            else
            {
                free(buff);
                return 0xFF;
            }
        }
        else if ((uint32_t)rec_dat < 0x100)
        {
            free(buff);
            return (uint32_t)rec_dat;
        }
        else
        {
            memcpy(&buff[4], (void *)rec_dat, buff[3]);
            free(rec_dat);
        }
    }
    else if (modbus_coil[i].coils)
    {
        start -= modbus_coil[i].start_addr;
        CopyCoilFromBuffer(start, size, i, &buff[4]);
    }
    else
    {
        free(buff);
        return 0xFF;
    }
    return (uint32_t)buff;
}



//  读多个寄存器
uint32_t ModbusSlave_03_Reply(uint8_t code, uint8_t *dat)
{
    uint8_t *buff, *rec_dat;
    int8_t i = MatchModbusRegister(code,
                                    (dat[2] << 8) + dat[3],
                                    (dat[4] << 8) + dat[5]);
    if (i < 0)
    {
        return -i;
    }
    buff = 0;
    while ((buff = malloc(dat[5] * 2 + 4)) == 0)
    {
        OSTimeDly(1);
    }
    buff[1] = ModbusLocalAddr();
    buff[2] = 0x03;
    buff[3] = dat[5] * 2;
    buff[0] = buff[3] + 3;

    if (modbus_register[i].func)
    {
        rec_dat = (void *)modbus_register[i].func(&modbus_register[i], &dat[1]);
        if (rec_dat == 0)
        {
            if (modbus_register[i].buffer)
            {
                HalfWordBigEndianCopy(&buff[4],
                                      &modbus_register[i].buffer[(dat[2] << 8) + dat[3] - modbus_register[i].start_addr],
                                      buff[3]);
            }
            else
            {
                free(buff);
                return 0xFF;
            }
        }
        else if ((uint32_t)rec_dat < 0x100)
        {
            free(buff);
            return (uint32_t)rec_dat;
        }
        else
        {
            memcpy(&buff[4], (void *)rec_dat, buff[3]);
            free(rec_dat);
        }
    }
    else if (modbus_register[i].buffer)
    {
        HalfWordBigEndianCopy(&buff[4],
                              &modbus_register[i].buffer[(dat[2] << 8) + dat[3] - modbus_register[i].start_addr],
                              buff[3]);
    }
    else
    {
        free(buff);
        return 0xFF;
    }
    return (uint32_t)buff;
}

//  读输入
uint32_t ModbusSlave_04_Reply(uint8_t code, uint8_t *dat)
{
    uint8_t *buff;
    int8_t  i = MatchModbusRegister(code, (dat[2] << 8) + dat[3], 1);

    if (i < 0)
    {
        return -i;
    }

    if (modbus_register[i].func)
    {
        buff = (void *)modbus_register[i].func(&modbus_register[i], &dat[1]);
        if (buff == 0)
        {
        }
        else if ((uint32_t)buff < 0x100)
        {
            return (uint32_t)buff;
        }
    }
    if (modbus_register[i].buffer)
    {
		buff = (void *)(((dat[2] << 8) + dat[3]) - modbus_register[i].start_addr);
        modbus_register[i].buffer[(uint32_t)buff] = (dat[4] << 8) + dat[5];
    }
    while ((buff = malloc(8)) == 0)
    {
        OSTimeDly(1);
    }
    buff[0] = 6;
    buff[1] = ModbusLocalAddr();
    buff[2] = 0x06;
    buff[3] = dat[2];
    buff[4] = dat[3];
    buff[5] = dat[4];
    buff[6] = dat[5];
    return (uint32_t)buff;
}


//  写单个线圈
uint32_t ModbusSlave_05_Reply(uint8_t code, uint8_t *dat)
{
    uint8_t *buff;
    uint16_t addr = (dat[2] << 8) + dat[3];
    uint16_t value = (dat[4] << 8) + dat[5];
    int8_t  i = MatchModbusCoils(code, addr, 1);
    
    if (i < 0)
    {
        return -i;
    }
    if (value == 0xFF00)
    {
        if (modbus_coil[i].func)
        {
            buff = (void *)modbus_coil[i].func(code, &dat[2]);
            if (buff == 0)
            {
            }
            else if ((uint32_t)buff < 0x100)
            {
                return (uint32_t)buff;
            }
        }
        if (modbus_coil[i].coils)
        {
            addr -= modbus_coil[i].start_addr;
            modbus_coil[i].coils[addr>>3] |= (1<<(addr & 7));
        }
        
    }
    else if (value == 0)
    {
        if (modbus_coil[i].func)
        {
            buff = (void *)modbus_coil[i].func(code, &dat[2]);
            if (buff == 0)
            {
            }
            else if ((uint32_t)buff < 0x100)
            {
                return (uint32_t)buff;
            }
        }
        if (modbus_coil[i].coils)
        {
            addr -= modbus_coil[i].start_addr;
            modbus_coil[i].coils[addr>>3] &= (~(1<<(addr & 7)));
        }
        
    }
    else{
        return 2;
    }
    
    while ((buff = malloc(8)) == 0)
    {
        OSTimeDly(1);
    }
    buff[0] = 6;
    buff[1] = ModbusLocalAddr();
    buff[2] = code;
    buff[3] = dat[2];
    buff[4] = dat[3];
    buff[5] = dat[4];
    buff[6] = dat[5];
    return (uint32_t)buff;
}


//  写单个寄存器
uint32_t ModbusSlave_06_Reply(uint8_t code, uint8_t *dat)
{
    uint8_t *buff;
    int8_t  i = MatchModbusRegister(code, (dat[2] << 8) + dat[3], 1);

    if (i < 0)
    {
        return -i;
    }

    if (modbus_register[i].func)
    {
        buff = (void *)modbus_register[i].func(&modbus_register[i], &dat[1]);
        if (buff == 0)
        {
        }
        else if ((uint32_t)buff < 0x100)
        {
            return (uint32_t)buff;
        }
    }
    if (modbus_register[i].buffer)
    {
		buff = (void *)(((dat[2] << 8) + dat[3]) - modbus_register[i].start_addr);
        modbus_register[i].buffer[(uint32_t)buff] = (dat[4] << 8) + dat[5];
    }
    while ((buff = malloc(8)) == 0)
    {
        OSTimeDly(1);
    }
    buff[0] = 6;
    buff[1] = ModbusLocalAddr();
    buff[2] = 0x06;
    buff[3] = dat[2];
    buff[4] = dat[3];
    buff[5] = dat[4];
    buff[6] = dat[5];
    return (uint32_t)buff;
}

//  写多个线圈
uint32_t ModbusSlave_0F_Reply(uint8_t code, uint8_t *dat)
{
    uint8_t *buff;
    uint16_t start= (dat[2] << 8) + dat[3], size = (dat[4] << 8) + dat[5];
    int8_t  i = MatchModbusCoils(code,start,size);

    if (i < 0)
    {
        return -i;
    }
    if (modbus_coil[i].func)
    {
        buff = (void *)modbus_coil[i].func(code, &dat[2]);
        if (buff == 0)
        {
        }
        else if ((uint32_t)buff < 0x100)
        {
            return (uint32_t)buff;
        }
    }
    if (modbus_coil[i].coils)
    {
        start -= modbus_coil[i].start_addr;
        CopyCoilToBuffer(start, size, i, &dat[7]);
    }
    while ((buff = malloc(8)) == 0)
    {
        OSTimeDly(1);
    }
    buff[0] = 6;
    buff[1] = ModbusLocalAddr();
    buff[2] = code;
    buff[3] = dat[2];
    buff[4] = dat[3];
    buff[5] = dat[4];
    buff[6] = dat[5];
    return (uint32_t)buff;
}

//  写多个寄存器
uint32_t ModbusSlave_10_Reply(uint8_t code, uint8_t *dat)
{
    uint8_t *buff;
    int8_t  i = MatchModbusRegister(code,
                                     (dat[2] << 8) + dat[3],
                                     (dat[4] << 8) + dat[5]);

    if (i < 0)
    {
        return -i;
    }
    if (modbus_register[i].func)
    {
        buff = (void *)modbus_register[i].func(&modbus_register[i], &dat[1]);
        if (buff == 0)
        {
        }
        else if ((uint32_t)buff < 0x100)
        {
            return (uint32_t)buff;
        }
    }
    if (modbus_register[i].buffer)
    {
        //memset(modbus_register[i].buffer, 0, modbus_register[i].length*2);
        buff = (uint8_t *)&modbus_register[i].buffer[(dat[2] << 8) + dat[3] - modbus_register[i].start_addr];
        HalfWordBigEndianCopy(buff, &dat[7], dat[5] * 2);
    }
    while ((buff = malloc(8)) == 0)
    {
        OSTimeDly(1);
    }
    buff[0] = 6;
    buff[1] = ModbusLocalAddr();
    buff[2] = code;
    buff[3] = dat[2];
    buff[4] = dat[3];
    buff[5] = dat[4];
    buff[6] = dat[5];
    return (uint32_t)buff;
}



//  读设备识别码
uint32_t ModbusSlave_15_Reply(uint8_t code, uint8_t *dat)
{
    uint8_t *buff = 0;
    uint32_t file_record;
    uint8_t  index = 0; 
    uint8_t  record_len;
    uint8_t  ret_len = 0, ret_index=0;
    
    if ((dat[2] <= 7) && (dat[2] >= 0xF5))
    {
        return 3;  //  数据长度错误
    }
    index = 3;
    while (1)
    {
        file_record = (dat[index+1] << 8) + dat[index + 2];
        file_record = (file_record << 16) + (dat[index+3] << 8) + dat[index + 4];
        record_len = (dat[index+5] << 8) + dat[index + 6];
        record_len = record_len;
        break;
    }
    while ((buff = malloc(ret_len+8)) == 0)
    {
        OSTimeDly(1);
    }
    memset(buff, 0, ret_len+8);
    buff[0] = ret_len+3;
    buff[1] = ModbusLocalAddr();
    buff[2] = 0x15;
    buff[3] = ret_len;
    ret_index = 4;
    index = 3;
    while (1)
    {
        memcpy(&buff[ret_index], &dat[index], 7);
        break;
    }
    
    return (uint32_t)buff;
}
typedef struct s_device_info
{
    uint16_t id;
    uint16_t len;
    uint8_t *data;
    int8_t (*read)(uint8_t *, uint8_t *);
} s_device_info_t;

#include "version.h"
#include "chip_communication.h"

const char VendorName[] = {"Sewage Disposal System"}; 
const char ProductCode[] = {"1234567890"};  
const char MajorMinorRevision[] = {VERSION};  
const char VendorUrl[] = {""};  
const char ProductName[] = {"Sewage Disposal System"};   
const char ModelName[] = {""};  
const char UserApplicationName[] = {"Sewage Disposal System"};  


int8_t ReadF1Version(uint8_t * buf, uint8_t *len)
{
    uint8_t buffer[32];
    if (ChipReadFrame(1, CH2_VERSION_ADDR, 32, buffer) >= 0)
    {
        buffer[31] = 0;
        *len = strlen((char *)buffer);
        strcpy((char *)buf, (char *)buffer);
        return 0;
    }
    return 3;
}


const s_device_info_t  sds_device_modbus_info[] = 
{
    {0, sizeof(VendorName), (uint8_t *)VendorName, 0},
    {1, sizeof(ProductCode), (uint8_t *)ProductCode, 0},
    {2, sizeof(MajorMinorRevision), (uint8_t *)MajorMinorRevision, 0},
    {3, sizeof(VendorUrl), (uint8_t *)VendorUrl, 0},
    {4, sizeof(ProductName), (uint8_t *)ProductName, 0},
    {5, sizeof(ModelName), (uint8_t *)ModelName, 0},
    {6, sizeof(UserApplicationName), (uint8_t *)UserApplicationName, 0},
    {7, 0, 0, ReadF1Version},
};
#define DEVICE_INFO_NUM   (sizeof(sds_device_modbus_info)/sizeof(s_device_info_t))
uint8_t * ReadDeviceIdentification(uint8_t *dat)
{
    uint8_t index;
    uint8_t *ret_buf = 0;
    uint8_t Device_id = dat[3];
    ret_buf = (void *)1;
    if ((Device_id == 1) || (Device_id == 2))
    {
        for (index=0; index<DEVICE_INFO_NUM; index++)
        {
            if (sds_device_modbus_info[index].id == dat[4])
            {
                break;
            }
        }
        if (index == DEVICE_INFO_NUM)
        {
            return (void *)2;
        }
        while ((ret_buf = malloc(256)) == 0)
        {
            OSTimeDly(1);
        }
        ret_buf[1] = dat[0];
        ret_buf[2] = dat[1];
        ret_buf[3] = dat[2];
        ret_buf[4] = dat[3];
        ret_buf[5] = dat[3];
        ret_buf[8] = 0;
        for (; index<DEVICE_INFO_NUM; index++)
        {
            //  读取数据
            //if ()
            {
            }
        }
        if (++index < DEVICE_INFO_NUM)
        {
            ret_buf[6] = 0xFF;
            ret_buf[7] = sds_device_modbus_info[index].id;
        }
        else
        {
            ret_buf[6] = 0;
            ret_buf[7] = 0;
        }
    }
    else 
    {
        ret_buf = (void *)2;  //  不支持数据等级
    }
    return ret_buf;
}

//  读设备识别码
uint32_t ModbusSlave_2B_Reply(uint8_t code, uint8_t *dat)
{
    uint32_t ret = 1;
    uint8_t subcode = dat[2];
    switch (subcode)
    {
        case 0x0E:
            //ret = (uint32_t)ReadDeviceIdentification(dat);
            break;
        default:
            ret = 1;
            break;
    }
    return ret;
}
const s_support_code_t support_code[SUPPORT_CODE_NUMBER] =
{
    {0x01, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  ModbusSlave_01_Reply},  //  Read Coils
    {0x02, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  ModbusSlave_02_Reply},  //  Read Discrete Inputs
    {0x03, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  ModbusSlave_03_Reply},  //  Read Holding Registers
    {0x04, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Read Input Register
    {0x05, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  ModbusSlave_05_Reply},  //  Write Single Coil
    {0x06, 0x04, 0x00, 0x00, 0x00,  0,  NULL,  ModbusSlave_06_Reply},  //  Write Single Register
    {0x07, 0x00, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Read Exception status
    {0x08, 0x02, 0x00, 0x02, 0x02,  0,  NULL,  NULL},  //  Diagnostic
    {0x0B, 0x00, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Get Com event counter
    {0x0C, 0x00, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Get Com Event Log
    {0x0F, 0x05, 0x04, 0x01, 0x01,  0,  NULL,  ModbusSlave_0F_Reply},  //  Write Multiple Coils
    {0x10, 0x05, 0x04, 0x01, 0x01,  0,  NULL,  ModbusSlave_10_Reply},  //  Write Multiple Registers
    {0x11, 0x00, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Report Server ID
    {0x14, 0x01, 0x00, 0x01, 0x01,  0,  NULL,  NULL},  //  Read File record
    {0x15, 0x01, 0x00, 0x01, 0x01,  0,  NULL,  ModbusSlave_15_Reply},  //  Write File record
    {0x16, 0x06, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Mask Write Register
    {0x17, 0x09, 0x08, 0x01, 0x01,  0,  NULL,  NULL},  //  Read/Write Multiple Registers
    {0x18, 0x02, 0x00, 0x00, 0x00,  0,  NULL,  NULL},  //  Read FIFO queue
    {0x2B, 0x03, 0x00, 0x00, 0x00,  0,  NULL,  ModbusSlave_2B_Reply}   //  Read device Identification
};




/*  查询本机是否支持该功能码  */
static int8_t MatchModbusCode(uint8_t code)
{
    uint8_t i = SUPPORT_CODE_NUMBER / 2, max_i = SUPPORT_CODE_NUMBER, min_i = 0;

    if (support_code[0].code == code)  // 确保第0个不是要找的code
    {
        if (support_code[0].response_func == NULL)
        {
            return -1;//不支持些功能码
        }
        return 0;
    }
    while (i != min_i)  // 判断是否到最后三个
    {
        //  采用二分法查找
        if (support_code[i].code > code)
        {
            //  需要找的code在小半区
            max_i = i;
            i = (i + min_i) >> 1;
        }
        else if (support_code[i].code < code)
        {
            //  需要找的code在大半区
            min_i = i;
            i = (i + max_i) >> 1;
        }
        else
        {
            if (support_code[i].response_func == NULL)
            {
                return -1;//不支持些功能码
            }
            return (int8_t)i;
        }
    }
    return -1;//不支持些功能码
}

enum e_modbus_state
{
    MODBUS_ST_ADDR,
    MODBUS_ST_CMD,
    MODBUS_ST_DAT_ADDR,
    MODBUS_ST_OTHER,
    MODBUS_ST_DATA,
    MODBUS_ST_CRC_H,
    MODBUS_ST_CRC_L,
    MODBUS_ST_RESPONSE,
    MODBUS_ST_ERRCODE,
};

/*  发送modbus数据  */
//#define  ModbusReply(buf, len)    IoWrite(MODBUS_PORT, buf, len)

/******************************************************************
** 函数名称:   ModbusReply
** 功能描述:   485发送数据
** 输入:	   
**			   
**             
** 输出:	   返回接收到的数值
** 全局变量:
** 调用模块: 
** 备注:
** 作者:	   arjun
** 日期:	   20150405
******************************************************************/
void ModbusReply(uint8_t *sendbuf,uint8_t len)
{
	uint8_t temp;
    uint16_t crc = 0xFFFF;
    crc = CRC16(crc, sendbuf, len);
//	GPIO_SetBits(GPIOB, RS485_CTL1);//485发送使能，接收禁止
//    for(temp=0; temp<50; temp++);  延时一会儿，保证IO口变化成功
	while (USART_GetFlagStatus(USART6, USART_FLAG_TXE) == RESET);
	for (temp = 0; temp < len; temp++)
	{
		USART_SendData(USART6,sendbuf[temp]); //通过外设 USART1 发送单个数据 
	    while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
	}
	USART_SendData(USART6,(crc>>8)&0xFF); //通过外设 USART1 发送单个数据 
	while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
	USART_SendData(USART6,crc&0xFF); //通过外设 USART1 发送单个数据 
	while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
	//CLAR_QUEUE_UART1R;//清接收队列
    Delay_ms(3);
//	GPIO_ResetBits(GPIOB, RS485_CTL1);//485接收使能，发送禁止
}
/*
dat -- 收发的数据
flag -- 调用函数时，是收结束还是发结束
*/

uint8_t *rvceivedata = NULL;
uint8_t reveive_stu;//为1时接收到正确数据
void ModbusSlaveReceive(uint8_t *dat)
{
    static int8_t modbus_state, code_index;
    static uint32_t data_length;
    static uint16_t modbus_crc, modbus_rec_index;
    static uint8_t modbus_rec_buffer[256];

    //两次中断时间超过5MS后，重新从地址位开始接收
	if(modbus_datatimeout == 0)
	{
		modbus_state = MODBUS_ST_ADDR;
	}
	modbus_datatimeout = 5;//获取当前的秒计数器

    switch (modbus_state)
    {
	    case MODBUS_ST_ADDR:
	        if ((ModbusLocalAddr() == *dat) || (*dat == 0))
	        {
	            modbus_rec_index = 0;
	            modbus_rec_buffer[modbus_rec_index++] = *dat;
	            modbus_crc = 0xFFFF;
	            modbus_crc = CRC16(modbus_crc, dat, 1);
	            modbus_state = MODBUS_ST_CMD;
	        }
	        else
	        {
	            modbus_state = MODBUS_ST_OTHER;
	        }
	        break;
	    case MODBUS_ST_OTHER:
	        //modbus_state = MODBUS_ST_ADDR;
	        break;
	    case MODBUS_ST_CMD:
	        code_index = MatchModbusCode(*dat);
	        if (code_index >= 0)
	        {
	            modbus_rec_buffer[modbus_rec_index++] = *dat;
	            modbus_crc = CRC16(modbus_crc, dat, 1);
	            if (support_code[code_index].length)
	            {
	                modbus_state = MODBUS_ST_DAT_ADDR;
	            }
	            else
	            {
	                modbus_state = MODBUS_ST_CRC_H;
	            }
	        }
	        else
	        {
	            modbus_state = MODBUS_ST_OTHER;
	        }
	        break;
	    case MODBUS_ST_DAT_ADDR:
	        modbus_rec_buffer[modbus_rec_index++] = *dat;
	        modbus_crc = CRC16(modbus_crc, dat, 1);
	        if (support_code[code_index].length > (modbus_rec_index - 2))
	        {
	            //modbus_state = MODBUS_ST_ERRCODE;
	        }
	        else if (support_code[code_index].data_align == 0)
	        {
	            modbus_state = MODBUS_ST_CRC_H;
	        }
	        else
	        {
	            if (support_code[code_index].length_type == 1)
	            {
	                data_length = modbus_rec_buffer[support_code[code_index].length_index + 2];
	            }
	            else if ((support_code[code_index].length_type == 2))
	            {
	                data_length = modbus_rec_buffer[support_code[code_index].length_index + 2] << 8;
	                data_length += modbus_rec_buffer[support_code[code_index].length_index + 3];
	            }
	            else if ((support_code[code_index].length_type == 3))
	            {
	                data_length = modbus_rec_buffer[support_code[code_index].length_index + 2] << 16;
	                data_length += modbus_rec_buffer[support_code[code_index].length_index + 3] << 8;
	                data_length += modbus_rec_buffer[support_code[code_index].length_index + 4];
	            }
	            else if ((support_code[code_index].length_type == 4))
	            {
	                data_length = modbus_rec_buffer[support_code[code_index].length_index + 2] << 24;
	                data_length += modbus_rec_buffer[support_code[code_index].length_index + 3] << 16;
	                data_length += modbus_rec_buffer[support_code[code_index].length_index + 4] << 8;
	                data_length += modbus_rec_buffer[support_code[code_index].length_index + 5];
	            }
	            data_length *= support_code[code_index].data_align;
	            if ((data_length + support_code[code_index].length + 4) >= 256)
	            {
	                //  所有数据总长度必须小于256byte
	                modbus_state = MODBUS_ST_OTHER;
	            }
	            else
	            {
	                modbus_state = MODBUS_ST_DATA;
	            }
	        }
	        break;
	    case MODBUS_ST_DATA:
	        modbus_rec_buffer[modbus_rec_index++] = *dat;
	        modbus_crc = CRC16(modbus_crc, dat, 1);
	        if (data_length <= (modbus_rec_index - support_code[code_index].length - 2))
	        {
	            modbus_state = MODBUS_ST_CRC_H;
	        }
	        break;
	    case MODBUS_ST_CRC_H:
	        modbus_rec_buffer[modbus_rec_index++] = *dat;
	        modbus_state = MODBUS_ST_CRC_L;
	        break;
	    case MODBUS_ST_CRC_L:
	        modbus_rec_buffer[modbus_rec_index] = *dat;

	        if (modbus_crc ==
	                ((modbus_rec_buffer[modbus_rec_index - 1] << 8) + modbus_rec_buffer[modbus_rec_index]))
	        {
	            if ((dat = malloc(modbus_rec_index + 8)) != 0)
	            {
	                memset(dat, 0, modbus_rec_index + 4);
	                dat[0] = modbus_rec_index >> 8;
	                dat[1] = modbus_rec_index;
	                memcpy(&dat[2], &modbus_rec_buffer, modbus_rec_index + 1);
					rvceivedata = dat;
					reveive_stu = 1;
	            }
	        }
	        modbus_state = MODBUS_ST_ADDR;
	        break;
	    default :
	        modbus_state = MODBUS_ST_OTHER;
	        break;
    }
}

/*必须为最高优先级*/
void TaskModbus(void)
{
    uint8_t temp, code_index;
    uint8_t *res_data;
    uint8_t *rec_dat;
//    s_flash_parameters_t par_temp = *program_parameters;
    if(reveive_stu == 1)
    {
        reveive_stu = 0;
		rec_dat = rvceivedata;//指向接收到的数据地址
		//配置寄存器的大小及指针位置
		modbus_register = parkinglock_register;
        modbus_coil = sds_coils;
        code_index = MatchModbusCode(rec_dat[3]);
		reg_number = REGISTER_NUM;
        coils_num = COILS_NUM;
        res_data = (void *)support_code[code_index].response_func(code_index, &rec_dat[2]);
		if (rec_dat[2] == 0)
        {
            //  如果是广播信息，不回复
            if ((uint32_t)res_data >= 0x100)
            {
                free(res_data);
            }
        }
        else if ((int32_t)res_data < 0x100)
        {
			Delay_ms(3);
            //  返回错误
            rec_dat[2] = ModbusLocalAddr();
            rec_dat[3] |= 0x80;
            rec_dat[4] = (uint32_t)res_data;
            //crc = 0xFFFF;
            //crc = CRC16(crc, &rec_dat[2], 3);
			//crc_dat[0] = (uint8_t)(crc>>8);
			//crc_dat[1] = (uint8_t)crc;
            ModbusReply(&rec_dat[2], 3);
            //ModbusReply(crc_dat, 2);
        }
        else
        {
			Delay_ms(3);
            //  正常回复
			res_data[1] = ModbusLocalAddr();
            temp = res_data[0];
            //crc = 0xFFFF;
            //crc = CRC16(crc, &res_data[1], temp);
			//crc_dat[0] = (uint8_t)(crc>>8);
			//crc_dat[1] = (uint8_t)crc;
            ModbusReply(&res_data[1], temp);
            //ModbusReply(crc_dat, 2);
            free(res_data);
            res_data = 0;
        }
        free(rec_dat);
        rec_dat = 0;
    }
    if (reboot_flag)
    {
        //ReBoot();
    }
}
