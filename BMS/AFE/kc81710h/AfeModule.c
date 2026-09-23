
#include "n32l40x.h"
#include "McuHal_N32L40X.h"
//#include "SysDefs.h"
#include "Board.h"
#include "DataFlash.h"
//#include "AdcConvert.h"
#include "AfeModule.h"
#include "Calib.h" 

typedef enum
{
	AFE_TRIG_INDEX_THM3,
	AFE_TRIG_INDEX_VPACK,
	AFE_TRIG_INDEX_NUM
} _AFE_TRIG_INDEX_T;

volatile uint8_t scanIndex;
static u8 TimerAfeFlag;
static s32 AfeCurNow = 0;
//AfeStatus_T AfeStatus;

t_KC81710H_CHIP qcgChip;

t_KC81710H_REQ_CH AfeTriggerChannel[AFE_TRIG_INDEX_NUM] =
{
	KC81710H_REQ_CH_THM3,
	KC81710H_REQ_CH_VPACK,
};

uint8_t i2c_read_block(uint8_t addr, uint8_t cmd, uint8_t len, uint8_t *buf)
{
	return I2C_ReadBuffer(I2C_AFE_INDEX, addr, cmd, buf, len);
}

uint8_t i2c_write_block(uint8_t addr, uint8_t cmd, uint8_t len, uint8_t *buf)
{
	return I2C_WriteBuffer(I2C_AFE_INDEX, addr, cmd, buf, len);
}

void AfeDelayMs(unsigned int ms)
{
	Delay1ms(ms);
}

void AfeTimerInit(void)
{	
	TimerAfeFlag = 0;
	CbState = 0;
}

void AfeTimerCallBack(u8 ticks)
{	
	TimerAfeFlag = 1;
}

void MosStatusFlush(void)
{
//    uint16_t buf;
    uint8_t ret;
//    ret = oz77218_read_word(ozChip.drv, OZ77218_OP_STATUS2, &buf);
//    if (ret) return;
//    ozChip.status[1] = buf;
//    BatStatus.Bits.AfeDsg = (ozChip.status[1]&0x100)>0?1:0;
//    BatStatus.Bits.AfeChg = (ozChip.status[1]&0x200)>0?1:0;
		ret = kc81710h_read_status(&qcgChip);
		if (ret) return;
	
	AfeStatus.Bytes.Status1 = qcgChip.status[1];
	AfeStatus.Bytes.Status2 = qcgChip.status[2];
	AfeStatus.Bytes.Status4 = qcgChip.status[4];
	
	BatStatus.Bits.AfeDsg = 0;
	if(AfeStatus.Bits.DSG_EN || AfeStatus.Bits.PDSG_EN)
	{
		BatStatus.Bits.AfeDsg = 1;
	}
	
	BatStatus.Bits.AfeChg = 0;
	if(AfeStatus.Bits.CHG_EN || AfeStatus.Bits.PCHG_EN)
	{
		BatStatus.Bits.AfeChg = 1;
	}
}

void AfeInit(void)
{
	uint8_t result = 0;
	uint8_t count = 0;

	scanIndex = 0;

	do{
		result = kc81710h_init();
		if(result == 0)
		{
			break;
		}
	}while(count++ < 5);

	if(result != 0)
	{
		BatStatus.Bits.AfeErr = 1;
	}
}

uint8_t AfeSetMosfet(uint8_t status)
{
	uint8_t ret;

	switch(status)
	{
		case 0:
			// clear mosfet
			ret = kc81710h_mosfet_clr(KC81710H_SW_CHG_EN_MSK | KC81710H_SW_DSG_EN_MSK);
			if (ret) return ret;
			break;
		case 1:
			ret = kc81710h_mosfet_clr(KC81710H_SW_CHG_EN_MSK);
			if (ret) return ret;
			ret = kc81710h_mosfet_set(KC81710H_SW_DSG_EN_MSK);
			if (ret) return ret;
			break;
		case 2:
			ret = kc81710h_mosfet_clr(KC81710H_SW_DSG_EN_MSK);
			if (ret) return ret;
			ret = kc81710h_mosfet_set(KC81710H_SW_CHG_EN_MSK);
			if (ret) return ret;
			break;
		case 3:
			ret = kc81710h_mosfet_set(KC81710H_SW_CHG_EN_MSK | KC81710H_SW_DSG_EN_MSK);
			if (ret) return ret;
			break;
		case 4:
			ret = kc81710h_sw_config_clr(KC81710H_SW_LOAD_CHK_MSK);
			if (ret) return ret;
			break;
	}

	return KC_STATUS_OK;
}

