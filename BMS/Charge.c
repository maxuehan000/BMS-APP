/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : Charge.c
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2017年1月6日 星期五
  最近修改   :
  功能描述   : 充电检测、执行C源文件
  函数列表   :
  修改历史   :
  1.日    期   : 2017年1月6日 星期五
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/
#include "board.h"
//#include "User.h"
//#include "mcuhal.h"
//#include "charge.h"
//#include "Balance.h"
//#include "soc.h"
//#include "dataflash.h"
//#include "BmsCtrl.h"
//#ifdef AFE_MODEULE
//#include "AfeModule.h"
//#endif

//static u8 ChgStartFlag = 0;                    // 充电起始标识，充电器移除时才能清除此标识

u8 ChgPreCheck(void);


void ChgMultclrSmall(void)
{
    ChgSmallCurTimer = 0;
}

/*****************************************************************************
 函 数 名  : ChgStart
 功能描述  : 开启充电
 输入参数  :
 返 回 值  :
*****************************************************************************/
void ChgStart(void)
{
	BatStatus.Bits.PowerOff = 0;
 
	CHG_MOS2_ON(); 
	BatStatus.Bits.Chg = 1;
//	ChgStartFlag = 1;
}

/*****************************************************************************
 函 数 名  : ChgStop
 功能描述  : 关闭充电
 输入参数  :
 返 回 值  :
*****************************************************************************/
void ChgStop(void)
{
	CHG_MOS2_OFF();	
	BatStatus.Bits.Chg = 0;

    // 充电器移除，需清除充电异常标识
    if(!BatStatus.Bits.ChgPlugin)
    {
//		ChgStartFlag = 0;
    }
}

//void ChgPlugInCheck(void)
//{ 
//	static u8 chg_in = 0;
//	static u8 cChgInCnt;
//	static u8 cChgOutCnt; 
//	static u8 cChgErrCnt; 
//	
//	if(BatStatus.Bits.Cmos)
//	{
//		if(BmsGetWorkMode() == BMS_WORK_CHG)
//		{
//			chg_in = 1;
//		}
//		else
//		{
//			chg_in = 0;
//		}
//		cChgErrCnt = 0;
//	}
//	else
//	{
//		if(BatData.VolPack > BatData.VolTotal + CHG_VOL_OFFSET)
//		{
//			chg_in = 1;
//			
//			if((BatData.VolPack < CHG_VALID_LOW) || (BatData.VolPack > CHG_VALID_HIGH))
//			{
//				chg_in = 0;
//				cChgErrCnt++;
//			}
//			else
//			{
//				cChgErrCnt = 0;
//			}	
//		}
//		else
//		{
//			cChgErrCnt = 0;
//			
//			if(BatData.VolPack < BatData.VolTotal)
//			{
//				chg_in = 0;
//			}
//		}
//	}
//	
//	if(chg_in == 1)									// 根据充电电流判定充电器接入
//	{
//		cChgInCnt++;
//		cChgOutCnt = 0;
//		
//		if(cChgInCnt > CHG_PLUGIN_DELAY)
//		{
//			cChgInCnt = CHG_PLUGIN_DELAY;
//			if(!BatStatus.Bits.ChgPlugin)
//			{
//				BatStatus.Bits.ChgPlugin = 1;
//				
//				BatStatus.Bits.KeyOn = 1;  			// -->强制设置上电激活标识
//				
//				BatStatus.Bits.DisUV = 0;
//				BatStatus.Bits.DisDUV = 0;
//				
//				BatStatus.Bits.PowerOff = 0;
//			}
//		}
//	}
//	else
//	{
//		cChgOutCnt++;
//		cChgInCnt = 0;
//		
//		if(cChgOutCnt > CHG_PLUGOUT_DELAY)
//		{
//			cChgOutCnt = CHG_PLUGOUT_DELAY;
//			if(BatStatus.Bits.ChgPlugin)
//			{
//				if(!BatStatus.Bits.DetIn)
//				{
//					BatStatus.Bits.KeyOn = 0;
//				}
//				BatStatus.Bits.ChgPlugin = 0; 
//				BatStatus.Bits.ChgER = 0; 
//				BatStatus.Bits.ChgOC = 0;
//			}
//		}
//	}

