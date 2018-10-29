#ifndef _MINI_PCIE_H_
#define _MINI_PCIE_H_


#include "stdint.h"
#include "stdarg.h"

extern void CheckWWANState(void);
enum
{
    MINIPCIE_POWRE_ON,
    MINIPCIE_POWRE_OFF,
    MINIPCIE_WWAN_STATE,
    MINIPCIE_SENDDATA_REALTIME,
};

extern int8_t  MiniPcieIoctl(int32_t port, uint32_t cmd, va_list args);
extern int8_t  MiniPcieOpen(int32_t port, const void *config, uint8_t len);
extern int8_t  MiniPcieClose(int32_t port);
extern int32_t MiniPcieRead(int32_t port, void *buf, uint32_t buf_len);
extern int8_t MiniPcieWrite(int32_t port, void *buf, uint32_t buf_len);


#endif

