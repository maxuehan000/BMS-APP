/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : multican.c
  版 本 号   : 初稿
  作    者   : William
  生成日期   : 2025年1月6日 
  最近修改   :
  功能描述   : 多包并联通讯C源文件
  函数列表   :
  修改历史   :
  1.日    期   : 2025年1月6日  
    作    者   : William
    修改内容   : 创建文件

******************************************************************************/
/*MtBmsControlJudge()开始是并包逻辑，约600行开始，前面是并包启动、运行、停止通讯逻辑*/
//#define ADDRDEFINE 
#include "board.h"
#define SAMEADDR_RESET                  /*打开则检测到有重复地址延迟重新广播*/ 
#define MAX_MT_NUM  		2			/*最大并包数量*/
#define BROADCASTMAXNUM  (MAX_MT_NUM*2) /*接收到的最大广播地址数量*/
#define MTCANBASEID 		0x1a0		/*参见MTCAN_ID定义*/ 
#define MTCANTESTID 		0X2a0		/*测试控制ID*/
#define UCIDADDR    (u8 *)0x1ffff7c0ul  /*128位ucid所在的地址*/
#define UIDADDR     (u8 *)0x1ffff7f0ul  /*96位uid所在的地址*/
#define BROADCASTTIME         20        /*广播延迟时间 200ms*/
#define SETADDRTIME           10        /*广播停止后延迟设置地址时间 100ms*/
#define MTSENDTIME            5         /*50ms周期进行并报信息播报*/ 
#define SAMEADDCHECKRESETDELAY ((500/10)/MTSENDTIME)  /*地址一致重启延迟*/
#define BROADFFTIMES          5         /*广播ff同步次数*/
#define CRCGROUPNUM           4         /*4组唯一码用于地址排队*/
#define MTBMSDATADELAY        100       /*数据接收有效延时，1s*/
/*
  1.准备4组uid、ucid、sn生成的8字节crc用于广播排队
  1.第一次启动0x1df8aa00 播报0xffffffff用于ucidcrc同步信号，已经处于广播中的，需重新开始
  2.收到相同crc信息如果已经播报出去丢弃,并重启广播，否则自身进行切换并允许当前信息
  3.收到不一样记录则保存并顺延播报延时
  4.播报记录无变化200ms后停止播报
  5.连续100ms无收到数据
  6.按大小排队生成1-8的地址
  7.收到地址相同或自身地址超过8，重启广播 打开SAMEADDR_RESET
  8.持续10s不收到主板数据，且无充放电电流，且无广播则关闭主动发送
  9.有按键按下或充电器插入，或外部有充电过温等待或有过流、短路等待需继续主动发送
*/   
#pragma pack(1)						// 按1字节对齐
typedef struct
{
	u8 SFunCode	: 8;   /*广播UCID校验码时为0，根据UCID自动生成,128位地址0x1FFF_F7C0*/
	u16 FunCode	: 8;   /*固定AA*/
	u8 CmdCode	: 3;
	u8 DesID	: 5;   /*固定1F*/
	u8 SouID	: 5;   /*固定1D*/
	u8 Unused	: 3;	
}MTCAN_ID;
//typedef union
//{
//	u32		  ExtID;
//	MTCAN_ID  SwID;
//}MT_Can_ID;
//static MT_Can_ID MtcanID;  

static u32 Ucidcrc[CRCGROUPNUM][2];
static u8 CurrentCrc;    /*4组唯一码的校验码，优先第一组，有重叠则++*/

static u8 MyAddr;

static u8 BroadcastCount,BroadcastMark;      /*每次广播需要重新接收*/
static u32 MtBmsBroadcastAddr[BROADCASTMAXNUM][2];/*广播的地址*/  
static u16 BroadcastDelay;         /*广播延迟*/

static u8 MT_OkMark; 			/*并包获取地址成功标志*/
static u8 Mt_Output = 0;		/*有外部的包已经开启放电Mos	*/
static u8 Mt_ChgPlugin = 0;		/*有外部的包检测到充电器在*/


static u8 noMainComMark; /*无主板通讯、无充放电流、无广播信息10s置位*/
static u16 NoMainTimer;  
static u8 delaycanTestmos;

/*充电异常*/
#define COC       1
#define CTOV      2
#define COT       15
/*放电异常*/
#define DOC       1
#define DSC       2
#define DOT       3
/*bms异常*/
#define FUSEERR   1
#define AFEERR    2
#define NTCERR    3   /*包含bms和mos的ntc*/
#define MOSERR    4
#define BATERR    5 
#define MCUERR    6 
#define MOSOT     15 

typedef struct
{
    u8 chgplugin: 1;   /*充电器在 */ 
	u8 chg	    : 1;   /*准备充电标志 */ 
	u8 chgmos	: 1;   /*充电mos状态 */ 
	u8 chgov    : 1;   /*过充 */ 
    u8 chgerr   : 4;   /*充电异常 1-15充电类型异常*/  
    
    u8 key      : 1;   /*按键*/
	u8 dis	    : 1;   /*准备放电标志 */ 
	u8 dismos	: 1;   /*放电mos状态 */ 
    u8 disuv    : 1;   /*过放 */ 
	u8 diserr   : 4;   /*放电异常 1-15放电类型异常 */ 
    
    u8 id       : 1;   /*识别信号状态*/
    u8 nc       : 3;   /*nc*/
	u8 bmserr	: 4;   /*1-15bms故障类型 */ 
}MTbmsstatus;
typedef union
{
	u8 		byte[3];
	MTbmsstatus	bits;
}MTBmsStatusUnion;
typedef struct
{
    MTBmsStatusUnion MTBmsStatus;
    u16 packvol;  /*10mv*/
    s16 cur;      /*10ma*/ 
    u8  iR;       /*内阻毫欧*/
}MtBmsDataStr;
static u8 OtherBMSIR[MAX_MT_NUM];

static MtBmsDataStr MtBmsData[MAX_MT_NUM],MyBmsData;
static u8 MtBmsDataDelay[MAX_MT_NUM];   /*多个包数据倒计时*/

static u16 delaycheckpacknovolpoweron;  /*每次上电或按键闭合或异常恢复检测到端口无电压根据通讯和端口电压判断是否强开端口*/
static u8 powerondelay; 

#pragma pack()	

u8 MultiMessagetoUart(u8 *buf)
{
    u8 len;
    len = sizeof(MyBmsData);
    memcpy(buf,(u8 *)&MyBmsData,len);
    *(buf+len) = MyAddr;
    return len+1;
}

