/*****************************************************************************
 文 件 名 : RGB.c
 功能描述 : WS2812/GRB RGB 灯驱动 (4 颗灯珠, 每颗 24bit GRB)
            数据脚 PA1 复用为 TIM2_CH2, PWM + DMA 方式输出
            逻辑0 = 25% 占空比, 逻辑1 = 50% 占空比, 起始/结束位 = 0
 移植说明 : 由原参考工程(48MHz / TIM1_CH1 / DMA_CH1) 移植到本工程
            (主频 144MHz, TIM2_CH2 / DMA1_CH3)
 *****************************************************************************/
#include "McuHal.h"
#include "User.h"
#include "soc.h"
#include "RGB.h"

/* ==================== 常量定义 ==================== */
#define RGB_NUM        4                            /* LED 颗数 */
#define RGB_SUM        24                           /* 每颗 LED 24 bit */
#define RGB_ALL_SUM    (RGB_NUM * RGB_SUM + RGB_SUM) /* 数据 + 复位间隙 */

/* 测试模式: 1 = 测试效果; 0 = 正常状态机(正式版本) */
#define RGB_TEST_MODE  0

/* 上电稳定延时: 上电瞬间 AFE/ADC 采样未就绪, 状态位会误判(如温度=0被当成低温),
 * 延时结束后才开始状态判定, 避免开机时出现不必要的红闪/黄闪 */
#define POWER_ON_STABLE_TICKS   300   /* 300 * 10ms = 3s */

/* TIM2 挂在 APB1: APB1 = 144M/4 = 36MHz, 定时器时钟 = APB1*2 = 72MHz
 * 72MHz / 800kHz = 90 => ARR = 89 */
#define RGB100D   90    /* 100% 占空比 */
#define RGB75D    67    /* 75%  占空比 */
#define RGB50D    45    /* 50%  占空比 */
#define RGB25D    22    /* 25%  占空比 */
#define RGB0D     0     /* 0%   占空比 */

#define RGB_0B     RGB25D   /* 逻辑0 */
#define RGB_1B     RGB50D   /* 逻辑1 */
#define RGB_STARTB RGB0D    /* 起始位 */
#define RGB_STOPB  RGB0D    /* 结束位 */

/* ==================== 颜色定义 (RGB 各 8bit) ==================== */
typedef struct
{
    u8 r;
    u8 g;
    u8 b;
} RGB_COLOR;

static const RGB_COLOR CLR_RED    = {255,   0,   0};
static const RGB_COLOR CLR_GREEN  = {  0, 255,   0};
static const RGB_COLOR CLR_YELLOW = {255, 255,   0};
static const RGB_COLOR CLR_BLACK  = {  0,   0,   0};

/* ==================== DMA 发送缓冲 ==================== */
static u16 RGB_Buffer[RGB_ALL_SUM];

/* ==================== 内部函数 ==================== */

/* 清空缓冲: 灯数据区填充逻辑0(灭), 尾部复位间隙填充低电平 */
static void RGB_Buffer_Int(void)
{
    u16 i;
    for (i = 0; i < RGB_NUM * RGB_SUM; i++)
    {
        RGB_Buffer[i] = RGB_0B;          /* 灭 = 24bit 逻辑0 */
    }
    for (i = RGB_NUM * RGB_SUM; i < RGB_ALL_SUM; i++)
    {
        RGB_Buffer[i] = RGB_STOPB;       /* 复位间隙 = 低电平 */
    }
}

/* 亮度缩放: brightness 0~100 */
static void Set_Brightness(RGB_COLOR* c, u8 brightness)
{
    c->r = (u8)((u16)c->r * brightness / 100);
    c->g = (u8)((u16)c->g * brightness / 100);
    c->b = (u8)((u16)c->b * brightness / 100);
}

/* 将单颗灯颜色写入 GRB 位流 (idx: 0 ~ RGB_NUM-1) */
static void RGBSetPixel(u8 idx, const RGB_COLOR* c)
{
    u32 grb = ((u32)c->g << 16) | ((u32)c->r << 8) | (u32)c->b;
    u16* p  = &RGB_Buffer[idx * RGB_SUM];
    u8  i;
    for (i = 0; i < RGB_SUM; i++)
    {
        p[i] = (grb & (0x800000UL >> i)) ? RGB_1B : RGB_0B;
    }
}

