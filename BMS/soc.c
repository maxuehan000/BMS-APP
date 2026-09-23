/******************************************************************************

                  版权所有 (C), 2001-2019, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : soc.c
  版 本 号   : 初稿
  作    者   : hbquan
  生成日期   : 2019年4月16日 星期二
  最近修改   :
  功能描述   : SOC模块源文件实现
  函数列表   :

  修改历史   :
  1.日    期   : 2019年4月16日 星期二
    作    者   : hbquan
    修改内容   : 创建文件

******************************************************************************/
#include "soc.h"
#include "DataFlash.h"
#include "BmsCtrl.h"
static s32 SocCur = 0;


#ifdef SOC_SUPPORT

/* 按开路电压曲线折线拐点取，分段线性化处理 */
#if (BAT_TYPE==BAT_SDI_35E)
#define OCV_NUM     21
const SOC_2_OCV SocOcvTbl[OCV_NUM] =
{
	{1, 3092},  {5, 3236},  {10, 3365}, {15, 3442}, {20, 3479},
	{25, 3537}, {30, 3587}, {35, 3629}, {40, 3664}, {45, 3703},
	{50, 3745}, {55, 3790}, {60, 3836}, {65, 3887}, {70, 3926},
	{75, 3975}, {80, 4032}, {85, 4069}, {90, 4081}, {95, 4105}, 
	{100, 4165}
};
const SOC_2_OCV SocOcvRealTbl[5][10] =
{
	{ {1,2750}, {4,2902}, {8,3046}, {12,3161}, {15,3224}, {85,4088},{88,4108},{93,4126},{97,4144},{100,4170} },	  //0-5A
	{ {1,2750}, {4,2858}, {8,2985}, {12,3086}, {15,3138}, {85,4088},{88,4108},{93,4126},{97,4144},{100,4170} },	  //5-10A
	{ {1,2750}, {4,2845}, {8,2950}, {12,3023}, {15,3063}, {85,4088},{88,4108},{93,4126},{97,4144},{100,4170} },	  //10-15A
	{ {1,2750}, {4,2812}, {8,2897}, {12,2961}, {15,2992}, {85,4088},{88,4108},{93,4126},{97,4144},{100,4170} },	  //15-20A
};

#elif (BAT_TYPE==BAT_SDI_50E)
#define OCV_NUM     20
const SOC_2_OCV SocOcvTbl[OCV_NUM] =
{
	{1, 3107},  {3, 3177},  {8, 3345},  {13, 3438},	{18, 3478},
	{24, 3541}, {29, 3594}, {35, 3634}, {40, 3669},	{46, 3711},
	{51, 3759}, {57, 3809}, {62, 3862}, {67, 3908},	{73, 3952},
	{78, 4013}, {84, 4066}, {89, 4082}, {95, 4104}, {100, 4165}
};

const SOC_2_OCV SocOcvRealTbl[4][10] =
{
	{ {1, 2750}, {4, 2948}, {8, 3065}, {12, 3164}, {15, 3211}, {85, 4086}, {88, 4102}, {93, 4120}, {97, 4148}, {100, 4170}},  //0-5A
    { {1, 2750}, {4, 2910}, {8, 3012}, {12, 3113}, {15, 3183}, {85, 4086}, {88, 4102}, {93, 4120}, {97, 4148}, {100, 4170}},  //5-10A
    { {1, 2750}, {4, 2857}, {8, 2970}, {12, 3066}, {15, 3133}, {85, 4086}, {88, 4102}, {93, 4120}, {97, 4148}, {100, 4170}},  //10-15A
    { {1, 2750}, {4, 2825}, {8, 2937}, {12, 3032}, {15, 3090}, {85, 4086}, {88, 4102}, {93, 4120}, {97, 4148}, {100, 4170}},  //15-20A
};
#elif (BAT_TYPE==BAT_DCM_26E)
#define OCV_NUM     13
const SOC_2_OCV SocOcvTbl[OCV_NUM] =
{
    {0, 3038},  {5, 3252},  {10, 3384},  {16, 3436}, {22, 3501},     // 5
    {35,3602},  {48,3668},  {57, 3733},  {67, 3842}, {81, 3946},	 // 10
    {92,4045},  {96,4071},  {100, 4090}
};
const SOC_2_OCV SocOcvRealTbl[4][10] =
{
	{ {0,2800},{4,3050},{8,3228},{12,3306},{15,3341} ,{85,4062},{89,4099},{94,4130},{98,4150},{100,4180}},	  //5A
	{ {0,2800},{4,2957},{8,3091},{12,3164},{15,3205} ,{85,4062},{89,4099},{93,4130},{97,4152},{100,4170}},	  //10A
	{ {0,2800},{4,2876},{8,3003},{12,3082},{15,3124} ,{85,4062},{89,4099},{93,4130},{97,4152},{100,4170}},	  //15A
	{ {0,2800},{4,2820},{8,2890},{12,2950},{15,3040} ,{85,4062},{89,4099},{93,4130},{97,4152},{100,4170}},	  //20A

};

