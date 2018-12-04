#ifndef _SYS_TASK_CONFIG_H
#define _SYS_TASK_CONFIG_H



typedef enum
{
    PRIOSTART = 3,
    FS_MUTEX_PRIO,
    TaskBspPrio,  // 4
    TaskTCPIPThreadPrio,//定义内核任务的优先级
    TaskDHCPPrio,       //定义DHCP任务的优先级
    TaskSpiUartPrio,
    TaskInitPrio,
    TaskServerCommPrio,
    TaskSimPrio,
    TaskSensorPrio,
    PRIOEND
} _task_prio_e;

#endif
