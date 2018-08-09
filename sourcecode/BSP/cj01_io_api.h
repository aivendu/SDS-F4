#ifndef __IO_API__
#define __IO_API__
#include "stdint.h"
#include "ucos_ii.h"

#define OS_SEM OS_EVENT *

typedef enum 
{
    gRTC_PORT,
    gCOM1,
    gCOM6,
    gCOM3,
    gGPIO_IN_01,
    gGPIO_IN_02,
    gGPIO_IN_03,
    gGPIO_IN_04,
    gGPIO_IN_05,
    gGPIO_IN_06,
    gGPIO_IN_07,
    gGPIO_IN_08,
    gGPIO_OUT_01,
    gGPIO_OUT_02,
    gGPIO_OUT_03,
    gGPIO_OUT_04,
    gGPIO_OUT_05,
    gGPIO_OUT_06,
    gGPIO_OUT_07,
    gGPIO_OUT_08,
    gSTLED,
    gGPIO_IN_MENU,
    gWDT,
    gPS2,
    gINT_FLASH,
    PORT_GROUP_END
}e_port_group_t; 

/*IO口类型定义*/
typedef enum
{
    RTC_PORT,
    COM1,       // 芯片的串口1，独立的串口
    COM6,		//芯片的串口6，独立的串口
	COM3,		//芯片的串口3，分时复用出来的串口A
    GPIO_IN_01,    //
    GPIO_IN_02,    //
    GPIO_IN_03,    //
    GPIO_IN_04,    //
    GPIO_IN_05,    // 
    GPIO_IN_06,    //
    GPIO_IN_07,    //
    GPIO_IN_08,    //
    GPIO_OUT_01,    //
    GPIO_OUT_02,    //
    GPIO_OUT_03,    //
    GPIO_OUT_04,    //
    GPIO_OUT_05,    // 
    GPIO_OUT_06,    //
    GPIO_OUT_07,    //
    GPIO_OUT_08,    //
    ST_LED,
    GPIO_IN_MENU,
    WDT,
    PS2,
    INT_FLASH,
    PORT_END
} e_IO_PORT_t;

#define UNDEF_PORT    0xFF

#define    GPIO_07      UNDEF_PORT  // 4 SHAKE
#define    GPIO_08      UNDEF_PORT  // 5 DOOR_UP
#define    GPIO_09      UNDEF_PORT  // 6 DOOR_DOWN
#define    GPIO_10      UNDEF_PORT  // 7 DOOR_SIDE
#define    GPIO_11      UNDEF_PORT  // 8 CASHBOX
#define    GPIO_12      UNDEF_PORT  // 9 COIN_RECEIVE
#define    GPIO_13      UNDEF_PORT  // 10 COIN_BACK
#define    GPIO_14      UNDEF_PORT  // 11 BUZZER
#define    GPIO_15      UNDEF_PORT  // 12 LEDM1
#define    GPIO_20      UNDEF_PORT  // 13 24V_POWER
#define    GPIO_21      GPIO_03  // 14 SET
//#define    COM0         UNDEF_PORT  // 15

#define    RJ45         UNDEF_PORT  // 19
#define    I2C01        UNDEF_PORT  // 20

//#define    COM5         UNDEF_PORT  // 23
//#define    COM6         UNDEF_PORT  // 24
//#define    COM7         UNDEF_PORT  // 25
#define    STORE        UNDEF_PORT  // 27


#define CREDIT_CARD   COM0

#define LCD     COM2
#define PRINT   COM2
#define IR1     COM5
#define IR2     COM5
#define WIFI    COM6
#define SIM     COM7






/*权限类型定义*/
#define NO_READ_WRITE  0x00 //不读不写
#define READ_ONLY  0x01 //只读
#define WRITE_ONLY 0x02 //只写
#define READ_WRITE 0x03 //读写



#define WRITE_MDB  0x04
//BSP控制命令定义
#define CLEAR_BUFFER        0x05
#define CLEAR_INPUT_BUFFER  0x06
#define CLEAR_OUTPUT_BUFFER 0x07
#define MDB_POWER_OP        0x08


typedef struct
{
    uint8_t task_prio;              //  操作该IO口的任务优先级
    OS_SEM  io_sem;         //  IO口信号量
} s_ioGroup_t;


typedef struct
{
    uint8_t port;           //  端口号
    uint16_t authority;     //  读写权限
    s_ioGroup_t *group;
    int8_t (*io_open)(int32_t port, const void *config, uint8_t len);
    int8_t (*io_close)(int32_t port);
    int32_t (*io_read)(int32_t port, void *buf, uint32_t buf_len);
    int8_t (*io_write)(int32_t port, void *buf, uint32_t buf_len);
    int8_t (*io_ioctl)(int32_t port, uint32_t cmd, va_list argp);
    int32_t (*io_asyn_read)(int32_t port, void *buf, uint32_t buf_len, void * callback);
    int8_t (*io_asyn_write)(int32_t port, void *buf, uint32_t buf_len, void * callback);
} s_IoOpreations_t;




typedef struct
{
    uint32_t wiegand_1            : 1;
    uint32_t wiegand_2            : 1;
    uint32_t timer_5ms            : 1;
    uint32_t timer_1ms            : 1;
} s_bsp_os_flag_t;

typedef union
{
    s_bsp_os_flag_t bsp_os_flag;
    uint32_t bsp_os_flag_word;
} u_bsp_os_flag_t;

#define OS_FLAG_WIEGAND_1            0x01  //  韦根1接收数据完成标志
#define OS_FLAG_WIEGAND_2            0x02  //  韦根2接收数据完成标志
#define OS_FLAG_TIMER_5MS            0x04  //  5ms定时完成标志
#define OS_FLAG_TIMER_1MS            0x08  //  1ms定时完成标志
extern OS_FLAG_GRP *bsp_os_flag;

#define INVALID_TASK_PRIO     0xFF

extern int8_t  IoOpen(int32_t port, const void *config, uint8_t config_len);
extern int8_t  IoClose(int32_t port);
extern int32_t IoRead(int32_t port, void *buf, uint32_t buf_len);
extern int32_t IoAsynchronRead(int32_t port, void *buf, uint32_t buf_len, void * callback);
extern int8_t  IoWrite(int32_t port, void *buf, uint32_t buf_len);
extern int8_t  IoAsynchronWrite(int32_t port, void *buf, uint32_t buf_len, void * callback);
extern int8_t  Ioctl(int32_t port, uint32_t cmd, ...);

extern uint8_t GetCurrentTaskPrio(void);
extern void TaskBsp(void *pdata);

#endif
