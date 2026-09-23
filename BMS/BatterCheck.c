/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : BatterCheck.c
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2017年1月6日 星期五
  最近修改   :
  功能描述   : 电芯、电量检测C源文件
  函数列表   :
  修改历史   :
  1.日    期   : 2017年1月6日 星期五
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/
#include "board.h"
//#include "User.h"
//#include "mcuhal.h"
//#include "battercheck.h"
//#ifdef AFE_MODEULE
//#include "AfeModule.h"
//#endif

static s32 OpaCurrent;

#ifdef VI_BAT
const u8 Calib_Cell_Vol_Tbl[CELL_NUM] =                        // VI线路单节电芯电压校准比例 根据实际电路修改 
{
    1, 2, 3, 4, 2, 1
};
const u8 VCELL_AD[CELL_NUM] = 
{
	ADC_AD1, ADC_AD2, ADC_AD3, ADC_AD4, ADC_AD5,ADC_AD6
};
#endif

#ifdef MCU_VDD_CHECK
#define VDD_UNCALIB_MAX_ERR		400
#define VDD_CALIB_MAX_ERR		150
#define VDD_VERIFY_CAL(_vdd)	(~(u16)(_vdd+0x0053))

static u16 VddMaxErr;
static u16 VddRef;

/*****************************************************************************
 函 数 名  : BatReadVref2Vdd
 功能描述  : 通过读取Vrefint(1.21v)的AD值反算VDD电压
 输入参数  :
 返 回 值  :
*****************************************************************************/
u16 BatReadVref2Vdd(void)
{
	u16 VddVol;
	
	VddVol = ADRead(ADC_VREF);
	VddVol = (u32)VREF_VOL * (ADBIT-1) / VddVol;		// Vrefint range:1.16-1.21-1.26V
	return VddVol;
}

/*****************************************************************************
 函 数 名  : BatVddVerify
 功能描述  : VDD电压校准参照值校正，校验值需匹配+偏差须在范围内。并设定Vdd参照
 			值和误差范围。
 输入参数  :
 返 回 值  : 1:ok, 0:fail
*****************************************************************************/
u8 BatVddVerify(void)
{
	u16 VerifyData;

	VerifyData = VDD_VERIFY_CAL(VDD_CALIB_DATA);
	
	if ( (VDD_VERIFY_DATA != VerifyData)
		|| (VDD_CALIB_DATA > (VDD_DESIGN_VOL+VDD_UNCALIB_MAX_ERR))
		|| (VDD_DESIGN_VOL > (VDD_CALIB_DATA+VDD_UNCALIB_MAX_ERR)) )
	{
		VddMaxErr = VDD_UNCALIB_MAX_ERR;
		VddRef = VDD_DESIGN_VOL;	
		return 0;
	}
	else
	{
		VddMaxErr = VDD_CALIB_MAX_ERR;
		VddRef = VDD_CALIB_DATA;
		return 1;
	}
}

/*****************************************************************************
 函 数 名  : BatVddCalib
 功能描述  : VDD电压校准操作，直接存储
 输入参数  :
 返 回 值  : 
*****************************************************************************/
void BatVddCalib(void)
{
	u16 Vdd2Vref;

	Vdd2Vref = BatReadVref2Vdd();
	VDD_CALIB_DATA = Vdd2Vref;
	VDD_VERIFY_DATA = VDD_VERIFY_CAL(Vdd2Vref);

	AtOnceSaveDataWrite();
}

