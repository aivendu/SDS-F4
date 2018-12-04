#ifndef __CRC_H__
#define __CRC_H__

#include <stdint.h>


//CRC16校验选用了一种常见的查表方法,类似的还有512字
//节、256字等查找表的，至于查找表的生成,这里也略过。
// ---------------- POPULAR POLYNOMIALS ----------------
// CCITT:      x^16 + x^12 + x^5 + x^0                 (0x1021)
// CRC-16:     x^16 + x^15 + x^2 + x^0                 (0x8005)
//#define        CRC_16_POLYNOMIALS      0x8005

extern uint16_t CRC16(uint16_t crc, uint8_t *crc_data, uint8_t crc_data_len);

extern uint32_t crc_32(uint8_t *buf, uint32_t length);

extern uint16_t CRC_ccitt(uint8_t *b, uint16_t crcx, uint8_t n);

#endif
