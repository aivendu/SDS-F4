
#include "sys_config.h"
#define  DEVICE_TYPE     0x58465344  //"XFSD"
s_sys_config_t  sys_config_ram;

#define DEFAULT_SYSTEM_CONFIG  \
{\
/*设备类型*/    DEVICE_TYPE,\
/*配置版本*/    1,\
/*数据校验*/    0,\
/*工艺类型*/    0,\
/*端口设置*/    {3,4,5,6,7,8,9,10,11,12,13,14,1,2,\
/*未使用*/       {0},\
                  0,0,0,\
/*未使用*/       {0},\
                 30,30,1440,\
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
               {0,0,0,0,0,0,0,0,0,\
               {0},\
               /*最大   最小    精度  端口  标定*/\
 /*t*/         {200,   -30,    1,    SensorPortADC(1)   , 0, 0, 0, 0, 0},\
 /*ph*/        {200,   -30,    1,    SensorPortADC(2)   , 0, 0, 0, 0, 0},\
 /*cod*/       {200,   -30,    1,    SensorPortMODBUS(4), 0, 0, 0, 0, 0},\
 /*orp*/       {200,   -30,    1,    SensorPortADC(3)   , 0, 0, 0, 0, 0},\
 /*nh3*/       {200,   -30,    1,    SensorPortMODBUS(3), 0, 0, 0, 0, 0},\
 /*DO*/        {200,   -30,    1,    SensorPortADC(4)   , 0, 0, 0, 0, 0},\
 /*ss*/        {200,   -30,    1,    SensorPortMODBUS(1), 0, 0, 0, 0, 0},\
 /*p*/         {200,   -30,    1,    SensorPortMODBUS(2), 0, 0, 0, 0, 0},\
 /*flux*/      {200,   -30,    1,    SensorPortMODBUS(5), 0, 0, 0, 0, 0},\
               },\
/*net  */    {0,\
/*sd   */    0,\
/*usb  */    0,\
/*save */    0,\
/*calib*/    0,\
/*     */    0,\
/*报警lift*/       1,\
/*    subm*/      1,\
/*    rflx*/      1,\
/*    watr*/      1,\
/*    dosg*/      1,\
/*    aera*/      1,\
/*    wash*/      1,\
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


#include "debug_log.h"
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