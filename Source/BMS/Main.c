#define __MAIN_DEF__

#include "McuHal.h"
#include "user.h"
//#include "interface.h"
#include "key.h"
#include "AdcM.h"
#include "battercheck.h"
#include "discharge.h"
#include "charge.h"
#include "Balance.h"
#include "PowerCtrl.h"
#include "UartBW.h"
#include "BmsCtrl.h"
#include "Calib.h"
#include "Interface.h"
#ifdef UART_CLIENT
#include "UartCT.h"
#endif
#ifdef AFE_MODEULE
#include "AfeModule.h"
#endif
#include "dataflash.h"
#include "soc.h"
#include "RGB.h"

#ifdef BOOTLOADER
#include "gotoboot.h"
#endif
#ifdef CAN_AND_PROTOCOL
#include "AND_CAN_PROTOCOL.h"
#endif

const char version[3] __attribute__((at(0x08003000))) = "0A";

/*****************************************************************************
 函 数 名  : VarTimerReset
 功能描述  : 计时器复位
 输入参数  :
 返 回 值  :
*****************************************************************************/
void VarTimerReset(void)
{
	XRAM u8 i;

    BatError1Timer = 0;
    BatError2Timer = 0;
    BatError3Timer = 0;

    for(i = 0; i < CELL_NUM; i++)
    {
        DisUnderVolTimer[i] = 0;
    }

    DisOverCurTimer[0] = 0;
    DisOverCurTimer[1] = 0;
    DisOverCurTimer[2] = 0;
    DisOverCurTimer[3] = 0;

    ChgSecTimer = 0;
    ChgOverNtcDispSecTimer = 0;
    ChgOverVolTimer[0] = 0;
	ChgOverVolTimer[1] = 0;
    ChgOverVolReTimer[0] = 0;
    ChgOverVolReTimer[1] = 0;
    ChgOverCurTimer[0] = 0;
    ChgOverCurTimer[1] = 0;
    ChgOverCurTimer[2] = 0;
    ChgSmallCurTimer = 0;
    NtcDisTimer = 0;
    NtcDisReTimer = 0;
    NtcChgTimer = 0;
    NtcChgReTimer = 0;

#ifdef SOC_SUPPORT
    SocBaseTimer = 0;
#endif

}

/*****************************************************************************
 函 数 名  : SysVarInit
 功能描述  : 系统变量初始化
 输入参数  :
 返 回 值  :
*****************************************************************************/
void SysVarInit(void)
{
	XRAM u8 i;

    BatStatus.Bytes.DisProt = 0;
    BatStatus.Bytes.ChgProt = 0;
    BatStatus.Bytes.HardFault = 0;
    BatStatus.Bytes.InputState = 0;
    BatStatus.Bytes.BmsState = 0;

    for(i=0; i<CELL_NUM; i++)
    {
        BatData.Bat[i].Vol = 0;
    }
	BatData.Vcc = VDD_DESIGN_VOL;

    TimerBatFlag = 0;
//    TimerNtcFlag = 0;
    TimerDisFlag = 0;
    TimerChgFlag = 0;
    TimerKeyFlag = 0;
    TimerLedFlag = 0;
    TimerAdcFlag = 0;
    TimerMiscFlag = 0;
    TimerUartBWFlag = 0;
    TimerDataLogFlag = 0;
	TimerSocFlag = 0;
    TimerPowFlag = 0;
    TimerBmsFlag = 0;
    TimerSafeFlag = 0;

    FactoryTestMode = 0;

    VarTimerReset();
}