#elif (BAT_TYPE==BAT_SDI_50P)
#define OCV_NUM     13
const SOC_2_OCV SocOcvTbl[OCV_NUM] =
{
	{0, 3078}, {5, 3238},  {11, 3392}, {18, 3456}, {23, 3513},
	{36, 3621},{46, 3683}, {57, 3780}, {62, 3842}, {75, 3937},
	{89, 4070},{94, 4090}, {100, 4126}
};
const SOC_2_OCV SocOcvRealTbl[8][10] =
{
	{ {0,2600},{4,2812},{8,2943},{12,3053},{15,3123}, {85,4100},{89,4119},{93,4136},{97,4152},{100,4165}},	//2A
	{ {0,2600},{4,2806},{8,2938},{12,3050},{15,3122}, {85,4100},{89,4119},{93,4136},{97,4152},{100,4165}},	//5A
	{ {0,2600},{4,2800},{8,2935},{12,3047},{15,3121}, {85,4100},{89,4119},{93,4136},{97,4152},{100,4165}},	//10A
	{ {0,2600},{4,2794},{8,2933},{12,3046},{15,3120}, {85,4095},{89,4113},{93,4126},{97,4148},{100,4165}},	//15A
	{ {0,2600},{4,2792},{8,2930},{12,3041},{15,3119}, {85,4095},{89,4113},{93,4126},{97,4148},{100,4165}},	//20A
	{ {0,2600},{4,2790},{8,2930},{12,3040},{15,3118}, {85,4095},{89,4113},{93,4126},{97,4148},{100,4165}},	//25A
	{ {0,2600},{4,2780},{8,2920},{12,3035},{15,3118}, {85,4095},{89,4113},{93,4126},{97,4148},{100,4165}},	//30A
	{ {0,2600},{4,2760},{8,2888},{12,3000},{15,3080}, {85,4095},{89,4113},{93,4126},{97,4148},{100,4165}},	//40A
};

#elif (BAT_TYPE==BAT_EVE_33V)
#define OCV_NUM     10
const SOC_2_OCV SocOcvTbl[OCV_NUM] =
{
    {0, 3002},  {6, 3215},  {11, 3354}, {16, 3452}, {27, 3556},     // 5
    {43, 3667}, {58, 3801}, {84, 4052}, {94, 4104}, {100, 4180}     // 10
};
const SOC_2_OCV SocOcvRealTbl[5][10] =
{
    { {0, 2750}, {5, 3022}, {8, 3200}, {12, 3317}, {15, 3371}, {85, 4053}, {89, 4085}, {93, 4123}, {97, 4152+5}, {100, 4170+10}},  //5A
    { {0, 2750}, {5, 2934}, {8, 3094}, {12, 3208}, {15, 3263}, {85, 4053}, {89, 4085}, {93, 4123}, {97, 4152}, {100, 4170}},  //10A
    { {0, 2750}, {5, 2905}, {8, 3034}, {12, 3100}, {15, 3146}, {85, 4053}, {89, 4085}, {93, 4123}, {97, 4152}, {100, 4170} }, //15A
    { {0, 2750}, {5, 2879}, {8, 3004}, {12, 3085}, {15, 3121}, {85, 4053}, {89, 4085}, {93, 4123}, {97, 4152}, {100, 4170}},  //20A
    { {0, 2750}, {5, 2868}, {8, 2974}, {12, 3038}, {15, 3063}, {85, 4053}, {89, 4085}, {93, 4123}, {97, 4152}, {100, 4170} }, //25A
};
#else
#error "no suitable battery type for SOC"
#endif


/*****************************************************************************
 函 数 名  : SocAlertInt
 功能描述  : BQ系列AFE的ALERT引脚中断触发，用于SOC计算的时间、电流信息输入
 输入参数  :
 返 回 值  :
*****************************************************************************/
void SocAlertInt(void)
{
    /* 中断进行时间更新，严格保证时间片精度 */
#if defined(AFE_MODEULE) && defined(BQ76930)
    SocDeltaTime = SocBaseTimer;
    SocBaseTimer = 0;
#endif
}

u8 SocOcvLookup(void)
{
	u8 i, soc;
    u16 delta_vol;
	
	if(BatData.VolMin <= SocOcvTbl[0].Vol)					/* 防止超范围 欠压SOC=0 */
	{
		soc = 0;
	}
	else if(BatData.VolMax >= SocOcvTbl[OCV_NUM - 1].Vol)	/* 防止超范围 过压SOC=100 */
	{
		soc = 100;
	}
	else
	{
		/* 查表根据OCV 表取soc，NOTE: 采用最低电压的一节电池来比较 */
		for(i = 1; i < OCV_NUM; i++)
		{
			if(BatData.VolMin == SocOcvTbl[i].Vol)
			{
				soc = SocOcvTbl[i].Soc;
				break;
			}
			else if((BatData.VolMin > SocOcvTbl[i - 1].Vol) && (BatData.VolMin < SocOcvTbl[i].Vol))
			{
				delta_vol = (u16)(SocOcvTbl[i].Soc - SocOcvTbl[i - 1].Soc) * 1000 / (SocOcvTbl[i].Vol - SocOcvTbl[i - 1].Vol);
				soc = SocOcvTbl[i - 1].Soc + (u8)((u32)(BatData.VolMin - SocOcvTbl[i - 1].Vol) * delta_vol / 1000);
				break;
			}
		}
	}

	return soc;
}

