#ifndef _DEVICE_PARAMETERS_H_
#define _DEVICE_PARAMETERS_H_

typedef struct s_device_parameters
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

//设置模块端口号，有待完善
    uint8_t mod_port_coin_receive;              //  硬币接收模块端口选择
    uint8_t mod_port_coin_change;               //  硬币找零模块端口选择
	uint8_t mod_port_bill_receive;			    //	纸币接收模块端口选择
    uint8_t mod_port_bill_change;               //  纸币找零模块端口选择

    uint8_t mod_port_barcode_scanner;           //  扫描枪模块端口选择
    uint8_t mod_port_barcode_eater;             //  验票头模块端口选择
    uint8_t mod_port_credit;                    //  银行卡模块端口选择
    uint8_t mod_port_led_screen;                //  LED屏幕端口选择
    uint8_t mod_port_lcd_screen;                //  LCD屏幕端口选择
    uint8_t mod_port_printer;                   //  打印机端口选择

    char prompt_info[60];

    s_CurrencyConfig_t currency_config[2];                  //存储的两种货币信息
    s_CashReceiveConfig_t coin_rec_config[16];              //硬币接收的货币信息
    s_CashDispenseConfig_t coin_DISPENSER_config[16];        //硬币找零的货币信息
    s_CashReceiveConfig_t bill_rec_config[16];              //纸币接收的货币信息
    s_CashDispenseConfig_t bill_DISPENSER_config[2];         //纸币找零的货币信息

	s_SlipInformation slip_config[2];   //票据格式
} s_device_parameters_t;



#endif

