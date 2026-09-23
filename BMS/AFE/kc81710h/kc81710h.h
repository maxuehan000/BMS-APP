/*****************************************************************************
 * Copyright(c) QCG, 2023. All rights reserved.
 *
 * QCG [S8] Source Code Reference Design
 * File: kc81710h.h
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

#ifndef _KC81710H_H_
#define _KC81710H_H_

/*****************************************************************************
 * #include section
 * add #include here
 *****************************************************************************/
#include "kc81710h_reg.h"
#include "kc81710h_user_setting.h"

#ifndef NULL
#define NULL	(void *)0
#endif

#ifdef USE_PRIVATE_INT
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef signed long long int64_t;
#else
#include "stdint.h"
#endif

//#define WEAK_FUNC       __weak          // weak function attribution definition

#pragma anon_unions

/*****************************************************************************
 * const section
 * add const #define here
 *****************************************************************************/
// Basic configuration

#define RSENSE                                  (500L) // uOhm
#define CELL_NUMBER                             (13)
//#define MAX_COMBINED_VOLATILE_REGS			(4)

// cell voltage
#define KC81710H_CELL_VOLT_LSB_MV				(5L) //(0.15625)mV/LSB
#define KC81710H_CELL_VOLT_FACTOR				(32L)
// internal thm / thm0 / thm1 / GP1V
#define KC81710H_THM_VOLT_LSB_MV				(5L) //(0.078125)mV/LSB
#define KC81710H_THM_VOLT_FACTOR				(64L)
#define KC81710H_THM_REF_VOLT_MV				(2560)	// reference voltage
#define KC81710H_THM_DIV_RES_OHM				(16000)	//divider resistor
#define KC81710H_THM_RES_LSB_KR					(1L) //(0.0125)KR/LSB
#define KC81710H_THM_RES_FACTOR					(80L)
#define KC81710H_THM_ITMP_REF_MV				(559)
#define KC81710H_THM_ITMP_REF_0C1				(250)
#define KC81710H_THM_ITMP_LSB_MV				(199)	// (0.199)mV/LSB
#define KC81710H_THM_ITMP_FACTOR				(1000)
// current
#define KC81710H_ISENS_VOLT_LSB_NV				(15625L) //(1.953125)uV/LSB
#define KC81710H_ISENS_VOLT_FACTOR				(8L)
// current 16 bits
//#define KC81710H_ISENS_VOLT_LSB_NV			(15625L) //(7.8125)uV/LSB
//#define KC81710H_ISENS_VOLT_FACTOR			(2L)
// high voltage
#define	KC81710H_HIGH_VOLT_LSB_MV				(45L) //(2.8125)mV/LSB
#define	KC81710H_HIGH_VOLT_FACTOR				(16L)
// low voltage
#define	KC81710H_LOW_VOLT_LSB_MV				(5L) // (0.3125)mV/LSB
#define	KC81710H_LOW_VOLT_FACTOR				(16L)
// Coulomb counter
#define KC81710H_CC_ACC_LSB_NV					(80000UL) //(8.888889)uVH/LSB
#define KC81710H_CC_ACC_FACTOR					(9L)

/**************************************************************
 * SDK EAGLE SENOIR VERSION
 **************************************************************/
#define KC81710H_VER							(0x0001U)

#define KC_STATUS_OK							0x00
#define KC_STATUS_ERROR							0x01

#define KC_STATUS_PECERR						0x81
#define KC_STATUS_NOT_MATCH						0x82
#define KC_STATUS_INVALID_PARAMETER				0x83
#define KC_STATUS_REGS_CHANGE					0x84
#define KC_STATUS_TIMEOUT						0x85

/**************************************************************
 * ADT section
 * add Abstract Data Type definition here
 **************************************************************/

typedef enum
{
	KC81710H_REQ_CH_VTS = 0,			// 0x00: VTS
	KC81710H_REQ_CH_CELL01 = 1,			// 0x01-0x11: CELL#1~#17
	KC81710H_REQ_CH_CELL02,
	KC81710H_REQ_CH_CELL03,
	KC81710H_REQ_CH_CELL04,
	KC81710H_REQ_CH_CELL05,
	KC81710H_REQ_CH_CELL06,
	KC81710H_REQ_CH_CELL07,
	KC81710H_REQ_CH_CELL08,
	KC81710H_REQ_CH_CELL09,
	KC81710H_REQ_CH_CELL10,
	KC81710H_REQ_CH_CELL11,
	KC81710H_REQ_CH_CELL12,
	KC81710H_REQ_CH_CELL13,
	KC81710H_REQ_CH_CELL14,
	KC81710H_REQ_CH_CELL15,
	KC81710H_REQ_CH_CELL16,
	KC81710H_REQ_CH_CELL17,
	KC81710H_REQ_CH_THM0 = 0x12,		// 0x12-0x14: TS0~TS2
	KC81710H_REQ_CH_THM1,
	KC81710H_REQ_CH_THM2,
	KC81710H_REQ_CH_VCC = 0x15,			// 0x15: VCC
	KC81710H_REQ_CH_THM3 = 0x17,		// 0x17: TS3
	KC81710H_REQ_CH_VPACK = 0x1D,		// 0x1D: VPACK
	KC81710H_REQ_CH_VDMCU = 0x1E,		// 0x1E: VDMCU
	KC81710H_REQ_CH_VDDA = 0x1F,		// 0x1F: VDDA
}t_KC81710H_REQ_CH;

