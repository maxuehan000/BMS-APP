/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : interface.c
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2017年1月6日 星期五
  最近修改   :
  功能描述   : 人机交互(LED)C源文件
  函数列表   :
  修改历史   :
  1.日    期   : 2017年1月6日 星期五
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/
#include "McuHal.h"
#include "user.h"
#include "interface.h"
#include "dataflash.h"
#include "soc.h"
#include "Key.h"


#define LED1_ON		0x01
#define LED1_OFF	0
#define LED2_ON		0x02
#define LED2_OFF	0
#define LED3_ON		0x04
#define LED3_OFF	0
#define LED4_ON		0x08
#define LED4_OFF	0
#define LED5_ON		0x10
#define LED5_OFF	0
#define LED6_ON		0x20
#define LED6_OFF	0
#define LED7_ON		0x40
#define LED7_OFF	0
#define LED8_ON		0x80
#define LED8_OFF	0


void LedSet(u8 LedOnOff)
{
	if (LedOnOff&LED1_ON)
	{
		LED_1_ON();
	}
	else
	{
		LED_1_OFF();
	}

	if (LedOnOff&LED2_ON)
	{
		LED_2_ON();
	}
	else
	{
		LED_2_OFF();
	}

	if (LedOnOff&LED3_ON)
	{
		LED_3_ON();
	}
	else
	{
		LED_3_OFF();
	}

	if (LedOnOff&LED4_ON)
	{
		LED_4_ON();
	}
	else
	{
		LED_4_OFF();
	}
	
	if (LedOnOff&LED5_ON)
	{
		LED_5_ON();
	}
	else
	{
		LED_5_OFF();
	}

	if (LedOnOff&LED6_ON)
	{
		LED_6_ON();
	}
	else
	{
		LED_6_OFF();
	}

	if (LedOnOff&LED7_ON)
	{
		LED_7_ON();
	}
	else
	{
		LED_7_OFF();
	}

	if (LedOnOff&LED8_ON)
	{
		LED_8_ON();
	}
	else
	{
		LED_8_OFF();
	}
}
void showChg(u8  toaggle)
{
	static uint8_t showToaggle =0;
	uint8_t  showUpdate  =0;
	static uint8_t showTyp =0;
	static uint16_t   ledShowIdx =0;
	if(showTyp!=ledShow.showType)
	{
		showTyp = ledShow.showType;
		ledShow.showTimeCnt =0;
	}
	if(showToaggle!=toaggle)
	{
		showToaggle = toaggle;
		showUpdate  =1;
	}
	// if(ledShow.showTimeCnt++ >=300 )//3s
	// {
	// 	ledShow.showTimeCnt =0;
	// 	ledShow.showType = LedNoShow;
	// 	showTyp = ledShow.showType;
	// 	BatStatus.Bits.KeyLed =0;
	// 	// return;
	// }
		if (BatStatus.Bits.ChgOV || BatStatus.Bits.ChgFC)
		{
			LedSet(LED2_ON|LED4_ON|LED6_ON|LED8_ON);
		}
		else if ((BatStatus.Bits.Chg == 1) || CHG_ONLY_WAIT_STATE())
		{		
			if (SocInfo.SocS < 95)
			{
				if(showUpdate)
				{
					ledShowIdx++ ;
					if(ledShowIdx >4)
					{
						ledShowIdx =0;
					}
				}
				switch (ledShowIdx) 
				{
					case 0:
					{
						LedSet(LED2_ON|LED4_OFF|LED6_OFF|LED8_OFF);
					}break;
					case 1:
					{
						LedSet(LED2_ON|LED4_ON|LED6_OFF|LED8_OFF);
					}break;
					case 2:
					{
						LedSet(LED2_ON|LED4_ON|LED6_ON|LED8_OFF);
					}break;
					case 3:
					{
						LedSet(LED2_ON|LED4_ON|LED6_ON|LED8_ON);
					}break;
					default:
					{
						LedSet(LED2_OFF|LED4_OFF|LED6_OFF|LED8_OFF);
					}break;
				}
			}
			else
			{
				if (toaggle)
				{
					LedSet(LED2_ON|LED4_ON|LED6_ON|LED8_ON);
				}
				else
				{
					LedSet(LED2_ON|LED4_ON|LED6_ON|LED8_OFF);
				}
			}
		}
		else if (BatStatus.Bytes.ChgProt || BatStatus.Bytes.HardFault)
		{
			//显示充电故障

				if (toaggle)
				{
					LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
				}
				else
				{				
					LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_ON);
				}	
				return;			
		}
		else
		{
			ledShow.showType = LedNoShow;			
			LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
		}
}
//extern uint8_t   reqPowerOffFlag;
void showDis(u8  toaggle)
{
	static uint8_t showTyp =0;
	if(showTyp!=ledShow.showType)
	{
		showTyp = ledShow.showType;
		ledShow.showTimeCnt =0;
	}
//	if(reqPowerOffFlag)
//	{
//		//关机请求显示
//		ledShow.showTimeCnt++ ;
//		if(ledShow.showTimeCnt <60 )//
//		{//开机闪灯
//			LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_ON);
//			// ledShow.showType = LedNoShow;
//			// showTyp = ledShow.showType;
//			return;
//		}else if(ledShow.showTimeCnt <120 )
//		{
//			LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
//			return;
//		}else 
//		{
//			ledShow.showTimeCnt =0;
//		}
//	}
//	ledShow.showTimeCnt++ ;
//	if(ledShow.showTimeCnt <60 )//3s
//	{//开机闪灯
//		LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_ON);
//		// ledShow.showType = LedNoShow;
//		// showTyp = ledShow.showType;
//		return;
//	}else if(ledShow.showTimeCnt <120 )
//	{
//		LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
//		return;
//	}
//	ledShow.showTimeCnt =120;

//	if (SocInfo.SocS <= 13)
//	{
//		if(toaggle)
//		{
//			LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
//		}else 
//		{
//			LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
//		}
//	}else if (SocInfo.SocS <= 25)
//	{
//		if(toaggle)
//		{
//			LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
//		}else 
//		{
//			LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
//		}
//	}else if (SocInfo.SocS <= 38)
//	{
//		if(toaggle)
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_OFF|LED4_OFF);
//		}else 
//		{
//			LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
//		}
//	}
//	else if (SocInfo.SocS <= 50)
//	{
//		if(toaggle)
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_OFF|LED4_OFF);
//		}else 
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_OFF|LED4_OFF);
//		}
//	}else if (SocInfo.SocS <= 63)
//	{
//		if(toaggle)
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_OFF);
//		}else 
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_OFF|LED4_OFF);
//		}
//	}
//	else if (SocInfo.SocS <= 75)
//	{
//		if(toaggle)
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_OFF);
//		}else 
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_OFF);
//		}
//	}else if (SocInfo.SocS <= 88)
//	{
//		if(toaggle)
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_ON);
//		}else 
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_OFF);
//		}
//	}
//	else // if (SocInfo.SocS < 100)
//	{
//		LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_ON);
//	}
	if (SocInfo.SocS <= 15)
	{
		if(toaggle)
		{
			LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
		}else 
		{
			LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
		}
	}else if (SocInfo.SocS <= 25)
	{
		LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
	}
	else if (SocInfo.SocS <= 50)
	{
		LedSet(LED2_ON|LED4_ON);
	}
	else if (SocInfo.SocS <= 75)
	{

		LedSet(LED2_ON|LED4_ON|LED6_ON|LED4_OFF);

	}
	else // if (SocInfo.SocS < 100)
	{
		LedSet(LED2_ON|LED4_ON|LED6_ON|LED8_ON);
	}
}
void showSoc(u8  toaggle)
{
	static uint8_t showTyp =0;
	if(showTyp!=ledShow.showType)
	{
		showTyp = ledShow.showType;
		ledShow.showTimeCnt =0;
	}

//	if(ledShow.showTimeCnt++ >=300 )//3s
//	{
//		ledShow.showTimeCnt =0;
//		ledShow.showType = LedNoShow;
//		showTyp = ledShow.showType;
//		BatStatus.Bits.Active =0;
//		return;
//	}
	if (SocInfo.SocS <= 15)
	{
		if(toaggle)
		{
			LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
		}else 
		{
			LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
		}
	}else if (SocInfo.SocS <= 25)
	{
		LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
	}
	else if (SocInfo.SocS <= 50)
	{
		LedSet(LED2_ON|LED4_ON);
	}
	else if (SocInfo.SocS <= 75)
	{

		LedSet(LED2_ON|LED4_ON|LED6_ON|LED4_OFF);

	}
	else // if (SocInfo.SocS < 100)
	{
		LedSet(LED2_ON|LED4_ON|LED6_ON|LED8_ON);
	}
}
void showSoh(u8  toaggle)
{
	static uint8_t showTyp =0;
	if(showTyp!=ledShow.showType)
	{
		showTyp = ledShow.showType;
		ledShow.showTimeCnt =0;
	}
//	if(ledShow.showTimeCnt++ >=300 )//3s
//	{
//		ledShow.showTimeCnt =0;
//		ledShow.showType = LedNoShow;
//		showTyp = ledShow.showType;
//		BatStatus.Bits.Active =0;
//		return;
//	}
//	if(DataFlashPowerDownSave.Soc.Soh <= 13)
//	{
//		if (toaggle)
//		{
//			LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
//		}
//		else
//		{				
//			LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
//		}	
//	}else if(DataFlashPowerDownSave.Soc.Soh <= 25)
//	{
//		LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
//	}else if(DataFlashPowerDownSave.Soc.Soh <= 38)
//	{
//		if (toaggle)
//		{
//			LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
//		}
//		else
//		{				
//			LedSet(LED1_ON|LED2_ON|LED3_OFF|LED4_OFF);
//		}	
//	}else if(DataFlashPowerDownSave.Soc.Soh <= 50)
//	{
//		LedSet(LED1_ON|LED2_ON|LED3_OFF|LED4_OFF);
//	}else if(DataFlashPowerDownSave.Soc.Soh <= 63)
//	{
//		if (toaggle)
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_OFF|LED4_OFF);
//		}
//		else
//		{				
//			LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_OFF);
//		}	
//	}else if(DataFlashPowerDownSave.Soc.Soh <= 75)
//	{
//		LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_OFF);
//	}else if(DataFlashPowerDownSave.Soc.Soh <= 88)
//	{
//		if (toaggle)
//		{
//			LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_OFF);
//		}
//		else
//		{				
//			LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_ON);
//		}	
//	}else 
//	{
//		LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_ON);		
//	}
	if (SocInfo.SocS <= 15)
	{
		if(toaggle)
		{
			LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
		}else 
		{
			LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
		}
	}else if (SocInfo.SocS <= 25)
	{
		LedSet(LED1_ON|LED2_OFF|LED3_OFF|LED4_OFF);
	}
	else if (SocInfo.SocS <= 50)
	{
		LedSet(LED2_ON|LED4_ON);
	}
	else if (SocInfo.SocS <= 75)
	{

		LedSet(LED2_ON|LED4_ON|LED6_ON|LED4_OFF);

	}
	else // if (SocInfo.SocS < 100)
	{
		LedSet(LED2_ON|LED4_ON|LED6_ON|LED8_ON);
	}
}
void showFault(u8  toaggle)
{
	static uint8_t showTyp =0;
	if(showTyp!=ledShow.showType)
	{
		showTyp = ledShow.showType;
		ledShow.showTimeCnt =0;
	}
//	if(ledShow.showTimeCnt++ >=500 )//5s
//	{
//		ledShow.showTimeCnt =0;
//		ledShow.showType = LedNoShow;
//		showTyp = ledShow.showType;
//		BatStatus.Bits.Active =0;
//		return;
//	}
	
//	if(BatStatus.Bytes.HardFault)
//	{
//		if (toaggle)
//		{
//			LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
//		}
//		else
//		{				
//			LedSet(LED1_OFF|LED2_OFF|LED3_ON|LED4_OFF);
//		}	
//		return;			
//	}
//	if(BatStatus.Bytes.DisProt)
//	{
//		//显示放电故障
//		if(BatStatus.Bits.DisOC)
//		{
//			if (toaggle)
//			{
//				LedSet(LED1_OFF|LED2_ON|LED3_OFF|LED4_OFF);
//			}
//			else
//			{				
//				LedSet(LED1_OFF|LED2_ON|LED3_OFF|LED4_ON);
//			}	
//			return;	
//		}
//		if(BatStatus.Bits.DisUV)
//		{
//			if (toaggle)
//			{
//				LedSet(LED1_OFF|LED2_OFF|LED3_ON|LED4_OFF);
//			}
//			else
//			{				
//				LedSet(LED1_OFF|LED2_ON|LED3_ON|LED4_OFF);
//			}	
//			return;
//		}
//		if(BatStatus.Bits.DisOT || BatStatus.Bits.DisMOT)
//		{
//			if (toaggle)
//			{
//				LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
//			}
//			else
//			{				
//				LedSet(LED1_OFF|LED2_ON|LED3_OFF|LED4_OFF);
//			}	
//			return;
//		}
//		//其它故障
//		if (toaggle)
//		{
//			LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
//		}
//		else
//		{				
//			LedSet(LED1_OFF|LED2_OFF|LED3_ON|LED4_OFF);
//		}	
//		return;		
//	}
//	if(BatStatus.Bytes.ChgProt)
//	{
//		//显示充电故障
//		if(BatStatus.Bits.ChgER)
//		{
//			if (toaggle)
//			{
//				LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
//			}
//			else
//			{				
//				LedSet(LED1_OFF|LED2_ON|LED3_ON|LED4_OFF);
//			}
//			return;
//		}	
//		if(BatStatus.Bits.ChgOC)
//		{
//			if (toaggle)
//			{
//				LedSet(LED1_OFF|LED2_ON|LED3_OFF|LED4_OFF);
//			}
//			else
//			{				
//				LedSet(LED1_OFF|LED2_ON|LED3_ON|LED4_OFF);
//			}
//			return;
//		}
//		if(BatStatus.Bits.ChgOT || BatStatus.Bits.ChgMOT)
//		{
//			if (toaggle)
//			{
//				LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
//			}
//			else
//			{				
//				LedSet(LED1_OFF|LED2_ON|LED3_OFF|LED4_ON);
//			}
//			return;
//		}
//	}
		if (toaggle)
		{
			LedSet(LED1_OFF|LED3_OFF|LED5_OFF|LED7_OFF);
		}
		else
		{				
			LedSet(LED1_ON|LED3_ON|LED5_ON|LED7_ON);
		}	
		return;			
}
/*****************************************************************************
 函 数 名  : LedCtrl
 功能描述  : LED指示控制总函数
 输入参数  :
 返 回 值  :
*****************************************************************************/
void LedCtrl(void)
{
	u8 LedToggle;
	static u8 LedSocTimer = 0;
	
    if(!TimerLedFlag)
	    return;
    TimerLedFlag = 0;

    if (IS_FCT_FORCE_CTRL())
    {
    	return;
    }
		
    if(IS_FACTORY_TEST_MODE())                                      // 生产测试模式,特殊指示
    {
    #ifdef KEY_TO_LED 
		LedSet(LED1_ON|LED2_ON|LED3_ON|LED4_ON|LED5_ON|LED6_ON|LED7_ON|LED8_ON);
	#endif
		LED_T_OFF();
        return;
    }

	LedSocTimer++;
	if (LedSocTimer <= 30)
	{
		LedToggle = 0;
	}
	else if (LedSocTimer < 60)
	{
		LedToggle = 1;
	}
	else
	{
		LedToggle = 1;
		LedSocTimer = 0;
	}
	
	// if(BatStatus.Bits.PowerOff)
	// {
	// 	LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
	// 	return;
	// }
	if((ledShow.showType == LedNoShow) && (!BatStatus.Bits.ChgPlugin)&&(!BatStatus.Bits.Dis))
	{
		LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF|LED5_OFF|LED6_OFF|LED7_OFF|LED8_OFF);
		return;
	}
    if (BmsGetWorkMode() == BMS_WORK_CHG && BatStatus.Bits.ChgPlugin == 1)
	{
		ledShow.showType = LedShowChg;
		showChg(LedToggle);
	}
	else if (BatStatus.Bits.Dis)
	{
		ledShow.showType = LedShowDis;
		showDis(LedToggle);
	}else if(BatStatus.Bits.Active)
	{
		if(BatStatus.Bytes.HardFault ||BatStatus.Bytes.DisProt)
		{
			ledShow.showType = LedShowFault;
		}
		switch (ledShow.showType) 
		{
			case LedNoShow:
			{
				LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
			}break;
			case LedShowSoc:
			{
				showSoc(LedToggle);
			}break;
			case LedShowSoh:
			{
				showSoh(LedToggle);
			}break;
			case LedShowFault:
			{
				showFault(LedToggle);
			}break;
			default:
			{
				LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
			}break;
		}
	}else if(BatStatus.Bytes.HardFault ||BatStatus.Bytes.DisProt)
	{
		if(ledShow.showType != LedNoShow)
		{
			ledShow.showType = LedShowFault;
			showFault(LedToggle);
		}
	}
	else
	{
		LedSet(LED1_OFF|LED2_OFF|LED3_OFF|LED4_OFF);
	}
}