/* 启动一帧 DMA 发送 */
static void RGBStartTransfer(void)
{
    TIM_EnableDma(TIM2, TIM_DMA_UPDATE, ENABLE);
}

/* ==================== 对外接口 ==================== */

/* 更新单颗灯颜色并发送 */
void NewRGBOneUpdata(u8 idx, u8 r, u8 g, u8 b)
{
    RGB_COLOR c;
    if (idx >= RGB_NUM)
    {
        return;
    }
    c.r = r;
    c.g = g;
    c.b = b;
    RGBSetPixel(idx, &c);
    RGBStartTransfer();
}

/* 全部灯同色并发送 */
static void RGBAllUpdata(const RGB_COLOR* c)
{
    u8 i;
    for (i = 0; i < RGB_NUM; i++)
    {
        RGBSetPixel(i, c);
    }
    RGBStartTransfer();
}

/* 全灭 */
void AllLedOff(void)
{
    RGBAllUpdata(&CLR_BLACK);
}

/* 故障: 全红 */
void RGBError(void)
{
    RGBAllUpdata(&CLR_RED);
}

/* 告警: 全黄 */
void RGBWarning(void)
{
    RGBAllUpdata(&CLR_YELLOW);
}

/* 低电量: 全红 */
void RGBLowBattery(void)
{
    RGBAllUpdata(&CLR_RED);
}

/* 正常/满充: 全绿 */
void RGBAllGreen(void)
{
    RGBAllUpdata(&CLR_GREEN);
}

/* 按 SOC 点亮绿灯, 4 档电量: 0-25%=1颗, 25-50%=2颗, 50-75%=3颗, 75-100%=4颗 */
void RGBSocDisplay(u8 soc)
{
    u8 nLed;
    u8 i;

    if (soc >= 75)      nLed = 4;
    else if (soc >= 50) nLed = 3;
    else if (soc >= 25) nLed = 2;
    else                nLed = 1;

    RGB_Buffer_Int();
    for (i = 0; i < RGB_NUM; i++)
    {
        if (i < nLed)
        {
            RGBSetPixel(i, &CLR_GREEN);
        }
        else
        {
            RGBSetPixel(i, &CLR_BLACK);
        }
    }
    RGBStartTransfer();
}

/* ==================== 闪烁 / 呼吸效果 ==================== */

#if !RGB_TEST_MODE
/* 全灯快闪: 2Hz (周期500ms, 亮250ms灭250ms) */
static void RGBFastBlink(const RGB_COLOR* c, u16 tick)
{
    if ((tick % 50) < 25)
    {
        RGBAllUpdata(c);
    }
    else
    {
        AllLedOff();
    }
}

/* 全灯慢闪: 0.5Hz (周期2s, 亮1s灭1s) */
static void RGBSlowBlink(const RGB_COLOR* c, u16 tick)
{
    if ((tick % 200) < 100)
    {
        RGBAllUpdata(c);
    }
    else
    {
        AllLedOff();
    }
}

/* 单颗灯快闪: 2Hz (第1颗灯) */
static void RGBSingleFastBlink(const RGB_COLOR* c, u16 tick)
{
    RGB_Buffer_Int();
    if ((tick % 50) < 25)
    {
        RGBSetPixel(0, c);
    }
    RGBStartTransfer();
}
#endif

/* 充电呼吸流水: 从左到右每颗灯依次呼吸(0->100->0) */
static void RGBChargeBreath(u16 tick)
{
    u8 idx   = (u8)((tick / 50) % RGB_NUM);   /* 每500ms切换一颗 */
    u8 phase = (u8)(tick % 50);               /* 0~49 */
    u8 brightness;
    RGB_COLOR c = CLR_YELLOW;

    if (phase < 25)
    {
        brightness = (u8)(phase * 4);         /* 0 -> 100 */
    }
    else
    {
        brightness = (u8)((50 - phase) * 4);  /* 100 -> 0 */
    }
    Set_Brightness(&c, brightness);

    RGB_Buffer_Int();
    RGBSetPixel(idx, &c);
    RGBStartTransfer();
}

