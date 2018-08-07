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
	uint8_t station_no;				//	վ����
	char  station_name[11];			//	վ������
	uint32_t gps_data[2];				//	 gps_data[0] -- ����;  gps_data[1]--ά��
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
	uint8_t regist_time[4];		//	��ʱ��
	uint8_t user_role;			//	�û���ɫ�������б��û��Ĺ���Ȩ��
	char staffid[7];			//	���ţ�֧�����7���ַ�
	char driver_name[8];		//	˾�����֣�֧�����4���ֵ�����
	uint8_t ID_card[8];			//	˾�����֤�ţ�X��0��ʾ��������
} _user_info_s;

typedef struct
{
	uint8_t guid[16];				//	GUID, ·��Ψһ���
	char vehicle_plate[8];		//	���ƺ�
	uint16_t routenum;			//	·�߱��
} _route_info_s;

//	оƬ��ͨ��ͬ������
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
