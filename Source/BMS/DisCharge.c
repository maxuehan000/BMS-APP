/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : DisCharge.c
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2017年1月6日 星期五
  最近修改   :
  功能描述   : 放电检测、执行C源文件
  函数列表   :
  修改历史   :
  1.日    期   : 2017年1月6日 星期五
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/
#include "board.h"
//#include "User.h"
//#include "mcuhal.h"
//#include "discharge.h"
//#include "BmsCtrl.h"
//#include "soc.h"
//#include "dataflash.h"
//#ifdef AFE_MODEULE
//#include "AfeModule.h"
//#endif

//#define DIS_MOS_FORCE_CONTROL
#ifdef DIS_MOS_FORCE_CONTROL
#define MCU_MOS_CLOSE_DELAY		10		// 100ms后强制关闭放电MOS
static u8 McuMosCloseTimer = 0;
#endif

#define AFE_STATE_DELAY			20		// Afe短路触发后,200ms置位Bms的状态
static u8 DisResetFlag = 0;
/*****************************************************************************
 函 数 名  : DisStart
 功能描述  : 开启放电
 输入参数  :
 返 回 值  :
*****************************************************************************/
void DisStart(void)
{
    BatStatus.Bits.Dis = 1;
    
#ifdef DIS_MOS_FORCE_CONTROL
    MCU_DMOS_ALLOW_ON();
    McuMosCloseTimer = MCU_MOS_CLOSE_DELAY;
#endif
}

/*****************************************************************************
 函 数 名  : DisStop
 功能描述  : 关闭放电
 输入参数  :
 返 回 值  :
*****************************************************************************/
void DisStop(void)
{  
#ifdef DIS_MOS_FORCE_CONTROL
	if (McuMosCloseTimer > 0)
	{
		McuMosCloseTimer--;
	}
	else
	{
		MCU_DMOS_FORCE_OFF();
	}
#endif

    BatStatus.Bits.Dis = 0;
}

