
#ifndef __CAN_OTA_SL_H__
#define __CAN_OTA_SL_H__

#define SL_OTA_PC_StdId1 		0x0A1	// PC主机的发送CAN标准帧ID1
#define SL_OTA_PC_StdId2 		0x0A2	// PC主机的发送CAN标准帧ID2

#define SL_OTA_BMS_StdId1 		0x0B1	// BMS从机的答复CAN标准帧ID1
#define SL_OTA_BMS_StdId2 		0x0B2	// BMS从机的答复CAN标准帧ID2

#define SL_OTA_PC1_Addr 		0xA1	// 主机(发给电池包)
#define SL_OTA_BMS1_Addr 		0xB1	// 从机(电池1回应)
#define SL_OTA_HEAD1 			0xE1	// 帧头1
#define SL_OTA_HEAD2			0x1E	// 帧头2

#define FOTA_CMD				0xFA	// 固件升级命令
#define SYS_DEVICE_INFO_GET_CMD	0xAA	// 设备信息获取命令

typedef enum {
	ACK_OK = 0,							// 成功
	FAIL_UNKNOWN= -1,					// 未知错误
	FAIL_BATTERY_LOW = -2,				// 电池电量不足
	FAIL_MEMORY_LOW = -3,				// 内存不足
	FAIL_DEV_STATUS_ERROR = -4,			// 设备状态错误
	FAIL_IMAGE_ERROR = -5,				// 镜像错误
	FAIL_TRANSPORT_ERROR = -6,			// 传输错误
	FAIL_INPUT_PARAMS_ERROR =-7,		// 参数错误
	FAIL_WRITE_IMAGE_ERROR =-8, 		// 写入错误
	FAIL_UNSUPPORT = -10,				// 不支持操作
	FAIL_CAN_COMMUINCATION_ERROR = -11,	// CAN通信异常
}SL_OTA_ACK;


typedef enum{
    SL_OTA_START = 0,       			// +0 启动升级
    SL_OTA_WRITE = 1,              		// +1 写入数据
    SL_OTA_COMMIT = 2,                	// +2 提交升级
    SL_OTA_ABORT = 3,      				// +3 中止升级
    SL_OTA_PREPARE = 4,      			// +4 准备升级
    SL_OTA_GET_VERSION = 8,       		// +8 获取版本
}SL_OTA_EVENT;

typedef union {
    u32 ul32;
    u8 uc8[4];
} u32_u8_union;


extern void OtaSlVarInit(void);

extern void OtaSlJudge(void);

extern void OtaSlReceiveProcess(CanRxMessage RxMsg);

extern void OtaSlTimerCallBack(u8 ticks);


#endif
