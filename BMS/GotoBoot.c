/******************************************************************************

                  版权所有 (C), 2016-2026, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : gotoBoot.c
  版 本 号   : 初稿
  作    者   : 李卫
  生成日期   : 2017年6月14日 星期三
  最近修改   :
  功能描述   : 完成清除boot标志及进入boot判断程序
  函数列表   :
  修改历史   :
  1.日    期   : 2017年6月14日 星期三
    作    者   : 李卫
    修改内容   : 创建文件

******************************************************************************/
#include "board.h"
//#include <stdio.h>
//#include <string.h>

//#include "User.h"
//#include "dataflash.h"
//#include "mcuhal.h"
//#include "gotoboot.h"
//#include "AfeModule.h"


#define FLASH_BOOT_WORD		0xAA55F813

// 对应boot的地址
#define PAGE_SIZE 			1024*2
#define BOOT_MARK_ADDR		(FLASH_BASE_ADDR + 1024*(128-2))    // set as last page at 126K to save the mark

static uint8_t XRAM bootBuf[24];
static uint8_t XRAM bootRxCount = 0;
const uint8_t messageErrorInfo[]={0xF2,0x01,0xA0,0x00,0x01,0x00,0x55,0xAA,0xF1};
//const uint8_t messageOkInfo[]={0xF2,0x01,0x70,0x00,0x04,0x00,0x00,0x00,0x00,0x55,0xAA,0xF1};
static u32 BootGotoFlag = 0;
#define BOOT_GOTO_KEY		0xA478FECD
void BootLoaderActive(void)
{
	BootGotoFlag = BOOT_GOTO_KEY;
}

void BootLoaderCancel(void)
{
	BootGotoFlag = 0;
}

u8 BootLoaderIsActived(void)
{
	return (BootGotoFlag == BOOT_GOTO_KEY);
}
/*****************************************************************************
 函 数 名  : BootGoto
 功能描述  : 设置BOOT标记，并进入boot引导
 输入参数  : void
 返 回 值  :
*****************************************************************************/
void BootGoto(void)			//此代码还可增加其他校验，保证程序准确运行
{
	DataFlashPowerDownSave.BootMark = FLASH_BOOT_WORD;
	DataLogPwrOffSave(1);

    AfeFetForceClose();
	// CHG_MCU_OFF();
	AFE_EFET_OFF();

	Delay1ms(3);

	HwMcuReset();
	// Trap the CPU
	while(1);
}

void BootReceive(uint8_t rxData)  //引导接收判断函数
{
	//F1 FA 70 000E 00000200 0200 0001 0000 00000000 55AA F2
	if(rxData == 0xF1 && bootRxCount <= 4)
	{
		bootRxCount = 0;
		bootBuf[bootRxCount] = rxData;
		bootRxCount ++;
	}
	else if(rxData == 0xFA && bootRxCount == 1)
	{
		bootBuf[bootRxCount] = rxData;
		bootRxCount ++;
	}
	else if(rxData == 0x70 && bootRxCount == 2)
	{
		bootBuf[bootRxCount] = rxData;
		bootRxCount ++;
	}
	else if(rxData == 0x00 && bootRxCount == 3)
	{
		bootBuf[bootRxCount] = rxData;
		bootRxCount ++;
	}
	else if(rxData == 0x0E && bootRxCount == 4)
	{
		bootBuf[bootRxCount] = rxData;
		bootRxCount ++;
	}
	else if(bootRxCount > 4 && bootRxCount < 22)
	{
		bootBuf[bootRxCount] = rxData;
		bootRxCount ++;

		if(bootRxCount >= 22)
		{
			bootRxCount = 0x5A;
		}
	}
	else if(bootRxCount != 0x5A)
	{
		bootRxCount = 0;
	}
}

