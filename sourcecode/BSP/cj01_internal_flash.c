#include	"..\bsp_includes.h"
#include "cj01_internal_flash.h"




/*****************************************************************************************
** 函数名称 ：	STMFLASH_ReadWord
** 函数功能 ：  读取指定地址的半字(16位数据) 
** 输    入 ：	u32 faddr			读地址 
** 输    出 ：	u32  			    对应数据.
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
uint32_t STMFLASH_ReadWord(uint32_t faddr)
{
	return *(vu32*)faddr; 
}

/*****************************************************************************************
** 函数名称 ：	FLASH_GetFlashSector
** 函数功能 ：  获取某个地址所在的flash扇区
** 输    入 ：	u32 addr			flash地址
** 输    出 ：	uint16_t  			返回值:0~11,即addr所在的扇区
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
uint16_t FLASH_GetFlashSector(uint32_t addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}

/*****************************************************************************************
** 函数名称 ：	FLASH_Write
** 函数功能 ：  从指定地址开始写入指定长度的数据
//特别注意:因为STM32F4的扇区实在太大,没办法本地保存扇区数据,所以本函数
//         写地址如果非0XFF,那么会先擦除整个扇区且不保存扇区数据.所以
//         写非0XFF的地址,将导致整个扇区数据丢失.建议写之前确保扇区里
//         没有重要数据,最好是整个扇区先擦除了,然后慢慢往后写.
//该函数对OTP区域也有效!可以用来写OTP区!
//OTP区域地址范围:0X1FFF7800~0X1FFF7A0F
** 输    入 ：	u32 WriteAddr			起始地址(此地址必须为4的倍数!!)
** 输    入 ：	u32 * pBuffer			数据指针
** 输    入 ：	u32 NumToWrite			字(32位)数(就是要写入的32位数据的个数.) 
** 输    出 ：	void  			
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t FLASH_Write_4Byte(uint32_t WriteAddr, uint32_t *pBuffer, uint32_t NumToWrite)
{ 
  FLASH_Status status = FLASH_COMPLETE;
  uint32_t addrx=0;
  uint32_t endaddr=0;
  if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return -1;	//非法地址
	FLASH_Unlock();									//解锁 
    FLASH_DataCacheCmd(DISABLE);//FLASH擦除期间,必须禁止数据缓存
 		
	addrx=WriteAddr;				//写入的起始地址
	endaddr=WriteAddr+NumToWrite*4;	//写入的结束地址
	if(addrx<0X1FFF0000)			//只有主存储区,才需要执行擦除操作!!
	{
		while(addrx<endaddr)		//扫清一切障碍.(对非FFFFFFFF的地方,先擦除)
		{
			if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//有非0XFFFFFFFF的地方,要擦除这个扇区
			{   
				status=FLASH_EraseSector(FLASH_GetFlashSector(addrx),VoltageRange_3);//VCC=2.7~3.6V之间!!
				if(status!=FLASH_COMPLETE)break;	//发生错误了
			}else addrx+=4;
		} 
	}
	if(status==FLASH_COMPLETE)
	{
		while(WriteAddr<endaddr)//写数据
		{
			if(FLASH_ProgramWord(WriteAddr,*pBuffer)!=FLASH_COMPLETE)//写入数据
			{ 
				break;	//写入异常
			}
			WriteAddr+=4;
			pBuffer++;
		} 
	}
    FLASH_DataCacheCmd(ENABLE);	//FLASH擦除结束,开启数据缓存
	FLASH_Lock();//上锁
    return 0;
} 


/*****************************************************************************************
** 函数名称 ：	FLASH_Read
** 函数功能 ：  从指定地址开始读出指定长度的数据
** 输    入 ：	u32 ReadAddr			起始地址
** 输    入 ：	u32 * pBuffer			数据指针
** 输    入 ：	u32 NumToRead			字(4位)数
** 输    出 ：	void  			
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
void FLASH_Read_4Byte(uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead)
{
    uint32_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//读取4个字节.
		ReadAddr+=4;//偏移4个字节.	
	}
}





/*****************************************************************************************
** 函数名称 ：	Flash_Open
** 函数功能 ：  
** 输    入 ：	int32_t port			
** 输    入 ：	void * config			
** 输    入 ：	uint8_t len			
** 输    出 ：	int8_t  			
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Flash_Open(int32_t port, const void *config, uint8_t len)
{
    return 0;
}


/*****************************************************************************************
** 函数名称 ：	Flash_Close
** 函数功能 ：  
** 输    入 ：	int32_t port			
** 输    出 ：	int8_t  			
** 全局变量 :   
** 调用模块 :   
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Flash_Close(int32_t port)
{
    return 0;
}





/*****************************************************************************************
** 函数名称 ：	Flash_Write
** 函数功能 ：  从指定地址开始写入指定长度的数据
//特别注意:因为STM32F4的扇区实在太大,没办法本地保存扇区数据,所以本函数
//         写地址如果非0XFF,那么会先擦除整个扇区且不保存扇区数据.所以
//         写非0XFF的地址,将导致整个扇区数据丢失.建议写之前确保扇区里
//         没有重要数据,最好是整个扇区先擦除了,然后慢慢往后写.
//该函数对OTP区域也有效!可以用来写OTP区!
//OTP区域地址范围:0X1FFF7800~0X1FFF7A0F
** 输    入 ：	u32 WriteAddr			起始地址(此地址必须为4的倍数!!)
** 输    入 ：	u32 * pBuffer			数据指针
** 输    入 ：	u32 NumToWrite			字(32位)数(就是要写入的32位数据的个数.)
** 输    出 ：	void
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Flash_Write(int32_t port, void *buf, uint32_t buf_len)
{
    int8_t return_flag;
    uint32_t buf_4byte_len;
    s_store_argv_t argv;
    memcpy(&argv, buf, sizeof(s_store_argv_t));
    buf_4byte_len = (argv.len/4+((argv.len%4)?1:0));//计算按照4字节写数据，应该写的长度。
    if((argv.addr < FLASH_SAVE_START_ADDR) || ((argv.addr + argv.len) > FLASH_SAVE_END_ADDR))
    {
        return FLASH_MEMORY_ADDR_OVERFLOW;
    }
    
    return_flag=FLASH_Write_4Byte (argv.addr,(u32*)argv.data,buf_4byte_len);
    return return_flag;
}


/*****************************************************************************************
** 函数名称 ：	Flash_Read
** 函数功能 ：  从指定地址开始读出指定长度的数据
** 输    入 ：	u32 ReadAddr			起始地址
** 输    入 ：	u32 * pBuffer			数据指针
** 输    入 ：	u32 NumToRead			字(4位)数
** 输    出 ：	void
** 全局变量 :
** 调用模块 :
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int32_t Flash_Read(int32_t port, void *buf, uint32_t buf_len)
{
    uint32_t buf_4byte_len;
    s_store_argv_t argv;
    memcpy(&argv, buf, sizeof(s_store_argv_t));
    buf_4byte_len = (argv.len/4+((argv.len%4)?1:0));//计算按照4字节写数据，应该写的长度。
    if((argv.addr < FLASH_SAVE_START_ADDR) || ((argv.addr + argv.len) > FLASH_SAVE_END_ADDR))
    {
        return FLASH_MEMORY_ADDR_OVERFLOW;
    }
    FLASH_Read_4Byte(argv.addr,(u32*)argv.data,buf_4byte_len);
    return 0;
}

/*****************************************************************************************
** 函数名称 ：	Flash_Ioctl
** 函数功能 ：	对PS2端口口操作控制
** 输    入 ：	int32_t port				IO端口号
** 输    入 ：	uint32_t cmd				读写权限
** 输    入 ：	va_list argp				要读、写的数据   要读、写的数据长度
** 输    出 ：	int8_t    			        0：操作成功 -1：操作失败
** 全局变量 :
** 调用模块 :   Ps2key_Write，Ps2key_Read
** 作　  者 : 	彭      俊
** 日　  期 : 	2017/07/07
*****************************************************************************************/
int8_t Flash_Ioctl(int32_t port, uint32_t cmd, va_list argp)
{
    int8_t ret = -1;

    switch (cmd)
    {
    case 0:

    default:
        break;
    }
    return ret;
}

