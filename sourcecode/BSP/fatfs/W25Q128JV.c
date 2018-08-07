#include "W25Q128JV.h"
#include "stm32f4xx.h"
#include "stdlib.h"
#include "string.h"

#define  W25QXX_SPI_SELECT      1

uint16_t W25QXX_TYPE=0;	//默认是W25Q128

#if  W25QXX_SPI_SELECT == 2
#define	W25QXX_CS 		GPIO_PIN_9  		//W25QXX的片选信号
#define W25QXX_CS_PORT  GPIOB
#define W25QXX_SPI      SPI2
#else
#define	W25QXX_CS 		GPIO_PIN_15  		//W25QXX的片选信号
#define W25QXX_CS_PORT  GPIOA
#define W25QXX_SPI      SPI1
#endif

static void W25QXXDelay_us(uint32_t t)
{
    t *= 20;
    while(t--);
}

void W25QxxSetCS(uint8_t st)
{
    BitAction cmd = st==0?GPIO_PIN_RESET:GPIO_PIN_SET;
    GPIO_WriteBit(W25QXX_CS_PORT, W25QXX_CS, cmd);
}

int8_t W25QXX_ReadWriteByte(uint8_t flag, uint8_t *tx, uint8_t *rx, uint32_t size)
{
    int8_t err = 0, temp;
	uint32_t i;
    W25QxxSetCS(0);
    
    for (i=0; i< size; i++)
    {
        while (SPI_I2S_GetFlagStatus(W25QXX_SPI, SPI_I2S_FLAG_TXE) == RESET){}//等待发送区空  
        
        if (tx)   {temp = tx[i];}
        else      temp = 0xFF;
        SPI_I2S_SendData(W25QXX_SPI, temp); //通过外设SPIx发送一个byte  数据
        
        while (SPI_I2S_GetFlagStatus(W25QXX_SPI, SPI_I2S_FLAG_RXNE) == RESET){} //等待接收完一个byte  
        
        temp = SPI_I2S_ReceiveData(W25QXX_SPI); //返回通过SPIx最近接收的数据	
        if (rx)   tx[i] = temp;
        
    }
    if (flag) 
    {
        W25QxxSetCS(1);        
    }
    return err;   
}

//4Kbytes为一个Sector
//16个扇区为1个Block
//W25Q128
//容量为16M字节,共有128个Block,4096个Sector 
													 
//初始化SPI FLASH的IO口
void W25QXX_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOB时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);//使能GPIOG时钟

	W25QxxSetCS(1);			//SPI FLASH不选中
    
	//GPIOB14
    GPIO_InitStructure.GPIO_Pin = W25QXX_CS;//PB14
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(W25QXX_CS_PORT, &GPIO_InitStructure);//初始化

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;//PB3~5复用功能输出	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1); //PB3复用为 SPI1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1); //PB4复用为 SPI1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1); //PB5复用为 SPI1

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//选择了串行时钟的稳态:时钟悬空低
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//数据捕获于第二个时钟沿
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    //设置为10M时钟,高速模式
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//定义波特率预分频的值:波特率预分频值为8	
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
    SPI_Init(W25QXX_SPI, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    SPI_Cmd(W25QXX_SPI, ENABLE); //使能SPI外设
    
    
    
    
    
    
    // GPIO_InitTypeDef  GPIO_InitStructure;
    // SPI_InitTypeDef  SPI_InitStructure;

    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOB时钟
    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOA时钟
    // RCC_APB1PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//使能SPI1时钟

    //SPI2初始化
	// GPIO_InitStructure.GPIO_Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
	// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	// GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	// GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //不带上下拉
	// GPIO_Init(GPIOB,&GPIO_InitStructure);
    
	// GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI1);
	// GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI1);
	// GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI1);
    
    //SPI1_CS初始化为输出
    // GPIO_InitStructure.GPIO_Pin = GPIO_PIN_15;
	// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	// GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	// GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //上拉
	// GPIO_Init(GPIOA,&GPIO_InitStructure); 
    // GPIO_WriteBit(GPIOA,  GPIO_PIN_15, GPIO_PIN_SET);

    //这里只针对SPI口初始化
    // RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI1,ENABLE);//复位SPI2
    // RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI1,DISABLE);//停止复位SPI2

    // SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    // SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	    //设置SPI工作模式:设置为主SPI
    // SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   //设置SPI的数据大小:SPI发送接收8位帧结构
    // SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		    //串行同步时钟的空闲状态为低电平
    // SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	    //串行同步时钟的第一个跳变沿（上升或下降）数据被采样
    // SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		    //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:Soft为软件控制
    // SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;//定义波特率预分频的值:波特率预分频值为256
    // SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    // SPI_InitStructure.SPI_CRCPolynomial = 7;	        //CRC值计算的多项式
    // SPI_Init(SPI2, &SPI_InitStructure);                 //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    // SPI_Cmd(SPI2, ENABLE); //使能SPI外设
    
    
    
    
    
    
    
    

    {
        uint8_t buffer[1] = {0xFF};
        W25QXX_ReadWriteByte(1, buffer,buffer,1);//启动传输		 
    }
	W25QXX_TYPE = W25QXX_ReadID();	//读取FLASH ID.
}  



