/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : key.c
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2017年1月6日 星期五
  最近修改   :
  功能描述   : 按键识别C源文件
  函数列表   :
  修改历史   :
  1.日    期   : 2017年1月6日 星期五
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/
#include "board.h"
//#include "User.h"
//#include "key.h"
//#include "mcuhal.h"
#define KEY_ON_DELAY			(3000/TIMEBASE_LOOP)	/* 按键按下检测时长: 3s */
//#define KEY_OFF_DELAY			(3000/TIMEBASE_LOOP)	/* 按键按下检测时长: 3s */
#define KEY_RELEASE_DELAY		(200/TIMEBASE_LOOP)    /* 按键释放检测时长: 200ms */
#define KEY_PRESS_DELAY			(80/TIMEBASE_LOOP)     /* 按键释放检测时长: 80ms */

#define KEY_SECOND_DELAY		(2500/TIMEBASE_LOOP)	/* 长按键时长: 2.5s */
#define KEY_ON_DELAY			(3000/TIMEBASE_LOOP)	/* 长按键时长: 3.0s */
#define KEY_OFF_DELAY			(3000/TIMEBASE_LOOP)	/* 长按键时长: 3.0s 发送关机请求 */

#define DET_IN_VOL_MAX			3475			  	// 2.8V
#define DET_IN_VOL_MID			2048			  	// 1.65V
#define DET_IN_VOL_MIN			400					// 0.32V

#define DET_IN_charge_VOL_MAX			1861	//	1.5V 1241	// 1V 3102			  // 2.5V
#define DET_IN_charge_VOL_MIN			372		// 0.3V 868					// 0.7V

typedef struct
{
    u8 ReleaseCnt;
    u8 ReleaseFlag;
//    u8 PressFlag;
    u16 PressTime;
    u16 PressCnt;
} KEY_TYPE;
KEY_TYPE KeyLed = {0};
KEY_TYPE KeyPackIn = {0};
KEY_TYPE KeyChargeIn = {0};

