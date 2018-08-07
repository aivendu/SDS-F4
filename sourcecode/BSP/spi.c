#include "spi.h"
#include "cj01_gpio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_gpio.h"

#define SPI2_NSS_F_GPIO_Port    GPIOA
#define SPI2_NSS_F_Pin          GPIO_PIN_15
#define SPI2_NSS_WK_GPIO_Port   GPIOB
#define SPI2_NSS_WK_Pin         GPIO_PIN_12

#define SPI_USED    SPI1

void InitSPI(int flag)
{    
    SPI_Cmd(SPI_USED, DISABLE); //使能SPI外设
    SPI_InitTypeDef  SPI_InitStructure;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//选择了串行时钟的稳态:时钟悬空低
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//数据捕获于第二个时钟沿
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    
    if (flag == 1)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//定义波特率预分频的值:波特率预分频值为2	
    }
    else
    {
        //设置为10M时钟,高速模式
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		//定义波特率预分频的值:波特率预分频值为8
    }
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
    SPI_Init(SPI_USED, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
    
    SPI_Cmd(SPI_USED, ENABLE); //使能SPI外设
}


static volatile uint8_t spi2_lock = 0;
#define SPI2_LOCK_W25Q     1
#define SPI2_LOCK_WK2124   2
#define SPI2_LOCK_MASK     0x80
#define isSPI2Lock(a)   ((spi2_lock &= ~a))
void W25QxxSetCS(uint8_t st)
{
    if (st == 0)
    {
        while ((spi2_lock & (SPI2_LOCK_MASK + SPI2_LOCK_W25Q)) == SPI2_LOCK_MASK)  OSTimeDly(2);
        if ((spi2_lock & SPI2_LOCK_W25Q) != SPI2_LOCK_W25Q)
        {
            InitSPI(1);
        }
        spi2_lock = SPI2_LOCK_W25Q + SPI2_LOCK_MASK;
        GPIO_WriteBit(SPI2_NSS_F_GPIO_Port, SPI2_NSS_F_Pin, GPIO_PIN_RESET);
    }
    else
    {
        GPIO_WriteBit(SPI2_NSS_F_GPIO_Port, SPI2_NSS_F_Pin, GPIO_PIN_SET);
        spi2_lock &= (~SPI2_LOCK_MASK);
    }
}

void WK2124NssSet(uint8_t st)
{
    if (st == 0)
    {
        while ((spi2_lock & (SPI2_LOCK_MASK + SPI2_LOCK_WK2124)) == SPI2_LOCK_MASK)  OSTimeDly(2);
        if ((spi2_lock & SPI2_LOCK_WK2124) != SPI2_LOCK_WK2124)
        {
            InitSPI(0);
        }
        spi2_lock = SPI2_LOCK_WK2124 + SPI2_LOCK_MASK;
        GPIO_WriteBit(SPI2_NSS_WK_GPIO_Port, SPI2_NSS_WK_Pin, GPIO_PIN_RESET);
    }
    else
    {
        GPIO_WriteBit(SPI2_NSS_WK_GPIO_Port, SPI2_NSS_WK_Pin, GPIO_PIN_SET);
        spi2_lock &= (~SPI2_LOCK_MASK);
    }
}


void SPIMspInit(void)
{
    SPI_TypeDef *spi = SPI_USED;
    
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOB时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);//使能GPIOG时钟

	  //GPIOB14
    GPIO_InitStructure.GPIO_Pin = SPI2_NSS_F_Pin;//PB14
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(SPI2_NSS_F_GPIO_Port, &GPIO_InitStructure);//初始化

	  //GPIOB14
    GPIO_InitStructure.GPIO_Pin = SPI2_NSS_WK_Pin;//PB14
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(SPI2_NSS_WK_GPIO_Port, &GPIO_InitStructure);//初始化

	W25QxxSetCS(1);			//SPI FLASH不选中
	WK2124NssSet(1);
	
    if (spi == SPI2)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        GPIO_InitStructure.GPIO_Pin = GPIO_PIN_13;//PB3~5复用功能输出	
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
        GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
        
        GPIO_InitStructure.GPIO_Pin = GPIO_PIN_3 | GPIO_PIN_2;//PB3~5复用功能输出	
        GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化
        
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2); //PB13复用为 SPI2
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_SPI2); //PC3复用为 SPI2
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2); //PC4复用为 SPI2
    }
    else if (spi == SPI1)
    {
        RCC_APB1PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
        GPIO_InitStructure.GPIO_Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;//PB3~5复用功能输出	
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
        GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1); //PB3复用为 SPI1
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1); //PB4复用为 SPI1
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1); //PB5复用为 SPI1
    }
    InitSPI(0);
//    {
//        uint8_t temp = 0xFF;
//        while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET)
//            SPI_I2S_SendData(spi, temp);
//        while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET){}
//            temp = SPI_I2S_ReceiveData(spi);
//    }
}


int8_t W25QXX_ReadWriteByte(uint8_t flag, uint8_t *tx, uint8_t *rx, uint32_t size)
{
    int8_t err = 0, temp;
    uint32_t i;
    W25QxxSetCS(0);
    for (i=0; i< size; i++)
    {
        while (SPI_I2S_GetFlagStatus(SPI_USED, SPI_I2S_FLAG_TXE) == RESET){}//等待发送区空  
        
        if (tx)   {temp = tx[i];}
        else      temp = 0xFF;
        SPI_I2S_SendData(SPI_USED, temp); //通过外设SPIx发送一个byte  数据
        
        while (SPI_I2S_GetFlagStatus(SPI_USED, SPI_I2S_FLAG_RXNE) == RESET){} //等待接收完一个byte  
        
        temp = SPI_I2S_ReceiveData(SPI_USED); //返回通过SPIx最近接收的数据	
        if (rx)   tx[i] = temp;
        
    }
    if (flag) 
    {
        W25QxxSetCS(1);        
    }
    return err;   
}

int8_t WK2124_ReadWriteByte(uint8_t flag, uint8_t *tx, uint8_t *rx, uint32_t size)
{
    int8_t err = 0, temp;
    uint32_t i;
    WK2124NssSet(0);
    for (i=0; i< size; i++)
    {
        while (SPI_I2S_GetFlagStatus(SPI_USED, SPI_I2S_FLAG_TXE) == RESET){}//等待发送区空  
        {
            if (tx)   temp = tx[i];
            else      temp = 0xFF;
            SPI_I2S_SendData(SPI_USED, temp); //通过外设SPIx发送一个byte  数据
        }   
        while (SPI_I2S_GetFlagStatus(SPI_USED, SPI_I2S_FLAG_RXNE) == RESET){} //等待接收完一个byte  
        {
            temp = SPI_I2S_ReceiveData(SPI_USED); //返回通过SPIx最近接收的数据	
            if (rx)   tx[i] = temp;
        }
    }
    if (flag) 
    {
        WK2124NssSet(1);        
    }
    return err;   
}


