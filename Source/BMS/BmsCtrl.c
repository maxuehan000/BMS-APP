//#include "User.h"
//#include "BmsCtrl.h"
//#include "Key.h"
#include "board.h"
//////////////////////////////////////////////////
#define PRE_DIS_DELAY					50  //50 		// 50*10 = 500ms
#define PRE_DIS_SHORT_CUR				600		// 0.6A
#define PACK_VOL_ERR					5000	// 5.0V
#define BMS_CUR_DET						30 		// 30*10 = 300ms

//////////////////////////////////////////////////
static _BMS_WORK_MODE_T BmsWorkMode = BMS_WORK_IDLE;
static _SYS_STATE_T SystemState = SYSTEM_WORK;
static BMS_LOG_EVENT WarnState = LOG_NONE;

//////////////////////////////////////////////////

static void BmsSetWorkMode(_BMS_WORK_MODE_T value)
{
    BmsWorkMode = value;
}
_BMS_WORK_MODE_T BmsGetWorkMode(void)
{
    return BmsWorkMode;
}

_SYS_STATE_T BmsGetSystemState(void)
{
    return SystemState;
}

BMS_LOG_EVENT BmsGetWarnState(void)
{
	return WarnState;
}

/*****************************************************************************
 函 数 名  : BmsModeCheck
 功能描述  : 根据电流确认当期Bms工作模式
 输入参数  :
 返 回 值  :
*****************************************************************************/
static void BmsModeCheck(void)
{
	static u16 dsgCount = 0;
	static u16 chgCount = 0;
	static u16 idleCount = 0;

    if (!BatData.Valid)       				// 未完成全部电芯电压采样，不检测
    {
		BmsWorkMode = BMS_WORK_IDLE;
		return;
    }

    dsgCount++;
    if(DisCurAvg <= DIS_UNLOAD_CUR)
    {
       dsgCount = 0;
    }
    if(dsgCount > BMS_CUR_DET)
    {
		BmsSetWorkMode(BMS_WORK_DSG);
    }

    chgCount++;
    if(ChgCurAvg <= CHG_SMALL_CUR)
    {
       	chgCount = 0;
    }
    if(chgCount > BMS_CUR_DET)
    {
		BmsSetWorkMode(BMS_WORK_CHG);
    }

    idleCount++;
    if((ChgCurAvg > CHG_SMALL_CUR) || (DisCurAvg > DIS_UNLOAD_CUR))
    {
		idleCount = 0;
    }
    if(idleCount > BMS_CUR_DET)
    {
    	BmsSetWorkMode(BMS_WORK_IDLE);
    }
    
    ////////////////////////////////////////////////////////
    SystemState = SYSTEM_WORK;

    if (BatStatus.Bytes.HardFault)
    {
    	SystemState = SYSTEM_FAULT;
    }
    else if (BMS_IS_UPGRADING())
    {
    	SystemState = SYSTEM_UPDATING;
    }
    else if(BatStatus.Bits.ChgPlugin)
    {
    	SystemState = SYSTEM_CHARGE_IN;
		if(BatStatus.Bits.ChgOV || BatStatus.Bits.ChgFC)
    	{
    		SystemState = SYSTEM_FULL_CHARGE;
    	}
    	else if(BatStatus.Bits.ChgOC || BatStatus.Bits.ChgTO || BatStatus.Bits.ChgER)
    	{
    		SystemState = SYSTEM_CHARGE_FAIL;
    	}
    	else if (BmsGetWorkMode() == BMS_WORK_CHG)
    	{
    		SystemState = SYSTEM_CHARGING;
    	}
    	else if (BmsGetWorkMode() == BMS_WORK_DSG)
    	{
    		SystemState = SYSTEM_DISCHARGING;
    	}
    }
    else
    {
    	if (BmsGetWorkMode() == BMS_WORK_DSG)
    	{
    		SystemState = SYSTEM_DISCHARGING;
    	}
    	else if (BatStatus.Bits.Sleep == 1)
    	{
    		SystemState = SYSTEM_SLEEP;//SYSTEM_WORK;
    	}
    	else if (BatStatus.Bytes.DisProt)
    	{
    		SystemState = SYSTEM_DISCHARGE_FAIL;
    	}
    }
}

