/*****************************************************************************
* Copyright(c) QCG, 2023. All rights reserved.
*
* QCG [S8] Source Code Reference Design
* File: user_setting.h
*
* This Source Code Reference Design for QCG [S8] access
* ("Reference Design") is solely for the use of PRODUCT INTEGRATION REFERENCE ONLY,
* and contains confidential and privileged information of QCG International
* Limited. QCG shall have no liability to any PARTY FOR THE RELIABILITY,
* SERVICEABILITY FOR THE RESULT OF PRODUCT INTEGRATION, or results from: (i) any
* modification or attempted modification of the Reference Design by any party, or
* (ii) the combination, operation or use of the Reference Design with non-QCG
* Reference Design. Use of the Reference Design is at user's discretion to qualify
* the final work result.
*****************************************************************************/

 #ifndef _KC81710H_USER_SETTING_H_
 #define _KC81710H_USER_SETTING_H_

/*****************************************************************************
 * Uset setting
 * add const #define here
 *****************************************************************************/
// Constant setting:

// ============================================================================
// KC8017H EFUSE 用户参数映射区配置 (0x54 ~ 0x5F)
// 排版：每个寄存器分开写「字段定义(短注释) + 位域说明(范围/含义)」，
//       所有 REG_CFGx_DATA / REG_IE1_DATA 组合宏统一集中在文件末尾「组合宏汇总」。
// 位域定义详见《KC8017H寄存器规格书_v1.1》第2章 2.2 ~ 2.13 (CFG0~CFG11)
// 注：0x54~0x5F 与 0x24~0x2F(EFUSE用户数据区) 一一映射，沿用 CFGx 命名。
// ============================================================================

// =============================== CFG0 (0x54) ================================
// 字段定义
#define KC81710H_OV_HYS_DATA                    5       // OVR_HYS：5*20mV
#define KC81710H_OCC1_DELAY                     2       // OCC1_DLY：1s
#define KC81710H_OV_TH_DATA                     220     // OV_TH：4.2V

/* 位域说明
   [15:11] OVR_HYS[4:0] 过压恢复迟滞, 范围 20~620mV, 步长 20mV      → 2 = 40mV
   [10:8]  OCC1_DLY[2:0] 充电过流1延时: 0/1/2/3/4/5/6/7 = 0.25/0.5/1/2/4/8/12/16s → 2 = 1s
   [7:0]   OV_TH[7:0]   过压阈值, 范围 2000~4550mV, 步长 10mV     → 220 = 4.2V
*/


// =============================== CFG1 (0x55) ================================
// 字段定义
#define KC81710H_UV_HYS_DATA                    0       // UVR_HYS：160mV
#define KC81710H_UV_DELAY                       0       // UV_DLY：500ms
#define KC81710H_OV_RLS                         0       // OV_RLS：电压恢复即恢复
#define KC81710H_UV_TH_DATA                     70      // UV_TH：2.4V

/* 位域说明
   [15:11] UVR_HYS[4:0] 欠压恢复迟滞, 范围 40~1240mV, 步长 40mV    → 4 = 160mV
   [10:8]  UV_DLY[2:0]  欠压延时: 0/1/2/3/4/5/6/7 = 0.5/1/2/4/8/16/32/48s → 0 = 500ms
   [7]     OV_RLS       0=电压回到OVR即恢复; 1=需充电器移除才恢复 → 0
   [6:0]   UV_TH[6:0]   欠压阈值, 范围 1000~3540mV, 步长 20mV     → 75 = 2.5V
*/


// =============================== CFG2 (0x56) ================================
// 字段定义
#define KC81710H_OCD2_TH_DATA                   7       // OCD2_TH：40mV
#define KC81710H_OV_DELAY                       1       // OV_DLY：1s
#define KC81710H_OCD1_DELAY                     0       // OCD1_DLY：500ms
#define KC81710H_OCD1_TH_DATA                   0       // OCD1_TH：关闭保护(故OCD1_DLY不生效)

/* 位域说明
   [15:12] OCD2_TH[3:0] 放电过流2阈值, 范围 10~160mV, 步长 10mV    → 3 = 40mV
   [11:10] OV_DLY[1:0]  过压延时: 0/1/2/3 = 0.5/1/2/4s            → 1 = 1s
   [9:7]   OCD1_DLY[2:0]放电过流1延时: 0/1/2/3/4/5/6/7 = 0.5/1/2/4/8/16/32/48s → 0 = 500ms
   [6:0]   OCD1_TH[6:0] 放电过流1阈值, 范围 1~127mV, 步长 1mV, 全0=关闭 → 0 = 关闭OCD1(延时无效)
*/


