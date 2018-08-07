#ifndef __INTERNAL_FLASH_H__
#define __INTERNAL_FLASH_H__
#include "stdint.h"


//内部FLASH的数据读写地址
//设置FLASH 数据保存地址(必须为偶数，且所在扇区,要大于本代码所占用到的扇区.
//否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机.
#define FLASH_SAVE_START_ADDR   0X08004000 	
#define FLASH_SAVE_END_ADDR     0X0800C000 	


int8_t Flash_Open(int32_t port, const void *config, uint8_t len);
int8_t Flash_Close(int32_t port);
int8_t Flash_Write(int32_t port, void *buf, uint32_t buf_len);
int32_t Flash_Read(int32_t port, void *buf, uint32_t buf_len);
int8_t Flash_Ioctl(int32_t port, uint32_t cmd, va_list argp);



/*IO口类型定义*/
typedef struct
{
    uint32_t addr;
    uint32_t len;
    uint8_t  *data;
} s_store_argv_t;




#define FLASH_MEMORY_ADDR_OVERFLOW            -1        //    地址溢出错误




//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址

//FLASH 扇区的起始地址
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//扇区0起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//扇区1起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//扇区2起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//扇区3起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//扇区4起始地址, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//扇区5起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//扇区6起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//扇区7起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//扇区8起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//扇区9起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//扇区10起始地址,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//扇区11起始地址,128 Kbytes  

					   
#endif

















