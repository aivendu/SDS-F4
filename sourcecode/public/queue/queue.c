﻿/****************************************Copyright (c)**************************************************
**                               广州周立功单片机发展有限公司
**                                     研    究    所
**                                        产品一部
**
**                                 http://www.zlgmcu.com
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: queue.c
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
** 描　述: 改正注释错误和常量引用错误
**
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#include "stdlib.h"
#include "queue.h"

/*typedef struct s_queue
{
    uint8_t (*create)(uint32_t);
    int8_t  (*empty)();
    int32_t (*size)();
    int8_t  (*front)();
    int8_t  (*back)();
    uint8_t (*push)();
    int8_t  (*emplace)();
    int8_t  (*pop)();
    int8_t  (*swap)();
    uint32_t *buffer;
} s_queue_t;*/


/*********************************************************************************************************
** 函数名称: QueueCreate
** 功能描述: 初始化数据队列
** 输　入: Buf      ：为队列分配的存储空间地址
**         SizeOfBuf：为队列分配的存储空间大小（字节）
**         ReadEmpty：为队列读空时处理程序
**         WriteFull：为队列写满时处理程序
** 输　出: NOT_OK:参数错误
**         QUEUE_OK:成功
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**
** 作　者: 陈明计
** 日　期: 2003年7月2日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
uint8_t QueueCreate(void *Buf,
    uint32_t SizeOfBuf,
    uint8_t(*ReadEmpty)(),
    uint8_t(*WriteFull)()
)
{
    DataQueue *Queue;
    if (Buf != NULL && SizeOfBuf >= (sizeof(DataQueue)))        /* 判断参数是否有效 */
    {
        Queue = (DataQueue *)Buf;
        /* 初始化结构体数据 */
        Queue->MaxData = (SizeOfBuf - (uint32_t)(((DataQueue *)0)->Buf)) /
                         sizeof(QUEUE_DATA_TYPE);               /* 计算队列可以存储的数据数目 */
        Queue->End = Queue->Buf + Queue->MaxData;               /* 计算数据缓冲的结束地址 */
        Queue->Out = Queue->Buf;
        Queue->In = Queue->Buf;
        Queue->NData = 0;
        Queue->ReadEmpty = ReadEmpty;
        Queue->WriteFull = WriteFull;
        return QUEUE_OK;
    }
    else
    {
        return NOT_OK;
    }
}


/*********************************************************************************************************
** 函数名称: QueueRead
** 功能描述: 获取队列中的数据
** 输　入: Ret:存储返回的消息的地址
**         Buf:指向队列的指针
** 输　出: NOT_OK     ：参数错误
**         QUEUE_OK   ：收到消息
**         QUEUE_EMPTY：无消息
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**
** 作　者: 陈明计
** 日　期: 2003年7月2日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
uint8_t QueueRead(QUEUE_DATA_TYPE *Ret, void *Buf)
{
    uint8_t err;
    DataQueue *Queue;

    err = NOT_OK;
    if (Buf != NULL)                                            /* 队列是否有效 */
    {
        /* 有效 */
        Queue = (DataQueue *)Buf;
        if (Queue->NData > 0)                                   /* 队列是否为空 */
        {
            /* 不空         */
            *Ret = Queue->Out[0];                               /* 数据出队     */
            Queue->Out++;                                       /* 调整出队指针 */
            if (Queue->Out >= Queue->End)
            {
                Queue->Out = Queue->Buf;
            }
            Queue->NData--;                                     /* 数据减少      */
            err = QUEUE_OK;
        }
        else
        {
            /* 空              */
            err = QUEUE_EMPTY;
            if (Queue->ReadEmpty != NULL)                       /* 调用用户处理函数 */
            {
                err = Queue->ReadEmpty(Ret, Queue);
            }
        }
    }
    return err;
}
uint8_t QueueCheck(QUEUE_DATA_TYPE *Ret, void *Buf)
{
    uint8_t err;
    DataQueue *Queue;

    err = NOT_OK;
    if (Buf != NULL)                                            /* 队列是否有效 */
    {
        /* 有效 */
        Queue = (DataQueue *)Buf;
        if (Queue->NData > 0)                                   /* 队列是否为空 */
        {
            /* 不空         */
            *Ret = Queue->Out[0];                               /* 数据出队     */
            //Queue->Out++;                                       /* 调整出队指针 */
            //if (Queue->Out >= Queue->End)
            //{
            //    Queue->Out = Queue->Buf;
            //}
            //Queue->NData--;                                     /* 数据减少      */
            err = QUEUE_OK;
        }
        else
        {
            /* 空              */
            err = QUEUE_EMPTY;
            if (Queue->ReadEmpty != NULL)                       /* 调用用户处理函数 */
            {
                err = Queue->ReadEmpty(Ret, Queue);
            }
        }
    }
    return err;
}