void AfeFetCtrl(void)
{
	volatile u8 FetCmd = 0;
	static u8 LastCmd = 0xFF;		// make sure do one time while enter this fuction first
	static u8 predisflag = 0;
	
	if(MtBmsAllow.bit.DisMosAllow)
    {
    	FetCmd = (u8)BatStatus.Bits.Dmos;
    }
	if(MtBmsAllow.bit.ChgMosAllow)
    {
    	FetCmd += ((u8)BatStatus.Bits.Cmos)<<1;
    }
    
    if (FetCmd != LastCmd)
	{
		if( KC_STATUS_OK == AfeSetMosfet(FetCmd) )
		{
			LastCmd = FetCmd;
		}
	}

	// 预放电配置
	if(BatStatus.Bits.PDmos && MtBmsAllow.bit.DisMosAllow)
	{
		if(predisflag == 0)
		{
			predisflag = 1;
			kc81710h_pdsg_timeout_set(800/KC81710H_PDSG_TIME_TICK);
		}
	}
	else
	{
		if(predisflag == 1)
		{
			kc81710h_pdsg_timeout_set(0);
		}
		predisflag = 0;
	}
	
//	MosStatusFlush();
	fetchangeflushstatus(FetCmd);
}

uint8_t AfeSetPowerMode(uint8_t mode)
{
	uint8_t ret;

	switch(mode)
	{
		case 0:
			ret = kc81710h_power_wakeup();
			if (ret) return ret;
			break;
		case 1:
			ret = kc81710h_power_sleep();
			if (ret) return ret;
			break;
		case 2:
			ret = kc81710h_power_shutdown();
			if (ret) return ret;
			break;
	}

	return KC_STATUS_OK;
}

uint8_t AfeSetPdsgTimeout(uint16_t ms)
{
	uint8_t ret;
	uint16_t ticks;

	// 255*16ms = 4080ms
	ticks = ms / KC81710H_PDSG_TIME_TICK;
	if(ticks > (KC81710H_PDSG_TIME_MSK >> KC81710H_PDSG_TIME_POS))
	{
		ticks = (KC81710H_PDSG_TIME_MSK >> KC81710H_PDSG_TIME_POS);
	}
	ret = kc81710h_pdsg_timeout_set(ticks << KC81710H_PDSG_TIME_POS);

	return ret;
}

uint8_t AfeClearFlags(void)
{
	uint8_t ret;
	uint16_t status;
	static uint16_t flagLast[4] = {0, 0, 0, 0};
	static uint8_t flagCount[4] = {0, 0, 0, 0};
	t_KC81710H_CHIP *chip = &qcgChip;

	#define CLEAR_FLAGS_COUNT_MAX		65

	status = chip->int_flag[0];
	if(status != flagLast[0])
	{
		flagLast[0] = status;
		flagCount[0] = 0;
	}
	if(status != 0)	// 0xD0
	{
		flagCount[0] += 1;
		if(flagCount[0] >= CLEAR_FLAGS_COUNT_MAX)
		{
			ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_IF0, chip->int_flag[0]);
			if (ret) return ret;
			flagCount[0] = 0;
		}
	}
	else
	{
		flagCount[0] = 0;
	}

	status = chip->int_flag[1];
	if(status != flagLast[1])
	{
		flagLast[1] = status;
		flagCount[1] = 0;
	}
	if(status != 0)	// 0xD1
	{
		flagCount[1] += 1;
		if(flagCount[1] >= CLEAR_FLAGS_COUNT_MAX)
		{
			ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_IF1, chip->int_flag[1]);
			if (ret) return ret;
			flagCount[1] = 0;
		}
	}
	else
	{
		flagCount[1] = 0;
	}

	status = (chip->int_flag[2] & 0xFF9E);
	if(status != flagLast[2])
	{
		flagLast[2] = status;
		flagCount[2] = 0;
	}
	if(status != 0)	// 0xD2
	{
		flagCount[2] += 1;
		if(flagCount[2] >= CLEAR_FLAGS_COUNT_MAX)
		{
			ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_IF2, chip->int_flag[2] & 0xFF9E);
			if (ret) return ret;
			flagCount[2] = 0;
		}
	}
	else
	{
		flagCount[2] = 0;
	}

	status = (chip->int_flag[3] & 0xFFF0);
	if(status != flagLast[3])
	{
		flagLast[3] = status;
		flagCount[3] = 0;
	}
	if(status != 0)	// 0xD3
	{
		flagCount[3] += 1;
		if(flagCount[3] >= CLEAR_FLAGS_COUNT_MAX)
		{
			ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_IF3, chip->int_flag[3] & 0xFFF0);
			if (ret) return ret;
			flagCount[3] = 0;
		}
	}
	else
	{
		flagCount[3] = 0;
	}

	return KC_STATUS_OK;
}

