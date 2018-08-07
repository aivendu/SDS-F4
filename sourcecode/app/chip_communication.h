#ifndef _CHIP_COMMUNICATION_H
#define _CHIP_COMMUNICATION_H

#include "stdint.h"
//#include "comm.h"
//#include "virtual_memery.h"

#ifndef TRUE
#define TRUE      1
#endif


#ifndef FALSE
#define FALSE      1
#endif


#define	CHIP_READ			0
#define	CHIP_WRITE			1

typedef struct
{
	uint16_t	year;
	uint8_t	month;
	uint8_t	day;
	uint8_t	hour;
	uint8_t	min;
	uint8_t	sec;
	uint8_t	msec;
} _time_s;

typedef struct
{
	uint8_t station_no;				//	站点编号
	char  station_name[11];			//	站点名字
	uint32_t gps_data[2];				//	 gps_data[0] -- 经度;  gps_data[1]--维度
} _station_mess_s;

typedef struct
{
	_station_mess_s station[30];
	uint16_t line_no;
	uint8_t  line_station_amount;
	char   line_version[9];
} _line_mess_s;

typedef struct
{
	uint8_t regist_time[4];		//	打卡时间
	uint8_t user_role;			//	用户角色，用于判别用户的功能权限
	char staffid[7];			//	工号，支持最多7个字符
	char driver_name[8];		//	司机名字，支持最多4个字的名字
	uint8_t ID_card[8];			//	司机身份证号，X用0表示，整形数
} _user_info_s;

typedef struct
{
	uint8_t guid[16];				//	GUID, 路线唯一编号
	char vehicle_plate[8];		//	车牌号
	uint16_t routenum;			//	路线编号
} _route_info_s;

//	芯片间通信同步数据
typedef struct
{
	_user_info_s uinfo;
	_route_info_s rinfo;
} _ccd_card_s;



#define WRITE			2
#define	READ			1





extern void ChipCommInit(void);
extern int8_t ChipWriteFrame(uint8_t fun, uint16_t addr, uint8_t len, void *data);
extern int8_t ChipReadFrame(uint8_t fun, uint16_t addr, uint8_t len, void *data);


#endif
