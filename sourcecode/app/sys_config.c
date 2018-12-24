
#include "sys_config.h"
#define  DEVICE_TYPE     0x58465344  //"XFSD"
s_sys_config_t  sys_config_ram;
#define  IP_ADDR_GET(a,b,c,d)   ((((a)&0xFF)<<24) + (((b)&0xFF)<<16) + \
                                 (((c)&0xFF)<<8) + (((d)&0xFF)))
#define DEFAULT_SYSTEM_CONFIG  \
{\
/*设备类型*/    DEVICE_TYPE,\
/*配置版本*/    1,\
/*数据校验*/    0,\
/*工艺类型*/   {1,\
/*Port, IP*/    8992, IP_ADDR_GET(61,147,198,178),\
/*pump min open*/    60,\
/*server interval*/  15,\
/*device id*/        0x0001,\
/*密码有效时间*/     600,\
/*unused*/      0},\
/*A2O端口设置*/ {6,7,8,2,9,14,3,10,4,11,5,12,1,13,\
/*未使用*/       {0},\
/*曝气泵*/       0xFFFFFE,0xFFFFFE,0xFFFFFE,\
/*未使用*/       {0},\
/*潜污泵*/       30,30,1440,\
/*未使用*/       {0},\
                 30,30,1440,\
/*未使用*/     {0},\
/*出水泵切换时间*/ 1440,\
/*未使用*/     {0},\
/*加药泵延时时间*/10,\
/*未使用*/     {0},\
/*提升泵切换时间*/1440,\
/*未使用*/     {0},\
               },\
/*MBR端口设置*/ {0},\
/*SBR端口设置*/ {0},\
/*sensor值*/   {20,7.5f,49,-300,6,3.5f,12,1,0,0,\
/*未使用*/     {0},\
               /*最大     最小       精度           端口                  标定*/\
 /*t*/         {20,      10,    0.01f,    SensorPortADC(1)   , 0, 0, 0, 0, 0},\
 /*ph*/        {9,        6,    0.01f,    SensorPortADC(2)   , 0, 0, 0, 0, 0},\
 /*cod*/       {55,      40,    0.01f,    SensorPortMODBUS(4), 0, 0, 0, 0, 0},\
 /*orp*/       {-200,  -700,        2,    SensorPortADC(3)   , 0, 0, 0, 0, 0},\
 /*nh3*/       {7,        5,    0.01f,    SensorPortMODBUS(3), 0, 0, 0, 0, 0},\
 /*DO*/        {5,        2,    0.01f,    SensorPortADC(4)   , 0, 0, 0, 0, 0},\
 /*ss*/        {15,      10,    0.01f,    SensorPortMODBUS(1), 0, 0, 0, 0, 0},\
 /*p*/         {1,      0.3,    0.01f,    SensorPortMODBUS(2), 0, 0, 0, 0, 0},\
 /*flux*/      {200,      0,       1,    SensorPortCOM2(1)  , 0, 0, 0, 0, 0},\
               },\
/*net  */    {0,\
/*sd   */    0,\
/*usb  */    0,\
/*save */    0,\
/*calib*/    0,\
/*pump_audo*/   1,\
/*init*/     0,\
/*reboot*/   0,\
/*back*/     0,\
/*logout*/   0,\
/*unused0*/  0,\
/*unused00*/ 0,\
/*报警lift*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
/*    */      0,\
/*传感器安装*/\
/*t*/             1,\
/*ph*/          1,\
/*cod*/          1,\
/*orp*/          1,\
/*nh3*/          1,\
/*DO*/          1,\
/*ss*/          1,\
/*p*/          1,\
/*flux*/          1,\
/**/               0,\
/**/               },\
/*用户名*/  "001",\
/*密码*/    "123456",\
/*通信方式位4G*/  CM_4G,\
/*DHCP使能*/      1,\
/*apn 连接点*/    "CTNET",\
/*apn 用户名*/    "",\
/*apn 密码*/      "",\
/*本地IP*/        0,\
/*本地子网掩码*/  0,\
/*本地网关*/      0,\
/*本地DNS1*/      0,\
/*本地DNS2*/      0,\
}

const s_sys_config_t sys_config_rom = DEFAULT_SYSTEM_CONFIG;

int8_t VersionControl(void)
{
    if (sys_config_ram.device_type != DEVICE_TYPE)
    {
        sys_config_ram = sys_config_rom;
        return 1;
    }
    //  数据校验处理
    //if (sys_config_ram.crc == 0)
    //{
    //}
    //  数据版本管理
    if (sys_config_ram.version == sys_config_rom.version)
    {
        return 0;
    }
    switch(sys_config_ram.version)
    {
        case 1:
        {
            break;
        }
        default :
            sys_config_ram = sys_config_rom;
            break;
    }
    return 1;
}

