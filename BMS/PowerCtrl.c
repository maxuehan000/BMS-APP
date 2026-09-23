//#include "User.h"
//#include "PowerCtrl.h"
//#ifdef AFE_MODEULE
//#include "AfeModule.h"
//#endif
//#include "dataflash.h"
//#include "Key.h"
//#include "soc.h"
//#include "battercheck.h"
#include "board.h"

#ifdef BMS_SLEEP_FUNC
static u8  DSCUnlockAction = 0;
#endif

void PowerShutDownTimerSet(u16 TickCnts)
{
	ShutDownTimer = TickCnts;
}
void PowerEnterSleepTimerSet(u32 TickCnts)
{
	EnterSleepTimer = TickCnts;
}


/*****************************************************************************
 函数名称: MiscRun(void)
功能描述: 测试模式超时5分钟退出检测
输入参数 : void
返回值 :
*****************************************************************************/
void MiscRun(void)
{
	static u8  FeedDogTimer = 0;
	static u16 TestModeTimer = 0;
	
    if(!TimerMiscFlag)                      //10ms
		return;
    TimerMiscFlag = 0;

    if (++FeedDogTimer >= 10)
    {
    	FeedDogTimer = 0;
    	HwClrWdt();                                 // 系统喂狗
    }

    if (IS_FACTORY_TEST_MODE())
    {
        if(++TestModeTimer > TEST_5MIN_TIME)
        {
            EXIT_FACTORY_TEST_MODE();
        }
    }
    else
    {
        TestModeTimer = 0;
        AteForceCtrlTimer = 0;
    }
}

/*****************************************************************************
 函数名称: PowerSafeCheck(void)
 功能描述: 防耗死关机控制函数,作为代码有bug、设计缺陷造成不关机的兜底防护机制，
 		正常应用逻辑下不应该触发。函数内部数据尽可能独立，避免被外部逻辑干扰。
                建议在main的主循环中调用。
 输入参数 : void
 返回值 : void
*****************************************************************************/
#define POWER_SAFE_MIN_VOL		(BAT_MIN_VALUE+200)			// 建议电压:高于禁充电压(避免触发禁充), 低于过放电压
#define POWER_SAFE_DELAY		(1000U/TIMEBASE_LOOP*40U)	// 建议延时:大于故障电芯延时、充电/预充电压抬升时间(禁充->触发电压)和报警相关应用延时
void PowerSafeCheck(void)
{	
	static u16 SafeEventTimer = 0;

	if (TimerSafeFlag == 0)		// TimerSafeFlag为10ms时间片任务控制flag
	{
		return;
	}
	TimerSafeFlag = 0;

	SafeEventTimer++;
	
	if (BatData.VolMin > POWER_SAFE_MIN_VOL)
	{
		SafeEventTimer = 0;
	}
	else if (SafeEventTimer > POWER_SAFE_DELAY)
	{
		SafeEventTimer = 0;
		AfeFetForceClose();	
		Delay1ms(2000);
		AfeSetPowerMode(2);
		Delay1ms(1000);			// 确保CON持续关闭，避免外部操作又重开。(延时函数内部已喂狗)
	}
}

#ifdef BMS_SLEEP_FUNC
/*****************************************************************************
函数名称: PowerLowEnter(void)
功能描述: 低功耗进入处理
输入参数 : void
返回值 :
*****************************************************************************/
void PowerLowEnter(void)
{
	/* AFE set sleep */
	AfeSleepConfig(BW_ENABLE);

	/* save log for unexpected BOR state */
//	DataLogPwrOffSave(0);
	
	/* LED turn off */
#ifdef KEY_TO_LED
	LED_1_OFF();
	LED_2_OFF();
	LED_3_OFF();
	LED_4_OFF();
	// LED_5_OFF();
#endif
	LED_T_OFF();

	BAT_POW_OFF();
	NTC_POW_OFF();
	OPA_BIAS_OFF();	
	CAN_5V_OFF();
	CAN_VIO_OFF();
	
	HwSleepEnter();
}

