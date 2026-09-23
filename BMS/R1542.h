#ifndef __R_1542_H__
#define __R_1542_H__

#include "User.h"


typedef struct
{
	u16 RMC;		// 1. the remaining capacity;
	u16 RME;		// 2. where possible, the remaining power capability;
	u8  EFC;		// 3. where possible, the remaining round trip efficiency;
	u8  KRate;		// 4. the evolution of self-discharging rates;
	u8  OHM;		// 5. where possible, the ohmic resistance;
	u8  SOH;
}State_Of_Health;


typedef struct
{
	u8 res;
}Life_Of_Time;


MainDef volatile u8 TimerR1542Flag;

#endif