void MtCanBroadcastAddInit(void)
{   /*用于广播本地唯一地址信息校验码的准备*/
    Ucidcrc[0][0] = SwCrc32(0,UCIDADDR,16);// 0xffff5555;//SwCrc32(0,UCIDADDR,16);
    Ucidcrc[0][1] = SwCrc32(0,UCIDADDR,4);
    Ucidcrc[0][1] = SwCrc32(Ucidcrc[0][1],UCIDADDR+12,4);// 0x3344;//SwCrc32(Ucidcrc[0][1],UCIDADDR+12,4);
    if(Ucidcrc[0][0]==0xffffffff&&Ucidcrc[0][1]==0xffffffff)
        Ucidcrc[0][1]=1;
    
    Ucidcrc[1][0] = SwCrc32(0,UIDADDR,12);//0XeeBBCCDD;//
    Ucidcrc[1][1] = SwCrc32(0,UIDADDR,4);
    Ucidcrc[1][1] = SwCrc32(Ucidcrc[1][1],UIDADDR+8,4);//0X33445566;//
    if(Ucidcrc[1][0]==0xffffffff&&Ucidcrc[1][1]==0xffffffff)
        Ucidcrc[1][1]=1;
    
    Ucidcrc[2][0] = SwCrc32(0,UIDADDR,12);
    Ucidcrc[2][0] = SwCrc32(Ucidcrc[2][0],DataFlashAtOnceSave.QRCode,32);
    Ucidcrc[2][1] = SwCrc32(0,DataFlashAtOnceSave.QRCode,8);
    Ucidcrc[2][1] = SwCrc32(Ucidcrc[2][1],DataFlashAtOnceSave.QRCode+24,8);
    if(Ucidcrc[2][0]==0xffffffff&&Ucidcrc[2][1]==0xffffffff)
        Ucidcrc[2][1]=1;
    
    
    Ucidcrc[3][0] = SwCrc32(0,UCIDADDR+10,6);
    Ucidcrc[3][0] = SwCrc32(Ucidcrc[3][0],DataFlashAtOnceSave.QRCode+16,16);
    Ucidcrc[3][1] = SwCrc32(0,UIDADDR+4,8);
    Ucidcrc[3][1] = SwCrc32(Ucidcrc[3][1],DataFlashAtOnceSave.QRCode+16,16);
    if(Ucidcrc[3][0]==0xffffffff&&Ucidcrc[3][1]==0xffffffff)
        Ucidcrc[3][1]=1;
}
void MyMtCanAddrSet(void)
{/*根据广播收取信息进行地址识别准备*/
    u8 i; 
    MyAddr = 1; 
    if(BroadcastCount!=0) 
    { 
        for(i=0;i<BroadcastCount;i++)
        {
            if(Ucidcrc[CurrentCrc][0]>MtBmsBroadcastAddr[i][0]
                ||(Ucidcrc[CurrentCrc][0]==MtBmsBroadcastAddr[i][0]&&Ucidcrc[CurrentCrc][1]>MtBmsBroadcastAddr[i][1]))
                MyAddr ++;
        }
    }
    BroadcastMark = 0; 
}
void BroadcastMessage(CanRxMessage *RxMsg)
{/*广播信息及地址信息处理，判断重复接收和地址重叠则丢弃并更换*/
    static u8 delayaddrsamereset; 
    u8 i;
    u32 candata1,candata2;
    i=(*RxMsg).StdId&0x0f;
    if(i>0)
    {   /*接收标准地址*/
        #ifdef SAMEADDR_RESET
        if(i == MyAddr||MyAddr>MAX_MT_NUM)
        {/*有重复地址，则需要重启广播*/
            if(delayaddrsamereset++>SAMEADDCHECKRESETDELAY) /*50ms*10*/
            {
                BroadcastCount = 0; 
                BroadcastDelay = 0;
                #ifndef ADDRDEFINE
                MyAddr = 0;
                #endif
            }
            return;
        }
        #endif
        /*数据接收解码处理*/
        ////////////////////////////////////////////////////////////////////
        if(i<=MAX_MT_NUM)
        { 
            if(MtBmsDataDelay[i-1]!=0&&MT_OkMark==0x5a)/*收到两次有效数据，允许并包判断*/
            {
                MT_OkMark=0x5b;
            }
            MtBmsDataDelay[i-1]=MTBMSDATADELAY;//(*RxMsg).ExtId&0xff;  
            memcpy((u8*)&MtBmsData[i-1],RxMsg->Data,8);
        }
        ////////////////////////////////////////////////////////////////////
    }
    else if(i==0)
    {   /*接收广播排名*/ 
        if(BroadcastMark==0)
        {/*接收到广播信息则需要重启广播*/
            BroadcastMark = 0X5A;
            BroadcastCount = 0; 
            BroadcastDelay = 0;
            #ifndef ADDRDEFINE
            MyAddr = 0;
            #endif
            NoMainTimer = 0;/*接收到广播需要响应*/
        }
        if(BroadcastDelay>BROADCASTTIME)
            BroadcastDelay = BROADCASTTIME;
        candata1 = ((*RxMsg).Data[0]<<24)+((*RxMsg).Data[1]<<16)+((*RxMsg).Data[2]<<8)+(*RxMsg).Data[3];
        candata2 = ((*RxMsg).Data[4]<<24)+((*RxMsg).Data[5]<<16)+((*RxMsg).Data[6]<<8)+(*RxMsg).Data[7];
        if(candata1 ==0xffffffff && candata2 == 0xffffffff)
        { /*收到0xffffffff说明有一个才开始，重新开始播报，可以全部重新开始*/
            BroadcastDelay = 0;
            #ifndef ADDRDEFINE
            MyAddr = 0;
            #endif
            BroadcastCount = 0; 
            return;
        } 
        if(BroadcastCount==0)
        {/*第一次接收*/
            BroadcastDelay = 0;
            if(Ucidcrc[CurrentCrc][0]==candata1&&Ucidcrc[CurrentCrc][1]==candata2)
            {/*收到和自身一样，则换一组*/
                if(CurrentCrc<CRCGROUPNUM-1)
                    CurrentCrc++;
                else 
                    CurrentCrc = 0; 
                if(BroadcastMark<0x5a+BROADFFTIMES)
                {/*还未广播，可以接收*/
                    MtBmsBroadcastAddr[0][0]=candata1; 
                    MtBmsBroadcastAddr[0][1]=candata2; 
                    BroadcastCount ++; 
                }
                else
                {/*启动一次丢数据*/ 
                    BroadcastMark = 0x5a; 
                }
            }
            else 
            { /*未收到和自身一样的可以缓冲*/
                MtBmsBroadcastAddr[0][0]=candata1; 
                MtBmsBroadcastAddr[0][1]=candata2; 
                BroadcastCount ++;  
            } 
        }
        else if(BroadcastCount<BROADCASTMAXNUM)
        {
            if(Ucidcrc[CurrentCrc][0]==candata1&&Ucidcrc[CurrentCrc][1]==candata2)
            {/*接收到相等，需要挪位*/
                if(CurrentCrc<CRCGROUPNUM-1)
                    CurrentCrc++;
                else 
                    CurrentCrc = 0; 
                if(BroadcastMark<0x5a+BROADFFTIMES)
                {/*还未广播发送则可以接收当前信息*/
                    for(i=0;i<BroadcastCount;i++)
                    {/*本地已接收到相等则不处理*/
                        if(MtBmsBroadcastAddr[i][0]==candata1&&MtBmsBroadcastAddr[i][1]==candata2)
                        { 
                            break;
                        }
                    }
                    if(i>=BroadcastCount)
                    {
                        MtBmsBroadcastAddr[BroadcastCount][0]=candata1; 
                        MtBmsBroadcastAddr[BroadcastCount][1]=candata2; 
                        BroadcastCount++;
                        BroadcastDelay = 0;
                    }
                }
                else
                {
                    BroadcastMark = 0x5a;
                    BroadcastDelay = 0; 
                }
            }
            else
            {/*正常接收处理*/
                for(i=0;i<BroadcastCount;i++)
                {/*本地已接收到相等则不处理*/
                    if(MtBmsBroadcastAddr[i][0]==candata1&&MtBmsBroadcastAddr[i][1]==candata2)
                    { 
                        break;
                    }
                }
                if(i>=BroadcastCount)
                {
                    MtBmsBroadcastAddr[BroadcastCount][0]=candata1; 
                    MtBmsBroadcastAddr[BroadcastCount][1]=candata2; 
                    BroadcastCount++;
                    BroadcastDelay = 0;
                }
            }
        }
        else
        {
            /*接收的缓冲超，需要重新开始*/
            for(i=0;i<BroadcastCount;i++)
            {/*本地已接收到相等则不处理*/
                if(MtBmsBroadcastAddr[i][0]==candata1&&MtBmsBroadcastAddr[i][1]==candata2)
                { 
                    break;
                }
            }
            if(i>=BroadcastCount)
            {
                BroadcastMark = 0x5a; 
                BroadcastDelay = 0;   
            }
        }
    } 
}
u8 MTCanReceiveProcess(CanRxMessage RxMsg, CanTxMessage *TxMsg)
{ /*数据接收解码*/
    if ((RxMsg.StdId&0x7f0) != MTCANBASEID)
    {
        NoMainTimer = 0;
        if(MTCANTESTID == RxMsg.StdId) /*测试控制指令*/
        {
            if(RxMsg.Data[0]==0xa5&& RxMsg.Data[1]==0x5a) 
            {
                BatStatus.Bits.PowerOff = 1;
                ShutDownTimer = 50;
                BatStatus.Bits.ShutDown = 1;
            }
            if(RxMsg.Data[0]==0x5a&& RxMsg.Data[1]==0xa5&&(BatStatus.Bits.DetIn==0)&&(BatStatus.Bits.Active==0))
            {
                MtBmsAllow.bit.CanTestMark = 1;
                delaycanTestmos = 0;
                ENTER_FACTORY_TEST_MODE();
            }
            else MtBmsAllow.bit.CanTestMark = 0;
        }
        return 0;
    }
	if (RxMsg.DLC < 2)
	{
		return 0;
	}
    BroadcastMessage(&RxMsg);
    return 0;
	
}
void MTCanTxMessageInit(CanTxMessage *TxMsg);
void CheckCanOffLine(void);
void  MtBmsControlJudge(void);
s16 multimaxcur,multimincur;/*并包的最大最小电流*/
s16 multitotalcur;
void MTCanCtrl(void)
{/*发送准备*/ 
    static u8 MultiComWaitMark; /*比如过流、短路、充电器重新插入等原因需要等待恢复，通过传递IR=1实现，告诉对方必须等待*/
    CanTxMessage TxMsg;
    u8 i;
    u8 irisone;   /*接收数据中任意一个ir为1，则改标志置位，一直发送数据且不可发0，等待都为0才倒计时休眠*/
    u8 irnozero;  /*接收数据中任意一个ir不为0，则改标志置位，一直发送数据且可以发0，等待都为0才倒计时休眠*/
  
  
    CheckCanOffLine();
    if(noMainComMark<0x5E)/*主板不通讯延时30s后该标志置位，则并包通讯停止*/
    {
        /*自身地址为0，则需先启动广播200ms*/
        #ifndef ADDRDEFINE
        if(MyAddr==0||BroadcastMark>=0X5a)
        {
            BroadcastDelay++;
            if((BroadcastDelay%2)==1&&BroadcastDelay<BROADCASTTIME)
            { 
                MyAddr = 0;
                MTCanTxMessageInit(&TxMsg);  
                MtCanBroadcastAddInit();
                if(BroadcastMark >= 0x5a+BROADFFTIMES)
                {
                    TxMsg.Data[0] = Ucidcrc[CurrentCrc][0]>>24; 
                    TxMsg.Data[1] = Ucidcrc[CurrentCrc][0]>>16; 
                    TxMsg.Data[2] = Ucidcrc[CurrentCrc][0]>>8; 
                    TxMsg.Data[3] = Ucidcrc[CurrentCrc][0]>>0; 
                    TxMsg.Data[4] = Ucidcrc[CurrentCrc][1]>>24; 
                    TxMsg.Data[5] = Ucidcrc[CurrentCrc][1]>>16; 
                    TxMsg.Data[6] = Ucidcrc[CurrentCrc][1]>>8; 
                    TxMsg.Data[7] = Ucidcrc[CurrentCrc][1]>>0;
                }
                else
                {   /*第一次广播发ff，用于同步*/
                    memset(TxMsg.Data,0xff,8);
                    BroadcastCount = 0; 
                    BroadcastDelay=0;
                }
                TxMsg.DLC = 8;
            //	queue_push_back(SwTxQueue, &TxMsg);
                queue_push_back(AndTxQueue, &TxMsg);
                if(BroadcastMark<0x5a)
                    BroadcastMark=0x5a;
                else if(BroadcastMark<0x5a+BROADFFTIMES)
                    BroadcastMark ++;
            }
            if(BroadcastDelay>BROADCASTTIME+SETADDRTIME)
            {
                MyMtCanAddrSet();
                BroadcastDelay = 0;
            }
            MT_OkMark = 0;
        }
        else 
				#else
					MyAddr = GetDetId();
        #endif
        {
            /*周期进行并包播报*/
            BroadcastDelay++;
            if(BroadcastDelay>=MTSENDTIME)
            {
                BroadcastDelay = 0;
                MTCanTxMessageInit(&TxMsg); 
                #if 0    
                TxMsg.Data[0] = Ucidcrc[CurrentCrc][0]>>24; 
                TxMsg.Data[1] = Ucidcrc[CurrentCrc][0]>>16; 
                TxMsg.Data[2] = Ucidcrc[CurrentCrc][0]>>8; 
                TxMsg.Data[3] = Ucidcrc[CurrentCrc][0]>>0; 
                TxMsg.Data[4] = Ucidcrc[CurrentCrc][1]>>24; 
                TxMsg.Data[5] = Ucidcrc[CurrentCrc][1]>>16; 
                TxMsg.Data[6] = Ucidcrc[CurrentCrc][1]>>8; 
                TxMsg.Data[7] = BroadcastCount;//Ucidcrc[CurrentCrc][1]>>0;
                #else
                memcpy(TxMsg.Data,(u8*)&MyBmsData,8);
                if(noMainComMark>=0x5a)
                {
                    TxMsg.Data[7] = 0;
                    noMainComMark++;
                }
                if(MultiComWaitMark==0x5a)/*通知外部还需继续维持并包*/
                    TxMsg.Data[7] = 1;
//                TxMsg.Data[7] = BroadcastCount;//Ucidcrc[CurrentCrc][1]>>0;
                #endif
                TxMsg.DLC = 8;
            //	queue_push_back(SwTxQueue, &TxMsg);
            	queue_push_back(AndTxQueue, &TxMsg);
            }
            MT_OkMark = 0X5A;
        }
        irisone = 0; 
        irnozero = 0;
        for(i=0;i<MAX_MT_NUM;i++)
        {/*并包数据有更新的有效倒计时*/
            if(MtBmsDataDelay[i]>0)
            {
                if(MtBmsData[i].iR!=0||DisCurAvg > Dis_Sleep_Cur_V||ChgCurAvg >CHG_SLEEP_CUR)/*对方主动暂停通讯则不启动倒计时*/
                    MtBmsDataDelay[i]--; 
                if(MtBmsData[i].iR==1)
                    irisone=0x5a;/*识别到有内阻=1的传递，表明对方要求需要继续通讯*/
                if(MtBmsData[i].iR>1)
                    OtherBMSIR[i]= MtBmsData[i].iR;
                if(MtBmsData[i].iR)
                    irnozero = 0x5a;
            }
        }
        /*总线ir有=1，则需要维持通讯，且不能发0，总线ir>1，也需要维持通讯，但能发0，总线有任意不为0，则需要继续倒计时，预防拔出识别不到*/
        if(irisone) /*总线上有1，则需要一直通讯，且本地不能发0*/
				{
            NoMainTimer = 0;   
				}
        if(irnozero)/*总线上不为0且不为1，本地需要发0则可以维持发0*/
        {
            /*任意一个不为0则需要继续发送等待同步进入休眠*/
            if(noMainComMark>=0x5a)
                noMainComMark = 0x5a;
        }
        if(irnozero||noMainComMark<0x5a)
        { /*有任意一个不为0，需要继续发送的情况下，为0的需要继续倒计时--,防止在此时间段拔出发0的，造成识别不到*/
            for(i=0;i<MAX_MT_NUM;i++)
            {
                if(MtBmsDataDelay[i]>0)
                {
                    if(MtBmsData[i].iR==0&&DisCurAvg <= Dis_Sleep_Cur_V&&ChgCurAvg <=CHG_SLEEP_CUR)
                        MtBmsDataDelay[i]--; 
                }
            }
        }
        
    }
    else /*noMainComMark>=0x5E不主动发送*/
    { 

    }
    MtBmsControlJudge();  /*并包逻辑处理*/
    
    /*判断是否主板不在通讯，且无充放电电流，且未广播延时10s，置位无主板通讯标志*/
    NoMainTimer ++;
    if(DisCurAvg > Dis_Sleep_Cur_V||ChgCurAvg >CHG_SLEEP_CUR
        ||multimaxcur>CHG_SLEEP_CUR/10||multimincur<-Dis_Sleep_Cur_V/10   /*并包的所有包电流*/
        ||((BatStatus.Bits.DisOC||BatStatus.Bits.DisSC||BatStatus.Bits.ChgOC)&&BatStatus.Bits.DisOCL==0))/*有过流短路，需要维持通讯*/
    { 
        NoMainTimer = 0; 
        if(DisCurAvg <= Dis_Sleep_Cur_V&&ChgCurAvg <=CHG_SLEEP_CUR&&((BatStatus.Bits.DisOC||BatStatus.Bits.DisSC||BatStatus.Bits.ChgOC)&&BatStatus.Bits.DisOCL==0))
            MultiComWaitMark = 0x5a;  /*置位表示本地无电流但有过流短路等待恢复，同时通过内阻改为1通知外部继续维持通讯*/
        else 
            MultiComWaitMark = 0;
    }
    else 
        MultiComWaitMark = 0;
    /*充电器重新插入需要播报0.5s的数字1通知其他包继续通讯*/
    static u8 chgoutmark;
    if(BatStatus.Bits.ChgPlugin==0)
    {
        chgoutmark = 50;
    }
    else
    {
        if(chgoutmark)
        {
            chgoutmark --; 
            NoMainTimer = 0; 

            MultiComWaitMark = 0x5a;/*用于通知其他包刚有充电器插入*/
        }
    } 
    static u16 nomaincomdelay =NOMAINCOM_DELAY;  
    static u16 loadchangedelay;
    if(ChgCurAvg > 1000|| DisCurAvg >1000||multitotalcur>100||multitotalcur<-100)   
    {     
        if(loadchangedelay++>1000) /*持续负载或充电电流>1a/10s则退出并包延迟从10s变为120s*/
            nomaincomdelay = NOMAINCOM_LOADDELAY;
    }
    else loadchangedelay = 0;
    if(nomaincomdelay < NOMAINCOM_DELAY)
        nomaincomdelay = NOMAINCOM_DELAY;
    if(NoMainTimer<nomaincomdelay)
    {/*有主板通讯或有电流，或充电过温，或过流短路，不能休眠倒计时，有按键或充电器插入或广播，清零一次*/
        ComOverSleepTimer = 0;
        noMainComMark = 0;  
    }
    else 
    {
        NoMainTimer = nomaincomdelay;
        if(noMainComMark<0x5a)
            noMainComMark = 0x5a;
    }
}
void MTCanTxMessageInit(CanTxMessage *TxMsg)
{/*can发送准备*/  
//	MtcanID.ExtID = MTCANBASEID+MyAddr;
	(*TxMsg).ExtId = 0;	
	(*TxMsg).StdId = MTCANBASEID+MyAddr;
	(*TxMsg).IDE = CAN_ID_STD;			// 标准帧标识符
	(*TxMsg).RTR = CAN_RTRQ_DATA;
}
u8 maxmark,maxnum,totalnum; /*maxmark=0单包/=0x5a最大包/=0xa5不是最大包，maxnum最大包数量(包括自己)，totalnum总并包数量(包括自己)*/
void CheckCanOffLine(void)
{/*检测can不在线或异常*/
    static u8 delayCheckcanerr,delayclrcanerr,clrcheckcanerr;
    if(IS_FACTORY_TEST_MODE())
    {/*产测模式下不让can异常影响正常充放电*/
        BatStatus.Bits.CanErr=0; 
        delayCheckcanerr = 0;
    }
    if(CAN->ESTS&0X74)
    {
        if(delayCheckcanerr++>10)
        {
            BatStatus.Bits.CanErr=1;
            MT_OkMark = 0;
            for(u8 i=0;i<MAX_MT_NUM;i++)
                MtBmsDataDelay[i]=0;
            /*用于休眠时判断如何补偿soc及进入休眠的电流条件*/
            maxmark = 0;
            maxnum = 0;
            totalnum = 1;     
            Dis_Sleep_Cur_V = DIS_SLEEP_CUR_MIN;
        }
    }
    if(delayCheckcanerr&&(CAN->ESTS&0X74)==0)
    {
        if(clrcheckcanerr++>10)
            delayCheckcanerr = 0;
    }
    else clrcheckcanerr = 0;
    if(BatStatus.Bits.CanErr)
    {
        delayCheckcanerr = 0;
        if(CAN->ESTS&0X74)
            delayclrcanerr = 0;
        if(delayclrcanerr++> 10)
        {
            BatStatus.Bits.CanErr = 0;
            noMainComMark = 0;
        }
        #ifndef ADDRDEFINE 
        MyAddr = 0; 
        #endif
        BroadcastMark=0; 
    }
    else delayclrcanerr = 0;
}


