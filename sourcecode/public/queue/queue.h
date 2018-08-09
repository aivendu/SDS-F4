/****************************************Copyright (c)**************************************************
**                               广州周立功单片机发展有限公司
**                                     研    究    所
**                                        产品一部
**
**                                 http://www.zlgmcu.com
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: queue.h
**创   建   人: 陈明计
**最后修改日期: 2003年7月2日
**描        述: 数据队列的中间件
**
**--------------历史版本信息----------------------------------------------------------------------------
** 创建人: 陈明计
** 版  本: v1.0
** 日　期: 2003年7月2日
** 描　述: 原始版本
**
**--------------当前版本修订------------------------------------------------------------------------------
** 修改人: 陈明计
** 日　期: 2004年5月19日
** 描　述: 改正注释错误
**
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef _QUEUE_H
#define _QUEUE_H

#define ATOMIC_LOCK_ENALBE         1

#if ATOMIC_LOCK_ENALBE == 1
#include "os_cpu.h"
#endif
#include "stdint.h"

#ifndef NOT_OK
#define NOT_OK              0xff                        /* 参数错误                                     */
#endif



#define QUEUE_FUNCTION      1

#define QUEUE_FULL          8                           /* 队列满   */
#define QUEUE_EMPTY         4                           /* 无数据   */
#define QUEUE_OK            1                           /* 操作成功 */
#define QUEUE_ERROR         0xff                        /* 参数错误                                        */


//#ifndef QUEUE_DATA_TYPE
#define QUEUE_DATA_TYPE     uint8_t
//#endif
#define EN_QUEUE_WRITE            1     /* 禁止(0)或允许(1)FIFO发送数据       */
//#define EN_QUEUE_WRITE_FRONT      0     /* 禁止(0)或允许(1)LIFO发送数据       */
#define EN_QUEUE_NDATA            1     /* 禁止(0)或允许(1)取得队列数据数目   */
#define EN_QUEUE_SIZE             1     /* 禁止(0)或允许(1)取得队列数据总容量 */
//#define EN_QUEUE_FLUSH            0     /* 禁止(0)或允许(1)清空队列           */


typedef struct
{
    QUEUE_DATA_TYPE     *Out;                   /* 指向数据输出位置         */
    QUEUE_DATA_TYPE     *In;                    /* 指向数据输入位置         */
    QUEUE_DATA_TYPE     *End;                   /* 指向Buf的结束位置        */
    uint16_t              NData;                  /* 队列中数据个数           */
    uint16_t              MaxData;                /* 队列中允许存储的数据个数 */

    uint8_t               (* ReadEmpty)();        /* 读空处理函数             */
    uint8_t               (* WriteFull)();        /* 写满处理函数             */
    QUEUE_DATA_TYPE     Buf[1];                 /* 存储数据的空间           */
} DataQueue;

typedef struct DataQueue2
{
    QUEUE_DATA_TYPE     *Out;                   /* 指向数据输出位置         */
    QUEUE_DATA_TYPE     *In;                    /* 指向数据输入位置         */
    QUEUE_DATA_TYPE     *End;                   /* 指向Buf的结束位置        */
    uint16_t              NData;                  /* 队列中数据个数           */
    QUEUE_DATA_TYPE     *Buf;                 /* 存储数据的空间           */
} DATAQEUE;



uint8_t QueueCreate(void *Buf,
                  uint32_t SizeOfBuf,
                  uint8_t (* ReadEmpty)(),
                  uint8_t (* WriteFull)()
                 );
/*********************************************************************************************************
** 函数名称: QueueCreate
** 功能描述: 初始化数据队列
** 输　入: Buf      ：为队列分配的存储空间地址
**         SizeOfBuf：为队列分配的存储空间大小（字节）
**         ReadEmpty：为队列读空时处理程序
**         WriteFull：为队列写满时处理程序
** 输　出: NOT_OK  :参数错误
**         QUEUE_OK:成功
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
********************************************************************************************************/

extern uint8_t QueueRead(QUEUE_DATA_TYPE *Ret, void *Buf);
/*********************************************************************************************************
** 函数名称: QueueRead
** 功能描述: 获取队列中的数据
** 输　入: Ret:存储返回的消息的地址
**         Buf:指向队列的指针
** 输　出: NOT_OK     ：参数错误
**         QUEUE_OK   ：收到消息
**         QUEUE_EMPTY：队列空
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
********************************************************************************************************/

extern uint8_t QueueWrite(void *Buf, QUEUE_DATA_TYPE Data);
/*********************************************************************************************************
** 函数名称: QueueWrite
** 功能描述: FIFO方式发送数据
** 输　入: Buf :指向队列的指针
**         Data:发送的数据
** 输　出: NOT_OK   ：参数错误
**         QUEUE_FULL:队列满
**         QUEUE_OK  :发送成功
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
********************************************************************************************************/

extern uint8_t QueueWriteFront(void *Buf, QUEUE_DATA_TYPE Data);
/*********************************************************************************************************
** 函数名称: QueueWrite
** 功能描述: LIFO方式发送数据
** 输　入: Buf:指向队列的指针
**         Data:消息数据
** 输　出: QUEUE_FULL:队列满
**         QUEUE_OK:发送成功
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
********************************************************************************************************/

extern uint16_t QueueNData(void *Buf);
/*********************************************************************************************************
** 函数名称: QueueNData
** 功能描述: 取得队列中数据数
** 输　入: Buf:指向队列的指针
** 输　出: 消息数
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
********************************************************************************************************/

extern uint16_t QueueSize(void *Buf);
/*********************************************************************************************************
** 函数名称: QueueSize
** 功能描述: 取得队列总容量
** 输　入: Buf:指向队列的指针
** 输　出: 队列总容量
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
********************************************************************************************************/

