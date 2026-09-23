#include "Calib.h" 
#include "DataFlash.h"

#if 0
/*****************************************************************************
 函 数 名  : CalibChargerVol
 功能描述  : 总电压校准
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void CalibChargerVol(u8 *uartbuf) // 充电器总电压校准
{	
    CalibFlag = 1;
    u32 Temp_Cur;
    Temp_Cur = uartbuf[0];                                  // 高位
    Temp_Cur = Temp_Cur << 8;                               // 高位左移
    Temp_Cur += uartbuf[1];                                 // 低位
    Temp_Cur = Temp_Cur << 8;                               // 高位左移
    Temp_Cur += uartbuf[2];                                 // 低位
    Temp_Cur = Temp_Cur << 8;                               // 高位左移
    Temp_Cur += uartbuf[3];                                 // 低位
    DataFlashAtOnceSave.VChargeKi = (u32)Temp_Cur*DataFlashAtOnceSave.VChargeKi/ChgAvgVol;
    if(DataFlashAtOnceSave.VChargeKi<VOL_K_MIN || DataFlashAtOnceSave.VChargeKi>VOL_K_MAX)
    {
        CalibFlag = 0;
        DataFlashAtOnceSave.VChargeKi = CUR_K_DEFAULT;
    }
    AtOnceSaveDataWrite();
}
#endif

u8 CalibPackVol(u8 *uartbuf)
{
//	u8 ret;
//	u16 ad_data = 0;
		u32 vol_cal, vol_ad, vol_diff;
	
//		vol_cal = ((u32)uartbuf[0]<<24);
//		vol_cal += ((u32)uartbuf[1]<<16);
//		vol_cal += ((u32)uartbuf[2]<<8);
//		vol_cal += uartbuf[3];

		vol_cal = BatData.VolTotal;
	
		vol_ad = BatData.VolPack;
				

		if(vol_cal >= vol_ad)
		{
			vol_diff = (u32)(vol_cal - vol_ad);
		}
		else
		{
			vol_diff = (u32)(vol_ad - vol_cal);
		} 
		

		if(vol_diff > (u32)2000)
		{
			return 0;
		}
		
		DataFlashAtOnceSave.VPackKi = (u16)( ((u32)vol_cal*VOL_K_DEFAULT) / ((u32)vol_ad) );	
		
    if (DATA_VOL_K_PACK<VOL_K_MIN || DATA_VOL_K_PACK>VOL_K_MAX)
    {
        DATA_VOL_K_PACK = VOL_K_DEFAULT;
    }
		
		return 1;
	
}

/*****************************************************************************
 函 数 名  : CalibBat
 功能描述  : 上位机下发校准数据，由MCU进行电芯自校准，确定校准K值
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void CalibBat(void)
{
}

s16 ZeroCur;
void CalibZeroCurRun(s32 Current)
{
    static u8 cnt = 0;
    static s32 CurSum = 0;
    
    if(cnt++ < 8)
    {
        CurSum += Current;
    }
    else
    {
        ZeroCur = (s16)(CurSum/8);
        CurSum = 0;
        cnt = 0;
    }
}

void CalibZeroCurrent(void)
{
    CalibFlag = 1; 
    DATA_AFE_MC_BIAS = (s32)ZeroCur;
    if(DATA_AFE_MC_BIAS>CUR_ZERO_MAX || DATA_AFE_MC_BIAS<-CUR_ZERO_MAX)
    {
        CalibFlag = 0;
        DATA_AFE_MC_BIAS = (s32)CUR_ZERO_DEFAULT;
    }
    AtOnceSaveDataWrite();
}

/*****************************************************************************
 函 数 名  : AfeMainCurCalib
 功能描述  : 上位机下发校准数据，由MCU进行电流自校准，确定校准K值
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void CalibDsgCur(u8 *uartbuf)
{
    CalibFlag = 1;
    u16 Temp_Cur;
    Temp_Cur = uartbuf[0];                                  // 高位
    Temp_Cur = Temp_Cur << 8;                               // 高位左移
    Temp_Cur += uartbuf[1];                                 // 低位
    DATA_CUR_K_DSGM = (u32)Temp_Cur*DATA_CUR_K_DSGM/DisCurAvg;
    if(DATA_CUR_K_DSGM<CUR_K_MIN || DATA_CUR_K_DSGM>CUR_K_MAX)
    {
        CalibFlag = 0;
        DATA_CUR_K_DSGM = CUR_K_DEFAULT;
    }
    AtOnceSaveDataWrite();
}
 
/*****************************************************************************
 函 数 名  : CalibChgCur
 功能描述  : 上位机下发校准数据，由MCU进行电流自校准，确定校准K值
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void CalibChgCur(u8 *uartbuf)
{
    CalibFlag = 1;
    u16 Temp_Cur;
    Temp_Cur = uartbuf[0];                                  // 高位
    Temp_Cur = Temp_Cur << 8;                               // 高位左移
    Temp_Cur += uartbuf[1];                                 // 低位
    DATA_CUR_K_CHG = (u32)Temp_Cur*DATA_CUR_K_CHG/ChgCurAvg;
    if(DATA_CUR_K_CHG<CUR_K_MIN || DATA_CUR_K_CHG>CUR_K_MAX)
    {
        CalibFlag = 0;
        DATA_CUR_K_CHG = CUR_K_DEFAULT;
    }
    AtOnceSaveDataWrite();
}

void CalibCurBias(void)
{
	u8 i;
	u16 TempCur;

	ChgCurZero = 0;

	for(i=0; i<8; i++)							// 读取放电零漂电流值
	{
		TempCur = ADRead(ADC_CCR_AD);
		if((TempCur>CHG_CUR_ZERO_MAX) || (TempCur<CHG_CUR_ZERO_MIN))
		{
			TempCur = CHG_CUR_ZERO_DEFAULT;
		}
		ChgCurZero += TempCur;
		Delay1ms(2);
	}

	ChgCurZero = ChgCurZero>>3;
}

void CalibInit(void)
{
	u8 WriteFlash = 0;
	
    if (DATA_CUR_K_DSGM<CUR_K_MIN || DATA_CUR_K_DSGM>CUR_K_MAX)
    {
        DATA_CUR_K_DSGM = CUR_K_DEFAULT;
        WriteFlash = 1;
    }
        
    if (DATA_CUR_K_CHG<CUR_K_MIN || DATA_CUR_K_CHG>CUR_K_MAX)
    {
        DATA_CUR_K_CHG = CUR_K_DEFAULT;
        WriteFlash = 1;
    }

    if (DATA_AFE_MC_BIAS>CUR_ZERO_MAX || DATA_AFE_MC_BIAS<-CUR_ZERO_MAX)
    {
        DATA_AFE_MC_BIAS = CUR_ZERO_DEFAULT;
        WriteFlash = 1;
    }
		
    if (DATA_VOL_K_PACK<VOL_K_MIN || DATA_VOL_K_PACK>VOL_K_MAX)
    {
        DATA_VOL_K_PACK = VOL_K_DEFAULT;
        WriteFlash = 1;
    }

    if (WriteFlash == 1)
    {
			AtOnceSaveDataWrite();
    }

#ifdef OPA_COC_EN
    CalibCurBias();
#endif
}