void TaskTickUpdate(void)
{
	static u8 TaskTick1ms = 0;
	u8 i,TickCnt;

	if (SysTickTimer == 0)
		return;

#ifdef OPA_COC_EN
	BatOpaCurUpdate();
#endif	

	TickCnt = SysTickTimer;
	SysTickTimer -= TickCnt;
	TaskTick1ms += TickCnt;

#ifdef SOC_SUPPORT
    SocBaseTimer += TickCnt;
#endif
#ifdef UART_CLIENT
    TimerUartCTFlag = 1;
#endif

#ifdef CAN_AND_PROTOCOL
	AndCanTimerCallBack(TickCnt);		// 1ms
#endif
	CanTimerCallBack(TickCnt);				// 1ms 
    // <10ms do none
    if (TaskTick1ms < TIMEBASE_LOOP)
    	return;
	TickCnt = TaskTick1ms / TIMEBASE_LOOP;
	TaskTick1ms -= (TickCnt*TIMEBASE_LOOP);

//  TimerNtcFlag = 1;
    TimerDisFlag = 1;
    TimerChgFlag = 1;
    TimerKeyFlag = 1;
    TimerDataLogFlag = 1;
    TimerLedFlag = 1;
    TimerAdcFlag = 1;
    TimerBatFlag = 1;
    TimerMiscFlag = 1;
    TimerBmsFlag = 1;
    TimerPowFlag = 1;
    TimerSafeFlag = 1;
    TimerUartBWFlag = 1;
#ifdef CAN_UPGRADE_BW
    UpgradeBwTimerCallBack(TickCnt);	// 10ms
#endif
#ifdef SOC_SUPPORT
    TimerSocFlag = 1;
#endif
#ifdef AFE_MODEULE
    AfeTimerCallBack(1);
	// TimerAfeFlag = 1;
#endif

    BatError1Timer += TickCnt;
    BatError2Timer += TickCnt;
    BatError3Timer += TickCnt;

    for(i = 0; i < CELL_NUM; i++)
    {
        DisUnderVolTimer[i] += TickCnt;
    }

    DisOverCurTimer[0] += TickCnt;
    DisOverCurTimer[1] += TickCnt;
    DisOverCurTimer[2] += TickCnt;
    DisOverCurTimer[3] += TickCnt;

    if(BatStatus.Bits.ChgPlugin)
    {
        ChgOverVolTimer[0] += TickCnt;
		ChgOverVolTimer[1] += TickCnt;
        ChgSecTimer += TickCnt;
        ChgOverNtcDispSecTimer += TickCnt;
        ChgOverCurTimer[0] += TickCnt;
        ChgOverCurTimer[1] += TickCnt;
        ChgOverCurTimer[2] += TickCnt;
        ChgSmallCurTimer += TickCnt;
    }

    NtcDisTimer += TickCnt;
    NtcDisReTimer += TickCnt;
    NtcChgTimer += TickCnt;
    NtcChgReTimer += TickCnt;

    if (AteForceCtrlTimer > TickCnt)
    {
    	AteForceCtrlTimer -= TickCnt;
    }
    else
    {
    	AteForceCtrlTimer = 0;
    }

#ifdef UART_DEBUG
    UartDebugCnt += TickCnt;
#endif
}

/******************************************************************************
The main C function.  Program execution starts
here after stack initialization.
******************************************************************************/
int main(void)
{
	Bootload_WriteUpgradeMark();					// 清Boot标识Flag
    HwIntDisable();                                 // 关中断
    HwMcuInit();                                    // 硬件初始化
	Delay1ms(22);									// 系统延时,OZ需要map操作推荐20ms后访问
	SysVarInit();                                   // 系统变量初始化
	DataRecInit();

	KeyInit();
	CalibInit();
#ifdef CAN_AND_PROTOCOL
	AndCanInit();
#endif		
	CanInit();										// CAN2 包间通信初始化
#ifdef SOC_SUPPORT
    SocInit();                                      // SOC初始化
#endif
#ifdef CAN_SW_PROTOCOL
    SwCanInit();
#endif
#ifdef AFE_MODEULE
	AfeInit();
#endif

#ifdef AFECOMPUTERIR
//	BatIRInit();									// 根据项目实际开启	--待调试确认
#endif
//	HwCompInit();									// 根据项目实际开启	--待调试确认
	Dis_Sleep_Cur_V = DIS_SLEEP_CUR_MIN; 

    RGBPwmInit();                                   // RGB灯初始化

    HwIntEnable();                                  // 开中断

#ifdef UART_DEBUG
    UartSendStr((u8 *)"SystemCoreClock = ");
    UartSendStrData((s32)SystemCoreClock, 10);      // 十进制打印
    UartSendStr((u8 *)" Hz (0x");
    UartSendStrData((s32)SystemCoreClock, 16);      // 十六进制打印(自带0x前缀)
    UartSendStr((u8 *)")\r\n");
#endif

   /* while(1)
	{
		TaskTickUpdate();
		AndCanCtrl();

	}*/
	while(1)
    {
    	TaskTickUpdate();

        MiscRun();                              	// 测试模式超时退出
        PowerSafeCheck();

        KeyCheck();                                 // 按键检测
        
        AdcM_Ctrl();								// ADC电压信号检测

        BatCheck();                                 // 电芯检测

        ChgCtrl();                                  // 充电控制

        DisCtrl();                                  // 放电控制
        
        BmsCtrl(); 									// 电池包控制管理

	#ifdef CAN_AND_PROTOCOL
    	AndCanCtrl();								// CAN通讯控制
	#endif

		CanCtrl(); 

#ifdef AFE_MODEULE
		AfeRun();
#endif

        UartBwCtrl();

		PowerCtrl();                          	// 系统电源控制

		LedCtrl();

#ifdef SOC_SUPPORT
        SocCtrl();
#endif
#ifdef CAN_OTA_SL
		OtaSlJudge();
#endif

#ifdef CAN_UPGRADE_BW
		UpgradeBWSchedule();
#endif
	
#ifdef BOOTLOADER
		BootJudge();
#endif
#ifdef UART_DEBUG
		UartDebug();                                // 调试信息打印
#endif
    }
}


