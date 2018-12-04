#ifndef __INCLUDES_H
#define __INCLUDES_H

#ifdef __cplusplus
extern "C" {
#endif
//#include    "os_cpu.h"
//#include    ".\os\os_cfg.h"
//#include    ".\os\ucos_ii.h"
#include    "ucos_ii.h"
#ifdef __cplusplus
}
#endif




/********************************/
//公用功能头文件
#include    <stdio.h>
#include    <ctype.h>
#include    <stdlib.h>
#include	<stdint.h>
#include    <setjmp.h>
#include    <rt_misc.h>
#include	<string.h>
#include 	<stdarg.h>
//#include	<time.h>

//	IO 驱动头文件
#include "bsp_includes.h"


#include "utility.h"
#include "queue.h"


#include "ff.h"

#include "lwipopts.h"



//应用层
#include "version.h"


//	进程定义，放在最后
#include "sys_config.h"
#include "sys_task_config.h"

/* 由于不同情况中断OSIntExit()对堆栈的使用情况不同，必须加上这个宏定义    */
#define OSIntCtxSw()    return

#endif
/**********************end****************************************/
