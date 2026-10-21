/******************************************************************************
 *  文 件 名   : kalman_soc.c
 *  版 本 号   : 初稿
 *  作    者   : AI
 *  生成日期   : 2026年10月14日
 *  功能描述   : 基于二阶 RC 等效电路模型的 EKF SOC 估计（独立参考算法，
 *               仅用于对比/调试，不参与任何现有 SOC 策略）
 *  算法来源   : 微信公众号《MATLAB 实现 EKF 电池 SOC 估计》(二阶RC + EKF)
 *
 *  【建模约定】
 *   状态 x = [SOC(0~1), V1(mV), V2(mV)]；电流符号：充电为正、放电为负。
 *   预测:  SOC += i*dt/(3600000*Qn)      (i:mA, dt:ms, Qn:mAh)
 *          V1 = A1*V1 + B1*i,  A1=exp(-dt/t1), B1=R1*(1-A1), t1=R1*C1
 *          V2 = A2*V2 + B2*i,  A2=exp(-dt/t2), B2=R2*(1-A2), t2=R2*C2
 *          F = diag(1, A1, A2)
 *   观测:  Vt = OCV(SOC) + R0*i + V1 + V2,  H = [dOCV/dSOC, 1, 1]
 *
 *  【单位】电压 mV、电流 mA、容量 mAh、时间 ms、电阻 Ω、电容 F。
 *          (电阻用 Ω，电流用 mA 时 R*i 单位 = Ω*mA = mV，与电压一致)
 *
 *  【依赖】需浮点运算(N32L40x 为 M4F 自带 FPU)；expf 来自 <math.h>；
 *          复用 soc.c 中电芯 OCV 表 SocOcvTbl（extern 引用，不复制）。
 ******************************************************************************/

#include "kalman_soc.h"
#include <math.h>          /* expf */

/* 复用 soc.c 中的电芯 OCV 表（全局符号，可 extern 引用） */
extern const SOC_2_OCV SocOcvTbl[];

/* OCV 表点数：须与 soc.c 当前电芯(BAT_SDI_50P)的 OCV_NUM=13 一致；换电芯时同步修改 */
#define KAL_OCV_NUM         13u

typedef struct
{
    float soc;                          /* SOC，0~1 */
    float v1;                           /* 快极化电压，mV */
    float v2;                           /* 慢极化电压，mV */
} KAL_STATE_T;

static KAL_STATE_T  s_x;                /* 状态估计 */
static float        s_P[3][3];          /* 误差协方差 */
static float        s_QnCell_mAh;       /* 单节容量，mAh */
static u8           s_inited = 0;
static u8           s_socPct = 50;      /* 参考 SOC，0~100 */

/* SOC(0~1) -> OCV(mV)，线性插值 */
static float KalOcvLookup(float soc01)
{
    float soc = soc01 * 100.0f;
    float vol = 0.0f;
    u8 i;

    if (soc <= (float)SocOcvTbl[0].Soc)
        return (float)SocOcvTbl[0].Vol;
    if (soc >= (float)SocOcvTbl[KAL_OCV_NUM - 1].Soc)
        return (float)SocOcvTbl[KAL_OCV_NUM - 1].Vol;

    for (i = 1; i < KAL_OCV_NUM; i++)
    {
        if (soc <= (float)SocOcvTbl[i].Soc)
        {
            float s0 = (float)SocOcvTbl[i - 1].Soc;
            float s1 = (float)SocOcvTbl[i].Soc;
            float v0 = (float)SocOcvTbl[i - 1].Vol;
            float v1 = (float)SocOcvTbl[i].Vol;
            vol = v0 + (v1 - v0) * (soc - s0) / (s1 - s0);
            break;
        }
    }
    return vol;
}

/* OCV(mV) -> SOC(0~100)，反查，用于初始化 */
static u8 KalVolToSoc(u16 vol_mV)
{
    float vol = (float)vol_mV;
    float soc = 0.0f;
    u8 i;

    if (vol <= (float)SocOcvTbl[0].Vol)
        return SocOcvTbl[0].Soc;
    if (vol >= (float)SocOcvTbl[KAL_OCV_NUM - 1].Vol)
        return SocOcvTbl[KAL_OCV_NUM - 1].Soc;

    for (i = 1; i < KAL_OCV_NUM; i++)
    {
        if (vol <= (float)SocOcvTbl[i].Vol)
        {
            float s0 = (float)SocOcvTbl[i - 1].Soc;
            float s1 = (float)SocOcvTbl[i].Soc;
            float v0 = (float)SocOcvTbl[i - 1].Vol;
            float v1 = (float)SocOcvTbl[i].Vol;
            soc = s0 + (s1 - s0) * (vol - v0) / (v1 - v0);
            break;
        }
    }
    return (u8)(soc + 0.5f);
}

/* dOCV/dSOC 斜率（观测雅可比 H 首项），中心差分，SOC 0~1 量纲，单位 mV */
static float KalOcvSlope(float soc01)
{
    const float h = 1e-4f;
    return (KalOcvLookup(soc01 + h) - KalOcvLookup(soc01 - h)) / (2.0f * h);
}