#define SLEEP_DSGOFF_WKUP_PERIOD		((u16)30*60)		// 30min, unit:Sec
#define SLEEP_DSGON_WKUP_PERIOD			((u16)1*60)			// 60S, uint:Sec
/*****************************************************************************
函数名称: PowerLowLoop(void)
功能描述: 低功耗循环
输入参数 : void
返回值 :
*****************************************************************************/
void PowerLowLoop(void)
{
	static u32 AcuCapmAS = 0;
	u32 SleepCapmAS = 0;
	u16 PeriodWkupTimer = 0;
	u16 SleepLoopTimer = 0;

	Rtc1STick = 0;
	HwClrWdt();

	while(1)
	{
//		TEST_PIN_OFF();
		HwStopMode();
		HwClrWdt();
//		TEST_PIN_ON();
	//	Delay1ms(1);
	
		if (Rtc1STick)
		{
			Rtc1STick = 0;
			PeriodWkupTimer++;
			SleepLoopTimer++;
		}

		/* read AFE altn PIN check for wakeup info */
		if (AFE_ALTN_VALID() && (BatStatus.Bits.AfeErr==0))
		{	
			break;
		}

		/* read CHG_IN PIN check for wakeup info */
		if (CHG_IN() != BatStatus.Bits.ChgPlugin)
		{	
			break;
		}

		/* read DET_IN PIN check for wakeup info */
		if (DET_IN() != BatStatus.Bits.DetIn)
		{	
			break;
		}

	#ifdef KEY_TO_LED
		/* read LED key for wakeup */
		if (KEY_LED_PRESS())
		{
			break;
		}
	#endif

		/* update info to check wakeup */
		if (BatStatus.Bits.DetIn==1 || BatStatus.Bits.ChgPlugin==1)
		{
			if (PeriodWkupTimer > SLEEP_DSGON_WKUP_PERIOD)
				break;
		}
		else
		{
			if (PeriodWkupTimer > SLEEP_DSGOFF_WKUP_PERIOD)
				break;
		}
	}

	/* soc caculation */
	if (DisCurAvg < -SOC_DEADDISCUR)
	{
		SleepCapmAS = SleepLoopTimer*2/10;			// 200uA
	}
	else
	{
		SleepCapmAS = SleepLoopTimer*DisCurAvg;
	}
	SleepCapmAS += AcuCapmAS;
	
	AcuCapmAS = SleepCapmAS%SECOND_TO_HOUR;
	SocUpdateByCap(SleepCapmAS/SECOND_TO_HOUR);

	BatStatus.Bytes.HardFault = 0;		// wakeup auto clear hardfault to retry
	EnterSleepTimer = ENTER_SLEEP_NORMAL_DELAY - SLEEP_TO_WAKEUP_TIME;
}

/*****************************************************************************
函数名称: PowerLowExit(void)
功能描述: 低功耗退出处理
输入参数 : void
返回值 :
*****************************************************************************/
void PowerLowExit(void)
{
	HwSleepExit();
	
	/* NTC turn off */
	BAT_POW_ON();
	NTC_POW_ON();
	OPA_BIAS_ON();
	
	/* AFE exit sleep */
	AfeSleepConfig(BW_DISABLE);
	AfeStepReset();

	VarTimerReset();
}