//	if(cChgErrCnt > CHG_PLUGIN_DELAY*2)
//	{
//		BatStatus.Bits.ChgER = 1;					// --->异常充电器检测，待验证！
//	}
//}

/*****************************************************************************
 函 数 名  : ChgPlugInCheck
 功能描述  : 充电器插入、是否有效检测器检测
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void ChgPlugInCheck(void)
{ 
	static u8 cChgInCnt,vChgInCnt;
	static u8 cChgOutCnt; 
    static u16 ccchgoutcnt;
    static u16 chginerrcnt;
    static u8 ChgClearMark = 0;
	
	if(BatStatus.Bits.PowerOff == 1) //正在关机不判断
	{
		return;
	}

	if (BatStatus.Bits.ChgPlugin == 0)
	{
		cChgOutCnt = 0;
		if (/*(BatStatus.Bits.Chg==1) &&*/ (ChgCurAvg>CHG_SMALL_CUR)||BatData.cur > 1000 || ReadEcuChgPluginState() == 1) //高压包靠ECU发送充电器状态识别
		{
			if (++cChgInCnt > CHG_PLUGIN_DELAY*2)
			{
				BatStatus.Bits.ChgPlugin = 1; 
				BatStatus.Bytes.ChgProt&=0x91;
				BatStatus.Bytes.DisProt&=0x1c;
				BatStatus.Bytes.HardFault=0;
				BatStatus.Bits.PowerOff = 0;
				BatStatus.Bits.Sleep = 0;
			}
			/*识别到正在充电，则休眠,关机暂停*/
			SleepOrPowerOffPause();
		}
		else
		{
			cChgInCnt = 0;
		}
	    if(BatData.VolPack>BatData.VolStack+CHG_VOL_OFFSET )
	    {
	        if(/*BatData.VolPack<CHG_VALID_LOW||*/BatData.VolPack>CHG_VALID_HIGH)
	        {
	            vChgInCnt = 0;
	            if(chginerrcnt++>CHG_PLUGIN_DELAY*2)
	            {
	                BatStatus.Bits.ChgER = 1;
	                BatStatus.Bits.ChgPlugin = 1; /*需置位，否则会造成SleepOrPowerOffPause一直调用无法关机*/
	            }
	        }
	        else 
			{
				chginerrcnt = 0;
			}
	                   
			if(++vChgInCnt>CHG_PLUGIN_DELAY)
			{
				BatStatus.Bits.ChgPlugin = 1; 
				BatStatus.Bytes.ChgProt&=0x91;
				BatStatus.Bytes.DisProt&=0x1c;
				BatStatus.Bytes.HardFault=0;
				BatStatus.Bits.PowerOff = 0;
				BatStatus.Bits.KeyOn = 1;
				BatStatus.Bits.Active = 1;
				BatStatus.Bits.Sleep = 0;
			}
	        /*识别到正在充电，则休眠,关机暂停*/
	        SleepOrPowerOffPause();
	    }
	    else 
	    {
	        vChgInCnt = 0;
	        chginerrcnt = 0;
	    }
	}
	else
	{
		cChgInCnt = 0;
		if (/*(BatStatus.Bits.Dis==1) && */(DisCurAvg>DIS_UNLOAD_CUR)||BatData.VolPack<BatData.VolStack/2 ||((ReadEcuChgPluginState() == 0 /*|| BatStatus.Bits.DetIn_charge == 0*/) && (ChgCurAvg<CHG_SMALL_CUR) ))
		{
			if (++cChgOutCnt > CHG_PLUGOUT_DELAY*2)
			{
				BatStatus.Bits.ChgPlugin = 0; 
			}
		}
		else
		{
			cChgOutCnt = 0;
		}
		
		if(ChgCurAvg<CHG_OUT_CUR&&BatData.VolPack<BatData.VolStack+250)
		{
				if(++ccchgoutcnt>CHG_PLUGOUT_DELAY*1)
				{
					BatStatus.Bits.ChgPlugin = 0; 
				}
		}
		else
		{
				ccchgoutcnt = 0;
		}
	}
	
	if (BatStatus.Bits.ChgPlugin == 0)
	{
		ChgClearMark = 0;
	}
	else
	{
		if(ChgClearMark == 0)
		{
			if(ReadMtChgPluginState())
			{
				ChgClearMark = 0x5A;
				BatStatus.Bytes.ChgProt&=0x91;
				BatStatus.Bytes.DisProt&=0x1c;
				BatStatus.Bytes.HardFault=0;
			}
		}
	}
}
/*****************************************************************************
 函 数 名  : ChgOverVolCheck
 功能描述  : 充电过充检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void ChgOverVolCheck(void)
{
    static u8 OvUnlockCnt = 0;
    static u16 Dis1ATimer = 0;
    if (BatStatus.Bits.ChgPlugin == 0)
    {
        ChgOverVolTimer[0] = 0;
        ChgSecTimer = 0;
        ChgMinTimer = 0;
//		ChgOverVolReTimer[0] = 0;
        OvUnlockCnt = 0;
//		return;
    }

    // 电池电压过充检测
	if (BatStatus.Bits.ChgOV == 1)
	{
		ChgOverVolTimer[0] = 0;		
	}
	
	if ((BatData.VolMax < CHG_OVER_VOL) && (BatData.VolTotal < CHG_OVER_VOL_TOTAL) )//&& (AfeErr.bit.cov==0)) 
	{
		ChgOverVolTimer[0] = 0;
	}
	
    if(ChgOverVolTimer[0] > CHG_OVER_VOL_DELAY)
    {
        if(!BatStatus.Bits.ChgOV)
        {
            BatStatus.Bits.ChgOV = 1;
            ChgMinTimer = 0;		// 清除充电计时
        }
    }

    // 充电总时长大于x小时状态检测
    if(BatStatus.Bits.Chg)
    {
        // 如充电总时长大于x小时状态检测
        if(ChgSecTimer >= 6000)     // 60S
        {
            ChgSecTimer = 0;
            ChgMinTimer++;
		#if 1
            if (ChgMinTimer >= 3)
            {
            	PRODUCT_ACTIVE();
            }
		#endif
            if(ChgMinTimer >= CHG_MAX_TIME)
            {
                ChgMinTimer = CHG_MAX_TIME;
            //    BatStatus.Bits.ChgTO = 1;
                // 总电压大于4.1*nCell，视为充饱，否则视为电芯坏
				if (BatData.VolTotal >= CHG_SMALL_CUR_TOTALVOL)
				{
					BatStatus.Bits.ChgFC = 1;
				}
				else
				{
					BatStatus.Bits.ChgTO = 1;
				}
            }
        }
    }
    else
    {
    	ChgSecTimer = 0;
    }

    if(BatStatus.Bits.ChgOV || BatStatus.Bits.ChgFC)
    {
    	ChgMinTimer = 0;
    	
        // 过充后，如还接上充电器且电芯电压小于4.05V，则清除过充标识，恢复充电
        if(BatData.VolMin < BAT_MIN_VALUE)
        {
            ChgOverVolReTimer[0] = 0;
        }
        
//        if(AfeErr.bit.cov)
//		{
//            ChgOverVolReTimer[0] = 0;
//        }
        
        if(BatData.VolMax < CHG_OVER_VOL_RE)
        {
            ChgOverVolReTimer[0]++;

            if(ChgOverVolReTimer[0] > CHG_OVER_VOL_RE_DELAY)
            {
            	ChgOverVolReTimer[0] = 0;
                BatStatus.Bits.ChgOV = 0;
                BatStatus.Bits.ChgFC = 0;
                
                OvUnlockCnt++;
				if (OvUnlockCnt >= 10)
				{
					BatStatus.Bits.ChgER = 1;			// SW需求，恢复10次以上不再复充
	            }
	        }
        }
        else
        {
            ChgOverVolReTimer[0] = 0;
        }
    }
    else
    {
    	ChgOverVolReTimer[0] = 0;
    }
#if 1
	/* 电量<=98%以后，放电电流>1A 10S清过充/满充标识 */
	if(BatStatus.Bits.ChgOV || BatStatus.Bits.ChgFC)
	{
		if (BmsGetWorkMode() != BMS_WORK_DSG)
		{
			Dis1ATimer = 0;
		}
		else if(SocInfo.SocS > 98)
		{
			Dis1ATimer = 0;
		}
		else if (DisCurAvg < DIS_1A_CUR) 
		{
			Dis1ATimer = 0;
		}
		else
		{
			Dis1ATimer++;
	    	if(Dis1ATimer > DIS_1A_CUR_DELAY)
	    	{
	    		BatStatus.Bits.ChgOV = 0;
				BatStatus.Bits.ChgFC = 0;
				OvUnlockCnt = 0;
	    	}
		}
	}
	else
    {
    	Dis1ATimer = 0;
    }
