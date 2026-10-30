/**
 * Copyright (C) 2024 HuiZhou Blueway Electronics CO.,LTD.
 * All rights reserved
 *
 * @file   AfeFunctional.h
 * @brief  AfeFunctional.c header file
 * @author Rock Shi
 * @date   2024/11/25
 * History       :
 * 1.Date        : 2024/11/25
 *   Author      : Rock Shi
 *   Modification: Created file
 */


#ifndef __AFEMODULE_H__
#define __AFEMODULE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "kc81710h.h"
#include "SysDefs.h"

typedef struct
{
    u16 Status0             : 16;    /* Byte1 */
    u16 Status1             : 16;    /* Byte2 */
    u16 Status2             : 16;    /* Byte3 */    
    u16 Status3             : 16;	 /* Byte4-5 */
    u16 Status4             : 16;    /* Byte6 */
} AfeStatusByte;

typedef struct
{
    /* 2Bytes(0-1) Status0 */    
    u8 OV_STATUS            : 1;    /* 电芯过充被确认        */
    u8 UV_STATUS_REAL       : 1;    /* 真实的电芯过放被确认  */
    u8 OCD1_STATUS          : 1;    /* 放电过流1被确认   	 */
    u8 OCD2_STATUS          : 1;    /* 放电过流2被确认    	 */
    //-------------------------------------------------------------------------------
    u8 SCD_STATUS           : 1;    /* 放电短路确认         */    
    u8 OCC1_STATUS        	: 1;    /* 充电过流1被确认      */   
    u8 						: 2;	/* 预留     		 */
    //-------------------------------------------------------------------------------
    u8 OCC2_STATUS          : 1;    /* 充电过流2被确认 */    
    u8 SOV_STATUS           : 1;    /* 二次过压被确认 */
    u8 OTD_STATUS       	: 1;    /* 任一外部温度被检测到放电过温且被确认 */
    u8 OTC_STATUS           : 1;    /* 任一外部温度被检测到充电过温且被确认 */
    //-------------------------------------------------------------------------------
    u8 UTC_STATUS           : 1;    /* 任一外部温度被检测到充电低温且被确认 */
    u8 UTD_STATUS           : 1;    /* 任一外部温度被检测到放电低温且被确认 */    
    u8 OHT_STATUS          	: 1;    /* 内部过热时(大约145度) */
    u8 CO_STATUS	        : 1;    /* 电芯断线被检出 */     
	
    /* 2Bytes(2-3) Status1 */
    u8 IDLE_STATUS       	: 1;    /* 不充不放状态被检出       */
    u8 INCHG_STATUS         : 1;    /* 充电状态被检出     */
    u8 INDSG_STATUS         : 1;    /* 放电状态被检出       */
	u8 LDON_STATUS          : 1;    /* 负载插入被检出       */   
    //-------------------------------------------------------------------------------
    u8 LDOFF_STATUS         : 1;    /* 负载移除被检出          */    
    u8 CHGRIN_STATUS        : 1;    /* 充电器插入被检出        */
    u8 CHGROFF_STATUS       : 1;    /* 充电器移除被检出      */
    u8  					: 1;	/* 预留				    */
	//-------------------------------------------------------------------------------
    u8 DEEP_SLEEP           : 1;    /* 1：处于deep sleep; 0: 其它状态。 */
    u8 NORMAL_SLEEP         : 1;    /* 1：在normal sleep；0: 其它状态。 */
    u8 						: 6;	/* 预留				 */

	/* 2Bytes(4-5) Status2 */ 
    u8 			            : 8;    /* 预留         */ 
	u8 			            : 1;    /* 预留         */ 
    u8 DSG_FAIL_STATUS		: 1;    /* 确认放电管失效 */
    u8 CHG_FAIL_STATUS      : 1;    /* 确认充电管失效 */ 
    u8 TS2_MOT_STATUS       : 1;    /* TS2被用作为检测MOS温度，发生过温且被确认 */
	//-------------------------------------------------------------------------------	
    u8 FUSE_OFF_STATUS		: 1;	/* 外部的FUSE被烧过置为1，只有重新上电才被清为0 */ 
	u8 V0_STATUS_REAL		: 1;	/* 达到0V禁充*/ 
	u8 EOC_STATUS			: 1;    /* 达到充电截止 */   
	u8 V0_STATUS_SLEEP		: 1;	/* Deep Sleep模式下的V0状态 */
    
    /* 2Bytes(6-7) Status3 */
    u8 TS0_OTD_STATUS       : 1;    /* TS0_OTD */ 
    u8 TS0_OTC_STATUS		: 1;	/* TS0_OTC */
    u8 TS0_UTC_STATUS		: 1;	/* TS0_UTC */ 
	u8 TS0_UTD_STATUS		: 1;	/* TS0_UTD */ 
	//-------------------------------------------------------------------------------
    u8 TS1_OTD_STATUS       : 1;    /* TS1_OTD */ 
    u8 TS1_OTC_STATUS		: 1;	/* TS1_OTC */
    u8 TS1_UTC_STATUS		: 1;	/* TS1_UTC */ 
	u8 TS1_UTD_STATUS		: 1;	/* TS1_UTD */ 
	//-------------------------------------------------------------------------------
    u8 TS2_OTD_STATUS       : 1;    /* TS2_OTD */ 
    u8 TS2_OTC_STATUS		: 1;	/* TS2_OTC */
    u8 TS2_UTC_STATUS		: 1;	/* TS2_UTC */ 
	u8 TS2_UTD_STATUS		: 1;	/* TS2_UTD */ 
	//-------------------------------------------------------------------------------
  	u8 			            : 3;    /* 预留        			 */ 
    u8 UV_STATUS_SLP		: 1;    /* Deep Sleep模式下的UV状态 */

	/* 2Bytes(8-9) Status4 */
    u8 DSG_EN				: 1;	/* 放电管  				1: 开启         0: 关闭 */
    u8 CHG_EN				: 1;	/* 充电管  				1: 开启         0: 关闭 */
    u8 PDSG_EN				: 1;	/* 预放  				1: 开启         0: 关闭 */
    u8 PCHG_EN				: 1;	/* 预充  				1: 开启         0: 关闭 */
	u8 						: 4;   /* 预留					 */ 
    //-------------------------------------------------------------------------------
    u8 						: 8;   /* 预留					 */   
	
}AfeStatusBit;

typedef union
{
    AfeStatusByte Bytes;
    AfeStatusBit Bits;
} AfeStatus_T;

MainDef AfeStatus_T AfeStatus;

#define AfeFetForceClose()		while(KC_STATUS_OK != AfeSetMosfet(0));

extern t_KC81710H_CHIP qcgChip;

extern void AfeInit(void);
extern void AfeRun(void);
extern uint8_t AfeSetMosfet(uint8_t status);
extern uint8_t AfeSetPowerMode(uint8_t mode);
extern uint8_t AfeSetPdsgTimeout(uint16_t ms);
extern uint8_t AfeChipStatusGet(void);
extern void AfeChipStatusClear(void);
extern void AfeDeepSleepManage(void);
extern void AfeTimerInit(void);
extern void AfeTimerCallBack(u8 ticks);
extern s32 AfeGetCurrentFilt(void);
extern u8 AfeSetBalance(u32 channels);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __AFEFUNCTIONAL_H__ */
