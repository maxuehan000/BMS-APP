#ifndef __CAN_UPGRADE_BW_H__
#define __CAN_UPGRADE_BW_H__

#include "McuHal.h"
#include "queue.h"

/* 蓝微 CAN 升级协议, 走 CAN1, 扩展帧 */
#define CAN_BWU_RX_ID        0x1F7006F1UL    /* PC -> BMS */
#define CAN_BWU_TX_ID        0x1F7006F2UL    /* BMS -> PC */

#define BMS_UPGRADE_ADDR     0x01            /* 升级从机地址 */

#define FLASH_FW_BACKUP_ADDR    UPDATE_BASE_ADDR    /* 固件备份区首地址 */

#define FW_UPGRADE_OUT_TIME     (10000/TIMEBASE_LOOP)   /* 升级超时时长 10s */
#define PACK_OUT_TIME           (200/TIMEBASE_LOOP)     /* 数据帧间超时 200ms */

extern void UpgradeBWSchedule(void);                    /* 主循环任务调用 */
extern void UpgradeBWMsgProcess(CanRxMessage RxMsg);    /* CAN 数据帧解析调用 */
extern void UpgradeBwTimerCallBack(u8 ticks);           /* 时基更新 */
extern void UpgradeBWRegister(queue TxQueue);           /* CAN 发送消息队列注册 */

#endif