/*888888888888888888888888888888888888888888888888888888888888888*/
/*                         并包判断                              */
/*888888888888888888888888888888888888888888888888888888888888888*/
/*1.有更低的包，且((其他更低包没开Cmos且(更低包不允许充电或高压包无充电器))或(其他更低包开了Cmos且本地>1A充电电流))；单包或最低的包快速延迟开DMOS；
  2.有更高的包，且（（其他更高包没开Dmos且（更高包不允许放电或系统有充电电流或更高包充电器在））或(更高包开了但本地有放电电流>1A)）；单包或最高的包快速延迟开CMOS；
  3.当前包不为最低电压，且(有充电电流或最高包有充电器)且总线有低压包能进入充电且总线无放电电流且其他最低包cmos关闭）或不能放电, 延迟判断关DMOS;  
  4.（不是最高包且(是最低包或最低包关Cmos)，且其他高压包能放电，且（本地和系统无充电电流或（本地充电>5A，外部放电>5A）），且其他高压包dmos关闭， 或 不能充电 或 （是最低包且本地有充电电流但外部是放电电流; 或高压包放电且mos开且本地允许，延迟关CMOS

  5.识别到单机模式，需端口电压小于过放电压才允许开放电MOS;
  6.通讯链路异常，端口无电压，尝试开放电MOS 1s建立通讯

  7.本地有放电异常，关DMOS允许信号，增加再开MOS延时，防止本地在允许后进入异常关MOS，但并网允许还是有效，恢复后直接并网产生对充；
  8.本地有充电异常，关CMOS允许信号，增加再开MOS延时，防止本地在允许后进入异常关MOS，但并网允许还是有效，恢复后直接并网产生对充；
  9.低压包充电中识别到充电器移除，关CMOS允许信号，增加再开CMOS延时，防止出现高压包再开DMOS时产生对充
  10.在充电mos不允许情况下判断是不是充电过温引起，当前为最大包则周期允许（无放电电流17s周期，有放电电流7s周期），保证并包时能探测到合适并包点且无充电电流
*/
#define MULTICONTROLDELAY 10  /*100ms后进行并包控制*/
#define VOLOFFER          30  /*300mv偏差*/
#define QVOLOFFER         80  /*退出并包的800mv偏差*/
#define DISONDELAY        20  /*放电MOS开延时*/
#define DISONDELAY1       50  /*总线有更低的包，更低的包cmos没开时的放电MOS开延时*/
#define CHGONDELAY        50  /*充电MOS开延时*/
#define CHGONDELAY1       150 /*总线有更高的包，更高的包dmos没开时的充电MOS开延时*/
#define MTOSDELAYCDON     200 /*双包转单包的充放电MOS开延时需加长*/
#define DISOFFDELAY       150  /*放电MOS关延时*/
#define CHGOFFDELAY       150 /*充电MOS关延时保证电流识别，所以这个地方需要>1s延迟*/
#define CHGOFFOTHERDISCUR -500 /*本地有充电电流但外部其他包是放电电流,达到一定值则关充电MOS，建议按最大允许充电电流设置，10mA*/ 
typedef struct
{
    u8 singlebat:           1;  /*单包模式*/
    u8 isminbat:            1;  /*=1当前是最低包或和最低包压差在0.3v内；=0不是最低的包*/
    u8 ismaxbat:            1;  /*=1当前最高电压包或和最高电压包电压差0.3v内；=0不是最高的包*/
    u8 ischgcur:            1;  /*处于充电中>CHGOUTCUR  ,小于CHGSLEEPCUR退出*/
    u8 otherminbatoffcmos:  1;  /*其他更低电压包未开CMOS*/
    u8 othermaxbatoffdmos:  1;  /*其他更高电压包没开dmos*/ 
    u8 otherminbatenchg:    1;  /*其他更低包允许充电中*/ 
    u8 othermaxbatendis:    1;  /*其他高压包允许放电中*/
    
    u8 otherisdis          :1;  /*其他包放电中>LOADCUR/2,小于DISSLEEPCUR退出*/
    u8 otherismaxbat       :1;  /*其他还有更高的包*/
    u8 otherisminbat       :1;  /*其他还有更低的包*/
    u8 otherbatchgwait     :1;  /*外部有包处于充电等待*/
    u8 othermaxbatovandchgin:1; /*外部最高包过充且有充电器在(不一定比自己高)*/  
    u8 maxchgplugin        :1;  /*最高包有充电器标志*/
    u8                     :2;
    
    u8 qisminbat:           1;  /*q**是用于退出并包判断的标志*/
    u8 qismaxbat:           1;
    u8 qotherminbatoffcmos: 1;
    u8 qothermaxbatoffdmos: 1;
    u8 qotherminbatenchg:   1;  /*其他更低包允许充电中*/ 
    u8 qothermaxbatendis:   1;  /*其他高压包允许放电中*/
    u8 qotherismaxbat:      1;
    u8 qotherisminbat:      1;
    
    u8                     :8;
}strmultiCondition;
typedef union
{
    u32 condition;
    strmultiCondition bit;
}unionmultiCondition; 
void ChgMultclrSmall(void);/*多包并联情况下，检测到其他包处于充电中，需调用该函数清除小电流判断*/
static unionmultiCondition  multiCondition;
void  MtBmsControlJudge(void)
{
    u8 i;
    u8 multipacknum = 0;
    u8 multidison = 0;
    u16 multimaxpackvol,multiminpackvol; /*并包的最大最小电压*/ 
    static u8 multiokDelay,delaydison,delaydisoff,delayhavelvtodison,delaychgon,delaychgoff,delayhavehvtochgon;
    static unionmultiCondition  multiCondition;
    s32 crv,mycrv;
    static u8 mtosDelay;  /*双包转单包的延时计数*/
    static u8 dmosofftoondelay;  /*dmos关闭后再开延迟加长*/
//	static u8 cmosofftoondelay;  /*cmos因为充电器移除关闭后再开延迟加长*/
    
    static u8 lowpackvoldelay; 
	  static u8 KeyFlag = 0;
    MyBmsData.packvol = BatData.VolStack/10;//BatData.VolStack/10;//BatData.VolPack/10; 
    
    MyBmsData.cur = BatData.cur/10; 
    /*
    if((BatData.cur>1000))        
        MyBmsData.cur = BatData.cur/10; 
    else 
        MyBmsData.cur = BatData.cadc/10; */
        
//    if((BatOpaCurGet()+3000<BatData.cadc&&BatOpaCurGet()<-1000)||(BatOpaCurGet()>BatData.cadc+3000&&BatOpaCurGet()>1000))        
//        MyBmsData.cur = BatOpaCurGet()/10; 
//    else 
//        MyBmsData.cur = BatData.cadc/10; 
	MyBmsData.iR = 10;//computeriRAVE;//INTERNAL_RES;  XHT-13S3P:总内阻  20mR [放大5倍]/10
    MyBmsData.MTBmsStatus.bits.chgmos = BatStatus.Bits.AfeChg;
    MyBmsData.MTBmsStatus.bits.dismos = BatStatus.Bits.AfeDsg;
    MyBmsData.MTBmsStatus.bits.dis = BatStatus.Bits.Dis;
    MyBmsData.MTBmsStatus.bits.chg = BatStatus.Bits.Chg;
    MyBmsData.MTBmsStatus.bits.chgplugin = BatStatus.Bits.ChgPlugin;
    MyBmsData.MTBmsStatus.bits.bmserr = (BatStatus.Bytes.DisProt&0XBf)>0||(BatStatus.Bytes.ChgProt&0X7E)>0||BatStatus.Bytes.HardFault>0||BatStatus.Bits.ChgOV2?1:0;
    MyBmsData.MTBmsStatus.bits.chgerr = (BatStatus.Bytes.ChgProt&0X7E)>0||BatStatus.Bytes.HardFault||BatStatus.Bits.ChgOV2?1:0;
    if(BatStatus.Bits.ChgOT||BatStatus.Bits.ChgMOT)
    {
        if((BatStatus.Bytes.ChgProt&0xcf)==0&&BatStatus.Bytes.HardFault==0&&BatStatus.Bits.ChgOV2==0)
            MyBmsData.MTBmsStatus.bits.chgerr = 15;
    }
    MyBmsData.MTBmsStatus.bits.diserr = (BatStatus.Bytes.DisProt&0XBE)>0||BatStatus.Bytes.HardFault?1:0;
    MyBmsData.MTBmsStatus.bits.chgov = BatStatus.Bits.ChgOV||BatStatus.Bits.ChgFC;
    MyBmsData.MTBmsStatus.bits.disuv = BatStatus.Bits.DisUV || BatStatus.Bits.DisDUV;
    MyBmsData.MTBmsStatus.bits.key = (BatStatus.Bits.KeyOn && BatStatus.Bits.Active);
    MyBmsData.MTBmsStatus.bits.id = BatStatus.Bits.DetIn;
    
//    extern u8 syncvolnum;
//    MyBmsData.MTBmsStatus.bits.nc = syncvolnum;
    
    #if 0
    MtBmsDataDelay[1]=10;
    MtBmsData[1].packvol = 4100;
    MtBmsData[1].cur = 0;
    MtBmsData[1].iR = 10;
    MtBmsData[1].MTBmsStatus.byte[0]=0;
    MtBmsData[1].MTBmsStatus.byte[1]=0x3;
    MtBmsData[1].MTBmsStatus.byte[2]=0;
    
    MtBmsDataDelay[0]=10;
    MtBmsData[0].packvol = 4300;
    MtBmsData[0].cur = 0;
    MtBmsData[0].iR = 10;
    MtBmsData[0].MTBmsStatus.byte[0]=0;
    MtBmsData[0].MTBmsStatus.byte[1]=0x3;
    MtBmsData[0].MTBmsStatus.byte[2]=0;
    static u16 delaychange;
    if(delaychange++>500)
    {
        MtBmsData[0].MTBmsStatus.byte[1]=0x0;
    }
    if(delaychange>1000)
        delaychange = 0;
    #endif
    multiCondition.condition&=0x01;
		
		if((MT_OkMark>=0x5a ||BatStatus.Bits.CanErr==0||SingleMosOnMark==0x5a)&&BatStatus.Bits.DetIn)/*如果和detin关联，需要让detin检测时间变长？*/
    {/*并包获取地址成功，或无通讯异常，进入mos控制判断，并包获取地址成功才可以进行开关mos控制,重新广播期间暂停mos控制，维持原状*/
        for(i=0;i<MAX_MT_NUM;i++)
        {
            if(MtBmsDataDelay[i])
            {
                if(MtBmsData[i].MTBmsStatus.bits.key)                        // 新接入包同步开机信号
                {
                    if( KeyFlag != 0x5A /*&& VLOCK_IN() == 0*/) //正在触发按键时不同步
                    {
											KeyFlag = 0x5A;
											BatStatus.Bits.KeyOn = 1;
                      BatStatus.Bits.Active = 1;
                    }
                }
						}
			}
		}

    if((MT_OkMark>=0x5a ||BatStatus.Bits.CanErr==0||SingleMosOnMark==0x5a)&&BatStatus.Bits.DetIn&&BatStatus.Bits.Active)/*如果和detin关联，需要让detin检测时间变长？*/
    {/*并包获取地址成功，或无通讯异常，进入mos控制判断，并包获取地址成功才可以进行开关mos控制,重新广播期间暂停mos控制，维持原状*/
        multipacknum = 0;
        multiminpackvol=0xffff;
        multimaxpackvol=0;
        multimaxcur = -32760;
        multimincur = 32760;
        multitotalcur = 0;
        mycrv = (s32)MyBmsData.cur*MyBmsData.iR/500;
        multiCondition.bit.otherminbatoffcmos=1;
        multiCondition.bit.othermaxbatoffdmos=1;
        multiCondition.bit.qotherminbatoffcmos=1;
        multiCondition.bit.qothermaxbatoffdmos=1;
        for(i=0;i<MAX_MT_NUM;i++)
        {
            if(MtBmsDataDelay[i])
            {
                
                /*并入包数据有效下则可以进行并包判断*/
                if((MtBmsData[i].MTBmsStatus.bits.diserr||MtBmsData[i].MTBmsStatus.bits.bmserr)&&MtBmsData[i].MTBmsStatus.bits.dismos==0)
                   ;
                if(MtBmsData[i].MTBmsStatus.bits.chgplugin&&MtBmsData[i].MTBmsStatus.bits.chgerr==15)
                {/*处于充电过温等待*/ 
                    multiCondition.bit.otherbatchgwait = 1; 
                } 
                /*找最大最小总线电压及最大最小电流*/ 
                crv = (s32)MtBmsData[i].cur*OtherBMSIR[i]/*MtBmsData[i].iR*//500; 
                if(multimaxpackvol<MtBmsData[i].packvol-crv)
                {
                    multimaxpackvol = MtBmsData[i].packvol-crv;  
                    if(MtBmsData[i].MTBmsStatus.bits.chgplugin&&(MtBmsData[i].MTBmsStatus.bits.chgov||MtBmsData[i].MTBmsStatus.bits.dismos==0))
                        multiCondition.bit.othermaxbatovandchgin = 1;
                    else
                        multiCondition.bit.othermaxbatovandchgin = 0;
                    if(MtBmsData[i].MTBmsStatus.bits.chgplugin)
                        multiCondition.bit.maxchgplugin = 1;
                    else
                        multiCondition.bit.maxchgplugin = 0;
                }
                if(multiminpackvol>MtBmsData[i].packvol-crv)
                {
                    multiminpackvol = MtBmsData[i].packvol-crv; 
                }
                /*进入并包300mv*/
                if(MtBmsData[i].packvol-crv+VOLOFFER<MyBmsData.packvol-mycrv)
                {/*判断更低包*/
                    if(MtBmsData[i].MTBmsStatus.bits.chg)/*有更低的包处于充电允许则置位*/
                        multiCondition.bit.otherminbatenchg=1;
                    if(MtBmsData[i].MTBmsStatus.bits.chgmos)/*有更低的包充电mos开，则清零*/
                        multiCondition.bit.otherminbatoffcmos=0;
                    multiCondition.bit.otherisminbat = 1; /*有更低的包在并包*/
                }
                if(MtBmsData[i].packvol-crv>MyBmsData.packvol-mycrv+VOLOFFER)
                {/*判断更高包*/
                    if(MtBmsData[i].MTBmsStatus.bits.dis)/*有更高的包处于放电允许则置位*/
                        multiCondition.bit.othermaxbatendis=1;
                    if(MtBmsData[i].MTBmsStatus.bits.dismos)/*有更高的包放电mos开，则清零*/
                        multiCondition.bit.othermaxbatoffdmos=0;
                    multiCondition.bit.otherismaxbat = 1; /*有更高的包在并包*/
                }
                
								if(MtBmsData[i].MTBmsStatus.bits.dismos)	/*有外部的包已经开启放电Mos*/
								{
               	 	multidison = 1;
               	}
                
                /*退出并包0.8v*/
                if(MtBmsData[i].packvol-crv+QVOLOFFER<MyBmsData.packvol-mycrv)
                {/*判断更低包*/
                    if(MtBmsData[i].MTBmsStatus.bits.chg)/*有更低的包处于充电允许则置位*/
                        multiCondition.bit.qotherminbatenchg=1;
                    if(MtBmsData[i].MTBmsStatus.bits.chgmos)/*有更低的包充电mos开，则清零*/
                        multiCondition.bit.qotherminbatoffcmos=0;
                    multiCondition.bit.qotherisminbat = 1; /*有更低的包在并包*/
                }
                if(MtBmsData[i].packvol-crv>MyBmsData.packvol-mycrv+QVOLOFFER)
                {/*判断更高包*/
                    if(MtBmsData[i].MTBmsStatus.bits.dis)/*有更高的包处于放电允许则置位*/
                        multiCondition.bit.qothermaxbatendis=1;
                    if(MtBmsData[i].MTBmsStatus.bits.dismos)/*有更高的包放电mos开，则清零*/
                        multiCondition.bit.qothermaxbatoffdmos=0;
                    multiCondition.bit.qotherismaxbat = 1; /*有更高的包在并包*/
                }
                
                if(multimaxcur<MtBmsData[i].cur)
                    multimaxcur = MtBmsData[i].cur;
                if(multimincur>MtBmsData[i].cur)
                    multimincur = MtBmsData[i].cur;
                multipacknum++;/*统计并包数量*/
                multitotalcur += MtBmsData[i].cur;
            }
            else 
            { 
            }            
        }
				
        if(multipacknum) /*并包数量*/
        {
            if(multiCondition.bit.maxchgplugin)
            {/*处于充电过温等待且最高包有充电器标志*/
                if(/*multimaxpackvol+VOLOFFER<MyBmsData.packvol-mycrv&&*/MyBmsData.MTBmsStatus.bits.chgplugin==0) 
                    multiCondition.bit.maxchgplugin = 0;
            } 
            if(multiCondition.bit.singlebat)
            {
                if(BatStatus.Bits.AfeDsg==0)/*本地主放处于关闭状态，从单包转多包，需要关闭Cmos，预防被对充*/
                {    
					MtBmsAllow.bit.ChgMosAllow = 0;
				}
				delaychgon = 0;
            }
            multiCondition.bit.singlebat = 0;/*不是单包工作中*/
            /*进入并包按300mv*/
            if(multiminpackvol+VOLOFFER<MyBmsData.packvol-mycrv)
                multiCondition.bit.isminbat=0; /*不是最低电压包*/
            else 
                multiCondition.bit.isminbat=1;
            if(multimaxpackvol>MyBmsData.packvol-mycrv+VOLOFFER)
                multiCondition.bit.ismaxbat=0; /*不是最高电压包*/
            else 
                multiCondition.bit.ismaxbat=1;
            /*退出的判断按800mv*/
            if(multiminpackvol+QVOLOFFER<MyBmsData.packvol-mycrv)
                multiCondition.bit.qisminbat=0; /*不是最低电压包*/
            else 
                multiCondition.bit.qisminbat=1;
            if(multimaxpackvol>MyBmsData.packvol-mycrv+QVOLOFFER)
                multiCondition.bit.qismaxbat=0; /*不是最高电压包*/
            else 
                multiCondition.bit.qismaxbat=1;
            mtosDelay = MTOSDELAYCDON;
            
            /*识别出当前是否是高压包，且统计有多少个，用于休眠时判断如何补偿soc及进入休眠的电流条件*/
            maxnum = 0;
            for(i=0;i<MAX_MT_NUM;i++)
            {
                if(MtBmsDataDelay[i])
                {
                    if(MtBmsData[i].packvol+VOLOFFER>multimaxpackvol)
                    {
                        maxnum ++;
                    }
                }
            }
            if(multiCondition.bit.ismaxbat)
            {
                maxmark = 0x5a; 
                maxnum ++;
            }
            else 
            {
                maxmark = 0xa5;
            }
            totalnum = multipacknum+1;
            if(totalnum>3&&totalnum>=maxnum*2)
                Dis_Sleep_Cur_V = DIS_SLEEP_CUR_MAX;
            else
                Dis_Sleep_Cur_V = DIS_SLEEP_CUR_MIN;
        }
        else 
        {
            multiCondition.bit.singlebat = 1;
            maxmark = 0;
            maxnum = 0;
            totalnum = 1;     
            Dis_Sleep_Cur_V = DIS_SLEEP_CUR_MIN;
        }
        static u8 isdison,ischgon;
        if(multimincur<-DIS_UNLOAD_CUR/20||isdison)
        {
            multiCondition.bit.otherisdis = 1;
            isdison = 1;
        }
        if(multimincur>-Dis_Sleep_Cur_V/10)
        {
            isdison = 0;
            multiCondition.bit.otherisdis = 0;
        }
        if(ChgCurAvg > CHG_OUT_CUR||ischgon)
        {   /*有充电电流*/
            multiCondition.bit.ischgcur = 1;
            ischgon = 1;
        }
        if(ChgCurAvg < CHG_SLEEP_CUR)
        {
            multiCondition.bit.ischgcur = 0;
            ischgon = 0;
        }
        if(MT_OkMark>=0x5a)
        { /*需要播报成功才进行mos控制判断*/

            if(multiokDelay++>=MULTICONTROLDELAY)
            {
	
                multiokDelay = MULTICONTROLDELAY;
                
                // 规则1: 外部有更低压包时,开Dmos的条件： 
                //   条件A: otherisminbat == 1  --> (存在比本包低300mV以上的包)
                //   分支1：低压包关了Cmos,且 [低压包本身不允许开chg,或者没有充电器在]
                //   分支1：低压包开了Cmos,且 本包有>1A充电流 -->需要开Dmos,延时: 500ms
                if(multiCondition.bit.otherisminbat 
                      	&& ( (multiCondition.bit.otherminbatoffcmos 
                      			&& ( multiCondition.bit.otherminbatenchg == 0
										|| multiCondition.bit.maxchgplugin == 0 ) )
						|| (multiCondition.bit.otherminbatoffcmos == 0 
								&& BatData.cur > 1000) ) )
                {//有更低的包，且((其他更低包没开Cmos且(更低包不允许充电或高压包无充电器))或(其他更低包开了Cmos且本地>1A充电电流))
                    delaydison = 0;
                    if((multitotalcur<-100||BatOpaCurGet()>1000)&&multiCondition.bit.otherminbatoffcmos)
                    {/*外部有>1a放电或本地出现>1a充电，且关cmos，则快速打开本地dmos*/
                        dmosofftoondelay = 0;
                        delayhavelvtodison  = DISONDELAY1;
                    }
                    if(dmosofftoondelay)
                    {
                        dmosofftoondelay--;
                    }
                    else 
                    {
	                    if(delayhavelvtodison++>=DISONDELAY1)
	                    {
	                        delayhavelvtodison  = DISONDELAY1;
	                        MtBmsAllow.bit.DisMosAllow = 1;
	                    }
	                }
                }
                else 
                {
                	delayhavelvtodison = 0;
                }

                // 规则2: 本包是最低压包/单包时,开Dmos的条件： 
                //  正常延迟: 200ms 开启DMos
                //  额外情况1: 双包转单包,需要额外+2s; 另外,单包:VolPack>过放总压 && ChgCurAvg<200mA,则不需要等待
                //  额外情况2: Dmos关后重开,需要额外+2s
                
                if(multiCondition.bit.singlebat||multiCondition.bit.isminbat/*||multiCondition.bit.otherminbatoffcmos*/)
                {//识别到当前包为最低或最低接近0.3v内，或单包，延迟判断开DMOS
                    if(mtosDelay&&multiCondition.bit.singlebat)
                    {/*从双包转单包，需要做延迟再判断是否立即开MOS*/
                        mtosDelay--;
                        delaydison  = 0;
                    }
                    if(dmosofftoondelay)
                    {
                        dmosofftoondelay--;
                        delaydison  = 0;
                    }
                    if(delaydison++>=DISONDELAY)			// 200ms
                    {
                        delaydison = DISONDELAY;
                        MtBmsAllow.bit.DisMosAllow = 1;
                    }
                    if(MtBmsAllow.bit.DisMosAllow==0)
                    {/*单机模式需要端口电压<过放电压且无充电电流才开放电,电流不需要？*/
                        if(multiCondition.bit.singlebat&&( BatData.VolPack > DIS_UNDER_VOL_TOTAL && ChgCurAvg<CHG_OUT_CUR))
						{
                            delaydison = 0;
						}
                    }
                }
                else 
				{
					delaydison = 0;
				}
                
                // 规则3: 外部有更高包时,开Cmos的条件： 
				//  条件A: otherismaxbat == 1     -->  (存在比本包高300mV以上的包)
				//   分支1：高压包Dmos关了, 且 [高压包不允许开dis,或总线/系统有充电电流,或者有充电器在]
				//   分支2：高压包Dmos开了, 且 本包有>1A放电流 -->需要开Cmos,延时: 1500ms
				//   特别处理：高压包Dmos关 && 总线充电>1A -->马上开Cmos
                if(multiCondition.bit.otherismaxbat &&
                      ( (multiCondition.bit.othermaxbatoffdmos && 
                           (multiCondition.bit.othermaxbatendis==0 
								|| multitotalcur>CHG_SLEEP_CUR/10 
								|| multiCondition.bit.maxchgplugin))
                        || (multiCondition.bit.othermaxbatoffdmos==0 && BatData.cur<-1000) ) )
                //有更高的包，且（（其他更高包没开Dmos且（更高包不允许放电或系统有充电电流或更高包充电器在））或(更高包开了但本地有放电电流>1A)）        
                {	
                    delaychgon = 0; 
                    if(multiCondition.bit.othermaxbatoffdmos && multitotalcur>100)
                    {/*插入双包充电，高压包关闭放电mos，充电电流>1a，快速开启本地cmos*/
			//			cmosofftoondelay  = 0;
                        delayhavehvtochgon = CHGONDELAY1;
                    }
			//			if(cmosofftoondelay)			// ---> No Use
			//				delayhavehvtochgon = 0; 	/*在识别到高压包要开放电下，需要一直清除该标志*/
                    if(delayhavehvtochgon++>=CHGONDELAY1)
                    {
                        delayhavehvtochgon = CHGONDELAY1;
                        if(MtBmsAllow.bit.ChgMosAllow==0)
                        {
                            delaychgoff = 0;
                        }
                        MtBmsAllow.bit.ChgMosAllow=1; 
                    }
                    
                    if(multitotalcur>CHG_SMALL_CUR/10)
                    {
                        delaychgoff = 0; /*本地不是最高电压外部有充电电流，则关CMOS延迟计数器清零*/
                    }
                    
                    if(BatStatus.Bits.DisSC||BatStatus.Bits.DisOC) /*短路或过流会一直能进来这里随时开cmos，造成恢复时可能低压包提前开了，所以需要清零*/
                    {
                        delayhavehvtochgon = 0;
                    }
                }
                else 
                {
                	delayhavehvtochgon = 0;
                }
                
                // 故障保护后,充电mos尝试开启：
				/*在充电mos不允许情况下判断是不是充电过温引起，外部有放电电流且当前为最大包则延时几秒周期允许，保证并包时不会太大压差*/
                static u16 delayismaxenchgable;
                if(BatStatus.Bits.ChgAble==0)
                {
                    if( (BatStatus.Bits.ChgMOT||BatStatus.Bits.ChgOT||(BatStatus.Bits.ChgOV||BatStatus.Bits.ChgFC)) && BatStatus.Bits.ChgPlugin==0)
                    {
                        if(multitotalcur<CHG_SLEEP_CUR/10 && multiCondition.bit.ismaxbat)
                        {	/*外部没有充电电流*/
                            if(multitotalcur<-DIS_UNLOAD_CUR/10)
                            {
                                if(delayismaxenchgable>500&&delayismaxenchgable<1500)
                                    delayismaxenchgable = 1500;
                            }
                            if(delayismaxenchgable++>1500)
                            {
                                BatStatus.Bits.ChgAble = 1;
                                chgcurlock = 0;
                            }
                            if(delayismaxenchgable>1800)
                                delayismaxenchgable = 0;
                        }
                        else delayismaxenchgable = 0;
                    }
                    else
                    {
						delayismaxenchgable = 0;
                    }
                } 
                
                
                // 规则4: 本包是最高压包/单包时,开Cmos的条件： 
                //  正常延迟: 500ms 开启CMos
                //		本包是最高包 + 放电电流>1A --> 立即开
                //		双包→单包: mtosDelay >0 -->  +2s
                if(multiCondition.bit.singlebat||
                    (multiCondition.bit.ismaxbat && BatStatus.Bits.ChgAble))
                    																//识别到当前包为最高电压或电压接近0.3v内，或单包，延迟判断开CMOS
                {
                    if(multiCondition.bit.ismaxbat && BatStatus.Bits.ChgAble)
                    {																/*当前最高包，且有放电电流，则快速开cmos*/
                        if(BatOpaCurGet()<-1000)									// Mcu采集电流,放电电流>1A, 马上开启
                        {
                            mtosDelay=0;
                            delaychgon = CHGONDELAY;
                        }
                    }
                    if(mtosDelay && multiCondition.bit.singlebat)
                    {																/* 从双包转单包，需要做延迟再判断是否立即开MOS */
                        mtosDelay--;
                        delaychgon  = 0;
                    }
			//		if(cmosofftoondelay)/*识别到充电器移除，需延迟再开允许*/   		// ---> No Use
			//			delaychgon = 0;
                    if(delaychgon++>=CHGONDELAY)
                    {
                        delaychgon = CHGONDELAY;
                        if(MtBmsAllow.bit.ChgMosAllow==0)
                        {
                            delaychgoff = 0;
                        }
                        MtBmsAllow.bit.ChgMosAllow=1; 
                    }
                }
                else 
                {
                	delaychgon = 0;  
                }


				// 规则5: 关DMos条件：
				// 	 不是最低包,有充电器在或有充电电流时,为了给最低包安全开启Cmos,需要关自身Dmos, 延迟: 1.5s
                //   DsgAble==0   --> 不能放电
                if( (multiCondition.bit.isminbat==0 
							&& (multiCondition.bit.ischgcur||multiCondition.bit.maxchgplugin)
							&& multiCondition.bit.otherminbatenchg
							&& multiCondition.bit.otherisdis == 0
							&& multiCondition.bit.otherminbatoffcmos )
                    || (BatStatus.Bits.DsgAble==0) )            
                {											//当前包不为最低电压，且(有充电电流或最高包有充电器)且总线有低压包能进入充电且总线无放电电流且其他最低包cmos关闭）或不能放电
                    if(delaydisoff++>=DISOFFDELAY)
                    {
                        delaydisoff  = DISOFFDELAY;
                        if(MtBmsAllow.bit.DisMosAllow)
                        {
                            delaydison = 0;
                            delayhavelvtodison = 0;
                        }
                        dmosofftoondelay = MTOSDELAYCDON;	/*关闭dmos则非最低包再开需要有延迟*/
                        MtBmsAllow.bit.DisMosAllow = 0;
                    }
                }
                else
                {
					delaydisoff = 0;
				}

             	// 规则6: 关CMos条件:
				// 	 分支1：不是最高包, 无充电电流时,高压包能放电 + 高压包Dmos关，为了给最高包安全开启Dmos,需要关自身Cmos 
				// 	 分支2：ChgAble==0, 且非单包时 --> 关闭CMos
				// 	 分支3：自身检测到充电电流>5A, 且外部总电流为放电电流>5A, 且非单包时 --> 关闭CMos
				// 	 分支4：低压包,自身检测到充电电流,但是总线无充电电流 --> 关闭CMos
				// 	 分支5：[高压在放电，且本包允许充电]其他高压包允许放电中,其他高压包放电mos已开，本包ChgAble==1,且本包无充放电电流
												 
                if( ( multiCondition.bit.ismaxbat==0
							&& multiCondition.bit.othermaxbatendis
							&& multiCondition.bit.othermaxbatoffdmos
							&& multitotalcur< CHG_SLEEP_CUR/10
							&& ChgCurAvg < CHG_SLEEP_CUR )
							
                    || (BatStatus.Bits.ChgAble==0 
							&& multiCondition.bit.singlebat == 0 )
							
                    || (BatData.cur>5000
							&& multitotalcur<CHGOFFOTHERDISCUR
							&& multiCondition.bit.singlebat==0 )
							
                    || (multiCondition.bit.qisminbat
							&& multiCondition.bit.qismaxbat==0
							&& ChgCurAvg>CHG_SMALL_CUR
							&& multitotalcur + (s16)(ChgCurAvg/10)<CHG_SMALL_CUR/10 )
							
                    || (multiCondition.bit.qothermaxbatendis
							&& multiCondition.bit.qothermaxbatoffdmos==0
							&& BatStatus.Bits.ChgAble
							&& DisCurAvg<DIS_UNLOAD_CUR
							&& ChgCurAvg<CHG_SLEEP_CUR)  ) /*高压在放电，且本地允许充电*/
                {
//（不是最高包且(是最低包或最低包关Cmos)，且其他高压包能放电，且（本地和系统无充电电流或（本地充电>5A，外部放电>5A）），且其他高压包dmos关闭， 或 不能充电 或 （是最低包且本地有充电电流但外部是放电电流）或高压在放电，且本地允许充电
                    if(delaychgoff++>=CHGOFFDELAY)
                    {
                    	
//						if(BatData.cur<-5000)
//						{	/*识别到当前放电电流>5a，暂时不关充电mos*/
//							delaychgoff -= CHGOFFDELAY/3;
//						}
//						else
                        {
                            delaychgoff = CHGOFFDELAY;
                            if(MtBmsAllow.bit.ChgMosAllow)
                            {
                                delaychgon = 0;
                                delayhavehvtochgon = 0;
                            }
                            MtBmsAllow.bit.ChgMosAllow=0; 
                        }
                    }
                }
                else 
                {
                	delaychgoff = 0;
                }
                
                /*有充电器接入和移除，清除CMOS允许，且再开cmos需延迟*/
//                static u8 Chgplugmark;
//                if(BatStatus.Bits.ChgPlugin)
//                {
//                    Chgplugmark = 0x5a;
//                }
//                else if(Chgplugmark)
//                {
//                    cmosofftoondelay=250;
//                    Chgplugmark = 0;
//                    MtBmsAllow.bit.ChgMosAllow = 0;
//                }
//                if(cmosofftoondelay)
//                {
//                    cmosofftoondelay --; 
//                } 
            }
        }
        else multiokDelay = 0;
    }
    else/*det移除或(通讯断链失效且地址失效)*/
    {        
        multimaxcur = 0;
        multimincur = 0;
        multitotalcur = 0;
        MtBmsAllow.bit.DisMosAllow=0;
        MtBmsAllow.bit.ChgMosAllow=0;
        multiokDelay = 0;
        multidison = 0;
        if(BatStatus.Bits.CanErr&&BatStatus.Bits.DetIn&&BatStatus.Bits.Active&&delaycheckpacknovolpoweron<200)
        {/*上电2s内通讯不上，且端口电压低则主动开放电MOS1s*/
            delaycheckpacknovolpoweron++;
            if(BatData.VolPack<CELL_NUM*BAT_MIN_VALUE)
            {
                lowpackvoldelay++;
                if(delaycheckpacknovolpoweron<60&&lowpackvoldelay>40)
                {
                    MtBmsAllow.bit.DisMosAllow = 1;
                    delaycheckpacknovolpoweron = 300;
                }
            }
            else lowpackvoldelay = 0;
        } 
        if(powerondelay++>250)
        {/*上电2.5s后禁止主动探测总线*/
            delaycheckpacknovolpoweron = 500;
        }
        if(delaycheckpacknovolpoweron>=300&&delaycheckpacknovolpoweron<=400)
        {
            delaycheckpacknovolpoweron++; 
            MtBmsAllow.bit.DisMosAllow = 1;
            if(BatStatus.Bits.CanErr == 0)
                delaycheckpacknovolpoweron = 500;
        }
    }
    
		if((MtBmsAllow.bit.ChgMosAllow==0||multimaxcur>CHG_SMALL_CUR/10||multitotalcur<-Dis_Sleep_Cur_V/10)&&BatStatus.Bits.Chg)
    {/*本地处于充电中，但外部充电电流大于截止或不具备开充电MOS情况下或外部其他包为放电电流不判断本地充电小电流截止*/
        ChgMultclrSmall();
    }
    /*按键断开后重新接入需要重连*//*充电器插入需要通知对方继续通讯,在MTCanCtrl中判断*/
    static u8 keyoffmark;
    if(BatStatus.Bits.KeyOn==0||BatStatus.Bits.DetIn==0|| BatStatus.Bits.Active==0)
    { 
        MtBmsAllow.bit.DisMosAllow=0;
        MtBmsAllow.bit.ChgMosAllow=0;     
        multiokDelay=0;delaydison=0;delaydisoff=0;delayhavelvtodison=0;delaychgon=0;delaychgoff=0;delayhavehvtochgon=0;             
        keyoffmark = 0x5a;
    }
    else
    {   
        if(keyoffmark)
        {

            keyoffmark = 0;
            #ifndef ADDRDEFINE 
            MyAddr = 0; 
            #endif
            #ifdef ADDRDEFINE
            if(BatStatus.Bits.DetIn)
            {

                MyAddr= GetDetId();/*添加地址信息*/
            }
            #endif
            BroadcastMark=0; 
            NoMainTimer = 0;
            powerondelay = 0;
            delaycheckpacknovolpoweron = 0;
        }
    }
    /*(系统外部有充电过温等待且最高包有充电器在)或（本地识别充电过温且充电器在，而外部最高包有识别到过充或本地为最高包或单包模式）则需要一直通讯*/
    if((multiCondition.bit.otherbatchgwait&&multiCondition.bit.maxchgplugin)
        ||((multiCondition.bit.othermaxbatovandchgin||multiCondition.bit.ismaxbat||multiCondition.bit.singlebat)&&BatStatus.Bits.ChgPlugin&&MyBmsData.MTBmsStatus.bits.chgerr==15))
    {       
        if(BatStatus.Bits.CanErr==0)
            NoMainTimer = 0;
    }
    
    /*主机通讯模式进行充放电MOS开关*/
    #define CANTESTMOSTIME 100
    if(BatStatus.Bits.DetIn&&BatStatus.Bits.Active)
        MtBmsAllow.bit.CanTestMark = 0;
    if( MtBmsAllow.bit.CanTestMark)
    {/*主机发送开CMOS、DMOS开命令，持续1s有效*/
        if(delaycanTestmos ++>CANTESTMOSTIME)
        {
            delaycanTestmos = CANTESTMOSTIME;
            MtBmsAllow.bit.CanTestMark = 0;
        }
        else
        {
            MtBmsAllow.bit.DisMosAllow=1;
            MtBmsAllow.bit.ChgMosAllow=1; 
        }
    }
    
    #ifndef MULTIPLE_PARALLEL
    MtBmsAllow.bit.DisMosAllow=1;
    MtBmsAllow.bit.ChgMosAllow=1;
    #endif
		
		if(BatStatus.Bits.DetIn_Test == 1 && BatStatus.Bits.ChgPlugin && BatStatus.Bits.CanErr)
		{
			MtBmsAllow.bit.DisMosAllow=1;
			MtBmsAllow.bit.ChgMosAllow=1;		
		}
		
#ifdef CIRCLE_TEST     
			MtBmsAllow.bit.DisMosAllow=1;
			MtBmsAllow.bit.ChgMosAllow=1;				
#endif
    
    Mt_Output = 0;
    if(multidison == 1)												// 有外部的包已经开启放电Mos 
    {
	    if((MT_OkMark>=0x5a) && (multiCondition.bit.singlebat == 0))// 并包成功(非单包模式)
		{
			if(BatData.VolPack > DIS_UNDER_VOL_TOTAL)
			{
				Mt_Output = 1;
			}
		}
	}
	

	


	
	Mt_ChgPlugin = 0;
	if((MT_OkMark>=0x5a) && (multiCondition.bit.singlebat == 0))	// 并包成功(非单包模式)
	{
		if(multiCondition.bit.maxchgplugin == 1)					// 外部包有充电器信号
		{
			Mt_ChgPlugin = 1;										// 有外部的包也检测到了充电器 
		}
	}
	
    
#ifdef ADCOMPUTERIR 
void SyncVIManage(void); 
    SyncVIManage();
#endif

void CanErrReCheck(void); /*自动恢复的场景，如果canerr，且端口电压很低，放电异常或故障恢复，需要重新尝试连接*/
    CanErrReCheck();
void CheckCDmosAndCur(void);    
    CheckCDmosAndCur(); /*预防性判断只开充电或只开放电下出现放电电流或充电电流的保护*/
void HandleCompCur(unionmultiCondition  *pmultiCondition);
    HandleCompCur(&multiCondition);/*针对多包并联下放电或充电，移除或故障下的快速切换*/
void CheckSingleMosOnMarkToClr(void);
    CheckSingleMosOnMarkToClr();/*单包开mos标志清除判断*/
}

