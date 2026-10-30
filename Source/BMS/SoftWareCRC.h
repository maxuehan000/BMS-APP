#ifndef __SW_CRC_H__
#define __SW_CRC_H__
  
extern unsigned int SwCrc32 (unsigned int crcini, unsigned char *buf, unsigned int len);

extern uint16_t CheckSum_CheckMemory(uint32_t checkAddr, uint32_t length);
uint16_t CRC16_ccitt(uint8_t *pBuf, uint16_t length)  ;
uint16_t CRC16_CheckConst(uint32_t checkAddr, uint32_t length);
u16 Crc16Calc(u8 data_In, u16 PreCheckSum);
uint16_t ModbusCrc16(uint8_t * puchMsg, uint16_t usDataLen);   //
uint16_t ModbusCrc16withcrc(uint16_t crcs,uint8_t * puchMsg, uint16_t usDataLen) ;   //crc16_modbus
uint16_t  Accumulation(uint8_t *buf,uint16_t len);
void ShortHLSwap(u8 *bytes);
void int4byteHLSwap(u8 *bytes);
void long8byteHLSwap(u8 *bytes);
#endif

