
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ucos_ii.h"
#include "cj01_usart.h"
#include "stm32f4xx.h"
#include "modbus_register.h"
#include "crc.h"
#include "modbus_slave.h"


#define CRC16_MODBUS_ENABLE      1
#define CRC_DNP_ENALBE           0
#define CRC32_ENABLE             0
//#include "modbus_crc.h"

uint32_t boot_program_addr;
uint32_t app_program_addr;

//extern s_flash_parameters_t * program_parameters;

typedef struct
{
    uint8_t code;     //  ������
    uint8_t length;   //  �������ݳ���
    uint8_t length_index;  //  ���ݳ��ȵ�����
    uint8_t length_align  : 4;   //  ���ݳ��ȶ�����ֽ���
    uint8_t length_type   : 4;   //  ���ݳ���ռ�õ��ֽ���
    //  ��һ������Ϊ�ù������������ţ��ڶ����ֽ�Ϊ���յ�������
    uint32_t (*code_func)(uint8_t, uint8_t *);
} s_support_code_t;

s_modbus_register_t *modbus_register;
s_modbus_record_t   *modbus_record;
s_modbus_coils_t    *modbus_coil;
uint32_t reg_number;
uint32_t record_num;
uint32_t coils_num;
uint8_t modbus_datatimeout;//���յ����������жϵ�ʱ����
uint8_t device_addr=1;
uint8_t reboot_flag;    //  485������־


