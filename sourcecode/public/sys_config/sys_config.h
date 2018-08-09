#ifndef _SYS_CONFIG_H
#define _SYS_CONFIG_H

#include "stdint.h"
#include "utility.h"


#define LANGUAGE_CHINESE         0  //  汉语
#define LANGUAGE_ENGLISH         1  //  英语

#define OAM_ENCRYPT_TYPE_NONE      0    //  OAM 通信不需要加密
#define OAM_ENCRYPT_TYPE_AES       1    //  OAM 通信需要AES加密
#define OAM_ENCRYPT_TYPE_SSL       2    //  OAM 通信需要SSL加密
#define OAM_ENCRYPT_TYPE_DES3      3    //  OAM 通信需要DES3加密



#define MACHINE_NAME            "PM8"
#define SYSTEM_CONFIG_NAME      "SYS"
#define SYSTEM_CONFIG_VERSION   3              //配置版本号
#define SYSTEM_CONFIG_SIZE      0x4000         //配置存储区大小


typedef struct
{
    char machine_name[8];   //设备名称
    char config_name[8];    //配置块名称
    uint8_t config_version; //配置块版本
    uint32_t config_size;   //配置块大小
}s_config_head_t;   //配置信息头


typedef struct
{                                        
    //设备名称
    //配置块名称
    //配置块版本
    //配置块大小
    s_config_head_t config_head;
    
	/*是否为车位锁模式*/
	uint32_t parking_lock_enable		: 4;    // 0--标准化模式，1--车位锁模式
    /*模式选择*/ 
    uint32_t modeselect                 : 4;    //模式的选择 0:POD 1:POF 2:POE
	//现金支付使能
	uint32_t cash_pay_en				: 1;    //0--禁止，1--使能
    
    
//模块的使能
    //信用卡使能          
    uint32_t credit_en                  : 4;    //0--禁止，1--p90,2.crt284,3.xac303
    //	硬币接收使能
    uint32_t coin_receive_en 		    : 4;    //0--禁止，1--使能C2，2--使能G13
    //	硬币找零使能
    uint32_t coin_change_en 		    : 4;    //0--禁止，1--使能C2，2--使能ict_hopper
    //	纸币接收模块使能
    uint32_t bill_receive_en 		    : 4;    //0--禁止，1-mdb协议的cashcode，2-广电运通
    //	纸币找零模块使能
    uint32_t bill_change_en 		    : 4;    //0--禁止，1--使能出钞机
	//  第三方支付
	uint32_t online_payment_en			: 4;	//0--禁止，1--使能
    //  扫描枪使能
    uint32_t barcode_scan_en            : 4;    //0--禁止，1--使能
    //  验票头使能
    uint32_t eater_en                   : 4;    //0--禁止，1--使能
    /*led滚动屏幕使能*/
    uint32_t led_rolling_en             : 1;    //0--禁止，1--使能
    /*票号输入使能*/ 
    uint32_t ticket_input_en            : 1;    //0-禁止，1-使能
	//	车牌输入使能
    uint32_t plate_en 		            : 2;    //0--禁止，1--使能，2--强制输入
    //	车位输入使能
    uint32_t stall_en 		            : 2;    //0--禁止，1--使能，2--强制输入
    //	折扣码输入使能
    uint32_t coupon_input_en 		    : 1;    //0--禁止，1--使能
	//	凭条打印使能
	uint32_t slip_print_en				: 2;    //0--禁止，1--使能，2--选择
    /*  验票使能            */
    uint32_t checkticket_en             : 1;    //0--不使用，1--使用
    /*  hotel磁条卡         */
    uint32_t hotel_card_en              : 1;    //0--不使用，1--使用
	//	设置开门报警使能
    uint32_t alarm_en 		            : 1;    //0--禁止，1--使能
	//是否自动打印报表
	uint32_t print_report_en 		    : 1;	//	1--是，0--否
	//开门报警使能
	uint32_t door_alarm_en 		        : 1;	//	1--是，0--否
    
    
    //第二语言        
    uint8_t  second_language;  //  第二语言
    //系统语言            
    uint8_t  language;          //  系统语言
    //交易超时时间        
    uint16_t transactiontmo;
	//输入超时时间        
    uint16_t input_tmo;
	//开门报警时间
	uint16_t door_alarm_tmo;

} s_SysConfigParameter_t;




#define NETWORK_CONFIG_NAME     "PM8-NET"
#define NETWORK_CONFIG_VERSION  2               //网络配置信息的版本号
#define NETWORK_CONFIG_SIZE     0x0500


typedef struct
{
    //设备名称
    //配置块名称
    //配置块版本
    //配置块大小
    s_config_head_t config_head;
    uint8_t local_ip_src;                             //本地IP获取方式，0-自动获取，1-手动获取
    uint8_t online_en;                              //联机使能，0-脱机使用，1-联机使用
    uint8_t com_mode;                               //通信模式，0-RJ45，1-GPRS，2-脱机
    uint8_t rj45_version;                           //RJ45版本，0-标配版本，1-v6.1.0xx
    uint8_t ct_encryption_type;                     //CT通信加密类型
    uint8_t lpr_encryption_type;                    //LPR通信加密类型
    uint16_t server_communication_wait_time;        //与中间服务器通讯时的响应等待时间
    uint8_t server_online_wait_time;               //服务器联机等待时间
    s_ipv4_t local_ip;          //  rj45 ip
    s_ipv4_t local_subnetmask;  // rj45子网掩码
    s_ipv4_t local_gateway;     //  rj45网关
    s_ipv4_t local_dns1;        //  rj45主DNS
    s_ipv4_t local_dns2;        //  rj45从DNS
    s_ipv4_t server_ip;         //  中间服务器IP
    s_ipv4_t oam_ip;            //  OAM服务器IP
    uint16_t server_port;       //  中间服务器端口
    uint16_t oam_port;          //  OAM服务器端口
    uint8_t kek4[4];            //通信KEK4
    uint8_t oam_dek[16];        //oam通信DEK
    uint8_t server_dek[16];     //Middle通信DEK
    uint8_t server_iv[16];      //Middle通信IV
    uint8_t kek1[4];            //通信KEK1
    uint8_t oam_aes_iv[16];     //通信AES-IV
    char terminal_ID[20];		//信用卡交易终端ID
    uint8_t kek3[4];            //通信KEK3
    uint8_t kek2[4];            //通信KEK2
    uint32_t linkuptmo;         //linkup启动时间间隔
    uint32_t messagetmo;        //oam通信等待时间
    uint32_t bps_rj45;			//	RJ45波特率   
    uint16_t auditcashboxtmo;
	char server_id[16];			//注册时分配的设备号
	char serial_num[16];		//设备序列号
}s_NetworkConfigParameter_t;







extern s_SysConfigParameter_t sys_config_ram;
extern const s_SysConfigParameter_t sys_config_rom;
extern s_NetworkConfigParameter_t network_config_ram;
extern const s_NetworkConfigParameter_t network_config_rom;


extern int16_t InitFileSystem(void);

extern int16_t SaveSystemCfg(uint32_t index, uint8_t *buff, uint32_t len);


extern int16_t SaveNetworkCfg(uint32_t index, uint8_t *buff, uint32_t len);


extern uint8_t SaveAllConfig(void);


#endif

