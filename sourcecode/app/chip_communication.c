#include "chip_communication.h"
#include "ucos_ii.h"
#include "crc_lib.h"
#include "stm32f4xx.h"
#include "sys_timer.h"

/******************************************************************
** 函数名称:   SPI2_Configuration
** 功能描述:   配置SPI2成主机模式 
** 输入:	   无
**
** 输出:	   无
** 全局变量:
** 调用模块: 
** 备注:
** 作者:	   arjun
** 日期:	   20170825
******************************************************************/ 
void ChipSPIInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//使能SPI1时钟

    //SPI2初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //不带上下拉
	GPIO_Init(GPIOB,&GPIO_InitStructure);
    
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);
    
    //SPI1_CS初始化为输出
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //上拉
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
    GPIO_WriteBit(GPIOB,  GPIO_PIN_12, GPIO_PIN_SET);

    //这里只针对SPI口初始化
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE);//复位SPI2
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);//停止复位SPI2

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	    //设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		    //串行同步时钟的空闲状态为低电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	    //串行同步时钟的第一个跳变沿（上升或下降）数据被采样
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		    //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:Soft为软件控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;//定义波特率预分频的值:波特率预分频值为256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;	        //CRC值计算的多项式
    SPI_Init(SPI2, &SPI_InitStructure);                 //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    SPI_Cmd(SPI2, ENABLE); //使能SPI外设
    
}


uint16_t CRCByte(uint16_t crc, uint8_t data)
{
    return CRC16(crc, &data, 1);
}

uint8_t Spi0TranceByte(uint8_t data)
{
    uint8_t temp;
    uint32_t timeout = 100000;
    
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_TXE) == RESET){  
        if ((timeout--) == 0)    break;
    }
    
    SPI_SendData(SPI2, data); 
    timeout = 100000;
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_RXNE) == RESET){
        if ((timeout--) == 0)    break;
    }
    
    temp = SPI_ReceiveData(SPI2); 
    if (timeout == 0)   ChipSPIInit();
    return temp;   
}

uint32_t chip_tick;
uint8_t  chip_communication_temp[32];
OS_EVENT *data_upload_sem, *chip_communication_sem;

void ChipCommInit(void)
{
    ChipSPIInit();
	data_upload_sem = OSSemCreate(0);		//	用于PAD 命令处理
	if (data_upload_sem == NULL)
	{
		while(1);
	}
	chip_communication_sem = OSSemCreate(1);
	if (chip_communication_sem == NULL)
	{
		while(1);
	}
}

void RequestUpload(void)
{
	OSSemPost(data_upload_sem);
}

#define ADDR_EX_MASK     0x1F
#define COMM_DATA_MAX_LEN  32

/**  通信数据头的数据结构
 */
//  标准帧的头数据结构
typedef struct 
{
    uint32_t unused  :  8;  //  未使用，头只有三个字节
    uint32_t comm_len:  8;  //  当前帧的数据长度
    uint32_t addr    : 10;  //  当前帧操作的地址
    uint32_t channel :  5;  //  当前帧操作的通道，当值为ADDR_EX_MASK，表示当前帧是扩展帧
    uint32_t rw      :  1;  //  当前帧的读写标识,1:读，0:写
} s_addr_t;

//  扩展帧的头数据结构
typedef struct 
{
    uint32_t comm_len:  8;  //  当前帧的数据长度
    uint32_t addr    : 14;  //  当前帧操作的地址
    uint32_t channel :  9;  //  当前帧操作的通道
    uint32_t rw      :  1;  //  当前帧的读写标识,1:读，0:写
} s_addr_ex_t;
//  数据帧头的数据结构，标准帧3byte, 扩展帧4byte
typedef union 
{
    uint8_t bytes[4];
    s_addr_t addr;        //  标准帧的头数据结构
    s_addr_ex_t addr_ex;  //  扩展帧的头数据结构
} u_addr_t;
uint8_t buffer_tt[128];
int8_t ChipWriteFrame(uint8_t fun, uint16_t addr, uint8_t len, void *data)
{
	uint8_t i, j=0;
	uint16_t bcc = 0;
	uint16_t res_bcc = 0;
    u_addr_t head;
    head.addr.rw = 0;
    head.addr.channel = fun;
    head.addr.addr = addr;
    head.addr.comm_len = len;
    GPIO_WriteBit(GPIOB,  GPIO_PIN_12, GPIO_PIN_RESET);
	buffer_tt[j++] = Spi0TranceByte('<');
    for (i=3; i > 0; i--)
    {
        buffer_tt[j++] = Spi0TranceByte(head.bytes[i]);
        bcc = CRCByte(bcc, head.bytes[i]);
    }
	for (i = 0; i < len; i++)
	{
		buffer_tt[j++] = Spi0TranceByte(((uint8_t *)data)[i]);
		bcc = CRCByte(bcc, ((uint8_t *)data)[i]);
	}
    delay_us(5);
	res_bcc = Spi0TranceByte((bcc) & 0xff);
    delay_us(5);
	res_bcc = ((res_bcc) & 0xff) + (Spi0TranceByte((uint8_t)((bcc >> 8) & 0xff)) << 8);
    for (i=50; i; i++);
    GPIO_WriteBit(GPIOB,  GPIO_PIN_12, GPIO_PIN_SET);
	if (res_bcc == bcc)
	{
		return 0;
	}
    //ChipSPIInit();
	return -1;
}


int8_t ChipReadFrame(uint8_t fun, uint16_t addr, uint8_t len, void *data)
{
	uint8_t i, temp, j=0;
	uint16_t bcc = 0, res_bcc = 0;
    u_addr_t head;
    head.addr.rw = 1;
    head.addr.channel = fun;
    head.addr.addr = addr;
    head.addr.comm_len = len;
	if (len > 32)
	{
		return -1;
	}
    GPIO_WriteBit(GPIOB,  GPIO_PIN_12, GPIO_PIN_RESET);
	Spi0TranceByte('<');
    for (i=3; i > 0; i--)
    {
        buffer_tt[j++] = Spi0TranceByte(head.bytes[i]);
        bcc = CRCByte(bcc, head.bytes[i]);
    }
    delay_us(15);
	for (i = 0; i < len; i++)
	{
		temp = Spi0TranceByte(0x00);
		((uint8_t *)data)[i] = temp;
		bcc = CRCByte(bcc, ((uint8_t *)data)[i]);
        delay_us(5);
	}
	res_bcc = Spi0TranceByte((bcc) & 0xff);
    delay_us(5);
	res_bcc = ((res_bcc) & 0xff) + (Spi0TranceByte((uint8_t)((bcc >> 8) & 0xff)) << 8);
    for (i=50; i; i++);
    GPIO_WriteBit(GPIOB,  GPIO_PIN_12, GPIO_PIN_SET);
    
	if (res_bcc == bcc)
	{
		return 0;
	}
    //ChipSPIInit();
	return -1;
}