#if RGB_TEST_MODE
/* 测试: 红绿交替闪烁, 4 颗灯同色, 每500ms切换一次颜色 */
static void RGBRedGreenBlink(u16 tick)
{
    RGB_COLOR c = ((tick % 100) < 50) ? CLR_RED : CLR_GREEN;  /* 红绿交替 */
    u8 i;

    RGB_Buffer_Int();
    for (i = 0; i < RGB_NUM; i++)
    {
        RGBSetPixel(i, &c);
    }
    RGBStartTransfer();
}
#endif

/* ==================== 10ms 周期状态机 ==================== */
u8 gLedTrigger = LED_OFF;   /* 当前 LED 触发状态 (供串口调试) */

void RGBPwmUp(void)
{
    static u16 tick = 0;

    tick++;

#if RGB_TEST_MODE
    /* 测试模式: 上电即显示红绿交替闪烁 */
    RGBRedGreenBlink(tick);
#else
    static u16 stableTick = 0;                /* 上电稳定计数 */
    u8  soc  = SocInfo.SocS;
    u16 diff;
    static u8 LowTempAlarm = 0;               /* 低温报警状态(带2℃滞回) */

    /* 上电稳定延时: 采样数据未就绪期间全灭, 不进入状态判定 */
    if (stableTick < POWER_ON_STABLE_TICKS)
    {
        stableTick++;
        gLedTrigger = LED_OFF;
        AllLedOff();
        return;
    }

    /* 1. 过充/过放/短路/过流保护 -> 红色快闪 2Hz */
    if (BatStatus.Bits.ChgOV  || BatStatus.Bits.ChgOV2 ||
        BatStatus.Bits.DisUV  || BatStatus.Bits.DisDUV ||
        BatStatus.Bits.DisSC  ||
        BatStatus.Bits.DisOC  || BatStatus.Bits.DisOCL ||
        BatStatus.Bits.ChgOC  ||
        BatStatus.Bits.ChgOT  || BatStatus.Bits.ChgMOT)   /* 充电异常(过温/过流) */
    {
        gLedTrigger = LED_PROTECT;
        RGBFastBlink(&CLR_RED, tick);
        return;
    }

    /* 2. 温度异常 (放电过温>60℃ 或 低温<0℃) -> 红色慢闪 0.5Hz
     * 低温判断带 2℃ 滞回, 防止 NTC 采样噪声在阈值附近抖动导致乱闪 */
    if (BatStatus.Bits.DisOT || BatStatus.Bits.DisMOT)
    {
        LowTempAlarm = 1;
    }
    else if (BatNtcTempMin < 2731)               /* <0℃ 触发 */
    {
        LowTempAlarm = 1;
    }
    else if (BatNtcTempMin > 2751)               /* >2℃ 解除 */
    {
        LowTempAlarm = 0;
    }

    if (LowTempAlarm)
    {
        gLedTrigger = LED_TEMP;
        RGBSlowBlink(&CLR_RED, tick);
        return;
    }

    /* 3. 电芯不均衡/老化 (SOH<80% 或 压差>200mV) -> 单颗黄色快闪 2Hz */
    diff = (u16)(BatData.VolMax - BatData.VolMin);
    if ((SocInfo.Soh < 80) || (diff > 200))
    {
        gLedTrigger = LED_IMBALANCE;
        RGBSingleFastBlink(&CLR_YELLOW, tick);
        return;
    }

    /* 4. 充电状态 (以充电器插入标志为准, Chg 是充电MOS状态, 未插充电器也会打开) */
    if (BatStatus.Bits.ChgPlugin)
    {
        if (BatStatus.Bits.ChgFC || (soc >= 100))    /* 充满 */
        {
            gLedTrigger = LED_CHG_FULL;
            RGBAllGreen();
        }
        else if (soc >= 95)                          /* 涓流充电 */
        {
            gLedTrigger = LED_CHG_TRICKLE;
            RGBAllUpdata(&CLR_YELLOW);
        }
        else                                         /* 0~95% 充电中 */
        {
            gLedTrigger = LED_CHARGING;
            RGBChargeBreath(tick);
        }
        return;
    }

    /* 4.5 低电量告警: 未充电工作状态下 SOC<15% -> 单颗绿灯快闪 2Hz
     * 放在充电判断之后, 表示充电中优先显示充电呼吸, 不闪低电量 */
    if ((BatStatus.Bits.KeyOn || BatStatus.Bits.Active) && (soc < 15))
    {
        gLedTrigger = LED_LOW_SOC;
        RGBSingleFastBlink(&CLR_GREEN, tick);
        return;
    }

    /* 5. 工作 -> 电量显示 (以 KeyOn/Active 为准; Dis 是放电MOS状态, 常开不能代表放电中) */
    if (BatStatus.Bits.KeyOn || BatStatus.Bits.Active)
    {
        gLedTrigger = LED_SOC;
        RGBSocDisplay(soc);
        return;
    }

    /* 6. 空闲/休眠 -> 全灭 */
    gLedTrigger = LED_IDLE;
    AllLedOff();
#endif
}