// =============================== CFG3 (0x57) ================================
// 字段定义
#define KC81710H_OCD2_DELAY                     9    	// OCD2_DLY：500ms
#define KC81710H_OCC2_TH_DATA                   4       // OCC2_TH：5mV
#define KC81710H_OCC_RLS                        1       // OCC_RLS：需充移除恢复
#define KC81710H_OCC1_TH_DATA                   0       // OCC1_TH：3mV

/* 位域说明
   [15:12] OCD2_DLY[3:0] 放电过流2延时(16档): 2ms~800ms(0=2,1=5,2=10,3=20,4=30,5=40,6=60,7=80,
            8=100,9=150,10=200,11=300,12=400,13=500,14=600,15=800ms) → 13 = 500ms
   [11:8]  OCC2_TH[3:0] 充电过流2阈值, 范围 5~80mV, 步长 5mV      → 0 = 5mV
   [7]     OCC_RLS       0=内部32s定时恢复; 1=需充电器移除才恢复  → 1
   [6:0]   OCC1_TH[6:0] 充电过流1阈值, 范围 1~127mV, 步长 1mV, 全0=关闭 → 3 = 3mV
*/


// =============================== CFG4 (0x58) ================================
// 字段定义
#define KC81710H_UT_DELAY                       0       // UT_DLY：2s
#define KC81710H_OTC_TH_DATA                    0       // OTC_TH：~75°C
#define KC81710H_OT_DELAY                       1       // OT_DLY：2s
#define KC81710H_OTD_TH_DATA                    0       // OTD_TH：~80°C

/* 位域说明
   [15:14] UT_DLY[1:0]   低温延时: 0/1/2/3 = 2/4/8/16s            → 0 = 2s
   [13:8]  OTC_TH[5:0]   充电过温(OTC)阈值, 阻值=(1.9k+OTC_TH*0.05k), 45~75°C → 0 = 1.9kΩ(~75°C)
   [7:6]   OT_DLY[1:0]   过温延时(含OTD/OTC): 0/1/2/3 = 1/2/4/8s  → 1 = 2s
   [5:0]   OTD_TH[5:0]   放电过温(OTD)阈值, 阻值=(1.65k+OTD_TH*0.05k), 46~80°C → 0 = 1.65kΩ(~80°C)
*/


// =============================== CFG5 (0x59) ================================
// 字段定义
#define KC81710H_SCD_DELAY                      7       // SCD_DLY：233us
#define KC81710H_SCD_TH_DATA                    4       // SCD_TH：60mV
#define KC81710H_OTCR_HYS_DATA                  0       // OTCR_HYS：0.3kΩ
#define KC81710H_OTDR_HYS_DATA                  0       // OTDR_HYS：0.2kΩ

/* 位域说明
   [15:12] SCD_DLY[3:0]  放电短路延时, 约 50~1000us, 步长 61us    → 3 = 233us
   [11:8]  SCD_TH[3:0]   放电短路阈值, 范围 20~320mV, 步长 20mV   → 2 = 60mV
   [7:4]   OTCR_HYS[3:0] 退出OTC滞回=(0.3k+OTCR_HYS*0.05k), 步长0.05k, 全0=无OTC → 0 = 0.3kΩ
   [3:0]   OTDR_HYS[3:0] 退出OTD滞回=(0.2k+OTDR_HYS*0.05k), 步长0.05k, 全0=无OTD → 0 = 0.2kΩ
*/


// =============================== CFG6 (0x5A) ================================
// 字段定义
#define KC81710H_TS2_CFG                        3       // TS2_CFG：MOSFET温度
#define KC81710H_UTC_TH_DATA                    0x3F    // UTC_TH：~-24°C
#define KC81710H_DSG_SAFETY_RLS                 0       // DSG_RLS：定时/迟滞恢复
#define KC81710H_UTD_TH_DATA                    0x3F    // UTD_TH：~-36°C

/* 位域说明
   [15:14] HW_TS2_CFG[1:0] TS2用途: 00/01=AUX, 10=电芯温度, 11=外部MOSFET温度 → 3 = MOSFET温度
   [13:8]  UTC_TH[5:0]   充电低温(UTC)阈值, 阻值=(21k+UTC_TH*1k), -24~6°C   → 63 = 84kΩ(~-24°C)
   [7:6]   DSG_SAFETY_RLS[1:0] 放电管安全释放: 00=定时32s/迟滞恢复, 01=需负载移除,
            10=需ELOCK OFF, 11=负载移除或ELOCK OFF                         → 0 = 定时/迟滞恢复
   [5:0]   UTD_TH[5:0]   放电低温(UTD)阈值, 阻值=(26k+UTD_TH*2k), -36~1°C   → 63 = 152kΩ(~-36°C)
*/