u8 PowerLowCheck(void)
{
#if 0
	static u16 SleepCurCnt = 0; 		// 防干扰电流
	
	if (DisCurAvg > DIS_UNLOAD_CUR)
	{
		if (SleepCurCnt < 300)
		{
			SleepCurCnt++;
		}
	}
	else
	{
		if (SleepCurCnt > 0)
		{
			SleepCurCnt--;
		}
	}
#endif

	if ( (BatStatus.Bits.Chg==1)
		|| (BatStatus.Bits.Dis==1)			//(BatStatus.Bits.Dis==1 && SleepCurCnt>200)
		|| (BatStatus.Bits.DisDUV==1)
	//	|| (BatStatus.Bits.DisUV==1)		
	//	|| (BatStatus.Bits.DisOC==1 && BatStatus.Bits.DisOCL==0)
		|| (BatStatus.Bits.ChgPlugin==1 && CHG_ONLY_OT_STATE())
	//	|| (BatStatus.Bits.AfeErr==1)		// wait for AFE recovered
		|| (BMS_IS_UPGRADING())
		|| (IS_FACTORY_TEST_MODE()) )
	{	
		BatStatus.Bits.Sleep = 0;			
		EnterSleepTimer = 0;
		return 0;
	}

	if (BatStatus.Bits.KeyLed == 1)
	{
		return 0;
	}
#if 0
	if (BatStatus.Bits.DetIn != DET_IN())
	{
		return 0;
	}
	if (BatStatus.Bits.KeyOn==0 && VLOCK_IN())
	{
		return 0;
	}
	if (BatStatus.Bits.ChgPlugin!=CHG_IN() && IS_DC_CHG_ACTIVED())
	{
		return 0;
	}
	if (AFE_ALTN_VALID() || AfeGetDSGStatus())
	{
		return 0;   // do done
	}
#endif

#ifdef BMS_SLEEP_FUNC
	if (DSCUnlockAction != 0)
	{
		return 0;
	}
#endif
	if (BatStatus.Bits.ChgPlugin == 1)
	{
		if (BatStatus.Bits.ChgOV == 1)
		{
			if (BatData.VolMax < CHG_OVER_VOL_RE)
			{
				return 0;
			}
		}
		else
		{
			if (BatData.VolMax > CHG_OVER_VOL)
			{
				return 0;
			}
		}
	}
#ifdef FUNC_SAFETY
	if (BatData.VolMax > CHG_OVER_VOL)
	{
		return 0;			// 过充点以上电压不进休眠，确保识别软件二级过充
	}
#endif
	if (BatStatus.Bits.DisUV == 1)
	{
		if (BatData.VolMin > DIS_UNDER_VOL_RE)
		{
			return 0;
		}
	}
	else
	{
		if (BatData.VolMin < DIS_UNDER_VOL)
		{
			return 0;
		}
	}
	if (BatStatus.Bits.DisDUV == 1)
	{
		if (BatData.VolMin > DIS_UV2_RE_VOL)
		{
			return 0;
		}
	}
	else
	{
		if (BatData.VolMin < DIS_UV2_VOL)
		{
			return 0;
		}
	}

	if (BatStatus.Bits.CellNtcErr == 0)
	{
		if (BatStatus.Bits.ChgOT == 1)
		{
			if ((BatNtcTempMax<NTC_CHG_HIGH_RE) && (BatNtcTempMin>NTC_CHG_LOW_RE))
			{
				return 0;
			}
		}
		else
		{
			if ((BatNtcTempMax>NTC_CHG_HIGH) || (BatNtcTempMin<NTC_CHG_LOW))
			{
				return 0;
			}
		}

		if (BatStatus.Bits.DisOT == 1)
		{
			if ((BatNtcTempMax<NTC_DIS_HIGH_RE) && (BatNtcTempMin>NTC_DIS_LOW_RE))
			{
				return 0;
			}
		}
		else
		{
			if ((BatNtcTempMax>NTC_DIS_HIGH) || (BatNtcTempMin<NTC_DIS_LOW))
			{
				return 0;
			}
		}
	}

	if (BatStatus.Bits.DNtcErr == 0)
	{
		if (BatStatus.Bits.DisMOT == 1)
		{
			if (MosNtcTempMax < NTC_DMOS_HIGH_RE)
			{
				return 0;
			}
		}
		else
		{
			if (MosNtcTempMax > NTC_DMOS_HIGH)
			{
				return 0;
			}
		}
	}
	if (BatStatus.Bits.DNtcErr == 0)
	{
		if (BatStatus.Bits.ChgMOT == 1)
		{
			if (MosNtcTempMax < NTC_CMOS_HIGH_RE)
			{
				return 0;
			}
		}
		else
		{
			if (MosNtcTempMax > NTC_CMOS_HIGH)
			{
				return 0;
			}
		}	
	}

	if (BatStatus.Bits.ChgPlugin == 0)
	{
		if (EnterSleepTimer < (ENTER_SLEEP_NORMAL_DELAY-POWER_OFF_REQ_DELAY))
		{
			EnterSleepTimer = ENTER_SLEEP_NORMAL_DELAY-POWER_OFF_REQ_DELAY;
		}
	}
	else
	{
		if (EnterSleepTimer < (ENTER_SLEEP_NORMAL_DELAY-POWER_OFF_REQ_CHI_DELAY))
		{
			EnterSleepTimer = ENTER_SLEEP_NORMAL_DELAY-POWER_OFF_REQ_CHI_DELAY;
		}
	}
	EnterSleepTimer++;
	if (EnterSleepTimer < ENTER_SLEEP_NORMAL_DELAY)
	{
		return 0;
	}

	// if (BatStatus.Bits.Sleep == 0)
	// {
	// 	BatStatus.Bits.Sleep = 1;
	// 	KeyOnReset();
	// 	EnterSleepTimer = ENTER_SLEEP_NORMAL_DELAY - ENTER_SLEEP_WAIT_DELAY;	// run 0.5S for main loop
	// 	return 0;
	// }

	// if (LS_ON == EBikeNodeStateGet(NODE_PARABMS))
	// {
	// 	EnterSleepTimer = ENTER_SLEEP_NORMAL_DELAY - ENTER_SLEEP_WAIT_DELAY;	// run 0.5S for main loop
	// 	return 0;
	// }

	return 1;
}