/*****************************************************************************
 函 数 名  : BatVddCheck
 功能描述  : VDD电压实时监测，偏出则置错
 输入参数  :
 返 回 值  : 
*****************************************************************************/
#define VDD_CHECK_PERIOD		5		// 50ms
static void BatVddCheck(void)
{
	static u8 VddCheckPeriod = 0;
	static u8 VddErrTimer = 0;	

	if (BatStatus.Bits.VddErr)
		return;
	
	if (++VddCheckPeriod >= VDD_CHECK_PERIOD)
	{
		VddCheckPeriod = 0;
		
		BatData.Vdd = BatReadVref2Vdd();
		
		if ( ((BatData.Vdd+VddMaxErr) > VddRef)
			&& ((VddRef+VddMaxErr) > BatData.Vdd) )
		{
			VddErrTimer = 0;
		}
		else if (VddErrTimer < VDD_ERR_DELAY)
		{
			VddErrTimer += VDD_CHECK_PERIOD;
		}
		else
		{
			VddErrTimer = 0;
			BatStatus.Bits.VddErr = 1;
		}
	}
}
#endif

/*****************************************************************************
 函 数 名  : BatErrCheck
 功能描述  : 电芯故障检查
 输入参数  :
 返 回 值  :
*****************************************************************************/
void BatErrCheck(void)
{
	static uint16_t afeBatErr =0;
	if((BatData.VolMin < BAT_MIN_VALUE) && (BatData.VolMax > BAT_MAX_VALUE))//&&AfeErr.bit.cto)
	{
		if(afeBatErr++ >30)//300ms
		{
			if(!BatStatus.Bits.BatErr)
			{
				BatStatus.Bits.BatErr = 1;
			}
		}
	}
	// 有效电芯检测: 最小值、最大值
	if((BatData.VolMin > BAT_MIN_VALUE) && (BatData.VolMax < BAT_MAX_VALUE))//&&AfeErr.bit.cto==0)
	{
		BatError1Timer = 0;
	}
	if(BatError1Timer > BAT_ERROR1_DELAY)
	{
		if(!BatStatus.Bits.BatErr)
		{
			BatStatus.Bits.BatErr = 1;
		}
	}

	// 充电电芯压差0.6V检测	
	if ((BatStatus.Bits.ChgPlugin==0) || (ChgCurAvg<CHG_SMALL_CUR))
	{
		BatError2Timer = 0;
	}
	else if(BatData.VolMax < (BatData.VolMin + BAT_ERROR_VALUE))
	{
		BatError2Timer = 0;
	}

	if(BatError2Timer > BAT_ERROR2_DELAY)
	{
		if (!BatStatus.Bits.BatErr)
		{
			BatStatus.Bits.BatErr = 1;
		}
	}

#ifdef BAT_SDI_DIFF
	// 充电3.6V以上，压差0.3V检查
	if((BatStatus.Bits.ChgPlugin==0) || (ChgCurAvg<CHG_SMALL_CUR))    
    {
		BatError3Timer = 0;
    }
	else if(BatData.VolMax < SDI_DIFF_VALID_VOL)
    {
        BatError3Timer = 0;
    }
	else if(BatData.VolMax < (BatData.VolMin + SDI_DIFF_CELL_VOL))
    {
        BatError3Timer = 0;
    }

	if(BatError3Timer > BAT_ERROR3_DELAY) 
    {
		if(!BatStatus.Bits.BatErr)
        {
            BatStatus.Bits.BatErr = 1;
        }
    }
#endif
}

/*****************************************************************************
 函 数 名  : BatCellVolUpdate
 功能描述  : 电芯电压更新
 输入参数  :
 返 回 值  :
*****************************************************************************/
void BatCellVolUpdate(void)
{
    u8 i;

    if (BatData.Bat[CELL_NUM-1].Vol == 0)
    	return;

//	BatData.VolTotal = 0;
	BatData.VolMin = BatData.Bat[0].Vol;
	BatData.VolMax = BatData.Bat[0].Vol;

	for(i = 0; i < CELL_NUM; i++)
	{
//		BatData.VolTotal += BatData.Bat[i].Vol;

		if(BatData.VolMin > BatData.Bat[i].Vol)
		{
			BatData.VolMin = BatData.Bat[i].Vol;
		}

		if(BatData.Bat[i].Vol > BatData.VolMax)
		{
			BatData.VolMax = BatData.Bat[i].Vol;
		}
	}

	BatData.Valid = 1;
}

