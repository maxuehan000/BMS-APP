#ifndef __UPGRADE_CAN_H__
#define __UPGRADE_CAN_H__

#include "McuHal.h"

extern uint16_t CheckSum_CheckMemory(uint32_t checkAddr, uint32_t length);
extern uint8_t AppUpgradeParse(CanRxMessage CanRxMsg, CanTxMessage *TxMsg);


#endif /* __can_H__ */


