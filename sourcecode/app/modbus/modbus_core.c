#include  "modbus_core.h"
#include  "stdint.h"




/*  modbus 命令在s_support_code_t 表中的索引*/
/*const uint8_t modbus_code_index[]=
{
    0xFF,    0,    1,    2,    3,    4,    5,    6,    7, 0xFF, 0xFF,    8,    9, 0xFF, 0xFF,   10,
      11,   12, 0xFF, 0xFF,   13,   14,   15,   16,   17, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   18, 0xFF, 0xFF, 0xFF, 0xFF,
};

uint8_t GetModbusRequestDataLen(uint8_t *data)
{
    s_support_code_t * command;
    uint8_t index = modbus_code_index[code];
    
    uint8_t len = master_command_code[index].length + data;
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
        data[j] = temp & 0xFF;
    }
    if (j) {
        data[j] &= bit_mask[size&0x03];     
    }
}*/

void SaveLittleEndianCopy(uint16_t *reg, uint8_t *data, uint32_t len)
{
    uint16_t temp;
    while (len)
    {
        temp = (data[0]<<8) + data[1];
        *reg = temp;
        reg++;
        data+=2;
        len -= 1;
    }
}

void ReadLittleEndianCopy(uint16_t *reg, uint8_t *data, uint32_t len)
{
    uint16_t temp;
    while (len)
    {
        temp = *reg;
        data[0] = (uint8_t)(temp>>8);
        data[1] = (uint8_t)temp;
        reg++;
        data+=2;
        len -= 1;
    }    
}

//大端操作
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