LedShowCtrler     ledShow ={0,0};
static u8 Bat_DetId = 0;
static u8 reqPowerOffFlag;
/*****************************************************************************
 函 数 名  : KeyPressCheck
 功能描述  : 按键按下状态检测
 输入参数  : 对应按键结构体变量指针
 返 回 值  : 1:按键按下状态触发, 0:其他
*****************************************************************************/
u8 KeyPressCheck(KEY_TYPE *Key)
{
    Key->ReleaseCnt = 0;

    if(Key->ReleaseFlag == 1)
    {
        Key->PressCnt++;

        if(Key->PressCnt >= Key->PressTime)
        {
            //  Key->PressFlag = 1;
            Key->ReleaseFlag = 0;
            return 1;
        }
    }

    return 0;
}
/*****************************************************************************
 函 数 名  : KeyReleaseCheck
 功能描述  : 按键松开状态检测
 输入参数  : 对应按键结构体变量指针
 返 回 值  : 1:按键松开状态触发, 0:其他
*****************************************************************************/
u8 KeyReleaseCheck(KEY_TYPE *Key)
{
    Key->PressCnt = 0;

    if(Key->ReleaseFlag == 0)
    {
        Key->ReleaseCnt++;

        if(Key->ReleaseCnt >= KEY_RELEASE_DELAY)
        {
            Key->ReleaseFlag = 1;
            //  Key->PressFlag = 0;
            return 1;
        }
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : KeyInit
 功能描述  : 按键模块初始化
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void KeyInit(void)
{
	KeyLed.ReleaseFlag = 1;						// valid while press in boot state 
	KeyLed.PressTime = KEY_PRESS_DELAY;

	KeyPackIn.ReleaseFlag = 1;					// valid while press in boot state 
	KeyPackIn.PressTime = KEY_ON_DELAY;
	
	KeyChargeIn.ReleaseFlag = 1;				// valid while press in boot state 
	KeyChargeIn.PressTime = KEY_PRESS_DELAY;
}


/*****************************************************************************
 函 数 名  : KeyScan
 功能描述  : 区分按键轻触/长按, 短按亮LED,长按关机掉电
 输入参数  : void
 返 回 值  : u8
*****************************************************************************/
u8 KeyScan(void)
{
	uint8_t result = KEY_NONE;
	uint8_t keyBits = 0;
	static uint16_t pressCount = 0;
	static uint16_t releaseCount = 0;
//	static uint16_t keyCount = 0;
//	static uint8_t confirmCount = 0;
//	static uint8_t  firstKeyOn =0;
	keyBits = VLOCK_IN();						// PB9 EN_INT引脚 - 在位信号
	
	if(keyBits == 0)
	{
		pressCount = 0;
		if(releaseCount < KEY_OFF_DELAY)		// 3S
		{
			releaseCount += 1;
		}
	}
	else
	{
		releaseCount = 0;
		if(pressCount < KEY_ON_DELAY)
		{
			pressCount += 1;
		}
	}

	if(releaseCount == KEY_OFF_DELAY)			// 3S
	{
		result = KEY_OFF;
//		if(confirmCount != 0)
//		{
//		    if(keyCount < KEY_SECOND_DELAY)		// <3s
//            {
//				result = KEY_MILD;				// 轻触
//            }  
//            else if(keyCount < KEY_ON_DELAY)	// 3~10s
//            {
//				result = KEY_SHORT;				// 短按
//            }     
//			keyCount = 0;
//			confirmCount = 0;
//			firstKeyOn =0; //	释放flag
//		}
	}

	if(pressCount == KEY_ON_DELAY)
	{
		result = KEY_ON;
//		if(confirmCount < 0xFF)
//		{
//			confirmCount += 1;
//		}
//		if(!BatStatus.Bits.KeyOn)
//		{
//			if((keyCount < KEY_ON_DELAY))
//			{
//				keyCount += 1;
//				if(keyCount == KEY_ON_DELAY)
//				{
//					result = KEY_ON;
//					firstKeyOn = 1;
//				}
//			}
//		}else if(!firstKeyOn)
//		{
//			if (keyCount < KEY_OFF_DELAY) 
//			{
//				keyCount += 1;
//				if(keyCount == KEY_OFF_DELAY)
//				{
//					result = KEY_OFF;
//				}		
//			}		
//		}
	}
	
    return result;
}

/*****************************************************************************
 函 数 名  : KeyLedPress
 功能描述  : 按亮LED
 输入参数  : void
 返 回 值  : u8
*****************************************************************************/
u8 KeyLedPress(void)
{
	uint8_t result = KEY_NONE;
	uint8_t keyBits = 0;
	static uint8_t pressCount = 0;
	static uint8_t releaseCount = 0;
	static uint16_t keyCount = 0;
	static uint8_t confirmCount = 0;
//	static uint8_t  firstKeyOn =0;
	keyBits = KEY_LED_PRESS();
	if(keyBits == 0)
	{
		pressCount = 0;
		if(releaseCount < KEY_RELEASE_DELAY)
		{
			releaseCount += 1;
		}
	}
	else
	{
		releaseCount = 0;
		if(pressCount < KEY_PRESS_DELAY)
		{
			pressCount += 1;
		}
	}

	if(releaseCount == KEY_RELEASE_DELAY)
	{
		if(confirmCount != 0)
		{
		    if(keyCount < KEY_SECOND_DELAY)		// <3s
            {
				result = KEY_MILD;				// 轻触
            }  
            else if(keyCount < KEY_ON_DELAY)	// 3~10s
            {
				result = KEY_SHORT;				// 短按
            }     
			keyCount = 0;
			confirmCount = 0;
//			firstKeyOn =0; //	释放flag
		}
	}

	if(pressCount == KEY_PRESS_DELAY)
	{
		if(confirmCount < 0xFF)
		{
			confirmCount += 1;
		}
		if(keyCount < KEY_ON_DELAY)
		{
			keyCount += 1;
			
			if(keyCount == KEY_ON_DELAY)
			{
				result = KEY_LONG;
			}
		}
	}
	
    return result;
}

/*****************************************************************************
 函 数 名  : KeyCheck
 功能描述  : 按键检测
 输入参数  : void
 返 回 值  :
*****************************************************************************/
void KeyCheck(void)
{
	u8 key_temp = 0;
	u16 det_sig = 0;

    if(!TimerKeyFlag)
        return;
    TimerKeyFlag = 0;
   
		//开关机按键
    key_temp = KeyScan();
    if (IS_FACTORY_TEST_MODE())					// 生产测试模式,按键动作不执行
    {
    	key_temp = KEY_NONE;
    }
    if(BatStatus.Bits.ChgPlugin)				// 充电不检测按键
    {
    	key_temp = KEY_NONE;
    }
    
    switch(key_temp)
	{
		case KEY_MILD:							// 轻触 <3S
			if(ledShow.showType==LedNoShow)
			{
				BatStatus.Bits.Active = 1;
				ledShow.showType = LedShowSoc;
			}
    		break;	
    		
    	case KEY_SHORT:							// 长按 3~10S
			if(ledShow.showType==LedNoShow)
			{
				BatStatus.Bits.Active = 1;
				ledShow.showType = LedShowSoc;
			}
    		break;
    		
		case KEY_LONG:							// 长按 >10S 
			break;
		case KEY_ON:
		{
//			if(BatStatus.Bits.DetIn)
//			{
				if(!BatStatus.Bits.KeyOn)
				{
					BatStatus.Bits.KeyOn = 1;
					BatStatus.Bits.Active = 1;
					if(ledShow.showType==LedNoShow)
					{
						ledShow.showType = LedShowDis;
					}
				}
//			}else 
//			{
//				if(BatStatus.Bits.KeyOn)
//				{
//					BatStatus.Bits.KeyOn = 0;
//				}
//				BatStatus.Bits.Active =1;
//				if(ledShow.showType==LedNoShow)
//				{
//					ledShow.showType = LedShowSoh;
//				}
//			}
		}break;
		case KEY_OFF:
		{
			reqPowerOffFlag = 1;
		}break;	
		default:	
			break;
	}
	
	//灯光按键
	key_temp = KeyLedPress();				// PC6 KEY_CK
    if (IS_FACTORY_TEST_MODE())				// 生产测试模式,按键动作不执行
    {
    	key_temp = KEY_NONE;
    }
    
    switch(key_temp)
	{
		case KEY_MILD:						// 轻触 <2.5
    	case KEY_SHORT:						// 长按 2.5~3S
		case KEY_LONG:						// 长按 >3S 
			if(ledShow.showType==LedNoShow)
			{
				BatStatus.Bits.Active =1;
				ledShow.showType = LedShowSoc;
			}

    	break;
			
		default:
			break;
	}	
	
	if (IS_FACTORY_TEST_MODE()  || BMS_IS_UPGRADING())
	{
		//
	}
	else
	{
		if(reqPowerOffFlag)
		{
			BatStatus.Bits.KeyOn = 0;
			if(!BatStatus.Bits.PowerOff)
			{
				BatStatus.Bits.PowerOff = 1;
				BatStatus.Bits.ShutDown = 1;  
				ShutDownTimer = 100;
			}
		}
	}
		
#ifdef LAB_TEST_MODE
		BatStatus.Bits.DetIn = 1;
		BatStatus.Bits.DetIn_charge = 1;
		return;
#endif
	
	det_sig = ADRead(ADC_DET_IN);				// PA2 DET-INT-AD 仓位信号/ID检测

    if ((det_sig > DET_IN_VOL_MIN) && (det_sig < DET_IN_VOL_MAX))
    {
        if (KeyPressCheck(&KeyPackIn))
        { 	
			BatStatus.Bits.DetIn = 1;
			if(det_sig < DET_IN_VOL_MID)
			{
				Bat_DetId = 1;
			}
			else
			{
				Bat_DetId = 2;
			}
        }
    }
    else
    {
        if (KeyReleaseCheck(&KeyPackIn))
        {
        	BatStatus.Bits.DetIn = 0;
        	Bat_DetId = 0;
        }
    }
}

void KeyLedReset(void)
{
	BatStatus.Bits.Active = 0;
}

void KeyOnReset(void)
{
	// NO USE
}

u8  GetDetId(void)
{
	return Bat_DetId;
}