uint8_t AfeGetInfo(t_KC81710H_CHIP *chip)
{
	uint8_t i;
	uint8_t ret = 0;
	uint16_t bufdata;
	t_KC81710H_TRIG trig = {0};
	static uint8_t trigIndex = 0;

	switch(scanIndex)
	{
		case 0:
			// write trigger channel
			ret = kc81710h_vadc_trig_ready();
			if (ret) break;
//			SET_TEST_LED_ON();
			trig.sw_req = KC81710H_SW_REQ_SINGLE;
			trig.sw_lsb = KC81710H_SW_ADC1_LSB_16;
			trig.req_ch = AfeTriggerChannel[trigIndex];
			ret = kc81710h_vadc_trig_set(trig);
			if (ret) break;
			scanIndex = 1;
			break;

		case 1:
			// read status
			ret = kc81710h_read_status(chip);
			if (ret) break;
			scanIndex = 2;
			break;

		case 2:
			// read int flag
			ret = kc81710h_read_flag(chip);
			if (ret) break;
			// cell balance_status
			ret = kc81710h_cb_read(&chip->cb_status);
			if (ret) break;
			scanIndex = 3;
			break;

		case 3:
			// read vbat voltage
			ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_VCC_ADC1, &bufdata);
			if (ret) break;
			chip->vbat = kc81710h_high_volt_mv_cal((int16_t)bufdata);
			// read cadc current
			ret = kc81710h_read_current(&chip->cadc_current);

			CalibZeroCurRun(qcgChip.cadc_current);
			if (ret) break;
			// read cc raw
			ret = kc81710h_read_cc_raw(&chip->cc_raw_data);
			if (ret) break;
			// calculate cc mah
			chip->cc_data = kc81710h_cc_mah_cal(chip->cc_raw_data, RSENSE);
			scanIndex = 4;
			break;

		case 4:
			// read cell voltage
			for (i = 0; i < CELL_NUMBER; i++)
			{
				ret = kc81710h_read_word(KC81710H_I2C_ADDR, (i == CELL_NUMBER - 1) ? KC81710H_OP_CH_CELL17 : (KC81710H_OP_CH_CELL01 + i), &bufdata);
				if (ret) break;
				chip->cell_volt[i] = kc81710h_cell_volt_mv_cal((int16_t)bufdata);
			}
			if (ret) break;
			scanIndex = 5;
			break;

		case 5:
			// read temperature
			ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_TS0R, &bufdata);
			if (ret) break;
			chip->thm[0] = kc81710h_thm_0degC1_cell_cal((int16_t)bufdata);
			ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_TS1R, &bufdata);
			if (ret) break;
			chip->thm[1] = kc81710h_thm_0degC1_cell_cal((int16_t)bufdata);
//			ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_TS2R, &bufdata);
//			if (ret) break;
//			chip->thm[2] = kc81710h_thm_0degC1_cell_cal((int16_t)bufdata);
			scanIndex = 6;
			break;

		case 6:
			// read trigger channel
			ret = kc81710h_vadc_trig_ready();
			if (ret) break;