void PowerLowProcess(void)
{
	if (BatStatus.Bits.Sleep == 0)
	{

	}
}

/*****************************************************************************
函数名称: PowerLowCtrl(void)
功能描述: 低功耗控制函数
输入参数 : void
返回值 :
*****************************************************************************/
void PowerLowCtrl(void)
{
	if (1 == PowerLowCheck())
	{
		PowerLowEnter();

		PowerLowLoop();

		PowerLowExit();
	}

	PowerLowProcess();
}
#endif

/*****************************************************************************
 函 数 名  : PowerStatusProcess
 功能描述  : 待机切换检查
 输入参数  :
 返 回 值  :
*****************************************************************************/
#define DSC_LOCK_TIME					(20000/TIMEBASE_LOOP)
#define CHARGER_PROT_ADD_TIME			(60000/TIMEBASE_LOOP - POWER_OFF_REQ_DELAY)
#define KEY_LED_ADD_TIME				(5000/TIMEBASE_LOOP)
void PowerStatusProcess(void)
{
#ifdef BMS_SLEEP_FUNC
	static u16 DSCLockTimer = 0;

    if (DSCUnlockAction)
    {
    	if (DSCLockTimer < DSC_LOCK_TIME)
    	{
    		DSCLockTimer++;
    	}
    	else
    	{
    		DSCLockTimer = 0;
    		DSCUnlockAction = 0;
    	}
    }
    else
    {
    	DSCLockTimer = 0;
    }
#endif

	/* UI event clear BatStatus */
    if (EVENT_NONE != UIEvent)
    {
        if ((EVENT_KEYDOWN==UIEvent) && (1==BatStatus.Bits.DetIn))
        {     
        #ifdef BMS_SLEEP_FUNC
            if (BatStatus.Bits.DisSC==1 && DSCUnlockAction==0)
            {
            	DSCUnlockAction = 1;
            	BatStatus.Bits.DisSC = 0;
            }
        #endif
            BatStatus.Bits.DisOC = 0;
            BatStatus.Bits.DisOCL = 0;
            BatStatus.Bits.DisUL = 0;

            BatStatus.Bits.PowerOff = 0;
        }
        else if ((EVENT_CHGIN == UIEvent) && (1==BatStatus.Bits.ChgPlugin) )
        {  	
       	#ifdef BMS_SLEEP_FUNC
            if (BatStatus.Bits.DisSC==1 && DSCUnlockAction==0)
            {
            	DSCUnlockAction = 1;
            	BatStatus.Bits.DisSC = 0;
            }
        #endif
            BatStatus.Bits.DisOC = 0;
            BatStatus.Bits.DisOCL = 0;
            BatStatus.Bits.DisUL = 0;

            BatStatus.Bits.PowerOff = 0;
        }
        else if ((EVENT_CHGOUT==UIEvent) && (0==BatStatus.Bits.ChgPlugin))
        {
        //    BatStatus.Bytes.ChgProt = 0;
        	BatStatus.Bits.ChgOC = 0;
        	BatStatus.Bits.ChgTO = 0;
        	BatStatus.Bits.ChgER = 0;
        	BatStatus.Bits.ChgOV = 0;
            BatStatus.Bits.ChgFC = 0;
        }		

		PowerOffTimer = 0;
		EnterSleepTimer = 0;
        UIEvent = EVENT_NONE;

        BatStatus.Bits.Sleep = 0;
    }
}