void BootJudge(void)
{
	uint8_t result = 0;
	uint16_t XRAM framLength;
	uint32_t XRAM fileLength;
	uint32_t XRAM softwareVesion;
	//CAN boot
	static u8 BootDelayCnt = 0;
	if (BootGotoFlag == BOOT_GOTO_KEY)
	{
		if (++BootDelayCnt < 10)
		{
			return;
		}
		BootDelayCnt = 0;
		
		DataLogPwrOffSave(1);

		if (DataFlashPowerDownSave.BootMark == FLASH_BOOT_BWK)
		{
			
		}
		else if (DataFlashPowerDownSave.BootMark == FLASH_BOOT_KEY)
		{
			
		}
		else
		{
			BootLoaderCancel();
			return;
		}

		Delay1ms(10);
		
		HwMcuReset();
		// Trap the CPU
		while(1);
	}
	//串口BOOT
	if(bootRxCount == 0x5A)
	{
		//F1 FA 70 000E 00000200 0200 0001 0000 00000000 55AA F2
		bootRxCount = 0;

		if(bootBuf[21] == 0xF2 && bootBuf[19] == 0x55 && bootBuf[20] == 0xAA)
		{
			result = 0;
			fileLength = bootBuf[5];
			fileLength <<= 8;
			fileLength +=  bootBuf[6];
			fileLength <<= 8;
			fileLength +=  bootBuf[7];
			fileLength <<= 8;
			fileLength +=  bootBuf[8];

			if(fileLength > (1024*(32-5-1)))	// file length max
			{
				result = 2;
			}

			framLength =  bootBuf[9];
			framLength <<= 8;
			framLength +=  bootBuf[10];

			if(framLength > 1024)					// fram size max
			{
				result = 3;
			}

			softwareVesion =  bootBuf[15];
			softwareVesion <<= 8;
			softwareVesion +=  bootBuf[16];
			softwareVesion <<= 8;
			softwareVesion +=  bootBuf[17];
			softwareVesion <<= 8;
			softwareVesion +=  bootBuf[18];

			if(softwareVesion != 0ul)
			{
				result = 4;
			}

		#ifdef AFE_MODEULE
			if(BatData.VolMin < DIS_UNDER_VOL_RE)
			{
				result = 5;				// AFE would close DMOS for UV protection
			}
		#endif
		
			if(result == 0)
			{
				// F2 01 70 00 04 00000000 55 AA F1
				Delay1ms(10);	// waiting for transmit message information
				BootGoto();
			}
			
			// F2 01 A0 00 01  00 55 AA F1
			memcpy(bootBuf, messageErrorInfo, sizeof(messageErrorInfo));
			bootBuf[5] = result;
		//	UartBmsStartSend(bootBuf,sizeof(messageErrorInfo));
			UartBmsRxDisable();
			Delay1ms(1);
			for(u8 i=0; i<sizeof(messageErrorInfo); i++)
			{
				UartBmsSendByte(*(bootBuf+i));		// polling send bytes
			}
			Delay1ms(5);
			UartBmsRxEnable();
		}
	}
}


/*******************************************************************************
 ** @ function name : Bootload_WriteUpgradeMark
 ** @ Description   : boot标识更新
 ** @ Input param   : uint8_t mark  
 ** @ Return Value  : 
*******************************************************************************/
void Bootload_WriteUpgradeMark(void)
{
    uint32_t boot_info[PAGE_SIZE/4];
    
    /* 为了尽量不改动标准库代码，把中断向量表重定义放这里了,中断偏移16K(boot程序空间16K) */
//	SCB->VTOR = FLASH_BASE|0x2000;

    // 读取存储boot页的信息
	HwDataReadRandom((uint32_t)BOOT_MARK_ADDR, (u32*)boot_info, PAGE_SIZE/4);

	if(boot_info[0] == FLASH_BOOT_WORD)
	{
		boot_info[0] = 0xffffffff;
		HwDataUnlock();
		HwDataPageWriteRandom(BOOT_MARK_ADDR, boot_info, PAGE_SIZE/4);
		HwDataLock();
	}
}