/* NTC阻值转化为AD值，须调整对应AD分辨率及分压阻值 */
#define TABLE_MAX		17 
/* K氏温度表，单位0.1K，摄氏温度 T = TempK - 2731。分别对应80、60、40、20、0、-20℃的K氏温度 */
const u16 TempKTable[TABLE_MAX] = 
{
    (120*10 + TMP_0C_01K), (110*10 + TMP_0C_01K), (100*10 + TMP_0C_01K), (90*10 + TMP_0C_01K),
	(80*10 + TMP_0C_01K),  (70*10 + TMP_0C_01K),  (60*10 + TMP_0C_01K),  (50*10 + TMP_0C_01K),
	(40*10 + TMP_0C_01K),  (30*10 + TMP_0C_01K),  (20*10 + TMP_0C_01K),  (10*10 + TMP_0C_01K),
	(0*10 + TMP_0C_01K),   (-10*10 + TMP_0C_01K), (-20*10 + TMP_0C_01K), (-30*10 + TMP_0C_01K),
	(-40*10 + TMP_0C_01K)
};

/* NTC对应阻值表，输入不同温度点下对应的阻值。分别对应80、60、40、20、0、-20℃的NTC阻值 */
const u16 AdcNtcTable[2][TABLE_MAX] = 
{   
{	BAT_NTC_P120, BAT_NTC_P110, BAT_NTC_P100, BAT_NTC_P90,
	BAT_NTC_P80,  BAT_NTC_P70,  BAT_NTC_P60,  BAT_NTC_P50,
	BAT_NTC_P40,  BAT_NTC_P30,  BAT_NTC_P20,  BAT_NTC_P10,
	BAT_NTC_P0,   BAT_NTC_N10,  BAT_NTC_N20,  BAT_NTC_N30,
	BAT_NTC_N40   }, 
{   MOS_NTC_P120, MOS_NTC_P110, MOS_NTC_P100, MOS_NTC_P90,
	MOS_NTC_P80,  MOS_NTC_P70,  MOS_NTC_P60,  MOS_NTC_P50,
	MOS_NTC_P40,  MOS_NTC_P30,  MOS_NTC_P20,  MOS_NTC_P10,
	MOS_NTC_P0,   MOS_NTC_N10,  MOS_NTC_N20,  MOS_NTC_N30,
	MOS_NTC_N40	},
};

//const u8 ADC_NTC_TABLE[ADC_NTC_NUM] = {ADC_CELL_NTC1,ADC_CELL_NTC2,ADC_DMOS_NTC,ADC_CMOS_NTC,ADC_DIODE_NTC};

/*****************************************************************************
 函 数 名  : AdcNtcToTempK
 功能描述  : NTC温度换算函数，查表法
 输入参数  : 
 			u16 AdcNtc: NTC的AD值 
 返 回 值  : 
 			u16，K氏温度返回值，0.1K
*****************************************************************************/
u16 AdcNtcToTempK(u8 NtcType, u16 AdcNtc)
{
	u8 i;
	u16 ad_diff, temp_dif, temp_k;
    if(NtcType>1)
        NtcType = 0;

	if (AdcNtc <= AdcNtcTable[NtcType][0])
		temp_k = TempKTable[0];	
	else
	{
		for (i = 1; i < TABLE_MAX; i++)
		{
			if(AdcNtc <= AdcNtcTable[NtcType][i])
				break;
		}

		if (i == TABLE_MAX)
			temp_k = TempKTable[TABLE_MAX-1];
		else
		{
			ad_diff = AdcNtc - AdcNtcTable[NtcType][i-1];
			temp_dif = (u16)((u32)ad_diff*(u32)(TempKTable[i-1] - TempKTable[i])/(u32)(AdcNtcTable[NtcType][i] - AdcNtcTable[NtcType][i-1]));
			temp_k = (TempKTable[i-1] - temp_dif);
		}
	}

	return temp_k;
}