/*****************************************************************************
 函 数 名  : SocOcv
 功能描述  : 基于OCV表，进行SOC重置
 输入参数  : 0:initial ocv, 1:static ocv
 返 回 值  :
            0: 重置成功     1: 重置失败
*****************************************************************************/
u8 SocOcvSet(u8 set)
{
    u8 soc;

    if(!BatData.Valid)
        return 0;

    soc = SocOcvLookup();

	DataFlashPowerDownSave.Soc.Soc = soc;
	DataFlashPowerDownSave.Soc.Rmc = ((u32)DataFlashPowerDownSave.Soc.Fcc * soc) / 100;
	DataFlashPowerDownSave.Soc.SocImAms = 0;
	
	if (set == OCV_INIT_SET)
	{
	    DataFlashPowerDownSave.Soc.SocS = soc;
	    DataFlashPowerDownSave.Soc.RmcDisp = DataFlashPowerDownSave.Soc.Rmc;	    
	    DataFlashPowerDownSave.Soc.SocImAmsDisp = 0;

	//    SocCtr.OcvFlag = 0;
	}

    return 1;
}

void SocUpdateByCap(u16 DsgmAh)
{
	if (DsgmAh > DataFlashPowerDownSave.Soc.Fcc)
	{
		return;
	}

	if (DsgmAh < DataFlashPowerDownSave.Soc.Rmc)
	{
		DataFlashPowerDownSave.Soc.Rmc -= DsgmAh;
	}
	if (DsgmAh < DataFlashPowerDownSave.Soc.RmcDisp)
	{
		DataFlashPowerDownSave.Soc.RmcDisp -= DsgmAh;
	}
/*
	else if (BatData.VolMin < DIS_UNDER_VOL+100)
	{
		DataFlashPowerDownSave.Soc.Rmc = 0;
	}	
	else
	{
		SocOcvSet(OCV_INIT_SET);
	}
*/

//	SocUpdate();
}

#define CYCLE_SOH_NUM			5
const u8 Cycle2Soh[CYCLE_SOH_NUM] = 
{
	5, 6, 7, 10, 12				// 100-200-300-400-500cycle SOH decent
};

static void SocSohCaculte(void)
{
	u8 n, index, calnum, soh;

	index = DataFlashPowerDownSave.Soc.Cycle/100;
	calnum = DataFlashPowerDownSave.Soc.Cycle - (u16)100*index;

	if (index >= CYCLE_SOH_NUM)
	{
		soh = 60;
	}
	else 
	{
		soh = 100;
		
		for (n=0; n<index; n++)
		{
			soh -= Cycle2Soh[n];
		}
		soh -= (u16)calnum*Cycle2Soh[n]/100;
	}
	
	DataFlashPowerDownSave.Soc.Soh = soh;
}

/*****************************************************************************
 函 数 名  : SocSohUpdate
 功能描述  : SOH更新
 输入参数  :
 返 回 值  :
*****************************************************************************/
static void SocSohUpdate(void)
{
    if(DataFlashPowerDownSave.Soc.DisCap > CYCLE_DSG_CAPACITY)
    {
        DataFlashPowerDownSave.Soc.Cycle++;
        DataFlashPowerDownSave.Soc.DisCap -= CYCLE_DSG_CAPACITY;

        SocSohCaculte();
    }
}

/*****************************************************************************
 函 数 名  : SocFrocUpdate
 功能描述  : SOC冻结容量更新
 输入参数  :
 返 回 值  :
*****************************************************************************/
#ifdef USE_FROZEN_CAPACITY
static void SocFrocUpdate(void)
{
    s16 DeltaRmc;
    s16 FroCap;

    /* RMC range limit */
    if((DataFlashPowerDownSave.Soc.Rmc > (CAPACITY_DESIGN + FCC_UPDATE_MAX))
       || (DataFlashPowerDownSave.Soc.Rmc < (DataFlashPowerDownSave.Soc.Fcc - RMC_FCC_DELTA))
       || (DataFlashPowerDownSave.Soc.Rmc > (DataFlashPowerDownSave.Soc.Fcc + RMC_FCC_DELTA)))
    {
        return;
    }

    /* frozen capacity update limit */
    DeltaRmc = (s16)DataFlashPowerDownSave.Soc.Fcc - (s16)DataFlashPowerDownSave.Soc.Rmc;

    if(DeltaRmc > (s16)FCC_UPDATE_MAX)
    {
        DeltaRmc = FCC_UPDATE_MAX;
    }
    else if(-DeltaRmc > (s16)FCC_UPDATE_MAX)
    {
        DeltaRmc = -(s16)FCC_UPDATE_MAX;
    }

    /* frozen capacity update filter */
    FroCap = (s16)DataFlashPowerDownSave.Soc.Froc + DeltaRmc;

    if(FroCap < 0)
    {
        FroCap = 0;
    }

    if(DataFlashPowerDownSave.Soc.Cycle < 3)
    {
        DataFlashPowerDownSave.Soc.Froc = ((u16)FroCap + DataFlashPowerDownSave.Soc.Froc) / 2;
    }
    else
    {
        DataFlashPowerDownSave.Soc.Froc = ((u16)FroCap + DataFlashPowerDownSave.Soc.Froc * 3) / 4;
    }

    /* frozen capacity limit */
    if(DataFlashPowerDownSave.Soc.Froc > MAX_FROZEN_CAPACITY)
    {
        DataFlashPowerDownSave.Soc.Froc = MAX_FROZEN_CAPACITY;
    }

    DataFlashPowerDownSave.Soc.Fcc = CAPACITY_DESIGN - DataFlashPowerDownSave.Soc.Froc;
    SocCtr.CapHalfP = DataFlashPowerDownSave.Soc.Fcc / 200;
    SocCtr.CapOneP = DataFlashPowerDownSave.Soc.Fcc / 100;
    SocCtr.CapDoubleP = DataFlashPowerDownSave.Soc.Fcc / 50;
}
#endif