// =============================== CFG7 (0x5B) ================================
// 字段定义
#define KC81710H_SOV_TH_DATA                    0       // SOV_TH：比OV高100mV
#define KC81710H_CLR_OV_LOW_VC17                1       // 低压清除OV/SOV
#define KC81710H_ELKON_WKUP_EN                  0       // 禁电子锁唤醒
#define KC81710H_LDON_WKUP_EN                   0       // 禁负载唤醒
#define KC81710H_OTCUTC_RLS                     0       // 温度恢复即恢复
#define KC81710H_UTCR_HYS_DATA                  0       // UTCR_HYS：1kΩ
#define KC81710H_UTDR_HYS_DATA                  0       // UTDR_HYS：2kΩ

/* 位域说明
   [15:12] SOV_TH[3:0]   二级过压(SOV)阈值, 比OV高 20~300mV, 步长20mV, 全0=关闭 → 5 = 高100mV
   [11]    CLR_OV_LOW_VC17 1=VC17/n低于OW_START_TH时清除OV/SOV并停止检测; 0=照常 → 1
   [10]    HW_ELKON_WKUP_EN  1=Deep Sleep允许电子锁OFF→ON唤醒; 0=禁止            → 0
   [9]     HW_LDON_WKUP_EN  1=Deep Sleep允许负载插入唤醒; 0=禁止                → 0
   [8]     OTCUTC_RLS     1=需充电器移除才恢复OTC/UTC; 0=温度回到恢复点即恢复  → 0
   [7:4]   UTCR_HYS[3:0]  恢复UTC滞回=(1k+UTCR_HYS*1k), 步长1k, 全0=无UTC      → 0 = 1kΩ
   [3:0]   UTDR_HYS[3:0]  恢复UTD滞回=(2k+UTDR_HYS*2k), 步长2k, 全0=无UTD      → 0 = 2kΩ
*/


// =============================== CFG8 (0x5C) ================================
// 字段定义
#define KC81710H_OCC2_DELAY                     0x0F    // OCC2_DLY：120ms
#define KC81710H_MOTR_HYS_DATA                  0       // MOTR_HYS：关闭MOT
#define KC81710H_MOT_DELAY                      0       // MOT_DLY：1s
#define KC81710H_MOT_TH_DATA                    0       // MOT_TH：~71°C

/* 位域说明
   [15:12] OCC2_DLY[3:0]  充电过流2延时(16档): 0.25/0.5/1/2/3/5/10/15/20/30/40/50/60/80/100/120ms
            → 15 = 120ms
   [11:8]  MOTR_HYS[3:0] 退出MOT滞回=(0.1k+MOTR_HYS*0.025k), 步长0.025k, 全0=关闭MOT → 0 = 关闭
   [7:6]   MOT_DLY[1:0]   MOSFET过温延时: 00/01/10/11 = 1/2/4/8s          → 0 = 1s
   [5:0]   MOT_TH[5:0]   放电过温(MOT)阈值, 阻值=(0.6k+MOT_TH*0.025k), 71~120°C → 0 = 0.6kΩ(~71°C)
*/


// =============================== CFG9 (0x5D) ================================
// 字段定义
#define KC81710H_CB_CTRL                        0       // 仅充电均衡
#define KC81710H_CB1_DIFF_DATA                  0       // CB1压差：80mV
#define KC81710H_OW_EN                          0       // 关闭断线检测
#define KC81710H_CB1_TH_DATA                    0       // CB1启动：关闭
#define KC81710H_CB2_DIFF_DATA                  1       // CB2压差：40mV
#define KC81710H_CB2_TH_DATA                    40      // CB2启动：~3.7V

/* 位域说明
   [15]    CB_CTRL       0=仅在充电状态均衡; 1=充电或不充不放都均衡    → 0
   [14:13] CB1_DIFF[1:0] 一级均衡压差: 00/01/10/11 = 80/120/160/200mV → 0 = 80mV
   [12]    OW_EN         1=支持硬件断线自动检测; 0=不支持              → 0
   [11:8]  CB1_TH[3:0]   一级均衡启动电压: 0=无; 1~15=2360~2920mV步长40mV → 0 = 关闭CB1
   [7:6]   CB2_DIFF[1:0] 二级均衡压差: 00/01/10/11 = 20/40/60/80mV    → 1 = 40mV
   [5:0]   CB2_TH[5:0]   二级均衡启动电压: 0=无; 1~63=2900~4140mV步长20mV → 40 = ~3.7V
*/