typedef enum {
	KC81710H_SW_CO_SEL_NORMAL = 0,
	KC81710H_SW_CO_SEL_0P5_0P25_1 = 1,
	KC81710H_SW_CO_SEL_0P5_0P25_2 = 2,
	KC81710H_SW_CO_SEL_0p5_PULL = 3,
}t_KC81710H_SW_CO_SEL;

typedef enum {
	KC81710H_SW_ADC1_LSB_14 = 0,
	KC81710H_SW_ADC1_LSB_16 = 1,
}t_KC81710H_SW_ADC1_LSB;

typedef enum {
	KC81710H_SW_REQ_NONE = 0,
	KC81710H_SW_REQ_SINGLE = 1,
	KC81710H_SW_REQ_THM_SCAN = 2,
	KC81710H_SW_REQ_VCC_CELLS_SCAN = 3,
}t_KC81710H_SW_REQ;

typedef union {
	uint16_t reg;
	struct {
		uint16_t req_ch:5;
		uint16_t rev1:1;
		uint16_t sw_co_sel:2;
		uint16_t rev2:2;
		uint16_t sw_lsb:1;
		uint16_t rev3:3;
		uint16_t sw_req:2;
	};
}t_KC81710H_TRIG;

typedef enum {
	KC81710H_SW_CC_NONE = 0,
	KC81710H_SW_CC_EN_NO_DEEP_SLEEP = 1,
	KC81710H_SW_CC_EN_ALWAYS = 3,	// 2b10/2b11
}t_KC81710H_SW_CC_MD;

typedef enum {
	KC81710H_SW_TS2_CFG_HW = 0,
	KC81710H_SW_TS2_CFG_AUX = 1,
	KC81710H_SW_TS2_CFG_CELL = 2,
	KC81710H_SW_TS2_CFG_MOSFET = 3,
}t_KC81710H_SW_TS2_CFG;

typedef enum {
	KC81710H_SW_TS3_CFG_AUX = 0,
	KC81710H_SW_TS3_CFG_THM = 1,
}t_KC81710H_SW_TS3_CFG;

typedef enum {
	KC81710H_SW_SLEEP_HW = 0,
	KC81710H_SW_SLEEP_DEEP = 1,
	KC81710H_SW_SLEEP_NORMAL_1S = 2,
	KC81710H_SW_SLEEP_NORMAL_2S = 3,
}t_KC81710H_SW_SLEEP_CTRL;

typedef enum {
	KC81710H_V0_DLY_2S = 0,
	KC81710H_V0_DLY_4S = 1,
	KC81710H_V0_DLY_8S = 2,
	KC81710H_V0_DLY_6S = 3,
	KC81710H_V0_DLY_24S = 4,
	KC81710H_V0_DLY_32S = 5,
	KC81710H_V0_DLY_48S = 6,
	KC81710H_V0_DLY_64S = 7,
}t_KC81710H_V0_DLY;

typedef union {
	uint16_t reg;
	struct {
		uint16_t v0_th:7;		// 0V~2.54V: N*20mV
		uint16_t v0_option:1;
		uint16_t v0_rls_hys:4;	// 0:none 1-15: N*40mV
		uint16_t v0_pre:1;
		uint16_t v0_dly:3;
	};
}t_KC81710H_V0_CTRL;


typedef enum {
	KC81710H_CO_START_TH_1V5 = 0,
	KC81710H_CO_START_TH_1V8 = 1,
	KC81710H_CO_START_TH_2V0 = 2,
	KC81710H_CO_START_TH_2V3 = 3,
}t_KC81710H_CO_START_TH;



/**************************************************************
 * DFE Data Struct
 **************************************************************/

typedef struct {
	int16_t cell_volt[CELL_NUMBER];	// 1mV
	int32_t vbat;					// 1mV
	int32_t vpack;					// 1mV
	int16_t thm[4]; 				// 0.1degC
	int32_t cadc_current;			// 1mA
	int32_t cc_raw_data;			// raw data
	int32_t cc_data;				// 1mAH

	uint16_t int_flag[4];
	uint16_t status[5];
	uint32_t cb_status;
//	uint32_t co_status;
} t_KC81710H_CHIP;
#if 0
struct chip
{
    int16_t cell_volt[CELL_NUMBER];
    int32_t cadc_current;
    int16_t external_temperature[3];
    uint16_t status0;
    uint16_t status1;
    uint16_t status2;
    uint16_t status3;
    uint16_t status4;
};
#endif
/*****************************************************************************
 * function prototype section
 * add function prototype here
 *****************************************************************************/