/*****************************************************************************
 函 数 名  : SocUpdate
 功能描述  : SOC更新: 依据剩余容量RMC，以百分比形式展示
 输入参数  :
 返 回 值  :
*****************************************************************************/
static void SocUpdate(void)
{
    u16 SocDisp;

    DataFlashPowerDownSave.Soc.Soc = ((u32)DataFlashPowerDownSave.Soc.Rmc * 100 / (u32)DataFlashPowerDownSave.Soc.Fcc);

    if(DataFlashPowerDownSave.Soc.Soc > 100)
    {
        DataFlashPowerDownSave.Soc.Soc = 100;
    }

    SocDisp = ((u32)DataFlashPowerDownSave.Soc.RmcDisp * 1000 / (u32)DataFlashPowerDownSave.Soc.Fcc);
	DataFlashPowerDownSave.Soc.SocS = SocDisp / 10;
    if((SocDisp % 10) >= 5)
    {
        DataFlashPowerDownSave.Soc.SocS += 1;
    }

    if(DataFlashPowerDownSave.Soc.SocS > 100)
    {
        DataFlashPowerDownSave.Soc.SocS = 100;
        DataFlashPowerDownSave.Soc.RmcDisp = DataFlashPowerDownSave.Soc.Fcc;
    }

    if (BatStatus.Bits.ChgPlugin)
	{
	    if (BatStatus.Bits.ChgOV || BatStatus.Bits.ChgFC)
	    {
	    	if(DataFlashPowerDownSave.Soc.SocS < 100)		// force update socs while OV calibration fail(small cur...)
	        {
	        	DataFlashPowerDownSave.Soc.SocS = 100;
	        	DataFlashPowerDownSave.Soc.RmcDisp = DataFlashPowerDownSave.Soc.Fcc;            
	        }
	    }
	    else if(IS_CHG_STATE())
	    {
	        if(DataFlashPowerDownSave.Soc.SocS > 99)
	        {
	            DataFlashPowerDownSave.Soc.SocS = 99;
	            DataFlashPowerDownSave.Soc.RmcDisp = DataFlashPowerDownSave.Soc.Fcc - (DataFlashPowerDownSave.Soc.Fcc>>7);
	        }
	    }
    }

    if (BatStatus.Bits.DisUV)
	{
		if (BatStatus.Bits.ChgPlugin == 0)
		{
			if (DataFlashPowerDownSave.Soc.SocS > 0)
			{
				DataFlashPowerDownSave.Soc.SocS = 0;		// force update socs while UV calibration fail(...)
				DataFlashPowerDownSave.Soc.RmcDisp = 0;
			}
		}
	}
//    else if (DataFlashPowerDownSave.Soc.RmcDisp<DataFlashPowerDownSave.Soc.Fcc>>7||DataFlashPowerDownSave.Soc.SocS == 0)
//    {
//        DataFlashPowerDownSave.Soc.SocS = 1;
//        DataFlashPowerDownSave.Soc.RmcDisp = DataFlashPowerDownSave.Soc.Fcc>>7;
//    }
}

