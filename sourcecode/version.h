#ifndef _VERSION_H
#define _VERSION_H

//	版本号
#define HW_V        "CB1.01-PB3.00-DB1.00"
#define PRIMARY_V   "WSCL-V1.0.2"
#define MINOR_V		"1026.01"      
#define VERSION     PRIMARY_V



/*
实现A2O控制逻辑和手动控制，实现了传感器modbus,ADC采集及值的修正功能, 实现4G数据发送功能，支持用户管理
支持传感器和所有工艺的参数设置，支持读取设备信息功能
---------------------------------------------------------
V1.0.2   2018-12-04
1. 解决modbus的CRC高地位反的问题
2. 优化GPRS逻辑
3. 解决串口接收丢数据的问题， 由于芯片通信关中断时间太长引起
4. FATFS中不使用mutex来做锁，由于需要多个优先级
5. 解决A2O.c中的警告
6. 增加密钥过期时间的配置，只是代屏存储，设置和使用都通过屏来控制
7. 解决通信脱机后不能重连的问题，servercomm中不能正常返回发起重连
8. GPRS波特率改回115200
*/

#endif