/*自动恢复的场景，如果canerr，且端口电压很低，放电异常或故障恢复，需要重新尝试连接*/
void CanErrReCheck(void) 
{
    static u8 rewaitmark;
    if(BatStatus.Bits.CanErr&&BatStatus.Bits.DetIn&&BatStatus.Bits.Active)
    {     
        if(BatData.VolPack<CELL_NUM*BAT_MIN_VALUE)
        {
            if(rewaitmark==0X5A)
            {
                rewaitmark = 0;
                #ifndef ADDRDEFINE 
                MyAddr = 0; 
                #endif
                BroadcastMark=0; 
                NoMainTimer = 0;
                powerondelay = 0;
                delaycheckpacknovolpoweron = 0;
            }
            /*有放电异常或故障，恢复则允许重新尝试连接can*/
            if(BatStatus.Bytes.DisProt||BatStatus.Bytes.HardFault)
                rewaitmark = 0xa5;
            if(rewaitmark==0xa5)
            { 
                if(BatStatus.Bytes.DisProt==0&&BatStatus.Bytes.HardFault==0)
                    rewaitmark = 0x5a;
            }
        }
    }
//    if(BatStatus.Bits.CanErr&&BatStatus.Bits.Active==0)
//		{ 
//         powerondelay = 0;
//         delaycheckpacknovolpoweron = 0;
//		}
}