/*****************************************************************************
 函 数 名  : SocInit
 功能描述  : Soc模块初始化，对应变量初始值
 输入参数  :
 返 回 值  :
*****************************************************************************/
void SocInit(void)
{
    SocBaseTimer = 0;
    Dynamic1mAh = MAMS_TO_MAH;

    SocCtr.OvFlag = 0;
    SocCtr.FcFlag = 0;
    SocCtr.OcvReset = 0;
    SocCtr.OcvWait = 0;

    /* 此处以FCC数值范围作为判断数据是否存放，建议可使用校验等方式处理 */
    if((DataFlashPowerDownSave.Soc.Fcc < CAPACITY_ERR_MIN) || (DataFlashPowerDownSave.Soc.Fcc > CAPACITY_ERR_MAX))
    {
        /* no valid data, use default configuration */
#ifdef USE_FROZEN_CAPACITY
        DataFlashPowerDownSave.Soc.Froc = DEFAULT_FROZEN_CAPACITY;
#else
        DataFlashPowerDownSave.Soc.Froc = 0;
#endif
        DataFlashPowerDownSave.Soc.Fcc = CAPACITY_TYPE - DataFlashPowerDownSave.Soc.Froc;
        DataFlashPowerDownSave.Soc.Ocv = 0;
        DataFlashPowerDownSave.Soc.Soh = 100;
        DataFlashPowerDownSave.Soc.Cycle = 0;
        DataFlashPowerDownSave.Soc.DisCap = 0;
    }

    if(DataFlashPowerDownSave.Soc.Ocv != SOC_OCV_VERIFY)
    {
        SocCtr.OcvWait = 1;
		SocCtr.InitDelay = SOC_INIT_DELAY;
    }
    else
    {
        SocCtr.OcvWait = 0;
        SocCtr.InitDelay = SOC_INIT_DELAY>>2;
    }

    if(DataFlashPowerDownSave.Soc.SocImAms >= MAMS_TO_MAH*2)
    {
        DataFlashPowerDownSave.Soc.SocImAms = 0;
    }

    if (DataFlashPowerDownSave.Soc.Cycle > 1000)
    {
    	DataFlashPowerDownSave.Soc.Cycle = 0;
    }

    if (DataFlashPowerDownSave.Soc.DisCap > CYCLE_DSG_CAPACITY*2)
    {
    	DataFlashPowerDownSave.Soc.DisCap = 0;
    	DataFlashPowerDownSave.Soc.Cycle = 0;
    }
}

/*****************************************************************************
 函 数 名  : SocTimeUpdate
 功能描述  : SOC库仑计时间更新
 输入参数  :
            flagTimeout: 时间更新超时标识
 返 回 值  :
*****************************************************************************/
void SocTimeUpdate(u8 flagTimeout)
{
    SocUpdateFlag = 1;

    /* BQ系列如无ALERT信号触发，进行超时检电流机制，安时积分时间调整为超时时长 */
    if(flagTimeout)
    {
//        SocDeltaTime += SocBaseTimer;
//        SocBaseTimer = 0;
    }
}

/*****************************************************************************
 函 数 名  : SocAAHIntegrate
 功能描述  : mA电流积分，用于正常工作模式
 输入参数  : DeltaTime时间间隔，单位ms
             Current工作电流，单位mA
 返 回 值  :
*****************************************************************************/
static void SocIntegrate(u16 deltaTime, s32 cur)
{
    static s32 LastCur = 0;
    s32 InterCur;
    u32 temp;
    u32 TimeMs;

    if(deltaTime > SOC_CUR_TIME_MAX)
    {
        TimeMs = SOC_CUR_TIME_MAX;
    }
    else
    {
        TimeMs = deltaTime;
    }

    /* 无效电流不处理 */
    if((cur > SOC_CHG_CUR_MAX) || (cur < SOC_DIS_CUR_MAX))
    {
        InterCur = LastCur;
    }
    else
    {
        LastCur = cur;
        InterCur = cur;
    }

    if(InterCur > 0)
    {
        /* 有充电电流 正向值 */
        temp = (u32)(InterCur * TimeMs);
        DataFlashPowerDownSave.Soc.SocImAms += temp;                                /* 电量累加 */

        if(DataFlashPowerDownSave.Soc.SocImAms > MAMS_TO_MAH)
        {
            if(DataFlashPowerDownSave.Soc.Rmc < CAPACITY_DESIGN)
            {
                DataFlashPowerDownSave.Soc.Rmc += 1;                                /* 充入1mAH容量 */
            }

            DataFlashPowerDownSave.Soc.SocImAms -= MAMS_TO_MAH;                     /* 计算尾数 */
        }

#ifdef SOC_1MAH_SMOOTH
        DataFlashPowerDownSave.Soc.SocImAmsDisp += temp;                                // 电量累加

        if(DataFlashPowerDownSave.Soc.SocImAmsDisp > Dynamic1mAh)
        {
            if(DataFlashPowerDownSave.Soc.RmcDisp < (DataFlashPowerDownSave.Soc.Fcc - (DataFlashPowerDownSave.Soc.Fcc >> 7)))
            {
                DataFlashPowerDownSave.Soc.RmcDisp += 1;                                // 充入1mAH容量
            }
            DataFlashPowerDownSave.Soc.SocImAmsDisp -= Dynamic1mAh;                  // 计算尾数
        }
#endif
    }
    else
    {
        /* 放电电流是负值 */
        temp = ((u32)(-InterCur) * TimeMs);

        while(DataFlashPowerDownSave.Soc.Rmc > 0)
        {
            if(DataFlashPowerDownSave.Soc.SocImAms >= temp)
            {
                DataFlashPowerDownSave.Soc.SocImAms -= temp;            /* 电量递减 */
                break;
            }
            else
            {
                DataFlashPowerDownSave.Soc.Rmc -= 1;        /* 放电1mAH容量 */
                DataFlashPowerDownSave.Soc.SocImAms += MAMS_TO_MAH;
                DataFlashPowerDownSave.Soc.DisCap++;
            }
        }

#ifdef SOC_1MAH_SMOOTH
        while(DataFlashPowerDownSave.Soc.RmcDisp > 0)
        {
            if(DataFlashPowerDownSave.Soc.SocImAmsDisp >= temp)
            {
                DataFlashPowerDownSave.Soc.SocImAmsDisp -= temp;            /* 电量递减 */
                break;
            }
            else
            {
                DataFlashPowerDownSave.Soc.RmcDisp -= 1;        /* 放电1mAH容量 */
                DataFlashPowerDownSave.Soc.SocImAmsDisp += Dynamic1mAh;
            }
        }
#endif
    }
}