//func_code:�����룻addr����ʼ��ַ��len:���ݵĳ���
//ƥ�书���롢��ʼ��ַ�����ݳ����Ƿ�����Э��涨
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
	if((addr == 0x0020)&&(func_code == 0x05)&&(len == 0x0001))//���⴦��0x0020��д����
	{
		return 3;	
	}
	else if((addr == 0x0020)&&(func_code == 0x02)&&(len == 0x0008))//���⴦��0x0020������������
	{
		return 2;
	}
	else if((addr == 0x0021)&&(func_code == 0x05)&&(len == 0x0001))//���⴦��0x0021��д����
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
    while (i != min_i)  // �ж��Ƿ��������
    {
        //  ���ö��ַ�����
        if ((modbus_register[i].start_addr > addr) || (modbus_register[i].length == 0))
        {
            //  ��Ҫ�ҵ�code��С����
            max_i = i;
            i = (i + min_i) >> 1;
        }
        else if (((modbus_register[i].length > 0) &&
                 ((modbus_register[i].start_addr + modbus_register[i].length) <= addr)) ||
                 ((modbus_register[i].length < 0) && (modbus_register[i].start_addr < addr)))
        {
            //  ��Ҫ�ҵ�code�ڴ����
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

//func_code:�����룻addr����ʼ��ַ��len:���ݵĳ���
//ƥ�书���롢��ʼ��ַ�����ݳ����Ƿ�����Э��涨
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
    while (i != min_i)  // �ж��Ƿ��������
    {
        //  ���ö��ַ�����
        if ((modbus_coil[i].start_addr > addr) || (modbus_coil[i].coils_num == 0))
        {
            //  ��Ҫ�ҵ�code��С����
            max_i = i;
            i = (i + min_i) >> 1;
        }
        else if (((modbus_coil[i].coils_num > 0) &&
                 ((modbus_coil[i].start_addr + modbus_coil[i].coils_num) <= addr)) ||
                 ((modbus_coil[i].coils_num < 0) && (modbus_coil[i].start_addr < addr)))
        {
            //  ��Ҫ�ҵ�code�ڴ����
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


//��˲���
void HalfWordBigEndianCopy(void *dst, void *src, uint32_t len)
{
    uint8_t *dst_t = dst, *src_t = src;
    while (len)
    {
        dst_t[0] = src_t[1];
        dst_t[1] = src_t[0];
        dst_t += 2;
        src_t += 2;
        len -= 2;
    }
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
    uint16_t start_byte = start>>3;  //  16bit ����
    uint16_t temp;
    for (j=0; j<((size+15)>>3); j++)
    {
        temp = ((modbus_coil[reg].coils[start_byte+j] >> offset) & bit_mask[8-offset])
                   |(((modbus_coil[reg].coils[start_byte+1+j]) & bit_mask[offset]) << (8-offset));
        data[j] = temp & 0xFF;
    }
    if (j) {
        data[(j-1)] &= bit_mask[size&0x03];    
    }
}
void CopyCoilToBuffer(uint16_t start, uint16_t size, uint32_t reg, uint8_t *data)
{
    uint8_t j;
    uint8_t offset = start & 0x07;
    uint16_t start_byte = start>>3;  //  16bit ����
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
        data[j] = temp & 0xFF;
    }
    if (j) {
        data[j] &= bit_mask[size&0x03];     
    }
}

//  �������Ȧ
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
    buff[3] = dat[(size+7) >> 3];
    buff[0] = buff[3] + 2;

    if (modbus_coil[i].func)
    {
        rec_dat = (void *)modbus_coil[i].func(code, &dat[2]);
        if (rec_dat == 0)
        {
            if (modbus_coil[i].coils)
            {
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
        CopyCoilFromBuffer(start, size, i, &buff[4]);
    }
    else
    {
        free(buff);
        return 0xFF;
    }
    return (uint32_t)buff;
}

//  ���������
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
    buff[3] = dat[(size+7) >> 3];
    buff[0] = buff[3] + 2;

    if (modbus_coil[i].func)
    {
        rec_dat = (void *)modbus_coil[i].func(code, &dat[2]);
        if (rec_dat == 0)
        {
            if (modbus_coil[i].coils)
            {
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
        CopyCoilFromBuffer(start, size, i, &buff[4]);
    }
    else
    {
        free(buff);
        return 0xFF;
    }
    return (uint32_t)buff;
}



//  ������Ĵ���
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
        rec_dat = (void *)modbus_register[i].func(0x03, &dat[2]);
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

//  ������
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
        buff = (void *)modbus_register[i].func(0x06, &dat[2]);
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


//  д������Ȧ
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


//  д�����Ĵ���
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
        buff = (void *)modbus_register[i].func(0x06, &dat[2]);
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

//  д����Ĵ���
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

//  д����Ĵ���
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
        buff = (void *)modbus_register[i].func(code, &dat[2]);
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
        memset(modbus_register[i].buffer, 0, modbus_register[i].length);
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



//  ���豸ʶ����
uint32_t ModbusSlave_15_Reply(uint8_t code, uint8_t *dat)
{
    uint8_t *buff = 0;
    uint32_t file_record;
    uint8_t  index = 0, record_len; 
    uint8_t  ret_len = 0, ret_index=0;
    
    if ((dat[2] <= 7) && (dat[2] >= 0xF5))
    {
        return 3;  //  ���ݳ��ȴ���
    }
    index = 3;
    while (1)
    {
        file_record = (dat[index+1] << 8) + dat[index + 2];
        file_record = (file_record << 16) + (dat[index+3] << 8) + dat[index + 4];
        record_len = (dat[index+5] << 8) + dat[index + 6];
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

//  ���豸ʶ����
uint32_t ModbusSlave_2B_Reply(uint8_t code, uint8_t *dat)
{
    return 0;
}
const s_support_code_t support_code[SUPPORT_CODE_NUMBER] =
{
    {0x01, 0x04, 0x00, 0x00, 0x00,  ModbusSlave_01_Reply},  //  Read Coils
    {0x02, 0x04, 0x00, 0x00, 0x00,  ModbusSlave_02_Reply},  //  Read Discrete Inputs
    {0x03, 0x04, 0x00, 0x00, 0x00,  ModbusSlave_03_Reply},  //  Read Holding Registers
    {0x04, 0x04, 0x00, 0x00, 0x00,  NULL},  //  Read Input Register
    {0x05, 0x04, 0x00, 0x00, 0x00,  ModbusSlave_05_Reply},  //  Write Single Coil
    {0x06, 0x04, 0x00, 0x00, 0x00,  ModbusSlave_06_Reply},  //  Write Single Register
    {0x07, 0x00, 0x00, 0x00, 0x00,  NULL},  //  Read Exception status
    {0x08, 0x02, 0x00, 0x02, 0x02,  NULL},  //  Diagnostic
    {0x0B, 0x00, 0x00, 0x00, 0x00,  NULL},  //  Get Com event counter
    {0x0C, 0x00, 0x00, 0x00, 0x00,  NULL},  //  Get Com Event Log
    {0x0F, 0x05, 0x04, 0x01, 0x01,  ModbusSlave_0F_Reply},  //  Write Multiple Coils
    {0x10, 0x05, 0x04, 0x01, 0x01,  ModbusSlave_10_Reply},  //  Write Multiple Registers
    {0x11, 0x00, 0x00, 0x00, 0x00,  NULL},  //  Report Server ID
    {0x14, 0x01, 0x00, 0x01, 0x01,  NULL},  //  Read File record
    {0x15, 0x01, 0x00, 0x01, 0x01,  ModbusSlave_15_Reply},  //  Write File record
    {0x16, 0x06, 0x00, 0x00, 0x00,  NULL},  //  Mask Write Register
    {0x17, 0x09, 0x08, 0x01, 0x01,  NULL},  //  Read/Write Multiple Registers
    {0x18, 0x02, 0x00, 0x00, 0x00,  NULL},  //  Read FIFO queue
    {0x2B, 0x03, 0x00, 0x00, 0x00,  ModbusSlave_2B_Reply}   //  Read device Identification
};




/*  ��ѯ�����Ƿ�֧�ָù�����  */
static int8_t MatchModbusCode(uint8_t code)
{
    uint8_t i = SUPPORT_CODE_NUMBER / 2, max_i = SUPPORT_CODE_NUMBER, min_i = 0;

    if (support_code[0].code == code)  // ȷ����0������Ҫ�ҵ�code
    {
        if (support_code[0].code_func == NULL)
        {
            return -1;//��֧��Щ������
        }
        return 0;
    }
    while (i != min_i)  // �ж��Ƿ��������
    {
        //  ���ö��ַ�����
        if (support_code[i].code > code)
        {
            //  ��Ҫ�ҵ�code��С����
            max_i = i;
            i = (i + min_i) >> 1;
        }
        else if (support_code[i].code < code)
        {
            //  ��Ҫ�ҵ�code�ڴ����
            min_i = i;
            i = (i + max_i) >> 1;
        }
        else
        {
            if (support_code[i].code_func == NULL)
            {
                return -1;//��֧��Щ������
            }
            return (int8_t)i;
        }
    }
    return -1;//��֧��Щ������
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

/*  ����modbus����  */
//#define  ModbusReply(buf, len)    IoWrite(MODBUS_PORT, buf, len)

/******************************************************************
** ��������:   ModbusReply
** ��������:   485��������
** ����:	   
**			   
**             
** ���:	   ���ؽ��յ�����ֵ
** ȫ�ֱ���:
** ����ģ��: 
** ��ע:
** ����:	   arjun
** ����:	   20150405
******************************************************************/
void ModbusReply(uint8_t *sendbuf,uint8_t len)
{
	uint8_t temp;
    uint16_t crc = 0xFFFF;
    crc = CRC16(crc, sendbuf, len);
//	GPIO_SetBits(GPIOB, RS485_CTL1);//485����ʹ�ܣ����ս�ֹ
//    for(temp=0; temp<50; temp++);  ��ʱһ�������֤IO�ڱ仯�ɹ�
	while (USART_GetFlagStatus(USART6, USART_FLAG_TXE) == RESET);
	for (temp = 0; temp < len; temp++)
	{
		USART_SendData(USART6,sendbuf[temp]); //ͨ������ USART1 ���͵������� 
	    while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
	}
	USART_SendData(USART6,(crc>>8)&0xFF); //ͨ������ USART1 ���͵������� 
	while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
	USART_SendData(USART6,crc&0xFF); //ͨ������ USART1 ���͵������� 
	while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
	//CLAR_QUEUE_UART1R;//����ն���
    Delay_ms(3);
//	GPIO_ResetBits(GPIOB, RS485_CTL1);//485����ʹ�ܣ����ͽ�ֹ
}
/*
dat -- �շ�������
flag -- ���ú���ʱ�����ս������Ƿ�����
*/

uint8_t *rvceivedata = NULL;
uint8_t reveive_stu;//Ϊ1ʱ���յ���ȷ����
void ModbusSlaveReceive(uint8_t *dat)
{
    static int8_t modbus_state, code_index;
    static uint32_t data_length;
    static uint16_t modbus_crc, modbus_rec_index;
    static uint8_t modbus_rec_buffer[256];

    //�����ж�ʱ�䳬��5MS�����´ӵ�ַλ��ʼ����
	if(modbus_datatimeout == 0)
	{
		modbus_state = MODBUS_ST_ADDR;
	}
	modbus_datatimeout = 5;//��ȡ��ǰ���������

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
	        else if (support_code[code_index].length_align == 0)
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
	            data_length *= support_code[code_index].length_align;
	            if ((data_length + support_code[code_index].length + 4) >= 256)
	            {
	                //  ���������ܳ��ȱ���С��256byte
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

/*����Ϊ������ȼ�*/
void TaskModbus(void)
{
    uint8_t temp, code_index;
    uint8_t *res_data;
    uint8_t *rec_dat;
//    s_flash_parameters_t par_temp = *program_parameters;
    if(reveive_stu == 1)
    {
        reveive_stu = 0;
		rec_dat = rvceivedata;//ָ����յ������ݵ�ַ
		//���üĴ����Ĵ�С��ָ��λ��
		modbus_register = parkinglock_register;
        modbus_coil = sds_coils;
        code_index = MatchModbusCode(rec_dat[3]);
		reg_number = REGISTER_NUM;
        coils_num = COILS_NUM;
        res_data = (void *)support_code[code_index].code_func(code_index, &rec_dat[2]);
		if (rec_dat[2] == 0)
        {
            //  ����ǹ㲥��Ϣ�����ظ�
            if ((uint32_t)res_data >= 0x100)
            {
                free(res_data);
            }
        }
        else if ((int32_t)res_data < 0x100)
        {
			Delay_ms(3);
            //  ���ش���
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
            //  �����ظ�
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