/*****************************************************************************
 函 数 名  : SleepOrPowerOffPause
 功能描述  : 调用此函数实现休眠或关机计数暂停，一般是在识别按键或充电器插入等识别需要时间计数的时候调用
             部分AFE关MOS后不能立即执行关机指令，需做延时等待则也可以调用，等状态稳定后执行真正关机动作
 输入参数  :
 返 回 值  :
*****************************************************************************/
void  SleepOrPowerOffPause(void)
{
    EnterSleepTimer = 0;
    ShutDownTimer = 50;
}

///*****************************************************************************
// 函 数 名  : PowerOffCtrl
// 功能描述  : 关机控制
// 输入参数  :
// 返 回 值  :
//*****************************************************************************/
//void PowerOffCtrl(void)
//{	
//#ifndef BMS_SLEEP_FUNC
//	static u16 DisSCDelayTimer = 0;
//	static u16 ChgProDelayTimer = 0;
//	static u16 KeyLedDelayTimer = 0;
//	static u8  keyLedStatus =0;
//#endif	
//	PowerOffTimer++;
//	if(IS_FACTORY_TEST_MODE()||(BMS_IS_UPGRADING()))
//	{
//		PowerOffTimer =0;
//		return;
//	}
//	if (BatStatus.Bits.DisSC == 1)						
//    {
//    	if (DisSCDelayTimer < DSC_LOCK_TIME)
//    	{
//    		DisSCDelayTimer++;							
//    		PowerOffTimer = 0;							// 短路锁定20S内只给解除1次，所以需延时20s关机
//    	}
//    }
//    else
//    {
//    	DisSCDelayTimer = 0;
//    }

//	if(BatStatus.Bytes.DisProt != 0)
//	{
//		BatStatus.Bits.PowerOff = 1;
//	}
//	else if(BatStatus.Bytes.HardFault != 0)
//	{
//		BatStatus.Bits.PowerOff = 1;
//	}
//	else if(BatStatus.Bits.ChgER || BatStatus.Bits.ChgOC)
//	{
//		BatStatus.Bits.PowerOff = 1;
//	}
//	else
//	{
//		if(BatStatus.Bits.KeyOn)
//		{
//			if (BatStatus.Bits.Dis == 1)
//		    {
//		    	PowerOffTimer = 0;
//		    }
//		    else if (BatStatus.Bits.Chg == 1)
//		    {
//		    	PowerOffTimer = 0;
//		    }
//		}
//	}
//	if(keyLedStatus != BatStatus.Bits.Active)
//	{
//		keyLedStatus = BatStatus.Bits.Active;
//		PowerOffTimer =0;
//	}
//    if (PowerOffTimer > POWER_OFF_REQ_DELAY)
//    {
//        BatStatus.Bits.PowerOff = 1;
//    }