#define CYCLE_DSG_HIGH_RATE
static u16 SocCycle2Fcc(void)
{
	/* 循环次数衰减数据 cycle:   0     100   200   300   400   500   */ 
#ifdef CYCLE_DSG_HIGH_RATE
	const u16 Cycle2SohTbl[6] = {1000, 950,  850,  750,  650,  600};      // 吸尘器产品: 0.5C充电, 5-8C放电
#else
	const u16 Cycle2SohTbl[6] = {1000, 970,  940,  920,  900,  880};      // 洗地机产品: 0.5C充电, 2-3C放电
#endif
	#define  CYCLE_INDEX   100
	#define  CYCLE_MAX     (CYCLE_INDEX*(sizeof(Cycle2SohTbl)/sizeof(Cycle2SohTbl[0])-1))
	u16 Soh1000;
	u8 Index;
	
	if (DataFlashPowerDownSave.Soc.Cycle >= CYCLE_MAX)
	{
		Soh1000 = Cycle2SohTbl[CYCLE_MAX/CYCLE_INDEX];
	}
	else
	{
		Index = DataFlashPowerDownSave.Soc.Cycle/CYCLE_INDEX;

		Soh1000 = Cycle2SohTbl[Index] -
			      ((u32)(Cycle2SohTbl[Index]-Cycle2SohTbl[Index+1])
		          * (DataFlashPowerDownSave.Soc.Cycle -(u16)CYCLE_INDEX*Index)
		          / CYCLE_INDEX);		
	}

	return ((u32)CAPACITY_TYPE*(Soh1000/10)/100);       // (Soh1000/10)先执行防数据溢出
}


static void SocOvFccUpdate(void)
{
	u16 CapMin, CapMax, CapAge;

	CapAge = SocCycle2Fcc();
	CapMin = CapAge - CAPACITY_TYPE/20;      // 衰减曲线-5%
	CapMax = CapAge + CAPACITY_TYPE/20;      // 衰减曲线+5%

	/* 过充RMC在衰减曲线±5%内，依据RMC进行3%限幅更新 */
    if((CapMin <= DataFlashPowerDownSave.Soc.Rmc) 
		&& (CapMax >= DataFlashPowerDownSave.Soc.Rmc))
    {
        if((DataFlashPowerDownSave.Soc.Fcc + FCC_UPDATE_MAX) < DataFlashPowerDownSave.Soc.Rmc)
        {
            DataFlashPowerDownSave.Soc.Fcc += FCC_UPDATE_MAX;
        }
        else if((DataFlashPowerDownSave.Soc.Fcc - FCC_UPDATE_MAX) > DataFlashPowerDownSave.Soc.Rmc)
        {
            DataFlashPowerDownSave.Soc.Fcc -= FCC_UPDATE_MAX;
        }
        else
        {
            DataFlashPowerDownSave.Soc.Fcc = DataFlashPowerDownSave.Soc.Rmc;
        }
    }
	/* 过充RMC不在在衰减曲线±5%内，依据CapType进行3%限幅更新 */
	else 
	{
		if (DataFlashPowerDownSave.Soc.Fcc < CapMin)
	    {
	        DataFlashPowerDownSave.Soc.Fcc += FCC_UPDATE_MAX;
	    }
        else if(DataFlashPowerDownSave.Soc.Fcc > CapMax)
        {
            DataFlashPowerDownSave.Soc.Fcc -= FCC_UPDATE_MAX;
        }
    }

	/* 上限设定CAPACITY_DESIGN, 下限设定CAPACITY_ERR_MIN */
    if(DataFlashPowerDownSave.Soc.Fcc > CAPACITY_DESIGN)
    {
        DataFlashPowerDownSave.Soc.Fcc = CAPACITY_DESIGN;
    }
		if(DataFlashPowerDownSave.Soc.Fcc < CAPACITY_ERR_MIN)
    {
        DataFlashPowerDownSave.Soc.Fcc = CAPACITY_ERR_MIN;
    }
}


