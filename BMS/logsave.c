
#include "W25X40.h"
#include "logsave.h"

uint32_t EventCount;



//////////////////////////////////////////////////////////////////////////////
//flash 操作  

void LogCountWrite(void);
//log数据保存
void LogWrite(BMS_LOG *log)
{
    u32 offer;
//    FlashUnLock();
    LogCountWrite();  //计数器++写入
    offer = (EventCount%EVEN_WRITEMAXNUM);
    offer *= sizeof(BMS_LOG); 
    w25x40_flash_write(EVENTFLASH_ADDR+offer,(u8 *)log,sizeof(BMS_LOG));
//    FlashLock();
}
//log数据读取
BMS_LOG *LogRead(uint16_t index)
{
    static BMS_LOG log;
    u32 offer;
    if(EventCount >= index)
    { 
        offer = ((EventCount-index)%EVEN_WRITEMAXNUM);
        offer *= sizeof(BMS_LOG);  //必须是4096的整数倍 现在是128正好32倍 
        w25x40_flash_read(EVENTFLASH_ADDR+offer,(u8 *)&log,sizeof(BMS_LOG));
        return &log;
    }
    else return 0;
}
//log数据计数器读取
void LogCountRead(void)
{
    u32 countbuf[2];u16 i; 
    for(i=0;i<FLASHMINSIZE/8;i++)      //max 4ms*512 实际应该在1s内故障
    {
        w25x40_flash_read(COUNTFLASH_ADDR+i*8,(u8 *)countbuf,8);
        if(countbuf[0]==0xffffffff||countbuf[0]!=~countbuf[1])
        {
            break;
        } 
    }
    if(i==0)  //第一个就不符合要求，则初始化为0
    {  
        countbuf[0]=0xffffffff; 
    }
    else if(i<FLASHMINSIZE/8)  //合法计数器
    {
        i--;
        w25x40_flash_read(COUNTFLASH_ADDR+i*8,(u8 *)countbuf,8);
    }
    else
    {
        if(countbuf[0]==0xffffffff||countbuf[0]!=~countbuf[1])
        {   
            countbuf[0]=0xffffffff; 
        }
    }
    EventCount = countbuf[0];
}
//log数据计数器++写入
void LogCountWrite(void)
{ 
    u32 countbuf[2];
    u16 i;
//    FlashUnLock();

    for(i=0;i<FLASHMINSIZE/8;i++)
    {
        w25x40_flash_read(COUNTFLASH_ADDR+i*8,(u8 *)countbuf,8);
        
        if(countbuf[0]==0xffffffff||countbuf[0]!=~countbuf[1])
        {
            break;
        }
    }
    
    EventCount++;
    
    if(i>=FLASHMINSIZE/8 )
    { 
        i=0;
    }  
    countbuf[0]= EventCount;countbuf[1]= ~countbuf[0]; 
    w25x40_flash_write(COUNTFLASH_ADDR+i*8,(u8 *)countbuf,8); 
//    FlashLock();
}

u8 LogManu = 0;

BMS_LOG testlog = {0};

void LogTest(void)
{
	if (LogManu == 0)
		return;

	switch (LogManu)
	{
		case 1:
			w25x40_flash_read(EVENTFLASH_ADDR,(u8 *)&testlog, sizeof(BMS_LOG));
			break;

		case 2:
			w25x40_flash_write(EVENTFLASH_ADDR,(u8 *)&testlog, sizeof(BMS_LOG));
			break;

		case 3:
			w25x40_flash_chip_erase();
			break;

		case 4 :
			w25x40_flash_page_erase(EVENTFLASH_ADDR);
			break;

		case 5:
			w25x40_flash_lock();
			break;

		case 6:
			w25x40_flash_unlock();
			break;

		case 7:
			w25x40_flash_powerdown();
			break;

		case 8:
			w25x40_flash_poweron();
			break;
	}
	
	LogManu = 0;
}