#ifdef ADCOMPUTERIR 
void SyncVIManage(void)
{ 
    #if 1
    static uint16_t mcuVoltage[5];
    static int32_t mcuCurrent[5];
    static u8 adcount;
    static u8 ircount; 
    u8 i;
    u16 resistance,checkresis;
	if(BatData.Valid == 0)
	{
		return;
	}     
    if(BatData.VolMin<IRCOMPUTERVMIN||BatData.VolMax>IRCOMPUTERVMAX)
    {
        adcount = 0;
        return;
    }
	mcuCurrent[adcount] = ADRead(ADC_CCR_AD)*OPA_CUR_GAIN;
	mcuVoltage[adcount] = ADRead(ADC_BAT_VOL)*BAT_VOL_GAIN;
    adcount++;
    if(adcount==2)
    {
        if(mcuCurrent[1]<mcuCurrent[0]+ICHANGEMIN&&mcuCurrent[1]+ICHANGEMIN>mcuCurrent[0])
        { 
        }
        else
        {
            adcount = 1; 
            mcuCurrent[0] = mcuCurrent[1];
            mcuVoltage[0] = mcuVoltage[1];
        }
    }
    else if(adcount == 3)
    { 
        if(mcuCurrent[2]<mcuCurrent[1]+IRCOMPUTERIMIN&&mcuCurrent[2]+IRCOMPUTERIMIN>mcuCurrent[1])
        {
            mcuCurrent[0] = mcuCurrent[2];
            mcuVoltage[0] = mcuVoltage[2];
            adcount = 1; 
        } 
    }
    else if(adcount == 4)
    {
        if(mcuCurrent[3]<mcuCurrent[2]+ICHANGEMIN&&mcuCurrent[3]+ICHANGEMIN>mcuCurrent[2])
        { 
            resistance = ((u32)abs(mcuVoltage[1]- mcuVoltage[2]))*1000/abs(mcuCurrent[1]-mcuCurrent[2]);
            resistance/=2;
            if(resistance < INTERNAL_RES_MAX&&resistance > INTERNAL_RES_MIN)
            {
                if(ircount>=CELL_NUM)
                    ircount = 0; 
                checkresis = DataFlashPowerDownSave.BATIR[CELL_NUM-1]; 
                if(ircount!=0)
                    checkresis = DataFlashPowerDownSave.BATIR[ircount-1]; 
                if(checkresis>=resistance+IRCHANGE)
                    resistance= checkresis-IRCHANGE;
                else if(checkresis+IRCHANGE<=resistance)
                    resistance = checkresis+IRCHANGE;
                DataFlashPowerDownSave.BATIR[ircount] = resistance; 
                ircount ++;
                computeriRAVE = 0;
                for(i=0;i<CELL_NUM;i++)
                    computeriRAVE += DataFlashPowerDownSave.BATIR[i];
                computeriRAVE /= CELL_NUM;
            }
            mcuCurrent[0] = mcuCurrent[3];
            mcuVoltage[0] = mcuVoltage[3];
            adcount = 1;
        } 
    }
    else if(adcount == 5)
    {
        if(mcuCurrent[4]<mcuCurrent[3]+ICHANGEMIN&&mcuCurrent[4]+ICHANGEMIN>mcuCurrent[3])
        { 
            resistance = ((u32)abs(mcuVoltage[1]- mcuVoltage[3]))*1000/abs(mcuCurrent[1]-mcuCurrent[3]);
            resistance/=2;
            if(resistance < INTERNAL_RES_MAX&&resistance > INTERNAL_RES_MIN)
            {
                if(ircount>=CELL_NUM)
                    ircount = 0;
                checkresis = DataFlashPowerDownSave.BATIR[CELL_NUM-1]; 
                if(ircount!=0)
                    checkresis = DataFlashPowerDownSave.BATIR[ircount-1];  
                if(checkresis>=resistance+IRCHANGE)
                    resistance= checkresis-IRCHANGE;
                else if(checkresis+IRCHANGE<=resistance)
                    resistance = checkresis+IRCHANGE;
                DataFlashPowerDownSave.BATIR[ircount] = resistance; 
                ircount ++;
                computeriRAVE = 0;
                for(i=0;i<CELL_NUM;i++)
                    computeriRAVE += DataFlashPowerDownSave.BATIR[i];
                computeriRAVE /= CELL_NUM;
            }
        } 
        mcuCurrent[0] = mcuCurrent[4];
        mcuVoltage[0] = mcuVoltage[4];
        adcount = 1;
    }
    #else
    static uint16_t mcuVoltage[2];
    static int32_t mcuCurrent[2];
	static int32_t currLast = 0; 
	static uint16_t mcuVolt = 0;
	static int32_t mcuCurr = 0;
	static uint8_t voltIndex = 10;
	static uint8_t storeIndex = 5;
	static uint16_t voltBuffer[5];
	static uint16_t pointVoltage[2];
	static int32_t pointCurrent[2];
    static u8 ircount; 
	int32_t current ;
	uint32_t voltage;
	int32_t delta;
    u8 i;

	if(BatData.Valid == 0)
	{
		return;
	}
    if(currLast==0)
    {
        currLast = ADRead(ADC_CCR_AD)*OPA_CUR_GAIN;
    }
	current = ADRead(ADC_CCR_AD)*OPA_CUR_GAIN;
	voltage = ADRead(ADC_BAT_VOL)*BAT_VOL_GAIN;
	delta = current - currLast;

	voltIndex += 1;
	if(voltIndex >= 5)
	{
		voltIndex = 0;
	}
	voltBuffer[voltIndex] = voltage;
	if(voltIndex == storeIndex)
	{
		storeIndex = 5;
		mcuVoltage[0] = pointVoltage[0];
		mcuCurrent[0] = pointCurrent[0];
		mcuVoltage[1] = voltage;
		mcuCurrent[1] = pointCurrent[1];
        if(BatData.VolMin>IRCOMPUTERVMIN&&BatData.VolMax<IRCOMPUTERVMAX)
        {
            u16 resistance = ((u32)abs(mcuVoltage[1]- mcuVoltage[0]))*1000/abs(mcuCurrent[1]-mcuCurrent[0]);
            resistance/=2;
            if(resistance < INTERNAL_RES_MAX&&resistance > INTERNAL_RES_MIN)
            {
                if(ircount>=CELL_NUM)
                    ircount = 0;
                DataFlashPowerDownSave.BATIR[ircount] = resistance;
                ircount ++;
                computeriRAVE = 0;
                for(i=0;i<CELL_NUM;i++)
                    computeriRAVE += DataFlashPowerDownSave.BATIR[i];
                computeriRAVE /= CELL_NUM;
            }
        }
	}

	if(delta > IRCOMPUTERIMIN || delta < -IRCOMPUTERIMIN)
	{ 
		storeIndex = (voltIndex + 2) % 5;
		pointVoltage[0] = mcuVolt;
		pointCurrent[0] = mcuCurr;
		pointVoltage[1] = voltage;
		pointCurrent[1] = current;
	}
	else
	{ 
		mcuVolt = voltBuffer[(voltIndex + (5-2)) % 5];
		mcuCurr = current;
	}

	currLast = current;
    #endif
    
}
void  BatIRInit(void)
{
    u8 i;
    computeriRAVE = 0;
    for(i=0;i<CELL_NUM;i++)
    {
        if(DataFlashPowerDownSave.BATIR[i]> INTERNAL_RES_MAX||DataFlashPowerDownSave.BATIR[i]<INTERNAL_RES_MIN)
            DataFlashPowerDownSave.BATIR[i] = INTERNAL_RES;
        computeriRAVE += DataFlashPowerDownSave.BATIR[i];
    }
    computeriRAVE /= CELL_NUM;    
}
#endif