/*****************************************************************************
 函 数 名  : SocCtrl
 功能描述  : Soc模块处理，状态更新、安时积分
 输入参数  :
 返 回 值  :
*****************************************************************************/
void SocCtrl(void)
{
    /* Ticks 10ms, SOC检测无需1ms判断，但时间积分需精确为1ms */
    if(!TimerSocFlag)
        return;
    TimerSocFlag--;
		
    if(!BatData.Valid)
        return;

    if(SocCtr.InitDelay > 0)                                /* 开机延时50*10ms才检测电流/Soc */
    {
        SocCtr.InitDelay--;
        SocBaseTimer = 0;                                   /* 用于容量积分计算 重新开始计算时间 */
		if ((SocCtr.InitDelay==0) && (DataFlashPowerDownSave.Soc.Ocv!=SOC_OCV_VERIFY))
		{
	//////////////////暂时注释：		SocOcvSet(OCV_INIT_SET);
			if (IS_PRODUCT_ACTIVED())
            	DataFlashPowerDownSave.Soc.Ocv = SOC_OCV_VERIFY;
            	
			SocCtr.OcvWait = 0;
		}		
        return;
    }

    if (SocCtr.OcvReset == 1)
    {
    	SocCtr.OcvReset = 0;
    	SocOcvSet(OCV_INIT_SET);
    	DataFlashPowerDownSave.Soc.Ocv = SOC_OCV_VERIFY;
    }
    
    /* BMS overvol or undervol force update soc */  
    if (IS_CHG_STATE() == 1)
    {
	    if((BatData.VolTotal>=CHG_SMALL_CUR_TOTALVOL) && (BatStatus.Bits.ChgFC==0))
	    {
	        SocCtr.FcFlag = 1;
	    }
	    if((BatData.VolMax>=CHG_OVER_VOL || BatData.VolTotal >= CHG_OVER_VOL_TOTAL) && (BatStatus.Bits.ChgOV==0))
	    {
	    	SocCtr.OvFlag = 1;
	    }
	}
    if( ((BatStatus.Bits.ChgOV==1) && (SocCtr.OvFlag==1))
    	|| ((BatStatus.Bits.ChgFC==1) && (SocCtr.FcFlag==1)) )
    {
        SocCtr.OvFlag = 0;
        SocCtr.FcFlag = 0;
        
#ifdef USE_FROZEN_CAPACITY
        SocFrocUpdate();
#endif

#if 0
        if((CAPACITY_ERR_MIN < DataFlashPowerDownSave.Soc.Rmc) && (CAPACITY_DESIGN >= DataFlashPowerDownSave.Soc.Rmc))
        {
            if((DataFlashPowerDownSave.Soc.Fcc + FCC_UPDATE_MAX) < DataFlashPowerDownSave.Soc.Rmc)
            {
                DataFlashPowerDownSave.Soc.Fcc += FCC_UPDATE_MAX;
            }
            else if((DataFlashPowerDownSave.Soc.Fcc - FCC_UPDATE_MAX) > DataFlashPowerDownSave.Soc.Rmc)
            {
                DataFlashPowerDownSave.Soc.Fcc -= FCC_UPDATE_MAX;
            }
            else
            {
                DataFlashPowerDownSave.Soc.Fcc = DataFlashPowerDownSave.Soc.Rmc;
            }

            if(DataFlashPowerDownSave.Soc.Fcc > CAPACITY_DESIGN)
            {
                DataFlashPowerDownSave.Soc.Fcc = CAPACITY_DESIGN;
            }
        }
#else
	SocOvFccUpdate();
    //	DataFlashPowerDownSave.Soc.Fcc = CAPACITY_TYPE - (DataFlashPowerDownSave.Soc.Cycle*FCC_CYCLE_DECEND);
#endif
        DataFlashPowerDownSave.Soc.Rmc = DataFlashPowerDownSave.Soc.Fcc;
        DataFlashPowerDownSave.Soc.RmcDisp = DataFlashPowerDownSave.Soc.Fcc;
        DataFlashPowerDownSave.Soc.Soc = 100;
        DataFlashPowerDownSave.Soc.SocS = 100;
    }
    else if((BatStatus.Bits.ChgOV==1 || BatStatus.Bits.ChgFC==1) && (DataFlashPowerDownSave.Soc.Soc<80))   // 启充保护做SOC大偏差时修正
    {
        DataFlashPowerDownSave.Soc.Soc = 95;
        DataFlashPowerDownSave.Soc.Rmc = (u32)DataFlashPowerDownSave.Soc.Fcc * DataFlashPowerDownSave.Soc.Soc / 100;
    }
    
    if(((BatData.VolMin <= DIS_UNDER_VOL) || (BatData.VolTotal <= DIS_UNDER_VOL_TOTAL))
       && (BatStatus.Bits.DisUV == 0))
    {
        SocCtr.UvFlag = 1;
    }
    else if((BatStatus.Bits.DisUV == 1) && (SocCtr.UvFlag == 1))
    {
        SocCtr.UvFlag = 0;
        DataFlashPowerDownSave.Soc.Rmc = 0;
        DataFlashPowerDownSave.Soc.Soc = 0;
        DataFlashPowerDownSave.Soc.RmcDisp = 0;
        DataFlashPowerDownSave.Soc.SocS = 0;
    }
  
    if(SocBaseTimer < 100)			/*100mS执行一次*/
        return;
	SocDeltaTime = SocBaseTimer;
	SocBaseTimer = 0;

    SocCur = (s32)ChgCurAvg - (s32)DisCurAvg;

    if ((SocCur>SOC_DEADDISCUR) && (SocCur<SOC_DEADCHGCUR))
    {
        SocCur = SOC_STATICCUR;
    }
			
#ifdef SOC_1MAH_SMOOTH
	SocDispCtrl();
#endif

    SocIntegrate(SocDeltaTime, SocCur);	/*电流是否采用100ms内平均电流*/

    SocUpdate();

    SocSohUpdate();
}

