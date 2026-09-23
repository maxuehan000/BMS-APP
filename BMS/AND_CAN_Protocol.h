#ifndef __AND_CAN_PROTOCOL_H__
#define __AND_CAN_PROTOCOL_H__



extern queue AndRxQueue;
extern queue AndTxQueue;

typedef enum
{
	CAN_WORK_NONE = 0,						// 0	None
	CAN_POWER_OFF = 1,						// 1	关机
	CAN_POWER_ON = 2,						// 2	开机
} _CAN_WORK_STATE_T;


typedef enum
{
	EBIKE_NONE = 0,
	EBIKE_NEW = 1, //新车
	EBIKE_OLD = 2, //旧车
}_EBIKE_MODEL_T;  

typedef enum
{
	PRIVATE_NONE = 0,
	PRIVATE_OPEN = 1, //开启
	PRIVATE_CLOSE = 2, //关闭
}_PRIVATE_STATE_T;  

extern void AndCanInit(void);
extern void AndCanCtrl(void);
extern void AndCanTimerCallBack(u8 ticks);
extern u8 ReadEcuChgPluginState(void);
extern u8 ReadEcuSleepState(void);
extern u8 ReadEcuLockPowerState(void);
extern void ClearEcuChgPluginState(void);
extern void ClearEcuSleepState(void);
extern void ClearEcuLockPowerState(void);

extern uint16_t CheckSum_CheckMemory_X(uint32_t checkAddr, uint32_t length);

#endif