void KalmanSocInit(void)
{
    u8 initSoc;

    /* 初始 SOC：优先 OCV 反查，无效时默认 50% 由 EKF 收敛 */
    initSoc = BatData.Valid ? KalVolToSoc(BatData.VolMin) : 50;

    s_x.soc = (float)initSoc / 100.0f;
    s_x.v1  = 0.0f;
    s_x.v2  = 0.0f;

    s_P[0][0] = KAL_P0_SOC;  s_P[0][1] = 0.0f;       s_P[0][2] = 0.0f;
    s_P[1][0] = 0.0f;        s_P[1][1] = KAL_P0_V1;  s_P[1][2] = 0.0f;
    s_P[2][0] = 0.0f;        s_P[2][1] = 0.0f;       s_P[2][2] = KAL_P0_V2;

    /* 单节容量 = 整包容量 / 并联数 */
    s_QnCell_mAh = (float)(CAPACITY_DESIGN / BAT_PARAL_NUM);

    s_socPct = initSoc;
    s_inited = 1;
}

void KalmanSocStep(s32 packCur_mA, u16 cellVol_mV, u16 dt_ms)
{
    float i, y, dt_s, a1, b1, a2, b2;
    float soc_p, v1_p, v2_p;
    float s, h, e, F[3], H[3], Pp[3][3], S, K[3], HPp[3];
    u8 i2, j;

    if (!s_inited)
        return;

    i    = (float)packCur_mA / (float)BAT_PARAL_NUM;   /* 单节电流 mA */
    y    = (float)cellVol_mV;                          /* 观测电压 mV */
    dt_s = (float)dt_ms / 1000.0f;

    /* RC 离散化系数 */
    a1 = expf(-dt_s / (KAL_R1_OHM * KAL_C1_FARAD));  b1 = KAL_R1_OHM * (1.0f - a1);
    a2 = expf(-dt_s / (KAL_R2_OHM * KAL_C2_FARAD));  b2 = KAL_R2_OHM * (1.0f - a2);

    /* ---- 预测 ---- */
    soc_p = s_x.soc + i * (float)dt_ms / (3600000.0f * s_QnCell_mAh);  /* 库仑积分 */
    v1_p  = a1 * s_x.v1 + b1 * i;
    v2_p  = a2 * s_x.v2 + b2 * i;

    /* Pp = F*P*F' + Q，F 为对角阵 */
    F[0] = 1.0f;  F[1] = a1;  F[2] = a2;
    for (i2 = 0; i2 < 3; i2++)
        for (j = 0; j < 3; j++)
            Pp[i2][j] = F[i2] * s_P[i2][j] * F[j];
    Pp[0][0] += KAL_Q_SOC;
    Pp[1][1] += KAL_Q_V1;
    Pp[2][2] += KAL_Q_V2;

    /* ---- 线性化观测 ---- */
    s = KalOcvSlope(soc_p);                                     /* dOCV/dSOC */
    h = KalOcvLookup(soc_p) + KAL_R0_OHM * i + v1_p + v2_p;     /* 预测端电压 */
    H[0] = s;  H[1] = 1.0f;  H[2] = 1.0f;

    /* ---- 更新 ---- */
    S = 0.0f;                                                   /* 新息协方差 */
    for (i2 = 0; i2 < 3; i2++)
        for (j = 0; j < 3; j++)
            S += H[i2] * Pp[i2][j] * H[j];
    S += KAL_MEAS_NOISE_MV2;

    for (i2 = 0; i2 < 3; i2++)                                  /* 卡尔曼增益 */
    {
        K[i2] = 0.0f;
        for (j = 0; j < 3; j++)
            K[i2] += Pp[i2][j] * H[j];
        K[i2] /= S;
    }

    e = y - h;                                                  /* 电压残差 */

    s_x.soc = soc_p + K[0] * e;                                 /* 状态更新 */
    s_x.v1  = v1_p  + K[1] * e;
    s_x.v2  = v2_p  + K[2] * e;

    if (s_x.soc < 0.0f) s_x.soc = 0.0f;                         /* SOC 限幅 */
    if (s_x.soc > 1.0f) s_x.soc = 1.0f;

    for (j = 0; j < 3; j++)                                     /* P = Pp - K*(H*Pp) */
    {
        HPp[j] = 0.0f;
        for (i2 = 0; i2 < 3; i2++)
            HPp[j] += H[i2] * Pp[i2][j];
    }
    for (i2 = 0; i2 < 3; i2++)
        for (j = 0; j < 3; j++)
            s_P[i2][j] = Pp[i2][j] - K[i2] * HPp[j];

    s_socPct = (u8)(s_x.soc * 100.0f + 0.5f);                  /* 输出 0~100 */
    if (s_socPct > 100) s_socPct = 100;
}

void KalmanSocRun(void)
{
    s32 cur;

    if (!BatData.Valid)
        return;
    if (!s_inited)
        KalmanSocInit();

    cur = (s32)ChgCurAvg - (s32)DisCurAvg;                      /* 充电为正 */
    KalmanSocStep(cur, BatData.VolMin, KAL_SAMPLE_MS);
}

u8 KalmanSocGet(void)
{
    return s_socPct;
}

float KalmanSocGetRef(void)
{
    return s_x.soc;
}
