#ifndef _VERSION_H
#define _VERSION_H

//	版本号
#define PRIMARY_V   "CJ01-U1.0R.5.15 "
#define MINOR_V		"1026.01"      
#define VERSION     PRIMARY_V



/*
支持迪文串口屏，ps2键盘，网络通信，SD卡日志存储，门控报警，震动传感，蜂鸣器
硬币接收机---G13
硬币找零机---ict_hopper
纸币找零机---cdm_4000
打印机---天名打印机
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.15
彭  俊 2017.08.15
1.修改，打印机的初始化程序，改为初始化的时候查询打印机状态，否则会造成无法正常的识别打印机是否连接。
2.增加，网线热插拔功能，实时监测网线状态和服务器状态，在网线拔掉后马上显示脱机，在网线连上后马上联机。
在服务器断开或者关闭后，马上显示脱机，在服务器正常打开后，马上联机。
3.修改，自检流程和网络联机流程同时进行，达到自检完成就已经联机的效果。
4.增加，LUP100 POS机的驱动和应用支持。
5.修改，在模块层的接口，增加Open和Close函数，为了适应某些模块是会主动上传串口数据。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.14
彭  俊 2017.08.14
1.修改，整理整个工程所有C文件的头文件包含关系，尽量少包含。
2.增加，串口分时复用驱动。
3.增加，网络模块的电源控制口初始化。
4.修改，PS2键盘驱动适应控制板。
5.增加，中断管理（cj01_IRQ_priority.h）。
6.修改，MDB驱动适应控制板。
7.修改，模块都使用可配置的COM口进行通信处理。
8.修改，把所有的清屏处理都放在切换屏幕后，这样达到看起来更加流畅的效果。
9.增加，增加PUBLIC层，把一些公共的，完全不涉及到硬件的内存放在此层。
方便所有层调用，包括：pub_crc_check.c,queue.c,utility.c,sys_config.c。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.13
彭  俊 2017.08.08
1.增加，MDB接口的C2硬币循环机和CashCode的驱动支持。
2.解决，由于开始找零信号量发送之后，没有进行任务调度就开始判断找零结果，造成的还没开始找零就结束的BUG。
3.删除，原ARM7平台的板间通信相关内容。
4.修改，现金模块的分类，分为硬币接收，硬币找零，硬币循环，纸币接收，纸币找零，纸币循环六大类。
5.修改，对文件夹和源文件进行名称整理，分类。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.12
彭  俊 2017.08.03
1.增加，CDM_4000出钞机的驱动支持（mod_cdm_4000.c,mod_cash_change）。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.11
彭  俊 2017.08.02
1.增加，mod_ict_hopper模块驱动（mod_ict_hopper.c）。
2.增加，扫描枪模块驱动（mod_scanner.c）。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.10
彭  俊 2017.07.26
1.修改，修改LWIP内核任务的优先级为最高，因为，如果太低会造成发送函数死机。
2.修改，版本，改为标准化的版本号和版本说明。
3.增加，网络通信，每次通信前先关闭套接字再新建套接字。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.09
彭  俊 2017.07.26
1.解决，由于自定义的内存池过大，导致内存不够用，无法跑到main函数的问题（res_save_config.c）。
2.增加，G13模块驱动（mod_g13.c），cctalk驱动（cctalk_CA.c）。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.08
彭  俊 2017.07.25
1.修改，存储设备配置信息相关内容，由外部存储改为内部FLASH存储（res_save_config.c）。
2.修改，串口液晶屏驱动程序，原来的驱动通信流程用在新的平台上出问题了，
直接移植了一个之前写好的完整的驱动（mod_dwin_lcd.c）。
3.修改，串口驱动，改为和公司其他STM32平台一样的驱动（cj01_usart.c）。
4.修改，队列函数，改为和公司其他STM32平台一样的队列（queue.c）。
5.删除，原来的外部存储模块驱动。
6.修改，修改除RES层和APP层以外的其它层的所有文件的头文件包含，改为用什么头文件包含什么，
而不是使用includes.h一次性包含完。
7.修改，把mod_menu改为res_menu，并移动到资源调度层。
8.增加，网络的资源调度（res_network.c）。
9.修改，通信流程，当设备注册成功后，马上发第一个心跳，提高设备开机后能正常工作的速度。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.07
彭  俊 2017.07.17
1.增加，移植LWIP协议栈。
2.增加，lan8720以太网芯片驱动（cj_lan8720.c）。
3.增加，SARM驱动（cj_sram.c）。
4.增加，内存管理模块（malloc.c）。
5.增加，资源管理层的通信资源（res_communiction.c）。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.06
彭  俊 2017.07.10
1.修改，看门狗驱动改为STM32的看门狗驱动。
2.修改，RTC时间驱动改为STM32的RTC时间驱动。
3.修改，PS2键盘驱动改为STM32的PS2键盘驱动。
4.增加，芯片内部FLASH驱动，用以存放配置信息。
5.增加，cj01_exti和cj01_timer，准备以后放，外部中断和定时器处理。
5.修改，规范代码排版，使用VS的插件进行的。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.05
彭  俊 2017.07.06 
1.增加，调试日志打印系统（res_debug_log.c）。
2.增加，日志打印模块（mod_sdcard_log.c）和交易日志打印（res_run_log.c）。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.04
彭  俊 2017.07.01
1.修改，增加堆栈大小。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.03
彭  俊 2017.07.01
1.增加，SD卡驱动。
2.增加，移植FATAS文件系统。
3.增加，移植EasyLogger日志系统。
----------------------------------------------------------------------------------------
CJ01-U1.0R.5.02
彭  俊 2017.07.01
1.修改，AJ板工程移植到STM32F407平台。





----------------------------------------------------------------------------------------
AJ01-U1.0R.5.01
张聪 2017.04.24
1.去掉菜单中现金维护的项目
2.修改找零金额为实际找零金额
3.修复auditcash中有时找零金额发送错误的问题
4.增加ict，g13，出钞机连接失败后的错误提示
5.增加广电运通纸币接收的支持
----------------------------------------------------------------------------------------
AJ01-U1.0R.4.02
张聪 2017.04.11
1.修复打印对账单卡死的问题
2.去掉菜单中多余的硬币找零模块设置
----------------------------------------------------------------------------------------
AJ01-U1.0R.4.01
张聪 2017.04.07
1.增加g13和ict hopper模块的支持
----------------------------------------------------------------------------------------
AJ01-U1.0R.3.01
程露 2017.02.23
1.增加银联功能
----------------------------------------------------------------------------------------
AJ01-U1.0R.2.01
程露 2017.01.05
1.增加降级功能
2.修复输入优惠券后金额不对的bug
----------------------------------------------------------------------------------------
AJ01-U1.0R.1.02
程露 2016.12.26
1.修复开关门进入菜单会卡住的问题
----------------------------------------------------------------------------------------
AJ01-U1.0R.1.01
程露 2016.12.19
1.发布第一个版本
----------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------
1.完善了现金接收和找零任务
2.合并最新的ps2键盘程序，修复按键重复的问题
3.菜单显示已经正常
4.网络通信可以正常使用
5.存储功能可以正常使用
6.扫描枪可以正常使用
7.扫票到解析付费信息正常
*/

#endif