void BatTempUpdate(void)
{
	u16 NtcAD;
	u8 i;

	/* MCU BAT NTC 适配调整处 */
	NtcAD = ADRead(ADC_CELL_NTC1);
	BatNtcTemp[0] = AdcNtcToTempK(0, NtcAD);
	
	/* MCU MOS NTC 适配调整处 */
	NtcAD = ADRead(ADC_CMOS_NTC);
	MosNtcTemp[0] = AdcNtcToTempK(1, NtcAD);
	NtcAD = ADRead(ADC_DMOS_NTC);
	MosNtcTemp[1] = AdcNtcToTempK(1, NtcAD);

	BatNtcTempMax = BatNtcTemp[0];
	BatNtcTempMin = BatNtcTemp[0];
	for (i=1; i<BAT_NTC_NUM; i++)
	{
		if (BatNtcTempMax < BatNtcTemp[i])
		{
			BatNtcTempMax = BatNtcTemp[i];
		}
		if (BatNtcTempMin > BatNtcTemp[i])
		{
			BatNtcTempMin = BatNtcTemp[i];
		}
	}
	MosNtcTempMax = MosNtcTemp[0];
	for (i=1; i<MOS_NTC_NUM; i++)
	{
		if (MosNtcTempMax < MosNtcTemp[i])
		{
			MosNtcTempMax = MosNtcTemp[i];
		}
	}
	CmosNtcTemp = MosNtcTemp[0];
	DmosNtcTemp = MosNtcTemp[1];
	
	
	
#if 0

	BatNtcTemp[0] = 2731+220;
	BatNtcTemp[1] = 2731+220;
	BatNtcTemp[2] = 2731+220;
	MosNtcTemp[0] = 2731+220; 
	MosNtcTemp[1] = 2731+220; 
	
	BatNtcTempMax = 2731+220;
	BatNtcTempMin = 2731+220;
	
	MosNtcTempMax = 2731+220;

	CmosNtcTemp = 2731+220;
	
	DmosNtcTemp = 2731+220;

#endif
}
void PackVolUpdate(void)
{
	u16 PackVolAD;
	
	PackVolAD = ADRead(ADC_BAT_VOL);
	PackAvgVol = (u32)PackVolAD * VDD_DESIGN_VOL / ADBIT * PACK_VOL_GAIN; 	// AD值换算为电压值 AD/4096*3300*50/3
}

void BatOpaCurUpdate(void)
{
    #define FILTERNUM 8
    
	static u16 CurFilter[FILTERNUM] = {0};
    static u8 filtercount = 0;
    u8 i;
	u16 AdData;
	s32 CurData;
    u32 ab;
	if(CurFilter[0]==0) 
    {
        for(i=0;i<FILTERNUM;i++)
            CurFilter[i] = ChgCurZero; 
        filtercount = 0;
    }
	
    AdData = ADRead(ADC_CCR_AD);
    CurFilter[filtercount] = AdData;
    if(++filtercount >= FILTERNUM)/*需重点关注，很容易溢出*/
    {
        filtercount=0;
    }
    ab = 0;
    for(i=0;i<FILTERNUM;i++)
        ab +=CurFilter[i];
    CurData = (s32)(ChgCurZero - ab/FILTERNUM);
    OpaCurrent = (s32)CurData*OPA_CUR_GAIN;
}


s32 BatOpaCurGet(void)
{
	return OpaCurrent;
}
/*****************************************************************************
 函 数 名  : BatCheck
 功能描述  : 电芯检测
            电压、电量取值
 输入参数  :
 返 回 值  :
*****************************************************************************/
void BatCheck(void)
{
    if (!TimerBatFlag)                                                   // 10ms TickS
		return;
    TimerBatFlag = 0;    

    if (BatData.Valid == 0)
    {
    	BatError1Timer = 0;
    	BatError2Timer = 0;
    	BatError3Timer = 0;
    }

	PackVolUpdate();

	BatCellVolUpdate();

	BatTempUpdate();

	BatErrCheck();
	
#ifdef MCU_VDD_CHECK	
	BatVddCheck();
#endif
}

