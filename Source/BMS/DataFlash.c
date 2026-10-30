//#include "string.h"
//#include "User.h"
//#include "mcuhal.h"
//#include "dataflash.h"
//#include "AfeModule.h"
//#include "gotoboot.h"
//#include "soc.h"
#include "board.h"
#define DATA_CRC_VERIFY


/***********************************************************************************************************/
XRAM StrDataFlashAtOnceSave  DataFlashAtOnceSave;         //立即调用保存的数据
XRAM StrDataFlashPowerDownSave DataFlashPowerDownSave;    //掉电时保存的数据
static u8 DataLogSaveFlag = 0;

/*****************************************************************************
 函 数 名  : CrcCalc
 功能描述  : CRC计算
 输入参数  : u8 *crcBuf  
             u16 nSize     
 返 回 值  : 
*****************************************************************************/
static u32 CrcCalc(u8 *pCrcBuf, u16 Size)
{
	u16 i;
	u32 crc;

	crc = 0x12345678;
	for (i=0; i<Size; i++)
	{
		crc += *(pCrcBuf+i);
	}

	return crc;
}

/*****************************************************************************
 函 数 名  : AtOnceSaveDataWrite
 功能描述  : 立即保存数据的写入
 输入参数  : AT_ONCE_SAVE_ADDR 立即保存数据结构首址   DataFlashAtOnceSave  立即保存的数据
 返 回 值  : DataFlashAtOnceSave  立即保存的数据
*****************************************************************************/
void AtOnceSaveDataWrite(void)
{
#ifdef DATA_CRC_VERIFY
	DataFlashAtOnceSave.Crc = CrcCalc((u8 *)&DataFlashAtOnceSave, sizeof(DataFlashAtOnceSave)-4);
#endif
	HwDataUnlock();
	HwDataPageWrite(AT_ONCE_SAVE_ADDR, (u32 *)&DataFlashAtOnceSave,	sizeof(DataFlashAtOnceSave)/4); //调用此代码必须保证地址范围在一个page内
	HwDataLock();
}
/*****************************************************************************
 函 数 名  : AtOnceSaveDataRead
 功能描述  : 立即保存数据的读取
 输入参数  : AT_ONCE_SAVE_ADDR 立即保存数据结构首址   DataFlashAtOnceSave  立即保存的数据
 返 回 值  : DataFlashAtOnceSave  立即保存的数据
*****************************************************************************/
void AtOnceSaveDataRead(void)
{
	HwDataRead(AT_ONCE_SAVE_ADDR, (u32 *)&DataFlashAtOnceSave, sizeof(DataFlashAtOnceSave)/4);
#ifdef DATA_CRC_VERIFY
//	if (DataFlashAtOnceSave.Crc != CrcCalc((u8 *)&DataFlashAtOnceSave, sizeof(DataFlashAtOnceSave)-4))
	{
		// do none for AtOnceSaveData
	}
#endif

}
/*****************************************************************************
 函 数 名  : PowerDownSaveDataWrite
 功能描述  : 掉电保存数据的写入
 输入参数  : POWER_DOWN_SAVE_ADDR 掉电保存数据结构首址   DataFlashPowerDownSave  掉电保存的数据
 返 回 值  : DataFlashPowerDownSave  掉电保存的数据
*****************************************************************************/
void PowerDownSaveDataWrite(void)
{
#ifdef DATA_CRC_VERIFY
	DataFlashPowerDownSave.Crc = CrcCalc(((u8 *)&DataFlashPowerDownSave)+4, sizeof(DataFlashPowerDownSave)-8);	// BootMark 4字节不纳入校验
#endif
	HwDataUnlock();
	HwDataPageWrite(POWER_DOWN_SAVE_ADDR, (u32 *)&DataFlashPowerDownSave, sizeof(DataFlashPowerDownSave)/4); //调用此代码必须保证地址范围在一个page内
	HwDataLock();
}
/*****************************************************************************
 函 数 名  : PowerDownSaveDataRead
 功能描述  : 掉电保存数据的读取
 输入参数  : POWER_DOWN_SAVE_ADDR 掉电保存数据结构首址   DataFlashPowerDownSave  掉电保存的数据
 返 回 值  : DataFlashPowerDownSave  掉电保存的数据
*****************************************************************************/
void PowerDownSaveDataRead(void)
{
	HwDataRead(POWER_DOWN_SAVE_ADDR, (u32 *)&DataFlashPowerDownSave, sizeof(DataFlashPowerDownSave)/4);
#ifdef DATA_CRC_VERIFY
	if (DataFlashPowerDownSave.Crc != CrcCalc(((u8 *)&DataFlashPowerDownSave)+4, sizeof(DataFlashPowerDownSave)-8))
	{
		memset((u8 *)&DataFlashPowerDownSave, 0, sizeof(DataFlashPowerDownSave));
	}
#endif
}

/*****************************************************************************
 函 数 名  : DataRecInit
 功能描述  : 数据记录初始化
 输入参数  : void
 返 回 值  :
*****************************************************************************/
void DataRecInit(void)
{
	AtOnceSaveDataRead();							// 读取DATAFLASH保存的数据
	PowerDownSaveDataRead();
//	UserLogInit();

	DataFlashPowerDownSave.BootMark = 0;

	if (DataFlashPowerDownSave.PowerMark != POWEROFF_MARK)
	{
		DataFlashPowerDownSave.Soc.Ocv = 0;
		DataFlashPowerDownSave.DisUVLock = 0;
		DataFlashPowerDownSave.DisOTLock = 0;
		DataFlashPowerDownSave.ChgOTLock = 0;
	}

	if (DataFlashPowerDownSave.DisUVLock == UV_LOCKED_MARK)
	{
		BatStatus.Bits.DisUV = 1;
	}
	
	DataFlashPowerDownSave.PowerMark = POWERON_MARK;
	
	PowerDownSaveDataWrite();
}

/*****************************************************************************
 函 数 名  : DataLogPwrOffSave
 功能描述  : 掉电保存数据 Flag {0:禁止重复写入 Flag=1:允许重复写入}
 输入参数  : Flag {0:禁止重复写入 Flag=1:允许重复写入}
 返 回 值  :
*****************************************************************************/
void DataLogPwrOffSave(u8 Flag)
{
	if(Flag == 0)				// Flag=0:禁止重复写入  Flag=1:允许重复写入
	{
		if(DataLogSaveFlag)		// 防止重复写入
		{
			return;
		}
		DataLogSaveFlag = 1;
	}

	DataFlashPowerDownSave.PowerMark = POWEROFF_MARK;
	PowerDownSaveDataWrite();
}

void DataLogPwrOnRecord(void)
{
	if (POWEROFF_MARK == DataFlashPowerDownSave.PowerMark)
	{
		DataFlashPowerDownSave.PowerMark = POWERON_MARK;
		PowerDownSaveDataWrite();

		DataLogSaveFlag = 0;
	}
}