//			SET_TEST_LED_OFF();
			ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_COMMON_ADC1, &bufdata);
			if (ret) break;
			switch(trigIndex)
			{
				case AFE_TRIG_INDEX_THM3:
					chip->thm[3] = kc81710h_thm_0degc1_mv_cal((int16_t)bufdata);
					break;
				case AFE_TRIG_INDEX_VPACK:
					chip->vpack = kc81710h_high_volt_mv_cal((int16_t)bufdata);
					break;
				default:
					break;
			}
			trigIndex += 1;
			if(trigIndex >= AFE_TRIG_INDEX_NUM) trigIndex = 0;
			scanIndex = 7;
			break;

		default:
			ret = AfeClearFlags();
			if (ret) break;
			scanIndex = 0;
			break;
	}

	return ret;
}
void AfeInfoUpdata(void)
{
	#define ERR_DELAY		5
	u8 i;
	static u8 ChgErrDelay[12] = {0};
	static u8 DisErrDelay[12] = {0};
	static u8 SysErrDelay[2] = {0};
	
	// 更新电压
	for(i=0;i<CELL_NUM;i++)
	{
		BatData.Bat[i].Vol = qcgChip.cell_volt[i];
	}
	
	// 更新Vbat
	if(!BatStatus.Bits.AfeErr)
	{
		BatData.VolTotal = qcgChip.vbat;
	}
	else
	{
		BatData.VolTotal = PackAvgVol;
	}
	
	// 更新Vpack
	BatData.VolPack = qcgChip.vpack * DATA_VOL_K_PACK / VOL_K_DEFAULT;
	BatData.VolStack = BatData.VolTotal;
	
	// 更新电流
	qcgChip.cadc_current = qcgChip.cadc_current- DATA_AFE_MC_BIAS;
	
	BatData.cur = qcgChip.cadc_current;
	if(qcgChip.cadc_current > 0)
	{
		DisCurAvg = 0;
		ChgCurAvg = qcgChip.cadc_current;
		ChgCurAvg = ChgCurAvg*DATA_CUR_K_CHG/CUR_K_DEFAULT;
		AfeCurNow = ChgCurAvg;
	}
	else
	{
		ChgCurAvg = 0;
		DisCurAvg = -qcgChip.cadc_current;
		DisCurAvg = DisCurAvg*DATA_CUR_K_DSGM/CUR_K_DEFAULT;
		AfeCurNow = -1 * (s32)DisCurAvg;
	}
	
	// 更新温度
	BatNtcTemp[1] = (u16)(2731+qcgChip.thm[0]);
	BatNtcTemp[2] = (u16)(2731+qcgChip.thm[1]);

	// 均衡节数
	CbState = 0;
	if(qcgChip.cb_status & 0x010000)
		CbState = 0x0800;
	CbState |= (qcgChip.cb_status & 0x7ff);

	// 状态更新

//	AfeStatus.Bytes.Status0 = qcgChip.status[0];
//	AfeStatus.Bytes.Status3 = qcgChip.status[3];
	
	AfeStatus.Bytes.Status1 = qcgChip.status[1];
	AfeStatus.Bytes.Status2 = qcgChip.status[2];
	AfeStatus.Bytes.Status4 = qcgChip.status[4];
	
	BatStatus.Bits.AfeDsg = 0;
	if(AfeStatus.Bits.DSG_EN || AfeStatus.Bits.PDSG_EN)
	{
		BatStatus.Bits.AfeDsg = 1;
	}
	BatStatus.Bits.AfeChg = 0;
	if(AfeStatus.Bits.CHG_EN || AfeStatus.Bits.PCHG_EN)
	{
		BatStatus.Bits.AfeChg = 1;
	}

	
	// 充电异常
	if(qcgChip.status[0] && BatStatus.Bits.ChgPlugin)
	{
		if(qcgChip.status[0] & 0x0001)
		{
			ChgErrDelay[0]++;
			if((ChgErrDelay[0]>ERR_DELAY) && !AfeStatus.Bits.OV_STATUS)
			{
				ChgErrDelay[0] = 0;
				AfeStatus.Bits.OV_STATUS = 1;
				BatStatus.Bits.ChgOV = 1;
			}
		}
		else
			ChgErrDelay[0] = 0;
		
		if(qcgChip.status[0] & 0x0020)
		{
			ChgErrDelay[1]++;
			if((ChgErrDelay[1]>ERR_DELAY) && !AfeStatus.Bits.OCC1_STATUS)
			{
				ChgErrDelay[1] = 0;
				AfeStatus.Bits.OCC1_STATUS = 1;
				BatStatus.Bits.ChgOC = 1;
			}
		}
		else
			ChgErrDelay[1] = 0;

		if(qcgChip.status[0] & 0x0100)
		{
			ChgErrDelay[2]++;
			if((ChgErrDelay[2]>ERR_DELAY) && !AfeStatus.Bits.OCC2_STATUS)
			{
				ChgErrDelay[2] = 0;
				AfeStatus.Bits.OCC2_STATUS = 1;
				BatStatus.Bits.ChgOC = 1;
			}
		}
		else
			ChgErrDelay[2] = 0;
		// 暂未启用
/*		if(qcgChip.status[0] & 0x0200)
		{
			ChgErrDelay[3]++;
			if((ChgErrDelay[3]>ERR_DELAY) && !AfeStatus.Bits.SOV_STATUS)
			{
				ChgErrDelay[3] = 0;
				AfeStatus.Bits.SOV_STATUS = 1;
			}
		}
		else
			ChgErrDelay[3] = 0;
*/
		if(qcgChip.status[0] & 0x0800)
		{
			ChgErrDelay[4]++;
			if((ChgErrDelay[4]>ERR_DELAY) && !AfeStatus.Bits.OTC_STATUS)
			{
				ChgErrDelay[4] = 0;
				AfeStatus.Bits.OTC_STATUS = 1;
			}
		}
		else
			ChgErrDelay[4] = 0;

		if(qcgChip.status[0] & 0x1000)
		{
			ChgErrDelay[5]++;
			if((ChgErrDelay[5]>ERR_DELAY) && !AfeStatus.Bits.UTC_STATUS)
			{
				ChgErrDelay[5] = 0;
				AfeStatus.Bits.UTC_STATUS = 1;
			}
		}
		else
			ChgErrDelay[5] = 0;		
	}
	else
	{
		ChgErrDelay[0] = 0;
		ChgErrDelay[1] = 0;
		ChgErrDelay[2] = 0;
		ChgErrDelay[3] = 0;
		ChgErrDelay[4] = 0;
		ChgErrDelay[5] = 0;	
	}

	if(qcgChip.status[3] && BatStatus.Bits.ChgPlugin)
	{
		if(qcgChip.status[3] & 0x0002)
		{
			ChgErrDelay[6]++;
			if((ChgErrDelay[6]>ERR_DELAY) && !AfeStatus.Bits.TS0_OTC_STATUS)
			{
				ChgErrDelay[6] = 0;
				AfeStatus.Bits.TS0_OTC_STATUS = 1;
			}
		}
		else
			ChgErrDelay[6] = 0;
		
		if(qcgChip.status[3] & 0x0004)
		{
			ChgErrDelay[7]++;
			if((ChgErrDelay[7]>ERR_DELAY) && !AfeStatus.Bits.TS0_UTC_STATUS)
			{
				ChgErrDelay[7] = 0;
				AfeStatus.Bits.TS0_UTC_STATUS = 1;
			}
		}
		else
			ChgErrDelay[7] = 0;

		if(qcgChip.status[3] & 0x0020)
		{
			ChgErrDelay[8]++;
			if((ChgErrDelay[8]>ERR_DELAY) && !AfeStatus.Bits.TS1_OTC_STATUS)
			{
				ChgErrDelay[8] = 0;
				AfeStatus.Bits.TS1_OTC_STATUS = 1;
			}
		}
		else
			ChgErrDelay[8] = 0;

		if(qcgChip.status[3] & 0x0040)
		{
			ChgErrDelay[9]++;
			if((ChgErrDelay[9]>ERR_DELAY) && !AfeStatus.Bits.TS1_UTC_STATUS)
			{
				ChgErrDelay[9] = 0;
				AfeStatus.Bits.TS1_UTC_STATUS = 1;
			}
		}
		else
			ChgErrDelay[9] = 0;

		if(qcgChip.status[3] & 0x0200)
		{
			ChgErrDelay[10]++;
			if((ChgErrDelay[10]>ERR_DELAY) && !AfeStatus.Bits.TS2_OTC_STATUS)
			{
				ChgErrDelay[10] = 0;
				AfeStatus.Bits.TS2_OTC_STATUS = 1;
			}
		}
		else
			ChgErrDelay[10] = 0;

		if(qcgChip.status[3] & 0x0400)
		{
			ChgErrDelay[11]++;
			if((ChgErrDelay[11]>ERR_DELAY) && !AfeStatus.Bits.TS2_UTC_STATUS)
			{
				ChgErrDelay[11] = 0;
				AfeStatus.Bits.TS2_UTC_STATUS = 1;
			}
		}
		else
			ChgErrDelay[11] = 0;		
	}
	else
	{
		ChgErrDelay[6] = 0;
		ChgErrDelay[7] = 0;
		ChgErrDelay[8] = 0;
		ChgErrDelay[9] = 0;
		ChgErrDelay[10] = 0;
		ChgErrDelay[11] = 0;
	}

	// 放电异常
	if(qcgChip.status[0] && !BatStatus.Bits.ChgPlugin)
	{
		if(qcgChip.status[0] & 0x0002)
		{
			DisErrDelay[0]++;
			if((DisErrDelay[0]>ERR_DELAY) && !AfeStatus.Bits.UV_STATUS_REAL)
			{
				DisErrDelay[0] = 0;
				AfeStatus.Bits.UV_STATUS_REAL = 1;
				BatStatus.Bits.DisUV = 1;
				
			}
		}
		else
			DisErrDelay[0] = 0;
		
		if(qcgChip.status[0] & 0x0004)
		{
			DisErrDelay[1]++;
			if((DisErrDelay[1]>ERR_DELAY) && !AfeStatus.Bits.OCD1_STATUS)
			{
				DisErrDelay[1] = 0;
				AfeStatus.Bits.OCD1_STATUS = 1;
				BatStatus.Bits.DisOC = 1;
			}
		}
		else
			DisErrDelay[1] = 0;

		if(qcgChip.status[0] & 0x0008)
		{
			DisErrDelay[2]++;
			if((DisErrDelay[2]>ERR_DELAY) && !AfeStatus.Bits.OCD2_STATUS)
			{
				DisErrDelay[2] = 0;
				AfeStatus.Bits.OCD2_STATUS = 1;
				BatStatus.Bits.DisOC = 1;
			}
		}
		else
			DisErrDelay[2] = 0;

		if(qcgChip.status[0] & 0x0010)
		{
			DisErrDelay[3]++;
			if((DisErrDelay[3]>ERR_DELAY) && !AfeStatus.Bits.SCD_STATUS)
			{
				DisErrDelay[3] = 0;
				AfeStatus.Bits.SCD_STATUS = 1;
				BatStatus.Bits.DisSC = 1;
			}
		}
		else
			DisErrDelay[3] = 0;

		if(qcgChip.status[0] & 0x0400)
		{
			DisErrDelay[4]++;
			if((DisErrDelay[4]>ERR_DELAY) && !AfeStatus.Bits.OTD_STATUS)
			{
				DisErrDelay[4] = 0;
				AfeStatus.Bits.OTD_STATUS = 1;
			}
		}
		else
			DisErrDelay[4] = 0;

		if(qcgChip.status[0] & 0x2000)
		{
			DisErrDelay[5]++;
			if((DisErrDelay[5]>ERR_DELAY) && !AfeStatus.Bits.UTD_STATUS)
			{
				DisErrDelay[5] = 0;
				AfeStatus.Bits.UTD_STATUS = 1;
			}
		}
		else
			DisErrDelay[5] = 0;		
	}
	else
	{
		DisErrDelay[0] = 0;
		DisErrDelay[1] = 0;
		DisErrDelay[2] = 0;
		DisErrDelay[3] = 0;
		DisErrDelay[4] = 0;
		DisErrDelay[5] = 0;	
	}

	if(qcgChip.status[3] && !BatStatus.Bits.ChgPlugin)
	{
		if(qcgChip.status[3] & 0x0001)
		{
			DisErrDelay[6]++;
			if((DisErrDelay[6]>ERR_DELAY) && !AfeStatus.Bits.TS0_OTD_STATUS)
			{
				DisErrDelay[6] = 0;
				AfeStatus.Bits.TS0_OTD_STATUS = 1;
			}
		}
		else
			DisErrDelay[6] = 0;
		
		if(qcgChip.status[3] & 0x0008)
		{
			DisErrDelay[7]++;
			if((ChgErrDelay[7]>ERR_DELAY) && !AfeStatus.Bits.TS0_UTD_STATUS)
			{
				DisErrDelay[7] = 0;
				AfeStatus.Bits.TS0_UTD_STATUS = 1;
			}
		}
		else
			DisErrDelay[7] = 0;

		if(qcgChip.status[3] & 0x0010)
		{
			DisErrDelay[8]++;
			if((DisErrDelay[8]>ERR_DELAY) && !AfeStatus.Bits.TS1_OTD_STATUS)
			{
				DisErrDelay[8] = 0;
				AfeStatus.Bits.TS1_OTD_STATUS = 1;
			}
		}
		else
			DisErrDelay[8] = 0;

		if(qcgChip.status[3] & 0x0080)
		{
			DisErrDelay[9]++;
			if((DisErrDelay[9]>ERR_DELAY) && !AfeStatus.Bits.TS1_UTD_STATUS)
			{
				DisErrDelay[9] = 0;
				AfeStatus.Bits.TS1_UTD_STATUS = 1;
			}
		}
		else
			DisErrDelay[9] = 0;

		if(qcgChip.status[3] & 0x0100)
		{
			DisErrDelay[10]++;
			if((DisErrDelay[10]>ERR_DELAY) && !AfeStatus.Bits.TS2_OTD_STATUS)
			{
				DisErrDelay[10] = 0;
				AfeStatus.Bits.TS2_OTD_STATUS = 1;
			}
		}
		else
			DisErrDelay[10] = 0;

		if(qcgChip.status[3] & 0x0800)
		{
			DisErrDelay[11]++;
			if((DisErrDelay[11]>ERR_DELAY) && !AfeStatus.Bits.TS2_UTD_STATUS)
			{
				DisErrDelay[11] = 0;
				AfeStatus.Bits.TS2_UTD_STATUS = 1;
			}
		}
		else
			DisErrDelay[11] = 0;		
	}
	else
	{
		DisErrDelay[6] = 0;
		DisErrDelay[7] = 0;
		DisErrDelay[8] = 0;
		DisErrDelay[9] = 0;
		DisErrDelay[10] = 0;
		DisErrDelay[11] = 0;		
	}

	// 内部过热 
	if(qcgChip.status[0] & 0x4000)
	{
		SysErrDelay[0]++;
		if((SysErrDelay[0]>ERR_DELAY) && !AfeStatus.Bits.OHT_STATUS)
		{
			SysErrDelay[0] = 0;
			AfeStatus.Bits.OHT_STATUS = 1;
		}
	}
	else
		SysErrDelay[0] = 0;
	
	// 断线 
	if(qcgChip.status[0] & 0x8000)
	{
		SysErrDelay[1]++;
		if((SysErrDelay[1]>ERR_DELAY) && !AfeStatus.Bits.CO_STATUS)
		{
			SysErrDelay[1] = 0;
			AfeStatus.Bits.CO_STATUS = 1;
			BatStatus.Bits.BatErr = 1;
		}
	}
	else
		SysErrDelay[1] = 0;

	// 充电异常恢复检测
	if(!BatStatus.Bits.ChgPlugin)
	{
		if(AfeStatus.Bits.OTC_STATUS || AfeStatus.Bits.UTC_STATUS 
			|| AfeStatus.Bits.TS0_OTC_STATUS || AfeStatus.Bits.TS0_UTC_STATUS
			|| AfeStatus.Bits.TS1_OTC_STATUS || AfeStatus.Bits.TS1_UTC_STATUS
			|| AfeStatus.Bits.TS2_OTC_STATUS || AfeStatus.Bits.TS2_UTC_STATUS
			|| AfeStatus.Bits.OCC1_STATUS	 || AfeStatus.Bits.OCC2_STATUS
			|| AfeStatus.Bits.OV_STATUS)
		{
			AfeStatus.Bits.TS0_OTC_STATUS = 0; 
			AfeStatus.Bits.TS0_UTC_STATUS = 0;
			AfeStatus.Bits.TS1_OTC_STATUS = 0; 
			AfeStatus.Bits.TS1_UTC_STATUS = 0;
			AfeStatus.Bits.TS2_OTC_STATUS = 0;
			AfeStatus.Bits.TS2_UTC_STATUS = 0;
			AfeStatus.Bits.OTC_STATUS = 0;
			AfeStatus.Bits.UTC_STATUS = 0;
			AfeStatus.Bits.OV_STATUS = 0;
			AfeStatus.Bits.OCC1_STATUS = 0;
			AfeStatus.Bits.OCC2_STATUS = 0;
		}
	}
	// 放电异常恢复检测
	if(BatStatus.Bits.ChgPlugin)
	{
		if(AfeStatus.Bits.UTD_STATUS || AfeStatus.Bits.OTD_STATUS
			|| AfeStatus.Bits.TS0_OTD_STATUS || AfeStatus.Bits.TS0_UTD_STATUS
			|| AfeStatus.Bits.TS1_OTD_STATUS || AfeStatus.Bits.TS1_UTD_STATUS
			|| AfeStatus.Bits.TS2_OTD_STATUS || AfeStatus.Bits.TS2_UTD_STATUS
	//		|| AfeStatus.Bits.OCD1_STATUS	 || AfeStatus.Bits.OCD2_STATUS
			|| AfeStatus.Bits.UV_STATUS_REAL)
		{
			AfeStatus.Bits.TS0_OTD_STATUS = 0; 
			AfeStatus.Bits.TS0_UTD_STATUS = 0;
			AfeStatus.Bits.TS1_OTD_STATUS = 0; 
			AfeStatus.Bits.TS1_UTD_STATUS = 0;
			AfeStatus.Bits.TS2_OTD_STATUS = 0;
			AfeStatus.Bits.TS2_UTD_STATUS = 0;
			AfeStatus.Bits.OTD_STATUS = 0;
			AfeStatus.Bits.UTD_STATUS = 0;
			AfeStatus.Bits.UV_STATUS_REAL = 0;
	//		AfeStatus.Bits.OCD1_STATUS = 0;
	//		AfeStatus.Bits.OCD2_STATUS = 0;
		}
	}
}

