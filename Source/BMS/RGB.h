#ifndef __RGB_H__
#define __RGB_H__

#include "SysDefs.h"

/* RGB 灯数据脚: PA1, 复用为 TIM2_CH2 */
#define RGB_DIN_Pin    GPIO_PIN_1
#define RGB_DIN_Port   GPIOA

extern void RGBPwmInit(void);
extern void RGBPwmUp(void);

extern void AllLedOff(void);
extern void RGBError(void);
extern void RGBWarning(void);
extern void RGBLowBattery(void);
extern void RGBAllGreen(void);
extern void RGBSocDisplay(u8 soc);
extern void NewRGBOneUpdata(u8 idx, u8 r, u8 g, u8 b);

/* ==================== LED 触发状态枚举 (串口调试打印用) ==================== */
typedef enum
{
    LED_OFF         = 0,   /* 未初始化                                   */
    LED_PROTECT     = 1,   /* 保护位(过充/过放/短路/过流) -> 红快闪 2Hz   */
    LED_TEMP        = 2,   /* 温度异常(放电过温/低温) -> 红慢闪 0.5Hz     */
    LED_IMBALANCE   = 3,   /* 不均衡(SOH<80% 或 压差>200mV) -> 单黄快闪   */
    LED_CHARGING    = 4,   /* 充电中(0~95%) -> 黄流水呼吸                 */
    LED_CHG_FULL    = 5,   /* 充满 -> 全绿                                */
    LED_CHG_TRICKLE = 6,   /* 涓流充电(95~100%) -> 全黄                   */
    LED_SOC         = 7,   /* 工作/SOC 电量显示 -> 绿灯 1~4 颗            */
    LED_IDLE        = 8,   /* 空闲/休眠 -> 全灭                           */
    LED_LOW_SOC     = 9,   /* 低电量(SOC<15%) -> 单颗绿灯快闪 2Hz         */
} LedTrigger_E;

extern u8 gLedTrigger;     /* 当前 LED 触发状态 (值见 LedTrigger_E) */

#endif
