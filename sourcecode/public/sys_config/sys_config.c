
#include "stdint.h"
#include "utility.h"
#include "sys_config.h"
#include "cj01_io_api.h"
s_SysConfigParameter_t sys_config_ram;
s_NetworkConfigParameter_t network_config_ram;

#define DEFAULT_SYSTEM_CONFIG   {\
/*  机器名字            */    MACHINE_NAME,\
/*  配置块名称            */  SYSTEM_CONFIG_NAME,\
/*  配置版本号           */   SYSTEM_CONFIG_VERSION,\
/*  配置存储区大小      */    SYSTEM_CONFIG_SIZE,\
/*	是否为车位锁模式	    */	 0,\
/*  模式的选择*/             1,\
/*  现金支付使能*/           1,\
/*  信用卡使能*/              1,\
/*	硬币接收模块使能*/        1,\
/*	硬币找零模块使能*/        1,\
/*	纸币接收模块使能*/        1,\
/*	纸币找零模块使能*/        1,\
/*  在线支付使能  */		    1,\
/*  扫描枪模块使能  */       1,\
/*  验票头模块使能  */       0,\
/*  LED滚动屏幕使能  */      0,\
/*  输入票号使能*/           0,\
/*	车牌输入使能*/           1,\
/*	车位输入使能*/           1, \
/*	折扣码输入使能*/         0,\
/*	凭条打印使能*/			1,\
/*  验票使能*/              1,\
/*  hotel磁条卡*/           0,\
/*	设置开门报警使能*/       0, \
/*是否自动打印报表*/       0,\
/*开门报警使能*/          0,\
/*  第二语言            */  LANGUAGE_CHINESE,\
/*  系统语言            */  LANGUAGE_CHINESE,\
/*  交易超时时间        */  90,\
/*  输入超时时间        */  90,\
/*  开门报警时间        */  90,\
}






#define DEFAULT_NETWORK_CONFIG  {\
MACHINE_NAME,\
NETWORK_CONFIG_NAME,\
NETWORK_CONFIG_VERSION,\
NETWORK_CONFIG_SIZE,\
/*  本地IP获取方式      */  1,\
/*  联机使能            */  1,\
/*  通信模式*/               0,\
/*  rj45配置标志  */       0,\
/*  CT通信加密类型*/       1,\
/*  LPR通信加密类型*/      1,\
/*  通讯的响应等待时间  */  500,\
/*  服务器联机等待时间  */  5,\
/*  本地ip              */  {0,0,0,0},\
/*  本地子网掩码        */  {255,255,255,0},\
/*  本地网关            */  {192,168,1,1},\
/*  本地主DNS           */  {0,0,0,0},\
/*  本地从DNS           */  {0,0,0,0},\
/*  中间服务器IP        */  {192,168,1,194},\
/*  OAM服务器IP  //106  */  {192,168,1,194},\
/*  中间服务器端口      */  10003,\
/*  OAM服务器端口       */  10003,\
/*  通信KEK4            */  {0,0,0,0},\
/*  oam通信DEK          */  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},\
/*  Middle通信DEK       */  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},\
/*  Middle通信IV        */  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},\
/*  通信KEK1            */  {0,0,0,0},\
/*  通信AES-IV          */  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},\
/*  信用卡交易终端ID		*/  "",\
/*  通信KEK3				*/  {0,0,0,0},\
/*  通信KEK2				*/  {0,0,0,0},\
/*  linkup启动时间间隔	*/  10,\
/*  oam通信等待时间		*/  3,\
/*  RJ45波特率			*/	115200,\
/*	上传钱箱信息时间间隔	*/	100,\
/*	服务器分配的ID		*/	"PM00000002",\
/*	设备的序列号			*/	"20170721",\
}


const s_NetworkConfigParameter_t network_config_rom = DEFAULT_NETWORK_CONFIG;

const s_SysConfigParameter_t sys_config_rom = DEFAULT_SYSTEM_CONFIG;



#include "ff.h"

FATFS flash_fatfs;

#define SYS_CFG_F_PATH        "0:sys_cfg_f.inc"
#define NET_CFG_F_PATH        "0:net_cfg_f.inc"


#include "debug_log.h"
#define cfgDebug       Printf_D

static FRESULT InitFileSystemT(void)
{
    FRESULT  fres;
    uint32_t index = 0;
    FIL      cfg_f;
    
    fres = f_mount(&flash_fatfs, "0:", 1);
    if (fres == FR_NO_FILESYSTEM)
    {
        fres = f_mkfs("1:", FM_EXFAT, 0, 0, 0);
        if (fres != FR_OK)
        {
            cfgDebug("fs_init","mkfs \"1:\" failed - %u\r\n", fres);
            return fres;
        }
        cfgDebug("fs_init","mkfs \"1:\" success\r\n");
        fres = f_mount(&flash_fatfs, "0:", 1);
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
        //f_write(&cfg_f, "12324123432145\r\n", 16, &index);
        if (fres != FR_OK)
        {
            sys_config_ram = sys_config_rom;
        }
        cfgDebug("fs_init","read \"%s\" - %u\r\n", SYS_CFG_F_PATH, fres);
    }
    f_close(&cfg_f);
    
    fres = f_open(&cfg_f, NET_CFG_F_PATH, FA_READ+FA_WRITE);
    if (fres == FR_NO_FILE)
    {
        fres = f_open(&cfg_f, NET_CFG_F_PATH, FA_CREATE_NEW + FA_WRITE);
        if (fres != FR_OK)
        {
            return fres;
        }
        cfgDebug("fs_init","create \"%s\" success\r\n", NET_CFG_F_PATH);
        network_config_ram = network_config_rom;
        index = sizeof(network_config_ram);
        fres = f_write(&cfg_f, &network_config_ram, index, &index);
        cfgDebug("fs_init","update \"%s\" - %u\r\n", NET_CFG_F_PATH, fres);
    }
    else
    {
        index = sizeof(network_config_ram);
        fres = f_read(&cfg_f, &network_config_ram, index, &index);
        //f_write(&cfg_f, "12324123432145\r\n", 16, &index);
        if (fres != FR_OK)
        {
            network_config_ram = network_config_rom;
        }
        cfgDebug("fs_init","read \"%s\" - %u\r\n", NET_CFG_F_PATH, fres);
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


int16_t SaveNetworkCfg(uint32_t index, uint8_t *buff, uint32_t len)
{
    FRESULT  fres;
    FIL      cfg_f;
    fres = f_open(&cfg_f, NET_CFG_F_PATH, FA_WRITE);
    if (fres == FR_OK)
    {
        fres = f_lseek(&cfg_f, index);
        if (fres == FR_OK)
            fres = f_write(&cfg_f, buff, len, &len);
        f_close(&cfg_f);
    }
    return fres;
}


uint8_t SaveAllConfig(void)
{
    int8_t err = 0;
    if (SaveSystemCfg(0, (uint8_t *)&sys_config_ram, sizeof(sys_config_ram)) != FR_OK)
    {
        err = -1;
    }
    if (SaveNetworkCfg(0, (uint8_t *)&network_config_ram, sizeof(network_config_ram)) != FR_OK)
    {
        err = -1;
    }
    return err;
}

