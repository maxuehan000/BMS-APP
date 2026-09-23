/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : SysDefs.h
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2016年11月30日 星期三
  最近修改   :
  功能描述   : 系统及硬件定义
  函数列表   :
  修改历史   :
  1.日    期   : 2016年11月30日 星期三
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/

#ifndef __SYSDEFS_H_
#define __SYSDEFS_H_

#include "McuHal.h"

#define CPU_CLOCK			48000000UL

/*!< STM8 Standard Peripheral Library old types (maintained for legacy purpose) */

typedef int32_t  		s32;
typedef int16_t 		s16;
typedef int8_t  		s8;

typedef uint32_t  		u32;
typedef uint16_t 		u16;
typedef uint8_t  		u8;
typedef uint8_t			bit;

#define U8_MAX     (255)
#define S8_MAX     (127)
#define S8_MIN     (-128)
#define U16_MAX    (65535u)
#define S16_MAX    (32767)
#define S16_MIN    (-32768)
#define U32_MAX    (4294967295uL)
#define S32_MAX    (2147483647)
#define S32_MIN    (-2147483648uL)

#define HIBYTE(V)				((u8)((V)>>8))
#define LOBYTE(V)				((u8)((V)&0xff))

typedef enum
{
	BW_DISABLE = 0,
	BW_ENABLE = !BW_DISABLE
}BW_ABLE_CMD;



#define BAT_SDI_50P				5



#if 1		
#define PROJ_XHT
#define BAT_TYPE				BAT_SDI_50P
#define SW_VERSION				"26EN001-01-b5"
#define HW_VERSION				"XHT-13S3P-0A"
#define SW_SUBVERSION     		0x0000
#define HW_SUBVERSION    		0

#define CELL_PARAL				3					// 13S3P
#define KEY_TO_LED
#endif



//#define COM_TIME_OUT
//#define UART_DEBUG                                // 使能Uart,打印调试信息控制宏
//#define UART_CLIENT                               // 客户产品uart通讯宏
#define BOOTLOADER
#define SOC_SUPPORT
#define AFE_MODEULE
#define OPA_COC_EN
//#define VI_BAT
// #define BMS_SLEEP_FUNC
//#define MCU_VDD_CHECK
#define SWD_DEBUG
//#define DSG_UNLOAD_EN
#define DOC_AUTO_RELEASE
//#define CAN_SW_PROTOCOL
#define CAN_AND_PROTOCOL
#define CAN_OTA_SL
#define CHG_OVER_NTC_DISP		//充电过温假充

//#define LAB_TEST_MODE    //可靠性测试模式


#define CELL_NUM                13
#define BAT_NTC_NUM				3
#define MOS_NTC_NUM				2

/* 时基 */
#define TIMEBASE_TICK       	1                       /* 单位: ms */
#define TIMEBASE_LOOP       	10                      /* 单位: ms */
#define TIMEBASE_LOAD       	10





#endif