//    if (ShutDownTimer)
//    {
//    	ShutDownTimer--;
//    }

//    /* 部分AFE关MOS后不能立即执行关机指令，需做延时等待，等状态稳定后执行真正关机动作 */
//    if ((BatStatus.Bits.PowerOff && (PowerOffTimer>(POWER_OFF_REQ_DELAY+POWER_OFF_DELAY)))
//       || (BatStatus.Bits.ShutDown && (ShutDownTimer==0)))
//    {
//    	PowerOffTimer = POWER_OFF_REQ_DELAY + POWER_OFF_DELAY;

//    	AfeFetForceClose();

//    	DataLogPwrOffSave(0);
//    	
//    	AfeSetPowerMode(2);

//    	Delay1ms(100);

//    //    while(1);			// wait power off
//    }
//    else
//    {
//        if (BatStatus.Bits.Dis || BatStatus.Bits.Chg)
//    	{
//    		DataLogPwrOnRecord();
//        }
//    }
//}

/*****************************************************************************
 函 数 名  : PowerOffCtrl
 功能描述  : 关机控制 过放或过流、短路锁定才关机
 输入参数  :
 返 回 值  :
*****************************************************************************/
void PowerOffCtrl(void)
{
    static u16 PowerOffTimer;
    static u8 poweroffsavemark;
    
    if(ComOverSleepTimer < COMOVER_DELAY)
    {
    	ComOverSleepTimer++;
    	
    	if(!BatStatus.Bits.PowerOff)
		{
    		PowerOffTimer = 0;
    	}
    }
    
    if(BatStatus.Bits.DisDUV || BatStatus.Bits.DisUV || (!BatStatus.Bits.KeyOn) || (BatStatus.Bytes.HardFault != 0))
    {
	//	if(BatStatus.Bits.AfeDsg==0&&BatStatus.Bits.AfeChg==0)
        {/*需要充放电mos关闭才允许倒计时关机，停止休眠允许计数*/
            EnterSleepTimer = 0;
            PowerOffTimer++;
        }
    }

 	if ((BmsGetWorkMode() == BMS_WORK_CHG) && BatStatus.Bits.Chg)
    {
    	PowerOffTimer = 0;
    }

	if (IS_FACTORY_TEST_MODE() /*|| (ComOverSleepTimer < COMOVER_DELAY)*/ || BMS_IS_UPGRADING())
    {
    	PowerOffTimer = 0;
    	return;
    }

    if (PowerOffTimer > POWER_OFF_REQ_DELAY)		// 60S
    {
        BatStatus.Bits.PowerOff = 1;
    }

    if (ShutDownTimer)
    {
        /*通讯置位，进行倒计时关机；或识别到按键按下、充电器接入，则置位该变量暂停进入关机*/
    	ShutDownTimer--;
    }

    if ((BatStatus.Bits.PowerOff && (PowerOffTimer>(POWER_OFF_REQ_DELAY + POWER_OFF_DELAY)))
       || (BatStatus.Bits.ShutDown && (ShutDownTimer==0)))
    {
			if(poweroffsavemark == 0)
			{
				DataLogPwrOffSave(0);
				poweroffsavemark++;
				Delay1ms(10);
			}
    	AfeFetForceClose();
	
    	AfeSetPowerMode(2);

		Delay1ms(150);
    }
    else
    {
		poweroffsavemark = 0;
    }
}

/*****************************************************************************
 函 数 名  : PowerCtrl
 功能描述  : 电源控制，确保电芯断电以防电芯耗死
 输入参数  :
 返 回 值  :
*****************************************************************************/
void PowerCtrl(void)
{
    if(!TimerPowFlag)
        return;
    TimerPowFlag = 0;

    PowerStatusProcess();

	  PowerOffCtrl();
	
#ifdef BMS_SLEEP_FUNC
	PowerLowCtrl();
#endif
}