// =============================== CFG10 (0x5E) ===============================
// 字段定义
#define KC81710H_DSGCTRL_INCHG                  0       // 充电开DSG
#define KC81710H_CHGCTRL_INDSG                  0       // 放电开CHG
#define KC81710H_VMCU2_DOWN_STDALONE            0       // 不影响VMCU2
#define KC81710H_PRE_UV                         0       // 不上电预设UV
#define KC81710H_AUTO_CELLCHK_PERIOD            0       // 电芯扫描125ms
#define KC81710H_AUTO_CRRTCHK_PERIOD            0       // 电流扫描125ms
#define KC81710H_AUTO_TSCHK_PERIOD              1       // 温度扫描500ms
#define KC81710H_LDO_MD                         1       // 1s检测100ms负载
#define KC81710H_CELL_CNT                       5       // 电芯=15串
#define KC81710H_VADC_CELL_LSB                  1       // 电芯ADC 16bit

/* 位域说明
   [15:14] HW_DSGCTRL_INCHG[1:0] 充电对放电管: 00=无, 01/10=充电需开DSG, 11=充电需关DSG → 1 = 充电需开DSG
   [13:12] HW_CHGCTRL_INDSG[1:0] 放电对充电管: 00=无, 01/10=放电需开CHG, 11=放电需关CHG → 1 = 放电需开CHG
   [9]     VMCU2_DOWN_STDALONE 0=独立模式不影响VMCU2; 1=强制关VMCU2     → 0
   [8]     PRE_UV         0=上电不预设UV; 1=上电预设UV并恢复判断        → 0
   [7]     AUTO_CELLCHK_PERIOD 0=电芯电压扫描125ms; 1=250ms            → 0
   [6]     AUTO_CRRTCHK_PERIOD 0=电流扫描125ms; 1=250ms                → 0
   [5]     AUTO_TSCHK_PERIOD   0=温度扫描250ms; 1=500ms                → 1
   [4]     LDD_MD         0=持续检测负载; 1=每1s检测100ms              → 1
   [3:1]   CELL_CNT[2:0] 电芯串数=N+10, 支持10~17串                    → 5 = 15串
   [0]     VADC_CELL_LSB 0=14位精度; 1=16位精度(电芯/VC17通道)         → 1
*/


// =============================== CFG11 (0x5F) ===============================
// 字段定义
#define KC81710H_CFG_LOCK                       0       // 不锁配置
#define KC81710H_ECTRL_SEL                      0       // 低电平关DSG
#define KC81710H_VMCU1_SEL                      1       // VMCU1=5V
#define KC81710H_DSGOFF_INOW                    0       // 断线仅关CHG
#define KC81710H_PCHG_TH_DATA                   0       // 关闭预充
#define KC81710H_HW_SLEEP_CTRL                  0       // 不支持睡眠
#define KC81710H_VADC_TS_LSB                    1       // 温度ADC 16bit
#define KC81710H_VMCU2_SEL                      0       // VMCU2=3.3V
#define KC81710H_IDLE_TH_DATA                   13       // 空闲~101.6uV

/* 位域说明
   [15]    CFG_LOCK       1=锁定0x24~0x2F不可修改; 0=可修改          → 0
   [14]    ECTRL_SEL      0=低电平关放电管; 1=高电平关(外部ECTRL)    → 0
   [13]    VMCU1_SEL      0=VMCU1输出3.3V; 1=输出5V                  → 1
   [12]    DSGOFF_INOW    1=断线时同时关充放电管; 0=仅关充电管        → 0
   [11:8]  PCHG_TH[3:0]   预充阈值: 全0=关闭; 非0=低于此值启动预充, 1500~3000mV步长100mV → 0 = 关闭
   [7:6]   HW_SLEEP_CTRL[1:0] 00=不支持睡眠, 01=Deep Sleep, 10/11=Normal Sleep → 0 = 不支持
   [5]     VADC_TS_LSB    0=温度通道14位; 1=16位精度                 → 1
   [4]     VMCU2_SEL      0=VMCU2输出3.3V; 1=输出5V                  → 0
   [3:0]   IDLE_TH[3:0]   空闲电流阈值, 范围 7.8125~242.1875uV, 步长15.625uV → 6 = ~101.6uV
*/