#ifdef SOC_1MAH_SMOOTH

/*****************************************************************************
 函 数 名  : SocToIndex
 功能描述  : 基于不同放电倍率的电流数据，线性查表获得对应的曲线Index
 输入参数  : SocCur
 返 回 值  : Index
*****************************************************************************/
int SocToIndex(int SocCur)
{
    /* 各档位上界（越负索引越大），单位与 SocCur 一致 */
    static const int thresholds[7] = {
        -2500, -7500, -12500, -17500, -22500, -27500, -35000
    };

    if (SocCur >= 0)
        return 0;

    for (int i = 0; i < 7; i++) 
    {
        if (SocCur > thresholds[i])
            return i;
    }
    return 7;
}

/*****************************************************************************
 函 数 名  : SocOcvRealTime
 功能描述  : 基于不同放电倍率的电压数据，线性查表获得对应的电压点修正SOC
 输入参数  : void
 返 回 值  : 电压修正SOC
*****************************************************************************/
u8 SocOcvRealTime(u16 OcvVol)
{
    u8 i;
    u8 j;
    u8 soc;
    u16 delta_vol;

    if(OcvVol <= SocOcvRealTbl[0][0].Vol)                         // 防止超范围 欠压SOC=0
    {
        soc = SocOcvRealTbl[0][0].Soc;
    }
    else if(OcvVol >= SocOcvRealTbl[0][9].Vol)                     // 防止超范围 过压SOC=100
    {
        soc = SocOcvRealTbl[0][9].Soc;
    }
    else
    {
        if(SocCur < 0)												// 放电模式为负数
        {
        	i = SocToIndex(SocCur);									// 根据放电电流查表
        }
        else
        {
            i = 0;
        }

        for(j = 1; j < 10; j++)
        {
            if(OcvVol == SocOcvRealTbl[i][j].Vol)
            {
                soc = SocOcvRealTbl[i][j].Soc;
                break;
            }
            else if((OcvVol > SocOcvRealTbl[i][j - 1].Vol) && (OcvVol < SocOcvRealTbl[i][j].Vol))
            {
            	delta_vol = (u16)((u32)1000*(SocOcvRealTbl[i][j].Soc - SocOcvRealTbl[i][j - 1].Soc) / (SocOcvRealTbl[i][j].Vol - SocOcvRealTbl[i][j - 1].Vol));
	            soc = SocOcvRealTbl[i][j - 1].Soc + (u8)((u32)(OcvVol - SocOcvRealTbl[i][j - 1].Vol) * delta_vol / 1000);
                break;
            }
        }
    }

    return soc;
}

/*****************************************************************************
 函 数 名  : SocDispCtrl
 功能描述  : 1
 输入参数  : void
 返 回 值  : 用于更新SOC显示,防止SOC在过充和过放时的跳变.
*****************************************************************************/
//extern uint16_t  uartSococv;

void SocDispCtrl(void)
{
    u8 SocOcv;
    u16 OcvVoltage;
    u8 SocUp, SocLow;

	if (IS_CHG_STATE() == 1)
    {
        OcvVoltage = BatData.VolMax;
    }
    else
    {
        OcvVoltage = BatData.VolMin;
    }

    SocOcv = SocOcvRealTime(OcvVoltage);        // 获得放电时OCV值
    //uartSococv = SocOcv;
	Dynamic1mAh = (u32)MAMS_TO_MAH;
	
  if (IS_CHG_STATE())
    {
    	SocUp = 100;
    	SocLow = 85;
    	
    	if (((SocUp >= SocOcv) && (SocLow <= SocOcv))
        	|| ((SocUp >= DataFlashPowerDownSave.Soc.SocS) && (SocLow <= DataFlashPowerDownSave.Soc.SocS)))  
	    {
	        Dynamic1mAh = (u32)MAMS_TO_MAH * (1001 - SocOcv*10) / (1001 - DataFlashPowerDownSave.Soc.SocS*10);
	    }
    }
    else
	{
		SocUp = 15;
    	SocLow = 0;
    		
	    if (((SocUp >= SocOcv) && (SocLow <= SocOcv))
			 || ((SocUp >= DataFlashPowerDownSave.Soc.SocS) && (SocLow <= DataFlashPowerDownSave.Soc.SocS)))
		{
	        Dynamic1mAh = (u32)MAMS_TO_MAH * (SocOcv*10 + 1) / (DataFlashPowerDownSave.Soc.SocS*10 + 1);
	    }
	}
}
#endif

u8 SocIsOcvWait(void)
{
	return SocCtr.OcvWait;
}

void SocOcvTodo(void)
{
	SocCtr.OcvReset = 1;
}

#endif  // SOC_SUPPORT