#include "ff.h"

FATFS flash_fatfs;

#define SYS_CFG_F_PATH        "1:sys_cfg_f.inc"


#include "my_debug.h"
#define cfgDebug       Printf_D

static FRESULT InitFileSystemT(void)
{
    FRESULT  fres;
    uint32_t index = 0;
    FIL      cfg_f;
    
    fres = f_mount(&flash_fatfs, "1:", 1);
    if (fres == FR_NO_FILESYSTEM)
    {
        fres = f_mkfs("1:", FM_EXFAT, 0, 0, 0);
        if (fres != FR_OK)
        {
            cfgDebug("fs_init","mkfs \"1:\" failed - %u\r\n", fres);
            return fres;
        }
        cfgDebug("fs_init","mkfs \"1:\" success\r\n");
        fres = f_mount(&flash_fatfs, "1:", 1);
        if (fres != FR_OK)
        {
            return fres;
        }
    }
    else if (fres != FR_OK)
    {
        return fres;
    }
    cfgDebug("fs_init","mount \"1:\" success\r\n");
    
    fres = f_open(&cfg_f, SYS_CFG_F_PATH, FA_READ+FA_WRITE);
    if (fres == FR_NO_FILE)
    {
        fres = f_open(&cfg_f, SYS_CFG_F_PATH, FA_CREATE_NEW + FA_WRITE);
        if (fres != FR_OK)
        {
            return fres;
        }
        cfgDebug("fs_init","create \"%s\" success\r\n", SYS_CFG_F_PATH);
        sys_config_ram = sys_config_rom;
        index = sizeof(sys_config_ram);
        fres = f_write(&cfg_f, &sys_config_ram, index, &index);
        cfgDebug("fs_init","update \"%s\" - %u\r\n", SYS_CFG_F_PATH, fres);
    }
    else
    {
        index = sizeof(sys_config_ram);
        fres = f_read(&cfg_f, &sys_config_ram, index, &index);
        if (fres != FR_OK)
        {
            sys_config_ram = sys_config_rom;
        }
        if (VersionControl())
        {
            index = sizeof(sys_config_ram);
            f_lseek(&cfg_f, 0);
            fres = f_write(&cfg_f, &sys_config_ram, index, &index);
        }
        cfgDebug("fs_init","read \"%s\" - %u\r\n", SYS_CFG_F_PATH, fres);
    }
    f_close(&cfg_f);
    //sys_config_ram = sys_config_rom;
    return FR_OK;
    
}

int16_t InitFileSystem(void)
{
    FRESULT fres = InitFileSystemT();
    if (fres != FR_OK)
    {
        sys_config_ram = sys_config_rom;
        cfgDebug("fs_init","cfg init unsuccess - %u \r\n", fres);
    }
    return fres;
}

int16_t SaveSystemCfg(uint32_t index, uint8_t *buff, uint32_t len)
{
    FRESULT  fres;
    FIL      cfg_f;
    fres = f_open(&cfg_f, SYS_CFG_F_PATH, FA_WRITE);
    if (fres == FR_OK)
    {
        fres = f_lseek(&cfg_f, index);
        if (fres == FR_OK)
            fres = f_write(&cfg_f, buff, len, &len);
        f_close(&cfg_f);
    }
    return fres;
}

int8_t CheckSenorPort (e_sensor_port_t type, uint8_t port)  
{
    switch(type)
    {
        case SENSOR_PORT_NONE:
            if (0 == port)
            {
                return 1;
            }
            break;
        case SENSOR_PORT_MODBUS:
            if ((port >= SENSOR_PORT_MODBUS) && (port < SENSOR_PORT_ADC))
            {
                return 1;
            }
            break;
        case SENSOR_PORT_ADC:
            if ((port >= SENSOR_PORT_ADC) && (port < (SENSOR_PORT_ADC+8)))
            {
                return 1;
            }
            break;
        case SENSOR_PORT_COM1:
            if ((port >= SENSOR_PORT_COM1) && (port < SENSOR_PORT_COM2))
            {
                return 1;
            }
            break;
        case SENSOR_PORT_COM2:
            if ((port >= SENSOR_PORT_COM2) && (port < (SENSOR_PORT_COM2+16)))
            {
                return 1;
            }
            break;
    }
    return 0;
}
