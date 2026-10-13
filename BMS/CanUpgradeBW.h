#ifndef __CAN_UPGRADE_BW_H__
#define __CAN_UPGRADE_BW_H__

//#define CAN_2_STAND
#define CAN_2_EXT

#ifdef CAN_2_STAND
#define CAN_BWU_RX_ID      0x6F1
#define CAN_BWU_TX_ID      0x6F2
#elif defined CAN_2_EXT
#define CAN_BWU_RX_ID      0x1F7006F1UL
#define CAN_BWU_TX_ID      0x1F7006F2UL
#endif

#define BMS_UPGRADE_ADDR   0x01     // 升级协议中从机地址

#define FLASH_FW_BACKUP_ADDR		UPDATE_BASE_ADDR 	// 固件备份区首地址，0x08010000

#define FW_UPGRADE_OUT_TIME        (10000/TIMEBASE_LOOP)// 升级超时时长，10Sec
#define PACK_OUT_TIME              (200/TIMEBASE_LOOP)  // 数据帧间超时，重新进行数据包组装


extern void UpgradeBWSchedule(void);                    // 主循环任务调用
extern void UpgradeBWMsgProcess(CanRxMessage RxMsg);    // 数据帧解析调用
extern void UpgradeBwTimerCallBack(u8 ticks);           // 时基更新
extern void UpgradeBWRegister(queue TxQueue);           // CAN发送消息队列注册

#endif
