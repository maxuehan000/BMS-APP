#ifndef __UART_BW_H__
#define __UART_BW_H__

#include "McuHal.h"


// UART 通讯协议
#define UART_TIMEOUT                    3                  // 3*10ms = 20ms 帧内数据时间间隔不高于20ms

#define DATA_HEAD                       0xF1                // 帧头 Header
#define DATA_END                        0xF2                // 帧尾 End


#define ID_BROADCASE                    0x00                // 广播
#define ID_BMS                          0x01                // 电池包BMS
#define ID_NORMAL_CTRL                  0x02                // 通用控制板
#define ID_BLDC_CTRL                    0x03                // BLDC控制板
#define ID_NORMAL_CHG                   0x04                // 通用充电器
#define ID_SPEC_CHG                     0x05                // 特殊充电器
#define ID_KEY_CTRL                     0x06                // 按键板
#define ID_UI_CTRL                      0x07                // UI控制板
#define ID_PC                           0x08                // PC(ATE)


#define FUNC_PRODUCE                    0x10                // 生产信息类
#define FUNC_VOLTAGE                    0x20                // 电压信息类
#define FUNC_CURRENT                    0x30                // 电流信息类
#define FUNC_NTC                        0x40                // 温度信息类
#define FUNC_STATUS                     0x50                // 状态信息类
#define FUNC_CTRL                       0x60                // 控制管理类
#define FUNC_CALID                      0x70                // 校准管理类
#define FUNC_SETTING                    0x80                // 配置管理类
#define FUNC_ATE_INFO					0xA0				/* ATE强控和所有信息*/


// 0x10 生产信息类
#define COMM_DATE                       0x00                // 读写产品生产日期
#define COMM_SN                         0x01                // 读写产品系列号
#define COMM_VERSION                    0x02                // 读写软件版本信息
#define COMM_HW_VER						0x03
#define COMM_TD_CODE                    0x06                //读写二维码
// 0x20 电压信息类
#define COMM_TOTAL_VOL                  0x00                // 读写总电芯电压值
#define COMM_SINGLE_VOL                 0x01                // 读写单节电芯电压值
#define COMM_ALL_VOL                    0x02                // 读写全部各节电芯电压值
#define COMM_CHG_VOL                    0x03                // 读充电器的电压值
#define COMM_PACK_VOL					0x06
#define COMM_VDD_VOL					0x10

// 0x30 电流信息类
#define COMM_CHG_CUR                    0x00                // 读取充电电流值
#define COMM_DISCHG_CUR1                0x01                // 读取马达1的电流值
#define COMM_DISCHG_CUR2                0x02                // 读取马达2的电流值
#define COMM_CHG_OPA_CUR				0x06
#define COMM_AFE_BIAS					0x10

// 0x40 温度信息类
#define COMM_PCM_NTC                    0x00                // 读取PCM温度值
#define COMM_MOS_NTC                    0x01                // 读取MOS温度值
#define COMM_PCM_NTC_K                  0x02                // 读取PCM温度值K


// 0x50 状态信息类
#define COMM_STATUS                     0x00                // 读取PCM状态信息
#define COMM_SET_CAP                    0xB0                // 读取PCM额定容量
#define COMM_NOW_CAP                    0xB1                // 读取PCM剩余容量
#define COMM_RATE_SOC                   0xB2
#define COMM_FCC_CAP                    0xB3
#define COMM_DIS_CNT                    0xD0                // 读取PCM放电总次数
#define COMM_CHG_CNT                    0xC0                // 读取PCM充电总次数
#define COMM_KEY_PRESS                  0xF0                // 读取按键当前是否按下的状态
#define COMM_DIS_SHORT                  0x18                // 读取是否发生放电短路保护状态
#define COMM_Brush_Block                0x19                // 读取是否发生Brush_Block保护状态
#define COMM_CHG_OVP                    0x20                // 读取是否发生硬件二级过充保护状态

// 0x60 控制管理类
#define COMM_CHG_MOS_ON                 0x00                // 开充电MOS
#define COMM_CHG_MOS_OFF                0x01                // 关充电MOS
#define COMM_DIS_MOS_ON                 0x02                // 开放电MOS
#define COMM_DISD_MOS_OFF               0x03                // 关放电MOS
#define COMM_SET_LED_ON                 0x06                // ALL LED ON
#define COMM_SET_LED_OFF                0x07                // ALL LED OFF
#define COMM_SET_BLANCE					0x08
#define COMM_SET_TEST_MODE              0x09                /* 进入测试模式 */
#define COMM_SET_TEST_MODE_DM			0x10				/* 进入测试模式，兼容旧追觅项目 */

