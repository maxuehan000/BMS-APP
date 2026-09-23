#ifndef __LOGSAVE__H__
#define __LOGSAVE__H__

#include "Mcuhal.h"
#include "User.h"

#define EVEN_READMAXNUM   1000
#define EVEN_WRITEMAXNUM  1056  //128*32*33    //1056条  log固定是128字节，如果变化需调整代码


//BMS 历史记录  128 bytes
typedef struct{
	uint16_t CellVol[CELL_NUM];		// 10/2 = 5
	uint32_t test[20];
}BMS_LOG;

void LogWrite(BMS_LOG *log);
void LogCountRead(void);  //上电读取当前计数器
BMS_LOG *LogRead(uint16_t index);   //在BMS_ReadLogCallback调用查询log数据 
void LogTest(void);

#endif