/*****************************************************************************
 函 数 名  : DisUVReCheck
 功能描述  : 放电欠压恢复检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void DisUVReCheck(void)
{
	static u8 XRAM UvReTimer = 0;
	
	if (BatStatus.Bits.DisUV)
	{
		UvReTimer++;
		if ((BatData.VolMin<DIS_UNDER_VOL_RE) || (BatData.VolTotal<DIS_UNDER_VOL_TOTAL_RE))
		{
			UvReTimer = 0;
		}
		if (UvReTimer > DIS_UNDER_VOL_RE_DELAY)
		{
			BatStatus.Bits.DisUV = 0;
		}
	}
	else
	{
		UvReTimer = 0;
	}
}

/*****************************************************************************
 函 数 名  : DisUVCheck
 功能描述  : 放电欠压检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void DisUVCheck(void)
{
	u8 i;
	u16 over_bat, over_time;
	static u16 recover_time = 0;
	
	if (BatStatus.Bits.DisUV)
	{
		recover_time++;
		if ((BatData.VolMin<=DIS_UNDER_VOL_RE) || (BatData.VolTotal<=DIS_UNDER_VOL_TOTAL_RE))//||AfeErr.bit.duv)
		{
			recover_time = 0;
		}
		if (recover_time > DIS_UNDER_VOL_RE_DELAY)
		{
			BatStatus.Bits.DisUV = 0;
		}
	}
	else
	{
		recover_time = 0;
	}

	if ( !BatData.Valid
		|| BatStatus.Bits.DisUV 
		|| BatStatus.Bits.BatErr || BatStatus.Bits.ChgPlugin)
	{
		for(i = 0; i < CELL_NUM; i++)
		{
			DisUnderVolTimer[i] = 0;
		}
		return;
	}

//	if (DisCurAvg > DIS_UNLOAD_CUR)
//	{
		over_bat = DIS_UNDER_VOL;
		over_time = DIS_UNDER_VOL_DELAY;
//	}
//	else if (BatStatus.Bits.Dis == 0)
//	{
//		over_bat = DIS_UNDER_VOL_RE;
//		over_time = DIS_UNDER_VOL_PRE_DELAY;
//	}
//	else
//	{
//		over_bat = DIS_UV_UNLOAD_VOL;
//		over_time = DIS_UV_POWER_OFF_DELAY;
//		if (BatData.VolMin < DIS_UNDER_VOL)
//		{
//			over_time = DIS_UNDER_VOL_DELAY;
//		}
//	}

	// 过放检测，每节电芯均要做单独检测
	for(i = 0; i < CELL_NUM; i++)
	{
		if((BatData.Bat[i].Vol > over_bat) && (BatData.VolTotal > DIS_UNDER_VOL_TOTAL))//&&AfeErr.bit.duv==0)
		{
			DisUnderVolTimer[i] = 0;
		}

		if(DisUnderVolTimer[i] > over_time)
		{
			BatStatus.Bits.DisUV = 1;
			break;
		}
	}
}

/*****************************************************************************
 函 数 名  : DisDUVCheck
 功能描述  : 放电二级欠压检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void DisDUVCheck(void)
{
	static u16 DisDUVTimer = 0;

	if (BatStatus.Bits.DisDUV == 0)
	{
		if(BatStatus.Bits.Chg && ((BmsGetWorkMode() == BMS_WORK_CHG)))
		{
			DisDUVTimer = 0;
		}
		if (BatData.VolMin > DIS_UV2_VOL)
		{
			DisDUVTimer = 0;
		}
		else if (++DisDUVTimer > DIS_UV2_DELAY)
		{
			DisDUVTimer = 0;
			BatStatus.Bits.DisDUV = 1U;
		}		
	}
	else
	{
		if (BatData.VolMin <= DIS_UV2_RE_VOL)
		{
			DisDUVTimer = 0;
		}
		else if (++DisDUVTimer > DIS_UV2_RE_DELAY)
		{
			DisDUVTimer = 0;
			BatStatus.Bits.DisDUV = 0U;
		}
	}
}

/*****************************************************************************
 函 数 名  : DisCurCheck
 功能描述  : 马达1过流检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void DisOverCurCheck(void)
{
    // 过流1检测
    if ((DisCurAvg < DIS_OVER_CUR1))
    {
        DisOverCurTimer[0] >>= 1;
    }

    if(DisOverCurTimer[0] > DIS_OVER_CUR1_MOTOR1_DELAY)
    {
        BatStatus.Bits.DisOC = 1;
    }

#if 1
    // 过流2检测
    if ((DisCurAvg < DIS_OVER_CUR2))
    {
    	DisOverCurTimer[1] >>= 1;        
    }
    if(DisOverCurTimer[1] > DIS_OVER_CUR2_MOTOR1_DELAY)
    {
        BatStatus.Bits.DisOC = 1;
    }
#endif

#if 0
	// 过流3检测
	if ((DisCurAvg < DIS_OVER_CUR3)&& (AfeErr.bit.doc == 0))
	{
		DisOverCurTimer[2] >>= 1;		 
	}
	if(DisOverCurTimer[2] > DIS_OVER_CUR3_MOTOR1_DELAY)
	{
		BatStatus.Bits.DisSC = 1;		// 当做短路处理，直接关MOS
	}

	// 过流4检测
	if (DisCurAvg < DIS_OVER_CUR4)
	{
		DisOverCurTimer[3] >>= 1;		 
	}
	if(DisOverCurTimer[3] > DIS_OVER_CUR4_MOTOR1_DELAY)
	{
		BatStatus.Bits.DisSC = 1;		// 当做短路处理，直接关MOS
	}
#endif
}

/*****************************************************************************
 函 数 名  : DisCurMonitor
 功能描述  : 放电电流管理监控
 输入参数  :
 返 回 值  :
*****************************************************************************/
void DisCurMonitor(void)
{
	static u16 DMosErrTimer = 0;	
	
#if 0
	static u8 delay_dsc = 0;
	static u8 delay_ldoff = 0;
	static u8 delay_ldon = 0;
	static u16 DisIdleTimer = 0;	
	
	if( (BatStatus.Bits.DisSC == 0))
    {
    	delay_dsc++;
        if(delay_dsc > AFE_STATE_DELAY)				// 20*10ms = 200ms
        {
            BatStatus.Bits.DisSC = 1;
        }
    }
    else
    { 
    	delay_dsc = 0;
    }
    
    if(BatStatus.Bits.LoadOff == 0)
    {
//    	if(AfeLoad.bit.ldoff)
//    	{
//    		delay_ldoff++;
//    		if(delay_ldoff > AFE_STATE_DELAY)		// 20*10ms = 200ms
//	        {
//	            BatStatus.Bits.LoadOff = 1;
//	        }
//    	}
//    	else
//    	{
//    		delay_ldoff = 0;
//    	}
    }
    else
    {
    	delay_ldoff = 0;
//    	if(AfeLoad.bit.ldoff == 0)
//    	{
//    		BatStatus.Bits.LoadOff = 0;
//    	}
    }
    
    if(BatStatus.Bits.LoadOn == 0)
    {
//    	if(AfeLoad.bit.ldon)
//    	{
//    		delay_ldon++;
//    		if(delay_ldon > AFE_STATE_DELAY)		// 20*10ms = 200ms
//	        {
//	            BatStatus.Bits.LoadOn = 1;
//	        }
//    	}
//    	else
//    	{
//    		delay_ldon = 0;
//    	}
    }
    else
    {
    	delay_ldon = 0;
//    	if(AfeLoad.bit.ldon == 0)
//    	{
//    		BatStatus.Bits.LoadOn = 0;
//    	}
    }
	
#endif
	if (BatStatus.Bits.Dis == 1)
	{
		DMosErrTimer = 0;
	}
	else if (DisCurAvg < DIS_OFF_ERR_CUR)
	{
		DMosErrTimer = 0;
	}
	else if (++DMosErrTimer > DIS_OFF_ERR_CUR_DELAY)
	{
		BatStatus.Bits.DMosErr = 1;
	}

#if 0
//#ifdef DOC_AUTO_RELEASE
	#define DIS_OC_RELEASE_DELAY			(3000/TIMEBASE_LOOP)	// 3S
	#define DIS_OC_RECOVER_DELAY			(60000/TIMEBASE_LOOP)	// 60S
	#define DIS_OC_LOCK_TIMES				3
	static u16 DocRecoverTimer = 0;
	static u16 DocReleaseTimer = 0;
	static u8  DocReCnt = 0;
	static u8  LoadOffAct = 0;

	if(BatStatus.Bits.LoadOff)
	{
		BatStatus.Bits.DisOCL = 0;
		DocReCnt = 0;
		
		if (BatStatus.Bits.DisOC || BatStatus.Bits.DisSC)
		{
			LoadOffAct = 1;				// 记录在过流.短路恢复等待中，有负载移除动作
		}
	}

	if (BatStatus.Bits.DisOCL == 1)
	{
		DocRecoverTimer = 0;
		DocReleaseTimer = 0;
		DocReCnt = 0;
	}
	else if (BatStatus.Bits.DisOC || BatStatus.Bits.DisSC)
	{
		DocRecoverTimer = 0;
		DocReleaseTimer++;
		if (DocReleaseTimer > DIS_OC_RELEASE_DELAY)
		{			
			DocReCnt++;
			if(LoadOffAct)
			{
				DocReCnt = 0;
				LoadOffAct = 0;
			}
			
			if (DocReCnt <= DIS_OC_LOCK_TIMES)
			{
				BatStatus.Bits.DisOC = 0;
				BatStatus.Bits.DisSC = 0;
			}
			else
			{
				BatStatus.Bits.DisOCL = 1;
				BatStatus.Bits.DisOC = 0;
				BatStatus.Bits.DisSC = 0;
			}
		}
	}
	else if (DocReCnt > 0)
	{
		DocReleaseTimer = 0;
		DocRecoverTimer++;
		if (DocRecoverTimer > DIS_OC_RECOVER_DELAY)
		{			
			DocReCnt = 0;
		}
	}
	else
	{
		LoadOffAct = 0;
		DocReleaseTimer = 0;
		DocRecoverTimer = 0;
	}
#endif

#ifdef DSG_UNLOAD_EN
	static u8  LoadTimer = 0;
	static u32 UnLoadTimer = 0;	

	if (!BatStatus.Bits.Dis || BatStatus.Bits.ChgPlugin)
	{
		UnLoadTimer = 0;
		LoadTimer = 0;
	}
	else if (DisCurAvg > DIS_UNLOAD_CUR)
	{
		if (++LoadTimer > 100)
		{
			LoadTimer = 0;
			UnLoadTimer = 0;
		}
	}
	else if (++UnLoadTimer > DIS_UNLOAD_CUR_DELAY)
	{
		LoadTimer = 0;
		UnLoadTimer = 0;	
		BatStatus.Bits.DisUL = 1;
	}
#endif

}

