#include "netif/ethernetif.h" 
#include "bsp_includes.h" 
//#include "res_network.h" 
#include "netif/etharp.h"  
#include "lwip/dhcp.h"
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "string.h"  
#include "my_malloc.h"
#include "tcpip.h"
#include "memp.h"

struct netif lwip_netif;

void lwip_pkt_handle(void)
{
    ethernetif_input(&lwip_netif);
}

extern uint32_t memp_get_memorysize(void);	//在memp.c里面定义
extern uint8_t *memp_memory;				//在memp.c里面定义.
extern uint8_t *ram_heap;					//在mem.c里面定义.

//lwip内核部分,内存释放
void lwip_comm_mem_free(void)
{ 	
	myfree(SRAMEX,memp_memory);
	myfree(SRAMEX,ram_heap);
}

//lwip内核部分,内存申请
//返回值:0,成功;
//    其他,失败
uint8_t lwip_comm_mem_malloc(void)
{
	uint32_t mempsize;
	uint32_t ramheapsize; 
	mempsize=memp_get_memorysize();			//得到memp_memory数组大小
	memp_memory=mymalloc(SRAMEX,mempsize);	//为memp_memory申请内存
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//得到ram heap大小
	ram_heap=mymalloc(SRAMEX,ramheapsize);	//为ram_heap申请内存 
	if(!memp_memory||!ram_heap)//有申请失败的
	{
		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}
//LWIP初始化(LWIP启动的时候使用,此函数在通信任务中调用，只调用一次)
//返回值:0,成功
//      1,内存错误
//      2,LAN8720初始化失败
//      3,网卡添加失败.
int8_t InitLocalNet(uint8_t dhcp, ip_addr_t ipaddr, ip_addr_t netmask, ip_addr_t gateway)
{   
	OS_CPU_SR cpu_sr;
	struct netif *Netif_Init_Flag;		//调用netif_add()函数时的返回值,用于判断网络初始化是否成功
	if(ETH_Mem_Malloc())
    {
        return 1;		//内存申请失败
    }
	if(lwip_comm_mem_malloc())
    {
        return 1;	//内存申请失败
    }
	if(LAN8720_Init())
    {
        return 2;			//初始化LAN8720失败 
    }
	tcpip_init(NULL,NULL);				//初始化tcp ip内核,该函数里面会创建tcpip_thread内核任务
    if (dhcp)
    {
        ipaddr.addr = 0;
        netmask.addr = 0;
        gateway.addr = 0;
    }
	OS_ENTER_CRITICAL();  //进入临界区
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gateway,NULL,&ethernetif_init,&tcpip_input);//向网卡列表中添加一个网口
	OS_EXIT_CRITICAL();  //退出临界区
	if(Netif_Init_Flag==NULL)
    {
        return 3;//网卡添加失败
    }
	else//网口添加成功后,设置netif为默认值,并且打开netif网口
	{
		netif_set_default(&lwip_netif); //设置netif为默认网口
		netif_set_up(&lwip_netif);		//打开netif网口
	}
    
    if (dhcp)
    {
        dhcp_start(&lwip_netif);//开启DHCP 
        //while (lwip_netif.dhcp->state != DHCP_BOUND)
        {
            OSTimeDly(OS_TICKS_PER_SEC/50);
        }
    }
	return 0;//初始化成功
}



//由ethernetif_init()调用用于初始化硬件
//netif:网卡结构体指针 
//返回值:ERR_OK,正常
//       其他,失败
static err_t low_level_init(struct netif *netif)
{
#ifdef CHECKSUM_BY_HARDWARE
	int i; 
#endif 
	//初始化MAC地址,设置什么地址由用户自己设置,但是不能与网络中其他设备MAC地址重复
	u32 sn0;
	sn0=*(vu32*)(0x1FFF7A10);//获取STM32的唯一ID的前24位作为MAC地址后三字节
   
	netif->hwaddr[0]=2; //高三字节(IEEE称之为组织唯一ID,OUI)地址固定为:2.0.0
	netif->hwaddr[1]='D'; 
	netif->hwaddr[2]='F';
	netif->hwaddr[3]=(sn0>>16)&0XFF;//低三字节用STM32的唯一ID
	netif->hwaddr[4]=(sn0>>8)&0XFFF;
	netif->hwaddr[5]=sn0&0XFF;
    
	netif->hwaddr_len = ETHARP_HWADDR_LEN; //设置MAC地址长度,为6个字节
	netif->mtu=1500; //最大允许传输单元,允许该网卡广播和ARP功能
	//并且该网卡允许有硬件链路连接
	netif->flags = NETIF_FLAG_BROADCAST|NETIF_FLAG_ETHARP|NETIF_FLAG_LINK_UP;
	
	ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr); //向STM32F4的MAC地址寄存器中写入MAC地址
	ETH_DMATxDescChainInit(DMATxDscrTab, Tx_Buff, ETH_TXBUFNB);
	ETH_DMARxDescChainInit(DMARxDscrTab, Rx_Buff, ETH_RXBUFNB);
