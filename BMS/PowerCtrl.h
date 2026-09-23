#ifndef __POWER_CTRL_H__
#define __POWER_CTRL_H__

MainDef u8 TimerPowFlag;
MainDef u8 TimerSafeFlag;

extern void  SleepOrPowerOffPause(void);
extern void PowerCtrl(void);
extern void MiscRun(void);
extern void PowerSafeCheck(void);

extern void PowerShutDownTimerSet(u16 TickCnts);
extern void PowerEnterSleepTimerSet(u32 TickCnts);
#endif