void MultiCanHold(void)
{
    NoMainTimer = 0;
}

u8 MultiCanAddr(void)
{
		//return 1;
    return (MyAddr);
}

/*开充电mos没开放电mos，但充电电流>2A，持续10s则报错*/
/*开放电mos没开充电mos，但放电电流>2A，持续10s则报错*/
void CheckCDmosAndCur(void)
{
    static u16 nodmosdelay,nocmosdelay;    
    if(BatStatus.Bits.AfeChg&&(MtBmsAllow.bit.DisMosAllow == 0||BatStatus.Bits.AfeDsg==0))
    {
        if(ChgCurAvg>2000)
        {
            nodmosdelay++;
            if(nodmosdelay> 1000)
            {
                BatStatus.Bits.CMosErr = 1;
            }
        }
        else
            nodmosdelay = 0;
        nocmosdelay = 0;
    }
    else if(BatStatus.Bits.AfeDsg&&(MtBmsAllow.bit.ChgMosAllow == 0||BatStatus.Bits.AfeChg==0))
    {
        if(DisCurAvg>2000)
        {
            nocmosdelay++;
            if(nocmosdelay> 1000)
            {
                BatStatus.Bits.DMosErr = 1;
            }
        }
        else
            nocmosdelay = 0;
        nodmosdelay = 0;
    }
    else
    {
        nodmosdelay = 0;
        nocmosdelay = 0;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*增加比较器和电流判断用于并包放电或充电移除高压包或低压包的快速切换*/
void HwCompInit(void)
{  
    /*比较器配置过滤和中断*/
    COMP_InitType COMP_Initial;    /*Initial comp*/
    COMP_StructInit(&COMP_Initial);  

    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_COMP| RCC_APB1_PERIPH_COMP_FILT, ENABLE);
    COMP_SetRefScl(63, 1,  3, 0);			// enable Vref(6bit:64)

    COMP_Initial.InpSel     = COMP2_CTRL_INPSEL_PA6;  			// brush BC+  
    COMP_Initial.InmSel     = COMP2_CTRL_INMSEL_VREF_VC2;    
    
//    COMP_Initial.SampWindow = 31;  /*SampW>Thresh SampW<2*Thresh*/  
//    COMP_Initial.Thresh     = 28;  /* 28->28us */
//    COMP_Initial.ClkPsc = 47;   /*47->1M*/
    
    COMP_Initial.SampWindow = 20;  /*SampW>Thresh SampW<2*Thresh*/   
    COMP_Initial.Thresh     = 19;  /* 19->100us  */
    COMP_Initial.ClkPsc = 1919; //239;   /*239->200k*/
    
    COMP_Initial.FilterEn = true;    
    COMP_Init(COMP2, &COMP_Initial); 
    
    
	EXTI_InitType EXTI_InitStructure;
	EXTI_ClrITPendBit(EXTI_LINE22);
	EXTI_InitStructure.EXTI_Line = EXTI_LINE22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitPeripheral(&EXTI_InitStructure);   
    
    COMP_Enable(COMP2, ENABLE);
    Delay1ms(1);  /*和滤波窗口有关系,需>滤波窗口*/ 
    COMP->INTSTS=0;  
     
}
void CompVolSet(bool up1low0) /*up1low0 =1超上限判断 =0 进行超下限判断*/
{/*250mv压差100us过滤，使用100mv压差，在电子负载带0.1a接入会引起触发*/
    u32 ab;
    ab = DataFlashAtOnceSave.OpaBias;
    if(up1low0)
    {
        ab += 8*25*4096/3300;
        COMP->Cmp2.CTRL &= (~COMP_POL_MASK);
    }
    else
    { 
        ab -= 6*25*4096/3300;
        COMP->Cmp2.CTRL |= COMP_POL_MASK;
    }
    ab *=3300;
    ab /=4096;
    ab *=64;
    ab /= 3300;
    COMP_SetRefScl(ab, 1,  3, 0);			// enable Vref(6bit:64)  
    Delay10us(120);  /*和滤波窗口有关系,需>滤波窗口*/ 
    COMP->INTSTS=0;        
} 
void CompEn(bool en) /*比较器中断使能*/
{    
    if(en)
    {   
        COMP_SetIntEn(2);		// IT enable comp2 
    }
    else
    {
        COMP_SetIntEn(0);		// IT enable comp2   
    }
}
static u8 Compdiscase,Compchgcase;
void CompIntCallBack(void)
{
    if(Compdiscase)
    {
        Compdiscase ++;
        if(Compdiscase==2)
        {
            MtBmsAllow.bit.QUICKChgMos = 1;  
            AfeTimerCallBack(1);
        }
        else if(Compdiscase==4)
        {
            MtBmsAllow.bit.QUICKChgMos = 0;  
            AfeTimerCallBack(1);
        }
    }
    if(BatStatus.Bits.DisSC)
    { 
        MtBmsAllow.bit.QUICKChgMos=0; 
    }
    if(Compchgcase)
    {
        if(Compchgcase==2)
        {
            if(BatStatus.Bits.DisSC==0)
            MtBmsAllow.bit.QUICKDisMos = 1;            
            AFE_EFET_ON();
        }
        else if(Compchgcase == 4)
        {
            MtBmsAllow.bit.QUICKDisMos = 0;
            AFE_EFET_OFF();
        }
        Compchgcase ++;
    }
    CompEn(0);
}
/*在并包放电情况下，低压包只开放电mos，识别到大电流放电快速打开cmos，延时5s或识别到慢速开启清除快速标志
  在并包充电情况下，高压包只开充电mos，识别到大电流充电快速打开dmos，延时5s或识别到慢速开启清除快速标志*/
void HandleCompCur(unionmultiCondition  *pmultiCondition)
{     
    static u8 delayreadyquickcmos,delayclrquickcmos;
    static u8 delaycurquickcmos;
    static u16 delaytimeclrqcmos;
    if(pmultiCondition->bit.singlebat==0&&pmultiCondition->bit.ismaxbat==0&&BatStatus.Bits.AfeDsg
        &&BatStatus.Bits.AfeChg==0&&Compdiscase==0)
    {/*放电下识别到并包且不是高压包,充电mos没开，则需要时刻准备快速切换*/
        if(multitotalcur<-1000)
            delayreadyquickcmos+=5;
        if(delayreadyquickcmos++>250)/*需要有足够长时间识别到处于多包状态，且本地没开充电mos*/
        {
            delayreadyquickcmos = 250; 
            Compdiscase = 1; 
            
            Compchgcase = 0;
            MtBmsAllow.bit.QUICKDisMos = 0;
        }
    }
    else delayreadyquickcmos=0; 
    if(Compdiscase)
    {
        if((MtBmsAllow.bit.ChgMosAllow&&BatStatus.Bits.ChgAble)
            ||BatStatus.Bits.AfeDsg==0)   
        {/*触发了快开，则等待慢开启动或放电mos关闭了，则关闭快开，或 触发充电电流结束(中断处理) */ 
            if(delayclrquickcmos++>100)
            {
                delayclrquickcmos = 100;
                if(MtBmsAllow.bit.QUICKChgMos)
                {
                    MtBmsAllow.bit.QUICKChgMos = 0;
                } 
                Compdiscase  = 0;
            }
        }
        else  delayclrquickcmos = 0;   
        if(MtBmsAllow.bit.QUICKChgMos)
        {/*快开5s后延时退出快开*/
            if(delaytimeclrqcmos++>500)
            {
                MtBmsAllow.bit.QUICKChgMos = 0;
                Compdiscase  = 0;
            }
        }
        else
        {
            delaytimeclrqcmos = 0;
        }
    }
    else
    {
        delaytimeclrqcmos = 0;
        delayclrquickcmos = 0;
    }
    if(Compdiscase == 2||Compdiscase == 4)
    {/*没触发比较器的，则进行电流判断的*/
        if(BatOpaCurGet()<-1000)
        {
            if(delaycurquickcmos++>2)
            {
                delaycurquickcmos = 100;
                MtBmsAllow.bit.QUICKChgMos = 1;
            }
        }
        else delaycurquickcmos= 0;
        if(BatOpaCurGet()>1000&&MtBmsAllow.bit.QUICKChgMos)
        {
            MtBmsAllow.bit.QUICKChgMos = 0;
        }
    }
    switch(Compdiscase)
    {
        case 0:
            if(Compchgcase==0)
            CompEn(0); 
            break;
        case 1:/*放电下开放电电流触发*/
            CompEn(0); 
            CompVolSet(1);
            CompEn(1);
            Compdiscase ++;
            break;
        case 2:
            break;
        case 3:  /*触发成功，立即开启cmos，设置有充电电流则快关cmos*/
            MtBmsAllow.bit.QUICKChgMos=1;  
            CompEn(0);  
            CompVolSet(0);
            CompEn(1);
            Compdiscase ++;
            break;
        case 4: 
            MtBmsAllow.bit.QUICKChgMos=1; 
            break;
        case 5: /*触发了充电电流，立即取消允许cmos*/
            CompEn(0); 
            MtBmsAllow.bit.QUICKChgMos=0; 
            Compdiscase  = 0;
            break;
        default:
            CompEn(0); 
            MtBmsAllow.bit.QUICKChgMos=0; 
            Compdiscase  = 0;
            break;
    }
    
    
    
    
    
#if 1    
    
    static u8 delayreadyquickdmos,delayclrquickdmos;
    static u8 delaycurquickdmos;
    static u16 delaytimeclrqdmos;
    if(pmultiCondition->bit.singlebat==0&&pmultiCondition->bit.isminbat==0&&BatStatus.Bits.AfeChg
        &&BatStatus.Bits.AfeDsg==0&&Compchgcase==0)
    {/*充电下识别到并包且不是最低包,充电mos没开，则需要时刻准备快速切换*/        
        if(multitotalcur>1000)
            delayreadyquickcmos+=5;
        if(delayreadyquickdmos++>250)/*需要有足够长时间识别到处于多包状态，且本地没开放电mos*/
        {
            delayreadyquickdmos = 250; 
            Compchgcase = 1;
            Compdiscase = 0;
            MtBmsAllow.bit.QUICKChgMos = 0;
        }
    }
    else delayreadyquickdmos=0; 
    if(Compchgcase)
    {
        if((MtBmsAllow.bit.DisMosAllow&&BatStatus.Bits.DsgAble)
            ||BatStatus.Bits.AfeChg==0)   
        {/*触发了快开，则等待慢开启动或充电mos关闭,则关闭快开，或 触发放电电流结束(中断处理) */
            if(delayclrquickdmos++>100)
            {
                delayclrquickdmos = 100;
                if(MtBmsAllow.bit.QUICKDisMos)
                {
                    MtBmsAllow.bit.QUICKDisMos = 0;
                } 
                Compchgcase  = 0;
            }
        }
        else  delayclrquickdmos = 0; 
        if(MtBmsAllow.bit.QUICKDisMos)
        {/*快开5s后延时退出快开*/
            if(delaytimeclrqdmos++>500)
            {
                MtBmsAllow.bit.QUICKDisMos = 0;
                Compchgcase  = 0;
            }
        }
        else
        {
            delaytimeclrqdmos = 0;
        }  
    }
    if(Compchgcase == 2||Compchgcase == 4)
    {/*没触发比较器的，则进行电流判断的*/
        if(BatOpaCurGet()>1000)
        {
            if(delaycurquickdmos++>2)
            {
                delaycurquickdmos = 100;
                MtBmsAllow.bit.QUICKDisMos = 1;
            }
        }
        else delaycurquickdmos= 0;
        if(BatOpaCurGet()<-1000&&MtBmsAllow.bit.QUICKDisMos)
        {
            MtBmsAllow.bit.QUICKDisMos = 0;
        }
    }
    
    switch(Compchgcase)
    {
        case 0:
            if(Compdiscase==0)
            CompEn(0); 
            break;
        case 1:/*充电下开充电电流触发*/
            CompEn(0); 
            CompVolSet(0);
            CompEn(1);
            Compchgcase ++;
            break;
        case 2:
            break;
        case 3:  /*触发成功，立即开启dmos，设置有放电电流则快关dmos*/
            MtBmsAllow.bit.QUICKDisMos=1; 
            CompEn(0);   
            CompVolSet(1);
            CompEn(1);
            Compchgcase ++;
            break;
        case 4: 
            MtBmsAllow.bit.QUICKDisMos=1; 
            break;
        case 5: /*触发了放电电流，立即取消允许dmos*/
            CompEn(0); 
            MtBmsAllow.bit.QUICKDisMos=0; 
            Compchgcase  = 0;
            break;
        default:
            CompEn(0); 
            MtBmsAllow.bit.QUICKDisMos=0; 
            Compchgcase  = 0;
            break;
    }
    
#endif
    if(BatStatus.Bits.DisSC)
    {
        MtBmsAllow.bit.QUICKDisMos=0; 
        MtBmsAllow.bit.QUICKChgMos=0; 
    }
    
}


/*识别到mos变化，立即更新mos状态，并主动发送出去，加速对方响应*/
void MosStatusFlush(void);
void fetchangeflushstatus(u8 fet)
{
    CanTxMessage TxMsg;
    static u8 fetbak;
    u8 i;
    if(fetbak!=fet)
    { 
        for(i=0;i<20;i++)
        {
            MosStatusFlush();
            if((fet&3)==3&&BatStatus.Bits.AfeChg&&BatStatus.Bits.AfeDsg)
                break;
            if((fet&3)==2&&BatStatus.Bits.AfeChg&&BatStatus.Bits.AfeDsg==0)
                break;
            if((fet&3)==1&&BatStatus.Bits.AfeChg==0&&BatStatus.Bits.AfeDsg)
                break;
            if((fet&3)==0&&BatStatus.Bits.AfeChg==0&&BatStatus.Bits.AfeDsg==0)
                break;
            Delay10us(20);
        }
        fetbak = fet;
        MyBmsData.MTBmsStatus.bits.chgmos = BatStatus.Bits.AfeChg;
        MyBmsData.MTBmsStatus.bits.dismos = BatStatus.Bits.AfeDsg;        
        if(MT_OkMark>=0x5a)
        {
            MTCanTxMessageInit(&TxMsg);  
            memcpy(TxMsg.Data,(u8*)&MyBmsData,8); 
            TxMsg.DLC = 8;
        	queue_push_front(AndTxQueue, &TxMsg); 
			//queue_push_back(AndTxQueue, &TxMsg);
        	queue_push_front(AndTxQueue, &TxMsg); 
        	//queue_push_back(AndTxQueue, &TxMsg);
        }
    }
}

u8 ReadMtOutputState(void)
{
	return Mt_Output;
}

u8 ReadMtChgPluginState(void)
{
	return Mt_ChgPlugin;
}
/*增加针对单包且mos开启，休眠后再唤醒，置位一个标志SingleMosOnMark，当canerr置位不影响正常工作，直到id移除或单机标志解除或有电流*/
/*当前单包且休眠前dmos处于开启,则自主唤醒不识别canerr  SingleMosOnMark缓冲在Batstatusbakcheck[3];*/
u8 SingleAndMosOnCheck(void)
{
    if(BatStatus.Bits.AfeDsg&&multiCondition.bit.singlebat&&BatStatus.Bits.DetIn)
    {
        return 0x5a;
    }
    return 0;
}

static u32 ecu_sleep_timeout = 0;
/*移除或有电流或非单包或放电mos关闭则清除该标志*/
void CheckSingleMosOnMarkToClr(void)
{
    static u8 delayclr;
    if(BatStatus.Bits.DetIn==0||DisCurAvg>1000||ChgCurAvg>CHG_OUT_CUR/*||multiCondition.bit.singlebat==0*/||BatStatus.Bits.AfeDsg==0)
    {
        if(delayclr++>150)
            SingleMosOnMark = 0;
    }
    else delayclr=0;
		
		if(SingleMosOnMark == 0x5a)
		{
			if(ecu_sleep_timeout++ > 5*60*100)
			{
				SingleMosOnMark = 0;
				ecu_sleep_timeout = 0;
			}
		}
}

void SetSingleMosOnState(void)
{
	SingleMosOnMark=0x5a;
	ecu_sleep_timeout = 0;
}

void ClearSingleMosOnState(void)
{
	SingleMosOnMark = 0;
	ecu_sleep_timeout = 0;
}

