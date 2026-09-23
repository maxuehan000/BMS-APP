#include "board.h"
//#include "user.h"
//#ifdef AFE_MODEULE
//#include "AfeModule.h"
//#endif

#define CHG_BALANCE_VOL_MIN			3700						/* 充电均衡起始电压 */
#define CHG_BALANCE_VOL_MAX			4200						/* 充电均衡结束电压 */
#define CHG_BALANCE_VOL_W			30							/* 充电均衡电压差 */
#define CHG_BALANCE_VOL_W_RE		20							/* 充电均衡恢复电压差 */
#define CHG_BALANCE_DELAY			(1000/TIMEBASE_LOOP)		/* 充电均衡检测时长 */
#define CHG_BALANCE_TIME			(150/TIMEBASE_LOOP)			/* 充电均衡开启时长 */
#define CHG_BALANCE_DEAD_TIME		(20/TIMEBASE_LOOP)			/* 充电均衡关闭后等待时长 */

#define BAT_TOGGLE_TIME				(60/TIMEBASE_LOOP)



static uint8_t ChgBalanceLock;


/*****************************************************************************
 函 数 名  : ChgBalanceGetSel
 功能描述  : 充电状态下，电芯均衡状态检测
 			奇偶节交替均衡，以便于测试确认
 输入参数  :   
 返 回 值  : 
 			chSel: bit位对应需均衡的电芯对应节数
*****************************************************************************/
static u16 ChgBalanceGetSel(void)
{
	u8 i;
	u8 balance_vol_diff;
	u32 chan_act = 0;
	static u32 chan = 0;
	static u8 swap = 0;

	/* 均衡未开启时，需清零均衡通道 */
	if (ChgBalanceLock == 0)
	{
		chan = 0;
	}

	/* 均衡电压初检测 */
	if ( (BatData.VolMin<CHG_BALANCE_VOL_MIN) || ((BatData.VolMax-BatData.VolMin)<CHG_BALANCE_VOL_W_RE) )
	{
		chan = 0;
        return 0;
	}
	
	for (i=0; i<CELL_NUM; i++)
	{
		if (chan & (1<<i))
		{
			/* 该节电芯此前已开启均衡，均衡压差阀值为 30mv，设定迟滞区间值 */
			balance_vol_diff = CHG_BALANCE_VOL_W_RE;
		}
		else
		{
			/* 该节电芯此前未开启均衡，均衡压差阀值为 50mv */
			balance_vol_diff = CHG_BALANCE_VOL_W;
		}
		
 		if ( (BatData.Bat[i].Vol<CHG_BALANCE_VOL_MAX)  
			&& ((BatData.Bat[i].Vol-BatData.VolMin)>= balance_vol_diff) )
		{
			chan |= ((u16)1 << i);
		}
		else
		{
			chan &= ~((u16)1 << i);
		}
	}

	if (swap == 0)
	{
		if (chan & 0x55555555)
			chan_act = (chan & 0x55555555);			
		else
			chan_act = (chan & 0xAAAAAAAA);

		swap = 1;
	}
	else
	{
		if (chan & 0xAAAAAAAA)
			chan_act = (chan & 0xAAAAAAAA);
		else
			chan_act = (chan & 0x55555555);

		swap = 0;
	}

	return chan_act ;
}

/*****************************************************************************
 函 数 名  : ChgBalanceExit
 功能描述  : 电芯均衡关闭
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
static void ChgBalanceExit(void)
{
	u8 ret = 0;
	
	if (ChgBalanceLock)
	{			
		ret = AfeSetBalance(0);
		if (ret == 0)
		{
			ChgBalanceLock = 0;
		}
	}
}

/*****************************************************************************
 函 数 名  : ChgBalance
 功能描述  : 电芯均衡
 			适用于开启均衡时，可以同时采集电芯电压采集，例如BQ系列
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void ChgBalance(void)
{
	u16 chg_sel;
	static u16 balance_timer = 0;				/* 充电均衡计时器 */
	
//	if (BatStatus.Bits.Chg == 0)
	if ((BatStatus.Bits.ChgPlugin==0) || (ChgCurAvg<CHG_SMALL_CUR))
	{
		balance_timer = 0;
		ChgBalanceExit();
		return;
	}

	/* CHG_BALANCE_DELAY: 1s 进行一次均衡检测 */
	balance_timer++;
	if (balance_timer > CHG_BALANCE_DELAY)
	{
		balance_timer = 0;
		chg_sel = ChgBalanceGetSel();
		if (chg_sel != 0)
		{
			ChgBalanceLock = 1;
			AfeSetBalance(chg_sel);
		}
		else
		{
			ChgBalanceExit();
		}
	}
}

/*****************************************************************************
 函 数 名  : ChgBalanceExt
 功能描述  : 电芯均衡
 			适用于开启均衡期间无法有效电芯电压采集方案，必须关闭均衡后再采集电芯电压
 			ChgBalanceTimer < CHG_BALANCE_TIME: 均衡开启，不允许电芯电压采集
 			CHG_BALANCE_TIME < ChgBalanceTimer < (CHG_BALANCE_TIME + CHG_BALANCE_DEAD_TIME): 均衡关闭，等待电芯电压稳定
 			ChgBalanceTimer > (CHG_BALANCE_TIME + CHG_BALANCE_DEAD_TIME): 电芯电压采集
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void ChgBalanceExt(void)
{
	u8 ret = 0;
	u16 chg_sel;
	static u16 balance_timer = 0;			/* 充电均衡计时器 */
	static u8  balance_flag = 0;			/* 充电开启均衡标识 */

	if (BatStatus.Bits.Chg == 0)
	{
		balance_timer = 0;
		if (ChgBalanceLock)
		{
			ret = AfeSetBalance(0);
			if (ret == 0)
			{
				ChgBalanceLock = 0;
				balance_flag = 0;
			}
		}
		return;
	}

	balance_timer++;
	/* 开启均衡 */
	if ((balance_timer<CHG_BALANCE_TIME) && !balance_flag)
	{
		chg_sel = ChgBalanceGetSel();
		if (chg_sel != 0)
		{
			balance_flag = 1;
			ChgBalanceLock = 1;
			AfeSetBalance(chg_sel);
		}
	}
	/* 关闭均衡，等待电芯电压稳定 */
	if ((balance_timer >= CHG_BALANCE_TIME) && balance_flag)
	{
		ret = AfeSetBalance(0);
		if (ret == 0)
		{
			balance_flag = 0;
		}
	}
	/* 关闭均衡，电芯电压采集 */
	if ((balance_timer>=(CHG_BALANCE_TIME + CHG_BALANCE_DEAD_TIME)) && !balance_flag)
	{
		ChgBalanceLock = 0;
	}
	/* 开始新一轮均衡控制 */
	if (balance_timer > (CHG_BALANCE_TIME + CHG_BALANCE_DEAD_TIME + CELL_NUM*BAT_TOGGLE_TIME))
 	{
		balance_timer = 0;
	}
}