//读取W25QXX的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
uint8_t W25QXX_ReadSR(void)   
{  
    uint8_t buffer[2] = {W25X_ReadStatusReg, 0Xff};
    W25QXX_ReadWriteByte(1, buffer, buffer, 2);
	return buffer[1];   
} 
//写W25QXX状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void W25QXX_Write_SR(uint8_t sr)   
{   
    uint8_t buffer[2] = {W25X_WriteStatusReg, 0Xff};
    buffer[1] = sr;
    W25QXX_ReadWriteByte(1, buffer, buffer, 2);	      
}   
//W25QXX写使能	
//将WEL置位   
void W25QXX_Write_Enable(void)   
{
    uint8_t buffer[1] = {W25X_WriteEnable};
    W25QXX_ReadWriteByte(1, buffer, buffer, 1);  	      
} 
//W25QXX写禁止	
//将WEL清零  
void W25QXX_Write_Disable(void)   
{  
    uint8_t buffer[1] = {W25X_WriteDisable};
    W25QXX_ReadWriteByte(1, buffer, buffer, sizeof(buffer));  	      
} 		
//读取芯片ID
//返回值如下:				   
//0XEF13,表示芯片型号为W25Q80  
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
//0XEF17,表示芯片型号为W25Q128 	  
uint16_t W25QXX_ReadID(void)
{
    uint8_t buffer[6] = {W25X_ManufactDeviceID, 0x00, 0x00, 0x00, 0xFF, 0xFF};
    W25QXX_ReadWriteByte(1, buffer, buffer, sizeof(buffer)); 	
    return (buffer[4]<<8) + buffer[5];
}   		    
//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
int8_t W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{
    uint8_t buffer[4];
    
    buffer[0] = W25X_ReadData;
    buffer[1] = (uint8_t)((ReadAddr)>>16);
    buffer[2] = (uint8_t)((ReadAddr)>>8);
    buffer[3] = (uint8_t)ReadAddr;
    if (W25QXX_ReadWriteByte(0, buffer, buffer, 4) >= 0)
    {
        //memcpy(pBuffer, &buffer[4], NumByteToRead);
        return W25QXX_ReadWriteByte(1, pBuffer, pBuffer, NumByteToRead);
    }
    return -1;
}  
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
int8_t W25QXX_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint8_t buffer[4];
    int8_t err;
    W25QXX_Write_Enable();                  //SET WEL 
    buffer[0] = W25X_PageProgram;
    buffer[1] = (uint8_t)((WriteAddr)>>16);
    buffer[2] = (uint8_t)((WriteAddr)>>8);
    buffer[3] = (uint8_t)WriteAddr;
    err = W25QXX_ReadWriteByte(0, buffer, buffer, 4); 
    if (err >= 0)
    {
        err = W25QXX_ReadWriteByte(1, pBuffer, pBuffer, NumByteToWrite); 
    }
	W25QXX_Wait_Busy();					   //等待写入结束
	return err;
} 
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
int8_t W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 			 		 
	uint16_t pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		if (W25QXX_Write_Page(pBuffer,WriteAddr,pageremain) < 0)
		{
            return -1;
		}
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};
        return 0;
} 
//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)						
//NumByteToWrite:要写入的字节数(最大65535)   
uint8_t W25QXX_BUFFER[4096];		 
int8_t W25QXX_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    
	uint8_t * W25QXX_BUF;	  
   	W25QXX_BUF=W25QXX_BUFFER;	     
 	secpos=WriteAddr/4096;//扇区地址  
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		if (W25QXX_Read(W25QXX_BUF,secpos*4096,4096) < 0)//读出整个扇区的内容
		    return -1;
		for(i=0;i<secremain;i++)//校验数据
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			W25QXX_Erase_Sector(secpos);//擦除这个扇区
			
			for(i=0;i<secremain;i++)	   //复制
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];	  
			}
			if (W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096) < 0)//写入整个扇区  
			    return -1;

		}else 
        {
		    if (W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain) < 0)//写已经擦除了的,直接写入扇区剩余区间. 				   
		        return -1;
		}
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	}
    return 0;
}
//擦除整个芯片		  
//等待时间超长...
void W25QXX_Erase_Chip(void)   
{                      
    uint8_t buffer[1] = {W25X_ChipErase};    
    W25QXX_Write_Enable();                  //SET WEL 
    W25QXX_Wait_Busy();   
    W25QXX_ReadWriteByte(1, buffer,buffer,1);        //发送片擦除命令     	      
	W25QXX_Wait_Busy();   				   //等待芯片擦除结束
}   
//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个山区的最少时间:150ms
void W25QXX_Erase_Sector(uint32_t Dst_Addr)   
{  
    uint8_t buffer[4];
	//监视falsh擦除情况,测试用   
 	Dst_Addr*=4096;
    W25QXX_Write_Enable();                  //SET WEL 	 
    W25QXX_Wait_Busy();
    buffer[0] = W25X_SectorErase;
    buffer[1] = (uint8_t)((Dst_Addr)>>16);
    buffer[2] = (uint8_t)((Dst_Addr)>>8);
    buffer[3] = (uint8_t)Dst_Addr;
    W25QXX_ReadWriteByte(1, buffer, buffer, 4);
    //HAL_SPI_TransmitReceive(w25q_hspi, buffer, buffer, 4, 10);
  	//W25QxxSetCS(0);                            //使能器件   
    //W25QXX_ReadWriteByte(W25X_SectorErase);      //发送扇区擦除指令 
    //W25QXX_ReadWriteByte((uint8_t)((Dst_Addr)>>16));  //发送24bit地址    
    //W25QXX_ReadWriteByte((uint8_t)((Dst_Addr)>>8));   
    //W25QXX_ReadWriteByte((uint8_t)Dst_Addr);  
	//W25QxxSetCS(1);                            //取消片选     	      
    W25QXX_Wait_Busy();   				   //等待擦除完成
}  
//等待空闲
void W25QXX_Wait_Busy(void)   
{   
	while((W25QXX_ReadSR()&0x01)==0x01);   // 等待BUSY位清空
}  
//进入掉电模式
void W25QXX_PowerDown(void)   
{ 
  	//W25QxxSetCS(0);                            //使能器件   
    //W25QXX_ReadWriteByte(W25X_PowerDown);        //发送掉电命令  
	//W25QxxSetCS(1);                            //取消片选     	  
	uint8_t buffer[1] = {W25X_PowerDown};
    W25QXX_ReadWriteByte(1, buffer, buffer, 1);
    W25QXXDelay_us(3);                               //等待TPD  
}   
//唤醒
void W25QXX_WAKEUP(void)   
{  
	uint8_t buffer[1] = {W25X_ReleasePowerDown};
    W25QXX_ReadWriteByte(1, buffer, buffer, 1);  	      
    W25QXXDelay_us(3);                               //等待TRES1
}   



































