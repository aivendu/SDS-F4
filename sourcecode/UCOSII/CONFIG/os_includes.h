/*
************************************************************************************************
主要的包含文件

文 件: INCLUDES.C ucos包含文件
作 者: Jean J. Labrosse
************************************************************************************************
*/

#ifndef __INCLUDES_H__
#define __INCLUDES_H__
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ucos_ii.h"
#include "os_cpu.h"
#include "os_cfg.h"

#include "stm32f4xx.h"

/*
*********************************************************************************************************
*                                    OLD ERROR CODE NAMES (< V2.84)
*********************************************************************************************************
*/
#define OS_NO_ERR                    OS_ERR_NONE
#define OS_TIMEOUT                   OS_ERR_TIMEOUT
#define OS_TASK_NOT_EXIST            OS_ERR_TASK_NOT_EXIST
#define OS_MBOX_FULL                 OS_ERR_MBOX_FULL
#define OS_Q_FULL                    OS_ERR_Q_FULL
#define OS_Q_EMPTY                   OS_ERR_Q_EMPTY
#define OS_PRIO_EXIST                OS_ERR_PRIO_EXIST
#define OS_PRIO_ERR                  OS_ERR_PRIO
#define OS_PRIO_INVALID              OS_ERR_PRIO_INVALID
#define OS_SEM_OVF                   OS_ERR_SEM_OVF
#define OS_TASK_DEL_ERR              OS_ERR_TASK_DEL
#define OS_TASK_DEL_IDLE             OS_ERR_TASK_DEL_IDLE
#define OS_TASK_DEL_REQ              OS_ERR_TASK_DEL_REQ
#define OS_TASK_DEL_ISR              OS_ERR_TASK_DEL_ISR
#define OS_NO_MORE_TCB               OS_ERR_TASK_NO_MORE_TCB
#define OS_TIME_NOT_DLY              OS_ERR_TIME_NOT_DLY
#define OS_TIME_INVALID_MINUTES      OS_ERR_TIME_INVALID_MINUTES
#define OS_TIME_INVALID_SECONDS      OS_ERR_TIME_INVALID_SECONDS
#define OS_TIME_INVALID_MS           OS_ERR_TIME_INVALID_MS
#define OS_TIME_ZERO_DLY             OS_ERR_TIME_ZERO_DLY
#define OS_TASK_SUSPEND_PRIO         OS_ERR_TASK_SUSPEND_PRIO
#define OS_TASK_SUSPEND_IDLE         OS_ERR_TASK_SUSPEND_IDLE
#define OS_TASK_RESUME_PRIO          OS_ERR_TASK_RESUME_PRIO
#define OS_TASK_NOT_SUSPENDED        OS_ERR_TASK_NOT_SUSPENDED
#define OS_MEM_INVALID_PART          OS_ERR_MEM_INVALID_PART
#define OS_MEM_INVALID_BLKS          OS_ERR_MEM_INVALID_BLKS
#define OS_MEM_INVALID_SIZE          OS_ERR_MEM_INVALID_SIZE
#define OS_MEM_NO_FREE_BLKS          OS_ERR_MEM_NO_FREE_BLKS
#define OS_MEM_FULL                  OS_ERR_MEM_FULL
#define OS_MEM_INVALID_PBLK          OS_ERR_MEM_INVALID_PBLK
#define OS_MEM_INVALID_PMEM          OS_ERR_MEM_INVALID_PMEM
#define OS_MEM_INVALID_PDATA         OS_ERR_MEM_INVALID_PDATA
#define OS_MEM_INVALID_ADDR          OS_ERR_MEM_INVALID_ADDR
#define OS_MEM_NAME_TOO_LONG         OS_ERR_MEM_NAME_TOO_LONG
#define OS_TASK_OPT_ERR              OS_ERR_TASK_OPT
#define OS_FLAG_INVALID_PGRP         OS_ERR_FLAG_INVALID_PGRP
#define OS_FLAG_ERR_WAIT_TYPE        OS_ERR_FLAG_WAIT_TYPE
#define OS_FLAG_ERR_NOT_RDY          OS_ERR_FLAG_NOT_RDY
#define OS_FLAG_INVALID_OPT          OS_ERR_FLAG_INVALID_OPT
#define OS_FLAG_GRP_DEPLETED         OS_ERR_FLAG_GRP_DEPLETED



#endif