/*****************************************************************************
 函 数 名  : DisNtcCheck
 功能描述  : 放电过温检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void DisNtcCheck(void)
{
	static u16 NtcFailTimer = 0;
	
    if(BatStatus.Bits.CellNtcErr)
    {
    	NtcDisTimer = 0;
    	NtcDisReTimer = 0;
    	BatStatus.Bits.DisOT = 0;
        return;
    }

    // NTC断线检测
    if((BatNtcTempMin <= NTC_CELL_OPEN) || (BatNtcTempMax >= NTC_CELL_SHORT))
    {
        if(++NtcFailTimer > NTC_FAIL_TIME)
        {        	
			NtcFailTimer = 0;
            BatStatus.Bits.CellNtcErr = 1;
        }
    }
    else
    {
        NtcFailTimer = 0;
    }
	
	// 放电过温检测
	if (BatStatus.Bits.DisOT)
	{
		NtcDisTimer = 0;
	}
	else if (0 == IS_DSG_OT_LOCKED())
	{
		if ((BatNtcTempMax<NTC_DIS_HIGH) && (BatNtcTempMin>NTC_DIS_LOW))
		{
			NtcDisTimer = 0;
		}
	}
	else
	{
		if ((BatNtcTempMax<NTC_DIS_HIGH_RE) && (BatNtcTempMin>NTC_DIS_LOW_RE))
		{
			NtcDisTimer = 0;
		}
	}

	if (NtcDisTimer > DIS_NTC_OT_DELAY)
	{
		BatStatus.Bits.DisOT = 1;
        DSG_OT_LOCK();
	}
	
    // 过温恢复检测
    if ((0==BatStatus.Bits.DisOT) && (0==IS_DSG_OT_LOCKED()))
    {
    	NtcDisReTimer = 0;
    }
    else if ((BatNtcTempMax>=NTC_DIS_HIGH_RE) || (BatNtcTempMin<=NTC_DIS_LOW_RE))
	{
		NtcDisReTimer = 0;
	}	
	else if (NtcDisReTimer > DIS_NTC_OT_RE_DELAY)
	{
		BatStatus.Bits.DisOT = 0;
		DSG_OT_UNLOCK();
	}	
}

/*****************************************************************************
 函 数 名  : DisMosNtcCheck
 功能描述  : 放电MOS过温检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void DisMosNtcCheck(void)
{
	static u16 MosNtcTimer = 0;
	static u16 MosFailTimer = 0;

	if (BatStatus.Bits.DNtcErr)
		return;

	// NTC断线检测
    if ( (DmosNtcTemp>=NTC_MOS_SHORT) || ((DmosNtcTemp<=NTC_MOS_OPEN) && (BatNtcTempMin>NTC_BAT_P10)) )
    {
        if(++MosFailTimer > NTC_FAIL_TIME)
        {        	
			MosFailTimer = 0;
			MosNtcTimer = 0;
            BatStatus.Bits.DNtcErr = 1;
            BatStatus.Bits.DisMOT = 0;
        }
    }
    else
    {
        MosFailTimer = 0;
    }
	
	if (BatStatus.Bits.DisMOT == 0)
	{
		if (DmosNtcTemp < NTC_DMOS_HIGH)
		{
			MosNtcTimer = 0;
		}
		else if (++MosNtcTimer > MOS_NTC_OT_DELAY)
		{
			MosNtcTimer = 0;
			BatStatus.Bits.DisMOT = 1;
		}		
	}
	else
	{
		if (DmosNtcTemp > NTC_DMOS_HIGH_RE)
		{
			MosNtcTimer = 0;
		}
		else if (++MosNtcTimer > MOS_NTC_OT_DELAY)
		{
			MosNtcTimer = 0;
			BatStatus.Bits.DisMOT = 0;
		}
	}
}

/*****************************************************************************
 函 数 名  : DisPreCheck
 功能描述  : 开启放电前的过温、过压预检测
 输入参数  :
 返 回 值  :
            0: 正常 1: 过温或过压
*****************************************************************************/
u8 DisPreCheck(void)
{
    if (BatStatus.Bits.Dis)
    {
        return 0;
    }

	if (IS_DSG_OT_LOCKED())
	{			
		if((BatNtcTempMax>=NTC_DIS_HIGH_RE) || (BatNtcTempMin<=NTC_DIS_LOW_RE))
	    {
	        return 1;
	    }
	}
	else
	{
	    if((BatNtcTempMax>=NTC_DIS_HIGH) || (BatNtcTempMin<=NTC_DIS_LOW))
	    {
	        return 1;
	    }
	}

	if ((BatData.VolMin <= DIS_UNDER_VOL_RE) || (BatData.VolTotal <= DIS_UNDER_VOL_TOTAL_RE))
	{
		return 1;
	}

    if (BatData.VolMax > BAT_FAIL_VALUE)
    {
        return 1;
    }

    if (!BatData.Valid)       // 如未完成全部电芯电压采样，不进行电压、电量检测
    {
        return 1;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : DisStopFlag
 功能描述  : 关闭放电检测:
 			无充电器，仅短路直接关MOS;
 			有充电器,仅正常或过放且正常充电才开MOS;
 输入参数  :
 返 回 值  :
*****************************************************************************/
u8 DisStopFlag(void)
{   
    if (!BatStatus.Bits.DetIn)
    {
        return 1;
    }
    if (!BatStatus.Bits.KeyOn)
    {
    	return 1;
    }

    if (BatStatus.Bits.PowerOff)
    {
        return 1;
    }

    if (BatStatus.Bytes.HardFault)
    {
    	return 1;
    }

	if (BatStatus.Bytes.DisProt)
	{
	    return 1;
	}
	
#ifdef SOC_SUPPORT
    if (SocIsOcvWait())
    {
        return 1;
    }
#endif

    if (!BatData.Valid)       // 如未完成全部电芯电压采样，不进行电压、电量检测
    {
        return 1;
    }

    return 0;
}

u8 DisErrCheck(void)
{
    if(BatStatus.Bytes.DisProt || BatStatus.Bytes.HardFault||1 == DisPreCheck())
        return 0;
    return 1;
}

void DisMosAbleUpdate(void)
{
    /*有充电电流或无放电禁止或未出现放电电流，则放电mos允许开；
    如果已经允许开，则判断充电电流小于0.5a且放电禁止，放电mos禁止开*/
    static u8 delaydisable,delaydisdisable,discurlockdelay;
    static u8 discurlock = 0xFF;
    static u8 lastdisable = 0xFF;
    
    u8 disable = DisErrCheck();

    if(disable==0)
    {/*不能放电情况下出现放电电流，则放电电流锁置位，不再主动允许dmos开,有充电电流或允许放电情况下可以清掉锁*/

        if(DisCurAvg>Dis_Sleep_Cur_V)
        {
            delaydisable = 0;
            if(discurlockdelay++>30)
			{
                discurlock = 0x5a;
			}
        }
        else 
		{
			discurlockdelay= 0;
		}
    }
    else 
    {
        discurlock  = 0;
        discurlockdelay=0;
    }

    if(BatStatus.Bits.DsgAble)
    {/*从能正常放电到异常，清除一下放电允许*/

        if(disable==0)
        {
            if(lastdisable == 1)
            {
                BatStatus.Bits.DsgAble = 0;
                delaydisable = 0;
                discurlock = 0x5a;
            }
        }
        lastdisable = disable;
    }

	/*无充电器触发了过放保护且无充电电流，关放电mos允许*/
	if(!BatStatus.Bits.ChgPlugin && (ChgCurAvg < CHG_SMALL_CUR) && (BatStatus.Bits.DisDUV || BatStatus.Bits.DisUV))
    {
        BatStatus.Bits.DsgAble = 0;
        delaydisable = 0;
    }

    if(BatStatus.Bits.DsgAble==0||discurlock==0)
    {		
        delaydisdisable=0;
        if(ChgCurAvg>CHG_SMALL_CUR||disable==1||discurlock==0)
        {
            if(delaydisable++>30)
            {
                BatStatus.Bits.DsgAble = 1;
                if(ChgCurAvg>CHG_SMALL_CUR&&discurlock)
				{
                    discurlock = 0;
				}
            }
        }
        else 
		{
			delaydisable = 0;

		}
    }
    else 
    {
        delaydisable = 0;
        if(ChgCurAvg<CHG_SMALL_CUR&&disable==0)
        {
			if(delaydisdisable++>30||discurlock)
			{
                BatStatus.Bits.DsgAble = 0;
			}
        }
        else 
		{
			delaydisdisable = 0;
		}
    }
}

void DisClearResetState(void)
{
	static u8 TimerDisReset;
	
	if(DisResetFlag) //放电复位标志
	{
		if(TimerDisReset++ >= 100) //1S清除复位标志
		{
			DisResetFlag = 0;
		}
	}
	else
	{
		TimerDisReset = 0;
	}
}


void SetDisResetState(void)
{
	DisResetFlag = 1;
}

u8 DisResetState(void)
{
	return DisResetFlag;
}
/*****************************************************************************
 函 数 名  : DisCtrl
 功能描述  : 放电控制
 输入参数  :
 返 回 值  :
*****************************************************************************/
void DisCtrl(void)
{
    if(!TimerDisFlag)
    {
        return;
    }
    TimerDisFlag = 0;

    DisUVCheck();
    DisDUVCheck();
    DisOverCurCheck();
    DisCurMonitor();
    DisNtcCheck();
    DisMosNtcCheck();
	
	DisMosAbleUpdate();
	DisClearResetState();

    if (IS_FCT_FORCE_CTRL())
    {
    	return;
    }

	if (1 == DisStopFlag())
	{
		DisStop();		
	}	
	else
	{
		if (0 == BatStatus.Bits.Dis)
		{
			if (0 == DisPreCheck())
			{
				DisStart();
			}
		}
	}
}