#endif
}

/*****************************************************************************
 函 数 名  : ChgOverVol2Check
 功能描述  : 充电过充检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void ChgOverVol2Check(void)
{

	// 过充检测
	if (ChgOV2 == 1)
	{
		ChgOverVolTimer[1] = 0;		
	}
	else if (BatData.VolMax < CHG_OVER_VOL2)
	{
		ChgOverVolTimer[1] = 0;
	}
	else if (ChgOverVolTimer[1] > CHG_OVER_VOL2_DELAY)
	{
		BatStatus.Bits.ChgOV = 1;
		ChgOV2 = 1;
	}

	// 过充恢复
	if (ChgOV2==0) 
	{
		ChgOverVolReTimer[1] = 0;
	}
	else if (BatData.VolMax >= CHG_OVER_VOL2_RE)
	{
		ChgOverVolReTimer[1] = 0;
	}
	else if(ChgOverVolReTimer[1] > CHG_OVER_VOL2_RE_DELAY)
	{
		BatStatus.Bits.ChgOV = 0;
		ChgOV2 = 0;
	}

}


/*****************************************************************************
 函 数 名  : ChgCurCheck
 功能描述  : 充电过流检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void ChgCurCheck(void)
{
// 小电流截止触发
    if ( (ChgCurAvg > CHG_SMALL_CUR)
            || (ChgCurAvg < CHG_OUT_CUR)
            || (BatStatus.Bits.ChgPlugin == 0)
                || (BatStatus.Bits.Chg == 0)
                || (IS_FACTORY_TEST_MODE()) )
    {
            ChgSmallCurTimer = 0;
    }
	if (ChgSmallCurTimer > CHG_SMALL_CUR_DELAY)
    {
    	if (BatData.VolTotal > CHG_SMALL_CUR_TOTALVOL)
    	{
    		if(ChgCurAvg > CHG_SMALL_CUR/2)
			{	
				BatStatus.Bits.ChgFC = 1;
			}
    	}
    }

    // 充电过流
    if ((ChgCurAvg < CHG_OVER_CUR))// && (AfeErr.bit.coc==0))
    {
    	ChgOverCurTimer[0] = 0;
    }
    if (ChgOverCurTimer[0] > CHG_OVER_CUR_DELAY)
    {
        BatStatus.Bits.ChgOC = 1;
    }

    // 充电过流2
    if ((ChgCurAvg < CHG_OVER_CUR2))// && (AfeErr.bit.coc==0))
    {
    	ChgOverCurTimer[1] = 0;
    }
    if (ChgOverCurTimer[1] > CHG_OVER_CUR2_DELAY)
    {
        BatStatus.Bits.ChgOC = 1;
    }

    // MCU运放过流保护
#if 0
//#ifdef OPA_COC_EN
    ChgCurAD = ADRead(ADC_CCR_AD);
    if (ChgCurAD < ChgCurZero)
    {
    	ChgCurAD = ChgCurZero - ChgCurAD;
    }
    else
    {
    	ChgCurAD = 0;
    }
    ChgCurAvgAD = (ChgCurAvgAD*7 + ChgCurAD)/8;

    if (ChgCurAvgAD < CHG_OVER_CUR_AD)
	{
		ChgOverCurTimer[2] = 0;
	}
	if (ChgOverCurTimer[2] > CHG_OVER_CUR_AD_DELAY)
	{
		BatStatus.Bits.ChgOC = 1;
	}
#endif
}

/*****************************************************************************
 函 数 名  : ChgNtcCheck
 功能描述  : 充电过温检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void ChgNtcCheck(void)
{
#ifdef CHG_OVER_NTC_DISP
    static u16 ChgOverNtcDispMinTimer = 0;     // 充电过温假充分钟计时器
    
	if (!BatStatus.Bits.ChgPlugin)
	{
        ChgOverNtcDispSecTimer = 0;
        ChgOverNtcDispMinTimer = 0;
	}
	else	// 充电过温时，只能维持x小时的假充提示状态
	{		
	    if(BatStatus.Bits.ChgOT || BatStatus.Bits.ChgMOT)
	    {
	        if(ChgOverNtcDispSecTimer >= 6000)  // 60S
	        {
	            ChgOverNtcDispSecTimer = 0;
	            ChgOverNtcDispMinTimer++;

	            if(ChgOverNtcDispMinTimer >= CHG_OVER_NTC_DISP_MAX_TIME)
	            {
	                if(BatStatus.Bits.ChgOT)
	                	BatStatus.Bits.CellNtcErr = 1;
	                if( BatStatus.Bits.ChgMOT)
	                	BatStatus.Bits.CNtcErr = 1;
	            }
	        }
	    }
	    else
	    {
	        ChgOverNtcDispSecTimer = 0;
	        ChgOverNtcDispMinTimer = 0;
	    }
	}
#endif

    if (BatStatus.Bits.CellNtcErr)
    {
    	NtcChgTimer = 0;
    	NtcChgReTimer = 0;
    	BatStatus.Bits.ChgOT = 0;
        return;
    }

	// 充电过温检测
    if(0 == IS_CHG_OT_LOCKED())
    {
        if((BatNtcTempMax < NTC_CHG_HIGH) && (BatNtcTempMin > NTC_CHG_LOW))
        {
            NtcChgTimer = 0;
        }
    }
    else
    {
    	if((BatNtcTempMax < NTC_CHG_HIGH_RE) && (BatNtcTempMin > NTC_CHG_LOW_RE))
        {
            NtcChgTimer = 0;
        }
    }

    if(NtcChgTimer > CHG_NTC_OT_DELAY)
    {
    	NtcChgTimer = CHG_NTC_OT_DELAY;
    	
    	CHG_OT_LOCK();
    	
        if(!BatStatus.Bits.ChgOT && BatStatus.Bits.ChgPlugin)
        {
            BatStatus.Bits.ChgOT = 1;
        }
    }

    // 充电过温恢复检测
	if(BatNtcTempMax >= NTC_CHG_HIGH_RE)
	{
		NtcChgReTimer = 0;
	}

    if(BatNtcTempMin <= NTC_CHG_LOW_RE)
    {
        NtcChgReTimer = 0;
    }

    if(NtcChgReTimer > CHG_NTC_OT_RE_DELAY)
    {
    	NtcChgReTimer = CHG_NTC_OT_RE_DELAY;
    	
    	CHG_OT_UNLOCK();
    	
        if (BatStatus.Bits.ChgOT == 1)
        {
            BatStatus.Bits.ChgOT = 0;
        }
    }
}

/*****************************************************************************
 函 数 名  : ChgMosNtcCheck
 功能描述  : 充电MOS过温检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
void ChgMosNtcCheck(void)
{
	static u16 MosNtcTimer = 0;
	static u16 MosFailTimer = 0;

	if (BatStatus.Bits.CNtcErr)
		return;

	// NTC断线检测
    if ( (CmosNtcTemp>=NTC_MOS_SHORT) || ((CmosNtcTemp<=NTC_MOS_OPEN) && (BatNtcTempMin>NTC_BAT_P10)) )
    {
        if(++MosFailTimer > NTC_FAIL_TIME)
        {        	
			MosFailTimer = 0;
			MosNtcTimer = 0;
            BatStatus.Bits.CNtcErr = 1;
            BatStatus.Bits.ChgMOT = 0;
        }
    }
    else
    {
        MosFailTimer = 0;
    }
	
	if (BatStatus.Bits.ChgMOT == 0)
	{
		if (CmosNtcTemp < NTC_CMOS_HIGH)
		{
			MosNtcTimer = 0;
		}
		else if (++MosNtcTimer > MOS_NTC_OT_DELAY)
		{
			MosNtcTimer = 0;
			BatStatus.Bits.ChgMOT = 1;
		}		
	}
	else
	{
		if (CmosNtcTemp > NTC_CMOS_HIGH_RE)
		{
			MosNtcTimer = 0;
		}
		else if (++MosNtcTimer > MOS_NTC_OT_DELAY)
		{
			MosNtcTimer = 0;
			BatStatus.Bits.ChgMOT = 0;
		}
	}
}


u8 ChgErrCheck(void)
{
	if (BatStatus.Bytes.ChgProt || BatStatus.Bytes.HardFault|| 1 == ChgPreCheck()) 
        return 0;
    return 1;
}

void ChgMosAbleUpdate(void)
{
    /*有放电电流或无充电禁止或未出现充电电流，则充电mos允许开；
    如果已经允许开，则判断放电电流<0.5a且不允许充电,充电mos禁止开*/
    static u8 delaychgable,delaychgdisable,chgcurlockdelay,lastchgable;
    u8 chgable=ChgErrCheck();

    if(chgable==0)
    {/*不能充电情况下出现充电电流，则充电电流锁置位，不再主动允许cmos开,有放电电流或允许充电情况下可以清掉锁*/
        if(ChgCurAvg>CHG_SLEEP_CUR)
        {
            delaychgable = 0;
            if(chgcurlockdelay++>30)
                chgcurlock = 0x5a;
        }
        else chgcurlockdelay= 0;
    }
    else 
    {
        chgcurlock  = 0;
        chgcurlockdelay=0;
    }
    if(BatStatus.Bits.ChgAble)
    {/*从能正常充电到异常，如果有充电电流，清除一下充电允许*/
        if(chgable==0)
        {
            if(lastchgable)
            {
                if(ChgCurAvg>CHG_SLEEP_CUR)
                {
                    BatStatus.Bits.ChgAble = 0;
                    delaychgable = 0;
                    chgcurlock = 0x5a;
                }
            }
        }
        lastchgable = chgable;
    }
    /*有充电器在情况下触发了充电保护且无放电电流，关充电mos允许*/
    if(BatStatus.Bits.ChgPlugin&&DisCurAvg<DIS_UNLOAD_CUR&&(BatStatus.Bits.ChgFC||BatStatus.Bits.ChgOV))
    {
        BatStatus.Bits.ChgAble = 0;
        delaychgable = 0;
    }
    if(BatStatus.Bits.ChgAble==0||chgcurlock==0)
    {
        delaychgdisable=0;
        if( (DisCurAvg>DIS_UNLOAD_CUR) || chgable==1 || (chgcurlock==0) )
        {/*有放电电流或允许充电或未充电电流锁下允许充电MOS开*/
            if(delaychgable++>50)
            {
                BatStatus.Bits.ChgAble = 1;
                if(DisCurAvg>DIS_UNLOAD_CUR&&chgcurlock)
                    chgcurlock = 0;
            }
        }
        else delaychgable = 0;
    }
    else 
    {
        delaychgable = 0;
        if(DisCurAvg<DIS_UNLOAD_CUR&&chgable==0)
        {
            if(delaychgdisable++>80||(chgcurlock))
                BatStatus.Bits.ChgAble = 0;
        }
        else delaychgdisable = 0;
    }
}

