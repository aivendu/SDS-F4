#ifndef _SPI_H_
#define _SPI_H_

#include "stdint.h"

extern void SPIMspInit(void);
extern void W25QxxSetCS(uint8_t st);
extern void WK2124NssSet(uint8_t st);
extern int8_t W25QXX_ReadWriteByte(uint8_t flag, uint8_t *tx, uint8_t *rx, uint32_t size);
extern int8_t WK2124_ReadWriteByte(uint8_t flag, uint8_t *tx, uint8_t *rx, uint32_t size);

#endif


