#ifndef _MALLOC_H
#define _MALLOC_H
#include "stdint.h"
	 



#ifndef NULL
#define NULL 0
#endif

//定义三个内存池
#define SRAMIN 	0  //内部内存池
#define SRAMEX 	1  //外部内存池
#define SRAMCCM 2  //CCM内存池(此部分SRAM仅仅CPU可以访问！！！)

#define SRAMBANK  3 //定义支持的SRAM块数


//mem1内存参数设定,mem1完全处于内部SRAM里面
#define MEM1_BLOCK_SIZE	32  			//内存块大小为32字节
#define MEM1_MAX_SIZE		55*1024 	//最大管理内存
#define MEM1_ALLOC_TABLE_SIZE MEM1_MAX_SIZE/MEM1_BLOCK_SIZE  //内存表大小

//mem2内存参数设定,mem2处于外部SRAM里面
#define MEM2_BLOCK_SIZE	32  			//内存块大小为32字节
#define MEM2_MAX_SIZE		1*1024 	//最大管理内存 k
#define MEM2_ALLOC_TABLE_SIZE MEM2_MAX_SIZE/MEM2_BLOCK_SIZE  //内存表大小

//mem3内存参数设定,mem3处于CCM,用于管理CCM(特别注意,这部分SRAM,近CPU可以访问)
#define MEM3_BLOCK_SIZE	32  			//内存块大小为32字节
#define MEM3_MAX_SIZE		1*1024 	//最大管理内存 k
#define MEM3_ALLOC_TABLE_SIZE MEM3_MAX_SIZE/MEM3_BLOCK_SIZE  //内存表大小

//内存管理控制器
struct _m_mallco_dev
{
	void (*init)(uint8_t);  		//初始化
	uint8_t (*perused)(uint8_t); 		//内存使用率
	uint8_t *membase[SRAMBANK]; //内存池,管理SRAMBANK个区域的内存
	uint16_t *memmap[SRAMBANK];  //内存状态表
	uint8_t memrdy[SRAMBANK];   //内存管理是否就绪
};
extern struct _m_mallco_dev mallco_dev;  //在malloc.c里面定义

void mymemset(void *s,uint8_t c,uint32_t count);	 //设置内存
void mymemcpy(void *des,void *src,uint32_t n);//复制内存     
void mymem_init(uint8_t memx);					 //内存管理初始化函数(外/内部调用)
uint32_t mymem_malloc(uint8_t memx,uint32_t size);		 //内存分配(内部调用)
uint8_t mymem_free(uint8_t memx,uint32_t offset);		 //内存释放(内部调用)
uint8_t mem_perused(uint8_t memx);				        //获得内存使用率(外/内部调用) 
////////////////////////////////////////////////////////////////////////////////
//用户调用函数
void myfree(uint8_t memx,void *ptr);  			//内存释放(外部调用)
void *mymalloc(uint8_t memx,uint32_t size);			//内存分配(外部调用)
void *myrealloc(uint8_t memx,void *ptr,uint32_t size);//重新分配内存(外部调用)
#endif