// ============================ 组合宏汇总 ====================================
// Configuration 0x54 (映射为CFG0)
#define KC81710H_REG_CFG0_DATA                  ((KC81710H_OV_HYS_DATA<<11) | (KC81710H_OCC1_DELAY<<8) | (KC81710H_OV_TH_DATA<<0))
// Configuration 0x55 (映射为CFG1)
#define KC81710H_REG_CFG1_DATA                  ((KC81710H_UV_HYS_DATA<<11) | (KC81710H_UV_DELAY<<8) | (KC81710H_OV_RLS<<7) | (KC81710H_UV_TH_DATA<<0))
// Configuration 0x56 (映射为CFG2)
#define KC81710H_REG_CFG2_DATA                  ((KC81710H_OCD2_TH_DATA<<12) | (KC81710H_OV_DELAY<<10) | (KC81710H_OCD1_DELAY<<7) | (KC81710H_OCD1_TH_DATA<<0))
// Configuration 0x57 (映射为CFG3)
#define KC81710H_REG_CFG3_DATA                  ((KC81710H_OCD2_DELAY<<12) | (KC81710H_OCC2_TH_DATA<<8) | (KC81710H_OCC_RLS<<7) | (KC81710H_OCC1_TH_DATA<<0))
// Configuration 0x58 (映射为CFG4)
#define KC81710H_REG_CFG4_DATA                  ((KC81710H_UT_DELAY<<14) | (KC81710H_OTC_TH_DATA<<8) | (KC81710H_OT_DELAY<<6) | (KC81710H_OTD_TH_DATA<<0))
// Configuration 0x59 (映射为CFG5)
#define KC81710H_REG_CFG5_DATA                  ((KC81710H_SCD_DELAY<<12) | (KC81710H_SCD_TH_DATA<<8) | (KC81710H_OTCR_HYS_DATA<<4) | (KC81710H_OTDR_HYS_DATA<<0))
// Configuration 0x5A (映射为CFG6)
#define KC81710H_REG_CFG6_DATA                  ((KC81710H_TS2_CFG<<14) | (KC81710H_UTC_TH_DATA<<8) | (KC81710H_DSG_SAFETY_RLS<<6) | (KC81710H_UTD_TH_DATA<<0))
// Configuration 0x5B (映射为CFG7)
#define KC81710H_REG_CFG7_DATA                  ((KC81710H_SOV_TH_DATA<<12) | (KC81710H_CLR_OV_LOW_VC17<<11) | (KC81710H_ELKON_WKUP_EN<<10) | (KC81710H_LDON_WKUP_EN<<9) | (KC81710H_OTCUTC_RLS<<8) | (KC81710H_UTCR_HYS_DATA<<4) | (KC81710H_UTDR_HYS_DATA<<0))
// Configuration 0x5C (映射为CFG8)
#define KC81710H_REG_CFG8_DATA                  ((KC81710H_OCC2_DELAY<<12) | (KC81710H_MOTR_HYS_DATA<<8) | (KC81710H_MOT_DELAY<<6) | (KC81710H_MOT_TH_DATA<<0))
// Configuration 0x5D (映射为CFG9)
#define KC81710H_REG_CFG9_DATA                  ((KC81710H_CB_CTRL<<15) | (KC81710H_CB1_DIFF_DATA<<13) | (KC81710H_OW_EN<<12) | (KC81710H_CB1_TH_DATA<<8) | (KC81710H_CB2_DIFF_DATA<<6) | (KC81710H_CB2_TH_DATA<<0))
// Configuration 0x5E (映射为CFG10)
#define KC81710H_REG_CFG10_DATA                 ((KC81710H_DSGCTRL_INCHG<<14) | (KC81710H_CHGCTRL_INDSG<<12) | (KC81710H_VMCU2_DOWN_STDALONE<<9) | (KC81710H_PRE_UV<<8) | (KC81710H_AUTO_CELLCHK_PERIOD<<7) | (KC81710H_AUTO_CRRTCHK_PERIOD<<6) | (KC81710H_AUTO_TSCHK_PERIOD<<5) | (KC81710H_LDO_MD<<4) | (KC81710H_CELL_CNT<<1) | (KC81710H_VADC_CELL_LSB<<0))
// Configuration 0x5F (映射为CFG11)
#define KC81710H_REG_CFG11_DATA                 ((KC81710H_CFG_LOCK<<15) | (KC81710H_ECTRL_SEL<<14) | (KC81710H_VMCU1_SEL<<13) | (KC81710H_DSGOFF_INOW<<12) | (KC81710H_PCHG_TH_DATA<<8) | (KC81710H_HW_SLEEP_CTRL<<6) | (KC81710H_VADC_TS_LSB<<5) | (KC81710H_VMCU2_SEL<<4) | (KC81710H_IDLE_TH_DATA<<0))
// IE1
#define KC81710H_REG_IE1_DATA                   (0x0000)


#endif