/*****************************************************************************
 函 数 名  : UartDebug
 功能描述  : 调试信息打印
 输入参数  :
 返 回 值  :
*****************************************************************************/
#ifdef UART_DEBUG
#define UART_SEND_PERIOD        (600/TIMEBASE_LOOP)
void UartDebug(void)
{
	XRAM u8 i;

//	if (Tx_Flag)
//		return;

    if(UartDebugCnt < UART_SEND_PERIOD)
    {
        return;
    }
    UartDebugCnt -= UART_SEND_PERIOD;
    
   // UartBmsRxDisable();

	UartSendStr("XHT: ");
	
	UartSendStrData(GetDetId(), 16);
	UartSendStr("  ");
	UartSendStrData(BatStatus.Bits.DetIn, 16);
	UartSendStr("  ");	
	UartSendStrData(BatStatus.Bits.Active, 16);
	UartSendStr("  ");
	UartSendStrData(BatStatus.Bits.KeyOn, 16);
	UartSendStr("  ");

	UartSendStrData(KEY_ID1_IN(), 16);
	UartSendStr("  ");
	
	UartSendStrData(KEY_ID2_IN(), 16);
	UartSendStr("  ");

	for(i = 0; i < CELL_NUM; i++)
	{
		UartSendStrData(BatData.Bat[i].Vol, 10);
		UartSendStr("  ");
	}

	UartSendStr(" I ");                          // AFE检测电流
	UartSendStrData(BatData.cur, 10);            // AFE原始电流(1mA)
	UartSendStr("  ");
#ifdef AFE_MODEULE
	UartSendStrData(AfeGetCurrentFilt(), 10);    // AFE滤波电流(1mA)
#else
	UartSendStrData(0, 10);
#endif
	
#if 0
	UartSendStrData(AdcData.ADVal[CH_PACK_VOL], 10);
	UartSendStr("  ");
	UartSendStrData(AdcData.ADVal[CH_BAT_VOL], 10);
	UartSendStr("  ");
	UartSendStrData(AdcData.ADVal[CH_OPAM_CUR], 10);
	UartSendStr("  ");
	UartSendStrData(AdcData.ADVal[CH_NTC_CHG], 10);
	UartSendStr("  ");
	UartSendStrData(AdcData.ADVal[CH_NTC1], 10);
	UartSendStr("  ");
	UartSendStrData(AdcData.ADVal[CH_NTC_DSG], 10);
	UartSendStr("  ");
#endif

/*	UartSendStr(" N ");
	
	UartSendStrData(BatData.VolMin, 10);
	UartSendStr("  ");
		
	UartSendStrData(BatData.VolMax, 10);
	UartSendStr("  ");
	
	UartSendStrData((MosNtcTempMax-TMP_0C_01K)/10, 10);
	UartSendStr("  ");
		
	UartSendStr(" x ");

	UartSendStrData((BatNtcTemp[0]-TMP_0C_01K)/10, 10);
	UartSendStr("  ");
	UartSendStrData((BatNtcTemp[1]-TMP_0C_01K)/10, 10);
	UartSendStr("  ");
	UartSendStrData((BatNtcTemp[2]-TMP_0C_01K)/10, 10);
	UartSendStr("  ");
	UartSendStrData((MosNtcTemp[0]-TMP_0C_01K)/10, 10);
	UartSendStr("  ");
	UartSendStrData((MosNtcTemp[1]-TMP_0C_01K)/10, 10);
	UartSendStr("  ");

	UartSendStr(" MOS ");                           // 三个MOS实际开启状态: 预放 放电 充电
#ifdef AFE_MODEULE
	UartSendStrData((BatStatus.Bits.PDmos && AfeStatus.Bits.PDSG_EN), 10);
	UartSendStr(" ");
	UartSendStrData((BatStatus.Bits.Dmos && AfeStatus.Bits.DSG_EN), 10);
	UartSendStr(" ");
	UartSendStrData((BatStatus.Bits.Cmos && AfeStatus.Bits.CHG_EN), 10);
#else
	UartSendStrData(BatStatus.Bits.PDmos, 10);
	UartSendStr(" ");
	UartSendStrData(BatStatus.Bits.Dmos, 10);
	UartSendStr(" ");
	UartSendStrData(BatStatus.Bits.Cmos, 10);
#endif
	UartSendStr("  ");
*/
#if 0

	UartSendStr(" S ");
	
	UartSendStrData(BatStatus.Bits.AfeDsg, 16);
	UartSendStr("  ");
	UartSendStrData(BatStatus.Bits.AfeChg, 16);
	UartSendStr("  ");
	UartSendStrData(AfeStatus.Bytes.Status0, 16);
	UartSendStr("  ");
	UartSendStrData(AfeStatus.Bytes.Status1, 16);
	UartSendStr("  ");
	UartSendStrData(AfeStatus.Bytes.Status2, 16);
	UartSendStr("  ");
	UartSendStrData(AfeStatus.Bytes.Status3, 16);
	UartSendStr("  ");
	UartSendStrData(AfeStatus.Bytes.Status4, 16);
	
	UartSendStr(" D ");
	UartSendStrData(BatStatus.Bits.Dis, 10);
	UartSendStr(" ");
	UartSendStrData(BatStatus.Bits.Chg, 10);
	UartSendStr(" ");
	UartSendStrData(BatStatus.Bits.PDmos, 10);
	UartSendStr(" ");
	UartSendStrData(BatStatus.Bits.Dmos, 10);
	UartSendStr(" ");
	UartSendStrData(BatStatus.Bits.Cmos, 10);
	
	UartSendStr(" M ");
	UartSendStrData(MtBmsAllow.bit.DisMosAllow, 10);
	UartSendStr(" ");
	UartSendStrData(MtBmsAllow.bit.ChgMosAllow, 10);
	UartSendStr(" ");
	UartSendStrData(MtBmsAllow.bit.QUICKDisMos, 10);
	UartSendStr(" ");
	UartSendStrData(MtBmsAllow.bit.QUICKChgMos, 10);
	
	UartSendStr(" F ");
	UartSendStrData(BatStatus.Bytes.DisProt, 16);
	UartSendStr(" ");
	UartSendStrData(BatStatus.Bytes.ChgProt, 16);
	UartSendStr(" ");
	UartSendStrData(BatStatus.Bytes.HardFault, 16);
	UartSendStr("  ");
	UartSendStrData(BatStatus.Bytes.BmsState, 16);
	UartSendStr("  ");
	UartSendStrData(BatStatus.Bytes.InputState, 16);
	
	UartSendStr(" V ");
	UartSendStrData(BatData.VolPack, 10);
	UartSendStr(" ");
	UartSendStrData(BatData.VolTotal, 10);
	UartSendStr(" ");
	
	UartSendStr(" C ");
	UartSendStrData(ChgCurAvg, 10);
	UartSendStr(" ");
	UartSendStrData(DisCurAvg, 10);
#endif

	UartSendStr(" L ");
	UartSendStrData(gLedTrigger, 10);

	UartSendStr("\r\n");
//	UartBmsRxEnable();
}
#endif