#ifdef CHECKSUM_BY_HARDWARE 	//使用硬件帧校验
	for(i=0;i<ETH_TXBUFNB;i++)	//使能TCP,UDP和ICMP的发送帧校验,TCP,UDP和ICMP的接收帧校验在DMA中配置了
	{
		ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
	}
#endif
	ETH_Start(); //开启MAC和DMA				
	return ERR_OK;
} 
//用于发送数据包的最底层函数(lwip通过netif->linkoutput指向该函数)
//netif:网卡结构体指针
//p:pbuf数据结构体指针
//返回值:ERR_OK,发送正常
//       ERR_MEM,发送失败
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	u8 res;
	struct pbuf *q;
	int l = 0;
	u8 *buffer=(u8 *)ETH_GetCurrentTxBuffer(); 
	for(q=p;q!=NULL;q=q->next) 
	{
		memcpy((u8_t*)&buffer[l], q->payload, q->len);
		l=l+q->len;
	} 
	res=ETH_Tx_Packet(l); 
	if(res==ETH_ERROR)return ERR_MEM;//返回错误状态
	return ERR_OK;
} 
//用于接收数据包的最底层函数
//neitif:网卡结构体指针
//返回值:pbuf数据结构体指针
static struct pbuf * low_level_input(struct netif *netif)
{  
	struct pbuf *p, *q;
	u16_t len;
	int l =0;
	FrameTypeDef frame;
	u8 *buffer;
	p = NULL;
	frame=ETH_Rx_Packet();
	len=frame.length;//得到包大小
	buffer=(u8 *)frame.buffer;//得到包数据地址 
	p=pbuf_alloc(PBUF_RAW,len,PBUF_POOL);//pbufs内存池分配pbuf
	if(p!=NULL)
	{
		for(q=p;q!=NULL;q=q->next)
		{
			memcpy((u8_t*)q->payload,(u8_t*)&buffer[l], q->len);
			l=l+q->len;
		}    
	}
	frame.descriptor->Status=ETH_DMARxDesc_OWN;//设置Rx描述符OWN位,buffer重归ETH DMA 
	if((ETH->DMASR&ETH_DMASR_RBUS)!=(u32)RESET)//当Rx Buffer不可用位(RBUS)被设置的时候,重置它.恢复传输
	{ 
		ETH->DMASR=ETH_DMASR_RBUS;//重置ETH DMA RBUS位 
		ETH->DMARPDR=0;//恢复DMA接收
	}
	return p;
} 
//网卡接收数据(lwip直接调用)
//netif:网卡结构体指针
//返回值:ERR_OK,发送正常
//       ERR_MEM,发送失败
err_t ethernetif_input(struct netif *netif)
{
	err_t err;
	struct pbuf *p;
	p=low_level_input(netif);
	if(p==NULL) return ERR_MEM;
	err=netif->input(p, netif);
	if(err!=ERR_OK)
	{
		LWIP_DEBUGF(NETIF_DEBUG,("ethernetif_input: IP input error\n"));
		pbuf_free(p);
		p = NULL;
	} 
	return err;
} 
//使用low_level_init()函数来初始化网络
//netif:网卡结构体指针
//返回值:ERR_OK,正常
//       其他,失败
err_t ethernetif_init(struct netif *netif)
{
	LWIP_ASSERT("netif!=NULL",(netif!=NULL));
#if LWIP_NETIF_HOSTNAME			//LWIP_NETIF_HOSTNAME 
	netif->hostname="lwip";  	//初始化名称
#endif 
	netif->name[0]=IFNAME0; 	//初始化变量netif的name字段
	netif->name[1]=IFNAME1; 	//在文件外定义这里不用关心具体值
	netif->output=etharp_output;//IP层发送数据包函数
	netif->linkoutput=low_level_output;//ARP模块发送数据包函数
	low_level_init(netif); 		//底层硬件初始化函数
	return ERR_OK;
}

int8_t IsNetCableConnect(void)
{
    return ((ETH_ReadPHYRegister(LAN8720_PHY_ADDRESS, PHY_BSR) & PHY_Linked_Status) != 0x00)?1:0;
}











