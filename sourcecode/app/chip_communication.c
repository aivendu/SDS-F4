#include "chip_communication.h"
#include "ucos_ii.h"
#include "CRC.h"
#include "stm32f4xx.h"

/******************************************************************
** ��������:   SPI2_Configuration
** ��������:   ����SPI2������ģʽ 
** ����:	   ��
**
** ���:	   ��
** ȫ�ֱ���:
** ����ģ��: 
** ��ע:
** ����:	   arjun
** ����:	   20170825
******************************************************************/ 
void ChipSPIInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOAʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//ʹ��SPI1ʱ��

    //SPI2��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //����������
	GPIO_Init(GPIOB,&GPIO_InitStructure);
    
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);
    
    //SPI1_CS��ʼ��Ϊ���
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //����
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
    GPIO_WriteBit(GPIOB,  GPIO_PIN_12, GPIO_PIN_SET);

    //����ֻ���SPI�ڳ�ʼ��
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE);//��λSPI2
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);//ֹͣ��λSPI2

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	    //����SPI����ģʽ:����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   //����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		    //����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	    //����ͬ��ʱ�ӵĵ�һ�������أ��������½������ݱ�����
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		    //NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:SoftΪ�������
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial = 7;	        //CRCֵ����Ķ���ʽ
    SPI_Init(SPI2, &SPI_InitStructure);                 //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

    SPI_Cmd(SPI2, ENABLE); //ʹ��SPI����
    
}


uint16_t CRCByte(uint16_t crc, uint8_t data)
{
    return CRC16(crc, &data, 1);
}

uint8_t Spi0TranceByte(uint8_t data)
{
    uint8_t temp;
    
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_TXE) == RESET){}
    
    SPI_SendData(SPI2, data); 
    
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_RXNE) == RESET){}
    
    temp = SPI_ReceiveData(SPI2); 
    return temp;   
}

uint32_t chip_tick;
uint8_t  chip_communication_temp[32];
OS_EVENT *data_upload_sem, *chip_communication_sem;

void ChipCommInit(void)
{
    ChipSPIInit();
	data_upload_sem = OSSemCreate(0);		//	����PAD �����
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

/**  ͨ������ͷ�����ݽṹ
 */
//  ��׼֡��ͷ���ݽṹ
typedef struct 
{
    uint32_t unused  :  8;  //  δʹ�ã�ͷֻ�������ֽ�
    uint32_t comm_len:  8;  //  ��ǰ֡�����ݳ���
    uint32_t addr    : 10;  //  ��ǰ֡�����ĵ�ַ
    uint32_t channel :  5;  //  ��ǰ֡������ͨ������ֵΪADDR_EX_MASK����ʾ��ǰ֡����չ֡
    uint32_t rw      :  1;  //  ��ǰ֡�Ķ�д��ʶ,1:����0:д
} s_addr_t;

//  ��չ֡��ͷ���ݽṹ
typedef struct 
{
    uint32_t comm_len:  8;  //  ��ǰ֡�����ݳ���
    uint32_t addr    : 14;  //  ��ǰ֡�����ĵ�ַ
    uint32_t channel :  9;  //  ��ǰ֡������ͨ��
    uint32_t rw      :  1;  //  ��ǰ֡�Ķ�д��ʶ,1:����0:д
} s_addr_ex_t;
//  ����֡ͷ�����ݽṹ����׼֡3byte, ��չ֡4byte
typedef union 
{
    uint8_t bytes[4];
    s_addr_t addr;        //  ��׼֡��ͷ���ݽṹ
    s_addr_ex_t addr_ex;  //  ��չ֡��ͷ���ݽṹ
} u_addr_t;
uint8_t buffer_tt[128];
int8_t ChipWriteFrame(uint8_t fun, uint16_t addr, uint8_t len, void *data)
{
	uint8_t i, j=0;
	uint16_t bcc = 0;
	uint16_t res_bcc = 0;
    uint32_t cpu_sr;
    u_addr_t head;
    head.addr.rw = 0;
    head.addr.channel = fun;
    head.addr.addr = addr;
    head.addr.comm_len = len;
    OS_ENTER_CRITICAL();
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
	res_bcc = Spi0TranceByte((bcc >> 8) & 0xff);
    delay_us(5);
	res_bcc = ((res_bcc << 8) & 0xff00) + Spi0TranceByte((uint8_t)(bcc & 0xff));
    for (i=50; i; i++);
    GPIO_WriteBit(GPIOB,  GPIO_PIN_12, GPIO_PIN_SET);
    OS_EXIT_CRITICAL();
	if (res_bcc == bcc)
	{
		return 0;
	}
	return -1;
}


int8_t ChipReadFrame(uint8_t fun, uint16_t addr, uint8_t len, void *data)
{
	uint8_t i, temp, j=0;
	uint16_t bcc = 0, res_bcc = 0;
    uint32_t cpu_sr;
    u_addr_t head;
    head.addr.rw = 1;
    head.addr.channel = fun;
    head.addr.addr = addr;
    head.addr.comm_len = len;
	if (len > 32)
	{
		return -1;
	}
    OS_ENTER_CRITICAL();
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
	res_bcc = Spi0TranceByte((bcc >> 8) & 0xff);
    delay_us(5);
	res_bcc = ((res_bcc << 8) & 0xff00) + Spi0TranceByte((uint8_t)(bcc & 0xff));
    for (i=50; i; i++);
    GPIO_WriteBit(GPIOB,  GPIO_PIN_12, GPIO_PIN_SET);
    
    OS_EXIT_CRITICAL();
	if (res_bcc == bcc)
	{
		return 0;
	}
	return -1;
}