extern void QueueFlush(void *Buf);
/*********************************************************************************************************
** 函数名称: OSQFlush
** 功能描述: 清空队列
** 输　入: Buf:指向队列的指针
** 输　出: 无
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
********************************************************************************************************/




















typedef struct
{
    QUEUE_DATA_TYPE     *Out;                   /* 指向数据输出位置         */
    QUEUE_DATA_TYPE     *In;                    /* 指向数据输入位置         */
    QUEUE_DATA_TYPE     *End;                   /* 指向Buf的结束位置        */
    uint16_t            NData;                  /* 队列中数据个数           */
    uint16_t            MaxData;                /* 队列中允许存储的数据个数 */
    uint8_t (* ReadEmpty)();                    /* 读空处理函数             */
    uint8_t (* WriteFull)();                    /* 写满处理函数             */
    QUEUE_DATA_TYPE     Buf[1];                 /* 存储数据的空间           */
} s_queue_t;


#define QueueCreateInline(buffer,size,func_r,func_w,err_r)   \
    {\
        if (((buffer) != NULL) && ((size) > (sizeof(s_queue_t))))\
        {\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_ENTER_CRITICAL();\
            }\
            ((s_queue_t *)(buffer))->MaxData = ((size) - (uint32_t)(((s_queue_t *)0)->Buf)) /\
                                               sizeof(QUEUE_DATA_TYPE);               \
            ((s_queue_t *)(buffer))->End = ((s_queue_t *)(buffer))->Buf + ((s_queue_t *)(buffer))->MaxData;\
            ((s_queue_t *)(buffer))->Out = ((s_queue_t *)(buffer))->Buf;\
            ((s_queue_t *)(buffer))->In = ((s_queue_t *)(buffer))->Buf;\
            ((s_queue_t *)(buffer))->NData = 0;\
            ((s_queue_t *)(buffer))->ReadEmpty = (func_r);\
            ((s_queue_t *)(buffer))->WriteFull = (func_w);\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_EXIT_CRITICAL();\
            }\
            (err_r) = QUEUE_OK;\
        }\
        else\
        {\
            (err_r) = QUEUE_ERROR;\
        }\
    }

#define QueueWriteInline(buffer,data,err_r)\
    {\
        if ((buffer) != NULL)\
        {\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_ENTER_CRITICAL();\
            }\
            if (((s_queue_t *)(buffer))->NData < ((s_queue_t *)(buffer))->MaxData)\
            {\
                ((s_queue_t *)(buffer))->In[0] = (data);\
                ((s_queue_t *)(buffer))->In++;\
                if (((s_queue_t *)(buffer))->In >= ((s_queue_t *)(buffer))->End)\
                {\
                    ((s_queue_t *)(buffer))->In = ((s_queue_t *)(buffer))->Buf;\
                }\
                ((s_queue_t *)(buffer))->NData++;\
                (err_r) = QUEUE_OK;\
            }\
            else\
            {\
                (err_r) = QUEUE_FULL;\
                if (((s_queue_t *)(buffer))->WriteFull != NULL)\
                {\
                    (err_r) = ((s_queue_t *)(buffer))->WriteFull();\
                }\
            }\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_EXIT_CRITICAL();\
            }\
        }\
    }


#define QueueReadInline(buffer,data_r,err_r)\
    {\
        if ((buffer) != NULL)\
        {\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_ENTER_CRITICAL();\
            }\
            if (((s_queue_t *)(buffer))->NData > 0)\
            {\
                *(QUEUE_DATA_TYPE *)(data_r) = ((s_queue_t *)(buffer))->Out[0];\
                ((s_queue_t *)(buffer))->Out++;\
                if (((s_queue_t *)(buffer))->Out >= ((s_queue_t *)(buffer))->End)\
                {\
                    ((s_queue_t *)(buffer))->Out = ((s_queue_t *)(buffer))->Buf;\
                }\
                ((s_queue_t *)(buffer))->NData--;\
                (err_r) = QUEUE_OK;\
            }\
            else\
            {\
                (err_r) = QUEUE_EMPTY;\
                if (((s_queue_t *)(buffer))->ReadEmpty != NULL)\
                {\
                    (err_r) = ((s_queue_t *)(buffer))->ReadEmpty((data_r), ((s_queue_t *)(buffer)));\
                }\
            }\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_EXIT_CRITICAL();\
            }\
        }\
    }

#define QueueDataCountInline(buffer,num_r)\
    {\
        if (buffer != NULL)\
        {\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_ENTER_CRITICAL();\
            }\
            (num_r) = ((s_queue_t *)(buffer))->NData;\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_EXIT_CRITICAL();\
            }\
        }\
    }

/*

*/
#define QueueSizeInline(buffer,num_r)\
    {\
        if ((buffer) != NULL)\
        {\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_ENTER_CRITICAL();\
            }\
            (num_r) = ((s_queue_t *)(buffer))->MaxData;\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_EXIT_CRITICAL();\
            }\
        }\
    }

#define QueueFlushInline(buffer)\
    {\
        if ((buffer) != NULL)\
        {\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_ENTER_CRITICAL();\
            }\
            ((s_queue_t *)(buffer))->Out = ((s_queue_t *)(buffer))->Buf;\
            ((s_queue_t *)(buffer))->In  = ((s_queue_t *)(buffer))->Buf;\
            ((s_queue_t *)(buffer))->NData = 0;\
            if (ATOMIC_LOCK_ENALBE==1)\
            {\
                OS_EXIT_CRITICAL();\
            }\
        }\
    }

#endif

/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