/*****************************************************************************
 函 数 名  : ChgStopFlag
 功能描述  : 充电关闭标识检测
 输入参数  :
 返 回 值  :
*****************************************************************************/
u8 ChgStopFlag(void)
{
//	if(!BatStatus.Bits.ChgPlugin)
//	{
//		return 1;
//	}
//    if (!BatStatus.Bits.DetIn_charge)
//    {
//        return 1;
//    }

    if(BatStatus.Bytes.ChgProt || BatStatus.Bytes.HardFault)
    {
        return 1;
    }

#if 1
    if(BatStatus.Bits.DisSC || BatStatus.Bits.DisOCL || BatStatus.Bits.DisOC)
    {
    	return 1;
    }
#endif
	if(BatStatus.Bits.PowerOff)
	{
    	return 1;
	}
    if(!BatData.Valid)                                               // 如未完成全部电芯电压采样，不进行电压、电量检测
    {
        return 1;
    }

#ifdef SOC_SUPPORT
	if(SocIsOcvWait())
	{
		return 1;
	}
#endif

    return 0;
}

/*****************************************************************************
 函 数 名  : ChgPreCheck
 功能描述  : 开启充电前的过温、过压预检测
 输入参数  :
 返 回 值  :
            0: 正常 1: 过温或过压
*****************************************************************************/
u8 ChgPreCheck(void)
{	
#if 1
	if (BatStatus.Bits.Chg)
	{
		return 0;
	}
#endif
	if (1 == IS_CHG_OT_LOCKED())
	{
	    if((BatNtcTempMax >= NTC_CHG_HIGH_RE) || (BatNtcTempMin <= NTC_CHG_LOW_RE))
	    {
	        return 1;
	    }
    }
    else
    {
    	if((BatNtcTempMax >= NTC_CHG_HIGH) || (BatNtcTempMin <= NTC_CHG_LOW))
	    {
	        return 1;
	    }
    }

    if(BatData.VolMin < BAT_MIN_VALUE)
    {
        return 1;
    }

#if 0
    if(ChgStartFlag == 1)
        return 0;

	if ((BatData.VolMax>=CHG_OVER_VOL_ST) || (BatData.VolTotal>=CHG_OVER_VOL_TOTAL_ST))
	{
	    return 1;
	}
#endif
    return 0;
}


/*****************************************************************************
 函 数 名  : ChgCtrl
 功能描述  : 充电控制
 输入参数  :
 返 回 值  :
*****************************************************************************/
void ChgCtrl(void)
{
    // Tick: 10ms
    if(!TimerChgFlag)
    {
        return;
    }
    TimerChgFlag = 0;

    ChgPlugInCheck();   // 充电器插入检测、有效充电器检测
    ChgOverVolCheck();  // 充电过充检测
		//ChgOverVol2Check();
    ChgCurCheck();      // 充电过流检测
    ChgNtcCheck();      // 充电过温检测
    ChgMosNtcCheck();
    ChgBalance();
    
    ChgMosAbleUpdate();

    if (IS_FCT_FORCE_CTRL())
    {
    	return;
    }

    // 充电关闭检测
    if(ChgStopFlag())
    {
        ChgStop();
        return;
    }

    // 如电芯电压在正常范围区间，开启充电
    if(!BatStatus.Bits.Chg)
    {
        if(ChgPreCheck() == 0)
        {
            ChgStart();
        }
    }
}