#define COMM_RESET_FACTORY              0x0C                // EEPROM RESET恢复出厂设置
#define COMM_SET_POWER_OFF              0x0D                // POWER OFF
#define COMM_SET_RESET					0x0F				// MCU RESET
#define COMM_READ_FLASH_CRC             0x5A                // 读取Flash地址内容对应的CheckSum(CRC16-X-MODEM)
#define COMM_CLEAR_STATUS				0xE0
#define COMM_RESET_SOC					0xE1
#define COMM_UVLOCK_ACTIVE				0xE2
#define COMM_AFEBIAS_RESET				0xE3
#define COMM_ENTER_SLEEP				0xE4


#define COMM_CONVERT_UART				0x21				// 切换uart口支持产测协议通讯

// 0x70 校准管理类
#define COMM_CALIB_TOTAL_VOL            0x00                // 电芯总电压校准
#define COMM_CALIB_CELL_VOL             0x01                // 各节电芯电压校准
#define COMM_CALIB_CHG_CURRENT          0x02                // 充电电流校准
#define COMM_CALIB_DIS_CURRENT          0x03
#define COMM_CALIB_ZERO_CURRENT         0x06
#define COMM_CALIB_CHARGER_VOL			0x07
#define COMM_CALIB_VPACK_VOL            0x08                // 电芯总电压校准
#define COMM_K_TOTAL_VOL                0x10                // 读取电芯总电压校准K值
#define COMM_K_CELL_VOL                 0x11                // 读取各节电芯电压校准K值
#define COMM_K_CHG_CURRENT              0x12                // 读取充电电流校准K值
#define COMM_K_DIS_CURRENT              0x13                // 读取充电电流校准K值
#define COMM_K_ZERO_CURRENT				0x16
#define COMM_CALIB_PCB_VDD				0x20
#define COMM_CALIB_RTC					0x21

// 0x80 配置管理类
#define COMM_CHG_OVER_NTC               0xC6                // 读写充电高温保护值
#define COMM_UNDER_TOTAL_VOL            0xD2                // 读写整体过放保护电压
#define COMM_DIS_OVER_NTC               0xD6                // 读写放电高温保护值
//#define COMM_TURN_TEST_MODE               0x5A                // 强制进入生产测试模式
#define COMM_CHECK_SHUTDOWN             0x11

/* 0xA0 厂测管理类 */
#define COMM_ATE_GENERAL_INFO			0x10		// 综合信息读取
#define COMM_ATE_MOS_LED_CTRL			0x00
#define COMM_ATE_GENERAL_DATA			0x11		// 综合数据读取


#define UART_TIMEOUT_MAX            (3)        /* 2*10ms = 20ms 帧内数据时间间隔不高于20ms */
#define UART_CMD_DELAY              (1)        /* 1*10ms = 10ms 命令回复延迟时间，确保单线通讯模式下数据正确 */

extern void UartReceive(u8 rdata);
extern void UartBmsStartSend(u8 *txbuf, u8 len);
extern void UartBmsSend(void);
extern void UartBwCtrl(void);

extern void UartSendStr(u8 *tx_pData);
extern void UartSendStrData(s32 tx_data, int base);
extern void UartDebugSendStr(u8 *Buf, u8 *tx_pData);
extern void UartDebugSendStrData(u8 *Buf, int32_t tx_data, int base);
extern void UartCmdAct(void);
extern void UartDebug(void);
extern void UartRxDisable(void);
extern void UartRxEnable(void);
//extern void FuncProduce(u8 RW_Flag);
extern void FuncVoltage(void);
extern void FuncCurrent(void);
extern void FuncNtc(void);
extern void FuncStatus(void);
extern void FuncCtrl(void);
//extern void FuncCalib(u8 RW_Flag);
extern void FuncSetting(void);
extern void FuncATE(u8 rwFlag);
extern u16 AdcNtcToTempK(u8 NtcType, u16 AdcNtc);
void devRegDataStaticInit(void);

#endif