void AfeRun(void)
{
	uint8_t result;
	static uint8_t faultCount = 0;

	// 10ms
	if (!TimerAfeFlag)  
		return;
	TimerAfeFlag = 0;

	// fet ctrl
	AfeFetCtrl();
	
//	result = kc81710h_entry(&qcgChip);
	result = AfeGetInfo(&qcgChip);	
	if(result != 0)
	{
		faultCount += 1;
		if(faultCount >= 100)
		{
			BatStatus.Bits.AfeErr = 1;
		}
	}
	else
	{
		faultCount = 0;
	}
	AfeInfoUpdata();
//	AfeDeepSleepManage();
}

uint8_t AfeChipStatusGet(void)
{
	return BatStatus.Bits.AfeErr;
}

void AfeChipStatusClear(void)
{
	if(BatStatus.Bits.AfeErr)
	{
		BatStatus.Bits.AfeErr = 0;
	}
}

void AfeDeepSleepManage(void)
{
	uint8_t result;
	static uint8_t testStatus = 0;
	static uint16_t testCount = 0;

	switch(testStatus)
	{
		case 0:	// delay 10 seconds
			testCount += 1;
			if(testCount >= 1000)
			{
				testCount = 0;
				testStatus = 1;
			}
			break;
		case 1:
			result = kc81710h_deep_sleep_prepare();
			if(result == 0)
			{
				testStatus = 2;
			}
			break;
		case 2:
			if(qcgChip.status[1] & KC81710H_STATUS1_DEEP_SLEEP_MSK)
			{
				result = kc81710h_deep_sleep_enter();
				if(result == 0)
				{
					testStatus = 3;
				}
			}
			break;
		case 3:
			if((qcgChip.status[1] & KC81710H_STATUS1_DEEP_SLEEP_MSK) == 0)
			{
				testStatus = 4;
			}
			break;
		case 4:
			result = kc81710h_deep_sleep_restore();
			if(result == 0)
			{
				testStatus = 5;
			}
			break;
		case 5:
			result = kc81710h_sleep_control_set(KC81710H_SW_SLEEP_HW);
			if(result == 0)
			{
				testCount = 0;
				testStatus = 0;
			}
			break;
		default:
			testCount = 0;
			break;
	}
}

u8 AfeSetBalance(u32 channels)
{
	u8 ret;
    return ret;
}

s32 AfeGetCurrentFilt(void)
{
	return AfeCurNow;
}
