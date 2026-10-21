/******************************************************************************
 *  文 件 名   : kalman_soc.h
 *  版 本 号   : 初稿
 *  作    者   : AI
 *  生成日期   : 2026年10月14日
 *  功能描述   : 基于二阶 RC 等效电路模型的扩展卡尔曼滤波(EKF) SOC 估计模块
 *               （独立参考算法，仅用于对比/调试，不参与任何现有 SOC 策略）
 *
 *  算法来源   : 微信公众号《MATLAB 实现 EKF 电池 SOC 估计》(二阶RC + EKF)
 *
 *  设计说明   :
 *   1. 本模块按【单节电芯】建模：
 *        - 电压单位为 mV（复用工程 BatData.VolMin 单节电压）
 *        - 电流单位为 mA（工程总电流 / 并联数 = 单节电流）
 *        - 容量单位为 mAh（单节容量 = CAPACITY_DESIGN / BAT_PARAL_NUM）
 *        - 时间单位为 ms
 *   2. 电流符号约定为【充电为正、放电为负】（与工程 SocCur=ChgCurAvg-DisCurAvg 一致）
 *   3. SOC 内部以 0~1 浮点表示，对外输出 0~100 百分比
 *   4. 本模块【只读】工程全局量，绝不写回 DataFlashPowerDownSave.Soc 等策略变量
 *
 *  修改历史   :
 *   1. 日    期   : 2026年10月14日
 *      修改内容   : 创建文件
 ******************************************************************************/

#ifndef __KALMAN_SOC_H__
#define __KALMAN_SOC_H__

/*
 * 依赖：复用 soc.h 中的以下内容
 *   - SOC_2_OCV 类型           : OCV-SOC 查表结构体
 *   - BAT_TYPE / CAPACITY_DESIGN / BAT_PARAL_NUM : 电芯容量相关宏
 *   - BatData / ChgCurAvg / DisCurAvg : 电压/电流全局量(extern)
 * 注意：soc.h 中 MainDef 宏受 __MAIN_DEF__ 控制，本文件编译单元不定义该宏，
 *       因此相关全局量均为 extern 声明，不会造成重复定义。
 */
#include "soc.h"

/* ============================================================================
 * 一、可配置参数
 *    下面参数当前为【文章 NMC 典型值】，请在实际拿到 HPPC 标定数据后替换！
 * ==========================================================================*/

/* ---- 1. 二阶 RC 等效电路参数（待 HPPC 标定值替换）---- */
#define KAL_R0_OHM       0.020f     /* 欧姆内阻，单位: Ω   （20mΩ）   */
#define KAL_R1_OHM       0.010f     /* 快极化电阻，单位: Ω  （10mΩ）   */
#define KAL_C1_FARAD     1500.0f    /* 快极化电容，单位: F   (τ1=15s)  */
#define KAL_R2_OHM       0.006f     /* 慢极化电阻，单位: Ω  （6mΩ）    */
#define KAL_C2_FARAD     150.0f     /* 慢极化电容，单位: F   (τ2=0.9s) */

/* ---- 2. 采样周期（与工程 SOC 计算 100ms 周期保持一致）---- */
#define KAL_SAMPLE_MS    100u       /* 默认采样周期，单位: ms */

/* ---- 3. 测量噪声 R（电压测量方差，单位 mV^2）----
 *     文章取 5mV 电压噪声 -> R = 5*5 = 25 mV^2
 */
#define KAL_MEAS_NOISE_MV2   25.0f

/* ---- 4. 过程噪声 Q（对角阵，单位分别见注释）----
 *     q_soc : 库仑积分误差，SOC(0~1) 量纲，极小
 *     q_v1/q_v2 : 极化支路模型误差，单位 mV^2，相对较大
 */
#define KAL_Q_SOC        1e-10f     /* SOC 过程噪声 (0~1)^2 */
#define KAL_Q_V1         10.0f      /* V1 过程噪声 mV^2 */
#define KAL_Q_V2         10.0f      /* V2 过程噪声 mV^2 */

/* ---- 5. 初始误差协方差 P0（对角阵）----
 *     对应初值不确定度：SOC σ=0.2(0~1)，V1 σ=50mV，V2 σ=40mV
 */
#define KAL_P0_SOC       0.04f      /* (0.2)^2 */
#define KAL_P0_V1        2500.0f    /* (50mV)^2 */
#define KAL_P0_V2        1600.0f    /* (40mV)^2 */

/* ============================================================================
 * 二、对外接口
 * ==========================================================================*/

/**
 * \brief  卡尔曼 SOC 模块初始化
 * \note   上电时调用一次；内部会用当前单节电压反查 OCV 得到初始 SOC，
 *         并初始化协方差矩阵 P 与 RC 离散化系数。
 */
void KalmanSocInit(void);

/**
 * \brief  EKF 单步迭代（通用接口，可自行喂数据）
 * \param  packCur_mA   电池包总电流，单位 mA，充电为正/放电为负
 * \param  cellVol_mV   单节电芯电压（观测值），单位 mV
 * \param  dt_ms        距离上次迭代的时间间隔，单位 ms
 * \note   每次调用执行一次 预测->更新；不修改任何工程策略变量。
 */
void KalmanSocStep(s32 packCur_mA, u16 cellVol_mV, u16 dt_ms);

/**
 * \brief  便捷接口：内部自动读取工程全局量并迭代一次
 * \note   内部取：电流 = (s32)ChgCurAvg - (s32)DisCurAvg
 *                电压 = BatData.VolMin
 *                周期 = KAL_SAMPLE_MS
 *         建议在工程 100ms 周期任务中调用（仅喂数据，不影响策略）。
 */
void KalmanSocRun(void);

/**
 * \brief  获取卡尔曼估计的参考 SOC
 * \return SOC 百分比 0~100
 */
u8 KalmanSocGet(void);

/**
 * \brief  获取卡尔曼估计的参考 SOC（浮点）
 * \return SOC 0~1
 */
float KalmanSocGetRef(void);

#endif  /* __KALMAN_SOC_H__ */