/*****************************************************************************
 函 数 名  : BmsMosCheck
 功能描述  : MOS控制，根据工作模式和保护状态更新MosState
 输入参数  :
 返 回 值  :
*****************************************************************************/
void BmsMosCheck(void)
{
	u8 DisAct = 0;
	static u8 PreDisStep = 0;
    static u16 PreDisTimer = 0;
//	static u16 PreDisShortCnt1 = 0;

	if (!BatData.Valid)
	{
		PreDisStep = 0;
		PreDisTimer = 0;
		return;
	}

	if(SleepOnMark == 0x5a)
	{
		if(BatStatus.Bits.Dis && BatStatus.Bits.Dmos)
		{
			PreDisStep = 2;									// 休眠唤醒后，跳过开预放mos,维持主放开启
		}
		SleepOnMark = 0;
	}

	DisAct = 0;
	if (BatStatus.Bits.Dis && MtBmsAllow.bit.DisMosAllow)	// 放电输出状态
	{
		DisAct = 1;
	}
	else if(BatStatus.Bits.DsgAble && MtBmsAllow.bit.DisMosAllow)
	{
		DisAct = 1;
	}
	else if(MtBmsAllow.bit.QUICKDisMos && BatStatus.Bits.DisSC==0)
	{
		DisAct = 1;
	}
	
	if(DisResetState())  									//放电复位
	{
		DisAct = 0;
	}
	
	if(DisAct)												// 允许开Mos
	{
		switch(PreDisStep)
		{
			case 0:
				/* 预放开启 */
				BatStatus.Bits.PDmos = 1;
				BatStatus.Bits.Dmos = 0;
				if(PreDisTimer < 2)							// 前20ms不检测
				{
//					PreDisShortCnt1 = 0;
				}
				else
				{
//					if(BatData.VolPack < 5000)				// 总压小于5V判定为短路
//					{
//						PreDisShortCnt1++;
//						
//						if(PreDisShortCnt1 > 36)			// 36*10ms = 360ms异常则报错
//						{

//							
//							BatStatus.Bits.DisSC = 1;
//							BatStatus.Bits.DisOC = 1;
//							BatStatus.Bits.PDmos = 0;
//							BatStatus.Bits.Dmos = 0;
//							PreDisTimer = 0;
//						}
//					}

				}
				
				if(ReadMtOutputState() == 1)				// 检测到外部已开启放电,直接转主放电Mos
				{
					BatStatus.Bits.Dmos = 1;				// 开启主放Mos
					PreDisTimer = PRE_DIS_DELAY;
				}
				
				PreDisTimer++;
				if(PreDisTimer > PRE_DIS_DELAY)
				{
					PreDisStep = 1;
//					PreDisShortCnt1 = 0;
				}
				break;

			case 1:
				/* 先保持主放Mos和预放Mos都开启100ms */
				BatStatus.Bits.PDmos = 1;
				BatStatus.Bits.Dmos = 1;
				PreDisTimer++;
				if(PreDisTimer > PRE_DIS_DELAY + 10)
				{
					PreDisStep = 2;
					PreDisTimer = 0;
				}
				break;

			case 2:
				/* 延迟100ms后关预放Mos,维持主放Mos开 */
				BatStatus.Bits.PDmos = 0;
				BatStatus.Bits.Dmos = 1;
				break;

			default:
				PreDisStep = 0;
				PreDisTimer = 0;
				BatStatus.Bits.PDmos = 0;
				BatStatus.Bits.Dmos = 0;
				break;
		}
	}
	else
	{
		PreDisStep = 0;
		PreDisTimer = 0;
		BatStatus.Bits.PDmos = 0;
		BatStatus.Bits.Dmos = 0;
//		PreDisShortCnt1 = 0;
	}

	if ((BmsGetWorkMode() == BMS_WORK_CHG) && BatStatus.Bits.Chg)	// 充电时，强制开放电Mos
	{
		BatStatus.Bits.Dmos = 1;
	}
	if ((BmsGetWorkMode() == BMS_WORK_DSG) && BatStatus.Bits.Dis)	// 放电时，强制开充电Mos
	{
		BatStatus.Bits.Cmos = 1;
	}
	else if (1 == BatStatus.Bits.Chg)								// 充电输出状态
	{
		BatStatus.Bits.Cmos = 1;
	}
	else
	{
		BatStatus.Bits.Cmos = 0;
	}
	
	if(BatStatus.Bits.Dis||BatStatus.Bits.Chg)
	{
		if(MtBmsAllow.bit.QUICKChgMos && BatStatus.Bits.DisSC==0)
		{
				BatStatus.Bits.Cmos = 1;                        	// 快开充电mos
		}
	}
}

/*****************************************************************************
 函 数 名  : BmsMosAction
 功能描述  : MOS动作执行，依据MosState
 输入参数  :
 返 回 值  :
*****************************************************************************/
void BmsMosAction(void)
{
	if (BatStatus.Bits.Dis || BatStatus.Bits.Chg)
	{
		AFE_EFET_ON();
	}
	else
	{
		AFE_EFET_OFF();
	}
}

/*****************************************************************************
 函 数 名  : BmsCtrl
 功能描述  : BMS执行控制逻辑
 输入参数  :
 返 回 值  :
*****************************************************************************/
void BmsCtrl(void)
{
	if (!TimerBmsFlag)			// 10ms ticks
		return;
	TimerBmsFlag = 0;

	BmsModeCheck();				// Bms工作模式

	BmsMosCheck();				// MOS控制

	BmsMosAction();				// MOS执行
}