#if 0
DWORD version(void);
uint16_t id(void);
void errno_clr(void);
uint8_t errno_get(void);
#endif
uint8_t kc81710h_init(void);
uint8_t kc81710h_entry(t_KC81710H_CHIP *chip);
// Basic register read / write functions
uint8_t kc81710h_read_word(uint8_t addr, uint8_t index, uint16_t *buf);
uint8_t kc81710h_write_word(uint8_t addr, uint8_t index, uint16_t buf);
uint8_t crc8_calc(uint8_t *pdata, uint16_t n);
uint8_t S17H_init(void);
uint8_t kc81710h_read_status(t_KC81710H_CHIP *chip);
uint8_t kc81710h_read_flag(t_KC81710H_CHIP *chip);
int32_t kc81710h_cell_volt_mv_cal(int16_t rawdata);
int32_t kc81710h_thm_volt_mv_cal(int16_t rawdata);
int32_t kc81710h_thm_0degC1_cell_cal(int16_t rawdata);
int32_t kc81710h_thm_0degC1_mosfet_cal(int16_t rawdata);
int16_t kc81710h_thm_0degc1_mv_cal(int16_t rawdata);
int16_t kc81710h_thm_intemp_cal(int16_t rawdata);
int32_t kc81710h_current_ma_cal(int32_t rawdata, int16_t r_ur);
int32_t kc81710h_high_volt_mv_cal(int16_t rawdata);
int32_t kc81710h_low_volt_mv_cal(int16_t rawdata);
int32_t kc81710h_cc_mah_cal(int32_t rawdata, int16_t r_ur);
uint8_t kc81710h_read_current(int32_t *ma);
uint8_t kc81710h_read_cc_raw(int32_t *cc_raw);
uint8_t kc81710h_write_cc_raw(int32_t cc_raw);
uint8_t kc81710h_set_cc_mode(uint8_t mode);
uint8_t kc81710h_vadc_trig_ready(void);
uint8_t kc81710h_vadc_trig_wait(void);
uint8_t kc81710h_vadc_trig_set(t_KC81710H_TRIG trig);
uint8_t kc81710h_vadc_trig_req(t_KC81710H_TRIG trig, uint8_t wait_comp);
uint8_t kc81710h_vadc_read_raw(t_KC81710H_REQ_CH ch, uint16_t *raw);
uint8_t kc81710h_cb_set(uint32_t cb);
uint8_t kc81710h_cb_read(uint32_t *cb);
uint8_t kc81710h_swcb_enable(void);
uint8_t kc81710h_swcb_disable(void);
uint8_t kc81710h_co_config_set(uint16_t bits);
uint8_t kc81710h_co_read(uint32_t* co);
uint8_t kc81710h_sw_release_enable(void);
uint8_t kc81710h_sw_config_set(uint16_t bits);
uint8_t kc81710h_sw_config_clr(uint16_t bits);
uint8_t kc81710h_fuse_blow_req(void);
uint8_t kc81710h_fuse_blow_release(void);
uint8_t kc81710h_fuse_config(uint16_t bits);
uint8_t kc81710h_chg_fail_release(void);
uint8_t kc81710h_dsg_fail_release(void);
uint8_t kc81710h_mosfet_set(uint16_t bits);
uint8_t kc81710h_mosfet_clr(uint16_t bits);
uint8_t kc81710h_pdsg_timeout_set(uint16_t ticks);
uint8_t kc81710h_pdsg_config_set(uint16_t bits);
uint8_t kc81710h_pdsg_config_clr(uint16_t bits);
uint8_t kc81710h_vmcu2_control_down(void);
uint8_t kc81710h_vmcu2_control_work(void);
uint8_t kc81710h_sleep_control_set(uint16_t cfg);
uint8_t kc81710h_power_sleep(void);
uint8_t kc81710h_power_wakeup(void);
uint8_t kc81710h_power_shutdown(void);
uint8_t kc81710h_ts_config(uint16_t ts2cfg, uint16_t ts3cfg);
uint8_t kc81710h_pchg_config(uint16_t bits);
uint8_t kc81710h_deep_sleep_prepare(void);
uint8_t kc81710h_deep_sleep_enter(void);
uint8_t kc81710h_deep_sleep_restore(void);
uint8_t kc81710h_auto_scan_stop_set(void);
uint8_t kc81710h_auto_scan_stop_clr(void);
uint8_t kc81710h_sw_check_all_req(uint8_t wait_comp);
uint8_t ZeroClib(uint8_t markupdate);

#if 0
uint8_t scan(uint8_t CH, int32_t *buf);
#endif
#endif