/* ==================== DMA 传输完成中断 ==================== */
void DMA1_Channel3_IRQHandler(void)
{
    if (DMA_GetIntStatus(DMA1_INT_TXC3, DMA1))
    {
        DMA_ClrIntPendingBit(DMA1_INT_TXC3, DMA1);
        TIM_EnableDma(TIM2, TIM_DMA_UPDATE, DISABLE);
    }
}

/* ==================== 初始化 ==================== */
void RGBPwmInit(void)
{
    GPIO_InitType GPIO_InitStructure;
    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    OCInitType TIM_OCInitStructure;
    DMA_InitType DMA_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    /* 时钟使能 */
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM2, ENABLE);
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA1, ENABLE);

    /* PA1 -> TIM2_CH2 复用推挽输出 */
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin        = RGB_DIN_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitPeripheral(RGB_DIN_Port, &GPIO_InitStructure);

    /* TIM2: 72MHz -> 800kHz PWM */
    TIM_DeInit(TIM2);
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.Period    = 72000000 / 800000 - 1;   /* 89 */
    TIM_TimeBaseStructure.Prescaler = 0;
    TIM_TimeBaseStructure.ClkDiv    = 0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;
    TIM_InitTimeBase(TIM2, &TIM_TimeBaseStructure);

    TIM_InitOcStruct(&TIM_OCInitStructure);
    TIM_OCInitStructure.OcMode      = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
    TIM_OCInitStructure.Pulse       = RGB0D;
    TIM_OCInitStructure.OcPolarity  = TIM_OC_POLARITY_HIGH;
    TIM_InitOc2(TIM2, &TIM_OCInitStructure);
    TIM_ConfigOc2Preload(TIM2, TIM_OC_PRE_LOAD_ENABLE);

    /* DMA1_CH3: 内存 RGB_Buffer -> TIM2->CCDAT2, 半字, 循环模式 */
    DMA_DeInit(DMA1_CH3);
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.PeriphAddr     = (u32)&TIM2->CCDAT2;
    DMA_InitStructure.MemAddr        = (u32)RGB_Buffer;
    DMA_InitStructure.Direction      = DMA_DIR_PERIPH_DST;
    DMA_InitStructure.BufSize        = RGB_ALL_SUM;
    DMA_InitStructure.PeriphInc      = DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc  = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_HALFWORD;
    DMA_InitStructure.MemDataSize    = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.CircularMode   = DMA_MODE_CIRCULAR;
    DMA_InitStructure.Priority       = DMA_PRIORITY_HIGH;
    DMA_InitStructure.Mem2Mem        = DMA_M2M_DISABLE;
    DMA_Init(DMA1_CH3, &DMA_InitStructure);
    DMA_RequestRemap(DMA1_REMAP_TIM2_UP, DMA1, DMA1_CH3, ENABLE);
    DMA_ConfigInt(DMA1_CH3, DMA_INT_TXC, ENABLE);
    DMA_EnableChannel(DMA1_CH3, ENABLE);

    /* NVIC: DMA1 通道3 */
    NVIC_InitStructure.NVIC_IRQChannel                   = DMA1_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 清空缓冲, 启动定时器 */
    RGB_Buffer_Int();
    TIM_Enable(TIM2, ENABLE);
}