/*********************************************************************************************************
** 函数名称: QueueWrite
** 功能描述: FIFO方式发送数据
** 输　入: Buf :指向队列的指针
**         Data:消息数据
** 输　出: NOT_OK   :参数错误
**         QUEUE_FULL:队列满
**         QUEUE_OK  :发送成功
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**
** 作　者: 陈明计
** 日　期: 2003年7月2日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_QUEUE_WRITE
#define EN_QUEUE_WRITE      0
#endif

#if EN_QUEUE_WRITE > 0

uint8_t QueueWrite(void *Buf, QUEUE_DATA_TYPE Data)
{
    uint8_t err = 0;
    s_queue_t *Queue;

    err = QUEUE_ERROR;

    if (Buf != NULL)                                                     /* 队列是否有效 */
    {
        Queue = (s_queue_t *)Buf;
        if (Queue->NData < Queue->MaxData)                               /* 队列是否满  */
        {
            /* 不满        */
            Queue->In[0] = Data;                                        /* 数据入队    */
            Queue->In++;                                                /* 调整入队指针*/

            if (Queue->In >= Queue->End)
            {
                Queue->In = Queue->Buf;
            }

            Queue->NData++;                                             /* 数据增加    */
            err = QUEUE_OK;
        }
        else
        {
            /* 满           */
            err = QUEUE_FULL;

            if (Queue->WriteFull != NULL)                                /* 调用用户处理函数 */
            {
                //rr = Queue->WriteFull(Queue, Data, Q_WRITE_MODE);
            }
        }
    }
    return err;
}
#endif

/*********************************************************************************************************
** 函数名称: QueueWriteFront
** 功能描述: LIFO方式发送数据
** 输　入: Buf:指向队列的指针
**         Data:消息数据
** 输　出: QUEUE_FULL:队列满
**         QUEUE_OK:发送成功
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**
** 作　者: 陈明计
** 日　期: 2003年7月2日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_QUEUE_WRITE_FRONT
#define EN_QUEUE_WRITE_FRONT    0
#endif

#if EN_QUEUE_WRITE_FRONT > 0

uint8_t QueueWriteFront(void *Buf, QUEUE_DATA_TYPE Data)
{
    uint8_t err = 0;
    s_queue_t *Queue;
    err = QUEUE_ERROR;

    if (Buf != NULL)                                                     /* 队列是否有效 */
    {
        Queue = (s_queue_t *)Buf;
        if (Queue->NData < Queue->MaxData)                               /* 队列是否满  */
        {
            /* 不满 */
            Queue->Out--;                                               /* 调整出队指针 */

            if (Queue->Out < Queue->Buf)
            {
                Queue->Out = Queue->End - 1;
            }

            Queue->Out[0] = Data;                                       /* 数据入队     */
            Queue->NData++;                                             /* 数据数目增加 */
            err = QUEUE_OK;
        }
        else
        {
            /* 满           */
            err = QUEUE_FULL;

            if (Queue->WriteFull != NULL)                                /* 调用用户处理函数 */
            {
                // err = Queue->WriteFull(Queue, Data, Q_WRITE_FRONT_MODE);
            }
        }
    }
    return err;
}

#endif



/*********************************************************************************************************
** 函数名称: QueueNData
** 功能描述: 取得队列中数据数
** 输　入: Buf:指向队列的指针
** 输　出: 消息数
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**
** 作　者: 陈明计
** 日　期: 2003年7月2日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_QUEUE_NDATA
#define EN_QUEUE_NDATA    0
#endif

#if EN_QUEUE_NDATA > 0

uint16_t QueueNData(void *Buf)
{
    uint16_t temp;

    temp = 0;                                                   /* 队列无效返回0 */
    if (Buf != NULL)
    {
        temp = ((DataQueue *)Buf)->NData;
    }
    return temp;
}

#endif

/*********************************************************************************************************
** 函数名称: QueueSize
** 功能描述: 取得队列总容量
** 输　入: Buf:指向队列的指针
** 输　出: 队列总容量
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**
** 作　者: 陈明计
** 日　期: 2003年7月2日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_QUEUE_SIZE
#define EN_QUEUE_SIZE    0
#endif

#if EN_QUEUE_SIZE > 0

uint16_t QueueSize(void *Buf)
{
    uint16_t temp;

    temp = 0;                                                   /* 队列无效返回0 */
    if (Buf != NULL)
    {
        temp = ((DataQueue *)Buf)->MaxData;
    }
    return temp;
}

#endif


/*********************************************************************************************************
** 函数名称: OSQFlush
** 功能描述: 清空队列
** 输　入: Buf:指向队列的指针
** 输　出: 无
** 全局变量: 无
** 调用模块: OS_ENTER_CRITICAL,OS_EXIT_CRITICAL
**
** 作　者: 陈明计
** 日　期: 2003年7月2日
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#ifndef EN_QUEUE_FLUSH
#define EN_QUEUE_FLUSH    1
#endif

#if EN_QUEUE_FLUSH > 0

void QueueFlush(void *Buf)
{
    DataQueue *Queue;

    if (Buf != NULL)                                                /* 队列是否有效 */
    {
        /* 有效         */
        Queue = (DataQueue *)Buf;
//        OS_ENTER_CRITICAL();
        Queue->Out = Queue->Buf;
        Queue->In = Queue->Buf;
        Queue->NData = 0;                                           /* 数据数目为0 */
//        OS_EXIT_CRITICAL();
    }
}

#endif
//uint8_t WriteFull(DataQueue *Buf, QUEUE_DATA_TYPE Data, uint8_t Mod)
//{
//    uint16_t temp;

//    Mod = Mod;

//    temp = QueueSize((void *)Buf);
//    while (temp <= QueueNData((void *)Buf))         /* 等待数据队列不满 */
//    {
//        OSTimeDly(OS_TICKS_PER_SEC/100);
//    }
//    return QueueWrite((void *)Buf, Data);           /* 数据重新入队 */
//}


/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
