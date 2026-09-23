/*****************************************************************************
 * Copyright(c) QCG, 2023. All rights reserved.
 *
 * QCG [S17H] Source Code Reference Design
 * File: S17H.c
 *
 * This Source Code Reference Design for QCG [S17H] access
 * ("Reference Design") is solely for the use of PRODUCT INTEGRATION REFERENCE ONLY,
 * and contains confidential and privileged information of QCG International
 * Limited. QCG shall have no liability to any PARTY FOR THE RELIABILITY,
 * SERVICEABILITY FOR THE RESULT OF PRODUCT INTEGRATION, or results from: (i) any
 * modification or attempted modification of the Reference Design by any party, or
 * (ii) the combination, operation or use of the Reference Design with non-QCG
 * Reference Design. Use of the Reference Design is at user's discretion to qualify
 * the final work result.
 *****************************************************************************/
/*****************************************************************************
 *	Include Section
 *	add all #include here
 *****************************************************************************/

#include "kc81710h.h"
#include "stdio.h"
#include "lut_therm.h"
#include "Board.h"
//#include "I2cSoftware.h"
#include "n32l40x.h"
#include "McuHal_N32L40X.h"
#include "dataflash.h"
#include "Calib.h" 

#if 0
#include "stm32f0xx_hal.h"
#include "i2cif.h"

/*****************************************************************************
 * Define section
 * add all #define here
 *****************************************************************************/
#define RETRY_TIMEOUT 1000

#define SUB_ABS(data1, data2) (data1 > data2) ? (data1 - data2) : (data2 - data1);

struct chip *p_dfe_device;
struct chip _chip;
static uint8_t errno = 0;
#endif
/******************************************************************************
 * Function prototype section
 * add prototypes for all functions called by this file,execept those
 * declared in header file
 *****************************************************************************/

/*****************************************************************************
 * global variables section
 * add declaration of global variables here
 * e.g.
 *	int8_t foo;
 ****************************************************************************/

/*****************************************************************************
 * local variables section
 * define local variables(will be refered only in this file) here,
 * static keyword should be used to limit scope of local variable to this file
 *****************************************************************************/

/*****************************************************************************
 * Description:
 *		one_latitude_table
 * Parameters:
 *	    number: number of the temperature data array
 *	    data: data buffer
 *	    value: x data in the array
 * Return:
 *		what does this function returned?
 *****************************************************************************/
static int32_t one_latitude_table(signed int number, one_latitude_data_t *data, int32_t value)
{
    int32_t j;
    int32_t res;

    for (j = 0; j < number; j++)
    {
        if (data[j].x == value)
        {
            res = data[j].y;
            return res;
        }
        if (data[j].x > value)
            break;
    }

    if (j == 0)
        res = data[j].y;
    else if (j == number)
        res = data[j - 1].y;
    else
    {
        res = ((value - data[j - 1].x) * (data[j].y - data[j - 1].y));

        if ((data[j].x - data[j - 1].x) != 0)
            res = res / (data[j].x - data[j - 1].x);
        res += data[j - 1].y;
    }

    return res;
}
#if 0
/*****************************************************************************
 * Description:
 *		vadc_cscan_raw
 *       vadc channel scan get raw data.
 * Parameters:
 *	    reg: the correspond reg
 *	    raw: the raw data buffer
 * Return:
 *		Error code
 *****************************************************************************/
static uint8_t vadc_cscan_raw(uint8_t reg, int16_t *raw)
{
    return kc81710h_read_word(KC81710H_I2C_ADDR, reg, (uint16_t *)raw);
}
#endif
/****************************************************************************
 *
 *			I N I T I A L 	M O D U L E
 *
 ****************************************************************************/

/*****************************************************************************
 * Description:
 *		Initial module for MCU using.
 * Parameters:
 *      chip:           KC81710H chip handle
 *
 * Return:
 *		Error code
 *****************************************************************************/
uint8_t kc81710h_init(void)
{
	// 0. Before enter this function, should initial I2C bus first.
	// 1. Save data in dfe Device

	uint8_t ret = 0;
	uint16_t bufdata = 0;
#if DEBUG
	uint16_t buftmp = 0;

	printf("init\r\n");
#endif
	// Read S17H ID
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_HWID1, &bufdata);
	if (ret) return ret;
	if (bufdata != KC81710H_HWID1)
	{
		return KC_STATUS_NOT_MATCH;
	}
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_HWID2, &bufdata);
	if (ret) return ret;
	if ((bufdata != KC81710H_HWID2_2) &&  (bufdata != KC81710H_HWID2_1))
	{
		return KC_STATUS_NOT_MATCH;
	}

#if DEBUG
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_HWID1, &bufdata);
	if (ret) return ret;
	printf("REG 0x%X =0x%4.4X\r\n", KC81710H_OP_HWID1, bufdata);
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_HWID2, &bufdata);
	if (ret) return ret;
	printf("REG 0x%X =0x%4.4X\r\n", KC81710H_OP_HWID2, bufdata);

	for (buftmp = 0x54; buftmp <= 0x5F; buftmp++)
	{
		ret = kc81710h_read_word(KC81710H_I2C_ADDR, buftmp, &bufdata);
		if (ret) return ret;
		printf("REG 0x%X =0x%4.4X\r\n", buftmp, bufdata);
	}
#endif

	ret = S17H_init();
	if (ret) return ret;

#if DEBUG
	for (buftmp = 0x54; buftmp <= 0x5F; buftmp++)
	{
		ret = kc81710h_read_word(KC81710H_I2C_ADDR, buftmp, &bufdata);
		if (ret) return ret;
		printf("REG 0x%X =0x%4.4X\r\n", buftmp, bufdata);
	}
#endif
#if 1
	// Clear Flag 0 once
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IF0, &bufdata);
	if (ret) return ret;
	if (bufdata != 0)
	{
		ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_IF0, bufdata);
		if (ret) return ret;
	 }

	// Clear Flag 1 once
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IF1, &bufdata);
	if (ret) return ret;
	if (bufdata != 0)
	{
		ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_IF1, bufdata);
		if (ret) return ret;
	}

	// Clear Flag 2 once
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IF2, &bufdata);
	if (ret) return ret;
	if (bufdata != 0)
	{
		ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_IF2, bufdata);
		if (ret) return ret;
	}

	// Clear Flag 3 once
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IF3, &bufdata);
	if (ret) return ret;
	if (bufdata != 0)
	{
		ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_IF3, bufdata);
		if (ret) return ret;
	}
#endif
	return ret;
}

/****************************************************************************
 *
 *			E N T R Y 	M O D U L E
 *
 ****************************************************************************/
#if 0
/*****************************************************************************
 * Description:
 *		errno_clr
 *       Clear the errno
 * Parameters:
 *       None
 * Return:
 *		None
 *****************************************************************************/
void errno_clr(void)
{
    errno = 0;
}

/*****************************************************************************
 * Description:
 *		errno_get
 *       Get the last error number of errno
 * Parameters:
 *       None
 * Return:
 *		The last error number
 *****************************************************************************/
uint8_t errno_get(void)
{
    return errno;
}
#endif
/****************************************************************************
 *
 *			R E G I S T E R		C O N T R O L		M O D U L E
 *
 ****************************************************************************/

/****************************************************************************
 * Description:
 *		I2C read command
 * Parameters:
 *		addr:		slave address
 *    cmd:		register
 *		buf: 		pointer to buffer to store read data
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t i2c_read_cmd(uint8_t addr, uint8_t cmd, uint16_t *buf)
{
	uint8_t ret = 0;
	uint8_t bdata_buf[6];
	uint8_t bdata_buf_read[4];
	uint8_t pec_cal[2];

//	ret = i2c_read_block(addr, cmd, 3, bdata_buf_read, 0);
	ret = I2C_ReadBuffer(I2C_AFE_INDEX, addr, cmd, bdata_buf_read, 3);
	if (ret)
	{
		return ret;
	}

	bdata_buf[0] = addr;
	bdata_buf[1] = cmd;
	bdata_buf[2] = addr | 1;
	bdata_buf[3] = bdata_buf_read[0];
	bdata_buf[4] = bdata_buf_read[1];

	pec_cal[0] = crc8_calc(bdata_buf, 5);

	if (pec_cal[0] != bdata_buf_read[2])
	{
		bdata_buf[3] ^= 0x80;
		pec_cal[1] = crc8_calc(bdata_buf, 5);
		bdata_buf[3] ^= 0x80;
		if (pec_cal[1] != bdata_buf_read[2])
		{
#if DETAIL
			printf("Bdata0,0x%2.2X\r\n", addr);
			printf("Bdata1,0x%2.2X\r\n", cmd);
			printf("Bdata2_REG,0x%2.2X\r\n", addr | 1);
			printf("Bdata3_DATA0,0x%2.2X\r\n", bdata_buf_read[0]);
			printf("Bdata4_DATA1,0x%2.2X\r\n", bdata_buf_read[1]);
			printf("Bdata5_CRC,0x%2.2X\r\n", bdata_buf_read[2]);
#endif
			return KC_STATUS_PECERR;
		}
	}

	if (buf != NULL)
	{
		buf[0] = bdata_buf_read[0] << 8;
		buf[0] |= bdata_buf_read[1];
	}

	return KC_STATUS_OK;
}

/****************************************************************************
 * Description:
 *		I2C write command
 * Parameters:
 *		addr:		slave address
 *    index:	register
 *		buf: 		pointer to buffer to store write data
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t i2c_write_cmd(uint8_t addr, uint8_t cmd, uint16_t buf)
{
    uint8_t ret = 0;
    uint8_t bdata_buf[8];
    uint8_t bdata_crc[5];

    bdata_buf[0] = (uint8_t)(buf >> 8);
    bdata_buf[1] = (uint8_t)buf;

    bdata_crc[0] = addr;
    bdata_crc[1] = cmd;
    bdata_crc[2] = bdata_buf[0];
    bdata_crc[3] = bdata_buf[1];

    bdata_buf[2] = crc8_calc(bdata_crc, 4);

//	ret = i2c_write_block(addr, cmd, 3, bdata_buf, 0);
    ret = I2C_WriteBuffer(I2C_AFE_INDEX, addr, cmd, bdata_buf, 3);

    return ret;
}

/****************************************************************************
 * Description:
 *		read S17H operation register
 * Parameters:
 *    addr:		S17H slave address
 *    index:	S17H operation register index
 *		buf: 		pointer to buffer to store read data
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_read_word(uint8_t addr, uint8_t index, uint16_t *buf)
{
	uint8_t ret;

	ret = i2c_read_cmd(addr, index, buf);

	return ret;
}

/****************************************************************************
 * Description:
 *		write S17H operation register
 * Parameters:
 *    addr:		S17H slave address
 *    index:	S17H operation register index
 *		buf: 		the data we write to register
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_write_word(uint8_t addr, uint8_t index, uint16_t buf)
{
	uint8_t ret;

	ret = i2c_write_cmd(addr, index, buf);

	return ret;
}

/****************************************************************************
 * Description:
 *		set KC81710H operation register bits
 * Parameters:
 *      index:	KC81710H operation register index
 *		bits: 		the bits we set to register
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_set_reg_bits(uint8_t addr, uint8_t index, uint16_t bits)
{
	uint8_t ret;
	uint16_t buf;

	ret = kc81710h_read_word(addr, index, &buf);
	if (ret != KC_STATUS_OK) return ret;
	buf |= bits;
	ret = kc81710h_write_word(addr, index, buf);

	return ret;
}

/****************************************************************************
 * Description:
 *		clear KC81710H operation register bits
 * Parameters:
 *    index:	KC81710H operation register index
 *		bits: 		the bits we clear to register
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_clear_reg_bits(uint8_t addr, uint8_t index, uint16_t bits)
{
	uint8_t ret;
	uint16_t buf;
	ret = kc81710h_read_word(addr, index, &buf);
	if (ret != KC_STATUS_OK) return ret;
	buf &= ~bits;
	ret = kc81710h_write_word(addr, index, buf);

	return ret;
}

void kc81710h_delay_ms(uint32_t tick)
{
	uint32_t i, j;

	while(tick--)
	{
		for(i = 0; i < 100; i++)		// 64M
		{
			for(j = 0; j < 160; j++)
			{
				;
			}
		}
	}
}

/*****************************************************************************
 * Description:
 *		crc8_calc
 * Parameters:
 *		*pdata: point to data buffer which need to calculate
 *		n:      data number
 * Return:
 *      calculate data
 *****************************************************************************/
uint8_t crc8_calc(uint8_t *pdata, uint16_t n)
{
    uint8_t crc = 0;
    uint8_t crcdata;
    uint16_t i, j;

    for (i = 0; i < n; i++)
    {
        crcdata = pdata[i];
        for (j = 0x80; j != 0; j >>= 1)
        {
            if ((crc & 0x80) != 0)
            {
                crc <<= 1;
                crc ^= 0x07;
            }
            else
                crc <<= 1;

            if ((crcdata & j) != 0)
                crc ^= 0x07;
        }
    }

    return crc;
}
#if 0
uint8_t calc_crc_read(uint8_t slave_addr, uint8_t reg_addr, uint16_t data)
{
    uint8_t pdata[5];

    pdata[0] = slave_addr;
    pdata[1] = reg_addr;
    pdata[2] = (uint8_t)(slave_addr | 0x01);
    pdata[3] = data >> 8;
    pdata[4] = (uint8_t)data;

    return crc8_calc(pdata, 5);
}

uint8_t calc_crc_write(uint8_t slave_addr, uint8_t reg_addr, uint16_t data)
{
    uint8_t pdata[5];

    pdata[0] = slave_addr;
    pdata[1] = reg_addr;
    pdata[2] = data >> 8;
    pdata[3] = (uint8_t)data;

    return crc8_calc(pdata, 4);
}

/*****************************************************************************
 * Description:
 *		crc4_calc
 * Parameters:
 *		*pdata: point to data buffer which need to calculate
 *		n:      data number
 * Return:
 *      calculate data
 *****************************************************************************/
uint8_t crc4_calc(uint8_t *pdata, int len)
{
    uint8_t crc = 0;
    uint8_t crcdata;
    uint8_t poly = 0x03; // poly
    int n, j;            // the length of the data
    for (n = 0; n < len; n++)
    {
        crcdata = pdata[n];
        for (j = 0x8; j > 0; j >>= 1)
        {
            if ((crc & 0x8) != 0)
            {
                crc <<= 1;
                crc ^= poly;
            }
            else
                crc <<= 1;

            if ((crcdata & j) != 0)
                crc ^= poly;
        }
        crc = (uint8_t)(crc & 0xf);
    }
    return crc;
}
#endif
uint8_t S17H_init(void)
{
	uint8_t ret;
//uint16_t temp;
	
//	// 读取中断配置字是否一配置过，是的话表示升级，则不再次配置
//	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IE1, &temp);
//	if (ret) return ret;
//	if(temp == KC81710H_REG_IE1_DATA)
//		return 0;

	// 0.Disable CHG_FET and DSG_FET
	ret = kc81710h_mosfet_clr(KC81710H_SW_CHG_EN_MSK | KC81710H_SW_DSG_EN_MSK);
	if (ret) return ret;

	// 1.0x70(PWD) Write 0xdfe8,make pwd_active=1 unlock
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;

	// 2.0x71(CFGMAPS TOP) Write 0xABC6, make auto_remap_cfg_stop=1
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG_MAP_STOP, KC81710H_AUTO_REMAP_CFG_STOP_DATA);
	if (ret) return ret;

	// 3.0x70(PWD) Write 0xdfe8,make pwd_active=1 unlock
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;

	// 4.0x72(CFGREGE N) Write 0xABC7 make cfg_reg_wr_en=1,Open to access 0x54 - 0x5F not to operate 0xFB,
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG_REG_EN, KC81710H_CFG_REG_WR_EN_DATA);
	if (ret) return ret;

	// 7.0xF1(BUFCFG)：Write 0x0001,set buf_chop_en=1
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_BUFCFG, KC81710H_BUF_CHOP_EN_MSK);
	if (ret) return ret;

	// 5.0x54~0x5F register map
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG00, KC81710H_REG_CFG0_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG01, KC81710H_REG_CFG1_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG02, KC81710H_REG_CFG2_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG03, KC81710H_REG_CFG3_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG04, KC81710H_REG_CFG4_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG05, KC81710H_REG_CFG5_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG06, KC81710H_REG_CFG6_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG07, KC81710H_REG_CFG7_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG08, KC81710H_REG_CFG8_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG09, KC81710H_REG_CFG9_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG10, KC81710H_REG_CFG10_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG11, KC81710H_REG_CFG11_DATA);
	if (ret) return ret;

	// 6.0x70(PWD) Write 0xdfe8,make pwd_active=1 unlock
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;

	// 7.0x72(CFGREGE N) Write 0x0000 make cfg_reg_wr_en=0, Close to access 0x54 - 0x5F
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CFG_REG_EN, 0);
	if (ret) return ret;

	// 0x70(PWD) Write 0, make pwd_active=0 lock
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);
	if (ret) return ret;

	// 8.0xD4(IE0)	Enable IE0
//	ret = kc81710h_write_word(KC81710H_I2C_ADDR, OP_IE0, OP_IE0_ENABLE_DATA);
//	if (ret) return ret;

	// 9.0xD5(IE1)	Enable IE1
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_IE1, KC81710H_REG_IE1_DATA);
	if (ret) return ret;

	// 10.0xD6(IE2) Enable IE2
//	ret = kc81710h_write_word(KC81710H_I2C_ADDR, OP_IE2, OP_IE2_ENABLE_DATA);
//	if (ret) return ret;

	// 11.0xD7(IE3) Enable IE3
//	ret = kc81710h_write_word(KC81710H_I2C_ADDR, OP_IE3, OP_IE3_ENABLE_DATA);
//	if (ret) return ret;

	// 12.0xA5: CC_VALUE
//	ret = kc81710h_write_cc_raw(0);
//	if (ret) return ret;

	// 13.0xE0: SW_CC_MD
	ret = kc81710h_set_cc_mode(KC81710H_SW_CC_EN_ALWAYS);
	if (ret) return ret;

	// 14.0xB1: SWCBEN
	ret = kc81710h_sw_release_enable();
	if (ret) return ret;

	// 15.0xB4: SWCFG
	ret = kc81710h_sw_config_set(KC81710H_CHG_FAILCHK_EN_MSK | KC81710H_DSG_FAILCHK_EN_MSK
//		| KC81710H_SW_CHGR_CHK_MSK
//		| KC81710H_SW_LOAD_CHK_MSK
//		| KC81710H_OHT_VMCU1_DOWN_MSK | KC81710H_OHT_VMCU2_DOWN_MSK | KC81710H_DSLP_VMCU2_DOWN_MSK
/*		| KC81710H_INCHG_WKUP_EN_MSK | KC81710H_INDSG_WKUP_EN_MSK*/);
	if (ret) return ret;

	ret = kc81710h_sleep_control_set(KC81710H_SW_SLEEP_HW);
	if (ret) return ret;

	// 16.0xC4: TS_CFG
	ret = kc81710h_ts_config(KC81710H_SW_TS2_CFG_CELL, KC81710H_SW_TS3_CFG_THM);
	if (ret) return ret;

	// 17.0xC6: FUSE_OFF_EN default: SOV_FUSE_CHGF_OFF
	ret = kc81710h_fuse_config(0/*KC81710H_CHGFAIL_FUSE_CHGF_OFF_MSK | KC81710H_DSGFAIL_FUSE_OFF_MSK | KC81710H_SOV_FUSE_CHGF_OFF_MSK*/);
	if (ret) return ret;

	// 18.0xC9: CO_CFG
//	ret = kc81710h_co_config_set(KC81710H_CO_OPTION_MSK | KC81710H_EXTRA_COCHK_MSK | KC81710H_CO_START_TH_2V0);
//	if (ret) return ret;

	// 19.0xCA: PCHG_CFG default: SW_PCHG_EN
	ret = kc81710h_pchg_config(/*KC81710H_V0_PCHG_DISABLE_MSK | */KC81710H_SW_PCHG_EN_MSK);
	if (ret) return ret;

	// 20.0xC0: PDSGCFG
	ret = kc81710h_pdsg_config_set(KC81710H_PDSG_ECTRL_MSK | KC81710H_DSGOFF_IN_PDSG_MSK);
//	ret = kc81710h_pdsg_config_set(KC81710H_PDSG_ECTRL_MSK);		// 0:DSG PDSG可同时开启  1：PDSG开启时需要关闭DSG
	if (ret) return ret;

	// 21.0xBB: PDSGTM
	ret = kc81710h_pdsg_timeout_set(0/KC81710H_PDSG_TIME_TICK);
	if (ret) return ret;

	// 22.0xBA: SWFET
//	ret = kc81710h_mosfet_set(KC81710H_SW_CHG_EN_MSK | KC81710H_SW_DSG_EN_MSK);
//	if (ret) return ret;

	// 23.0xBC: VMCU2_CTRL
//	ret = kc81710h_vmcu2_control_down();
//	if (ret) return ret;
//	ret = kc81710h_vmcu2_control_work();
//	if (ret) return ret;

	ret = ZeroClib(0);
	
	return ret;
}

u8 ZeroClib(u8 markupdate)
{
	u8 ret;
	s16 temp;
	u16 temp2;
	
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_ADC2_SYS_OFST, &temp2);
	if (ret) return ret;
	temp2 = ~temp2;
	
	temp = 0x1ff&DataFlashAtOnceSave.AfeMCBias;
	if(markupdate!=0)
	{
		temp += temp2;
		DataFlashAtOnceSave.AfeMCBias = temp;
	}
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_ADC2_SYS_OFST, ~temp);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

static uint32_t kc81710h_S24toS32(uint32_t input)
{
	if((input & 0x800000) == 0x800000)	// If the highest bit is 1, it is a negative number
	{
		input |= 0xFF000000;
	}
	return input;
}

/*****************************************************************************
 * Description:
 *		Entry module for MCU using.
 * Parameters:
 *		N/A
 *
 * Return:
 *		Error code
 *****************************************************************************/
uint8_t kc81710h_entry(t_KC81710H_CHIP *chip)
{
	// 0. May called every 1 second
	// 1. Update All cell voltage.0x90~0x97(CELL01~CELL08)
	// 2. Update Current 0xA4(H16 Bits)&0xEA(L2 Bits)
	// 3. Update thm 0 0xA7(TS0)
	// 4. Update thm 1 0xA8(TS1)
	// 5. Read 0xD0(FLAG0) - 0xD3(FLAG3)
	// 6. Read 0xDD(TS FLAG) clear by 0xD0
	unsigned int itmp = 0;
	uint8_t ret = 0;
	uint16_t bufdata = 0;

	//1. Status update
	ret = kc81710h_read_status(chip);
	if (ret) return ret;
	ret = kc81710h_read_flag(chip);
	if (ret) return ret;
	// Here is whole sample code to access all cell voltage according to AN-2
	//2. Read all cell voltage
	for (itmp = 0; itmp < CELL_NUMBER; itmp++)
	{
		ret = kc81710h_read_word(KC81710H_I2C_ADDR, (itmp == CELL_NUMBER - 1) ? KC81710H_OP_CH_CELL17 : (KC81710H_OP_CH_CELL01 + itmp), &bufdata);
		if (ret) return ret;
		chip->cell_volt[itmp] = kc81710h_cell_volt_mv_cal((int16_t)bufdata);
#if DEBUG
		printf("Cell%d = %d mV\r\n", itmp + 1, tmp[0]);
#endif
	}

	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_VCC_ADC1, &bufdata);
	if (ret) return ret;
	chip->vbat = kc81710h_high_volt_mv_cal((int16_t)bufdata);

	//3. update thmermal
#if DEBUG
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, OP_CH_TS0, &bufdata);
	printf("REG 0x%X =0x%4.4X\r\n", OP_CH_TS0, bufdata);
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, OP_CH_TS1, &bufdata);
	printf("REG 0x%X =0x%4.4X\r\n", OP_CH_TS1, bufdata);
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, OP_CH_TS2, &bufdata);
	printf("REG 0x%X =0x%4.4X\r\n", OP_CH_TS2, bufdata);
#endif
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_TS0R, &bufdata);
	if (ret) return ret;
	chip->thm[0] = kc81710h_thm_0degC1_cell_cal((int16_t)bufdata);
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_TS1R, &bufdata);
	if (ret) return ret;
	chip->thm[1] = kc81710h_thm_0degC1_cell_cal((int16_t)bufdata);
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_TS2R, &bufdata);
	if (ret) return ret;
	chip->thm[2] = kc81710h_thm_0degC1_mosfet_cal((int16_t)bufdata);

	// 4. Read CADC value
#if DEBUG
	ret = read_word(KC81710H_I2C_ADDR, OP_CH_CRRT0, &bufdata);
	printf("REG 0x%X =0x%4.4X\r\n", OP_CH_CRRT0, bufdata);
	ret = read_word(KC81710H_I2C_ADDR, OP_CH_CRRT1, &bufdata);
	printf("REG 0x%X =0x%4.4X\r\n", OP_CH_CRRT1, bufdata);
#endif
	ret = kc81710h_read_current(&chip->cadc_current);
	if (ret) return ret;

	// 5. Status update
	ret = kc81710h_read_cc_raw(&chip->cc_raw_data);
	if (ret) return ret;
	// calculate cc mah
	chip->cc_data = kc81710h_cc_mah_cal(chip->cc_raw_data, RSENSE);

	// 6. Trig Requst
	t_KC81710H_TRIG trig = {0};

	trig.sw_req = KC81710H_SW_REQ_SINGLE;
	trig.sw_lsb = KC81710H_SW_ADC1_LSB_16;
//	trig.sw_co_sel = KC81710H_SW_CO_SEL_NORMAL;

	trig.req_ch = KC81710H_REQ_CH_VPACK;
//	SET_TEST_LED_ON();
	ret = kc81710h_vadc_trig_req(trig, 1);
//	SET_TEST_LED_OFF();
	if (ret) return ret;
//	ret = kc81710h_vadc_read_raw((t_KC81710H_REQ_CH)trig.req_ch, &bufdata);
//	if (ret) return ret;
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_COMMON_ADC1, &bufdata);
	if (ret) return ret;
	chip->vpack = kc81710h_high_volt_mv_cal((int16_t)bufdata);

	trig.req_ch = KC81710H_REQ_CH_VTS;
//	SET_TEST_LED_ON();
	ret = kc81710h_vadc_trig_req(trig, 1);
//	SET_TEST_LED_OFF();
	if (ret) return ret;
//	ret = kc81710h_vadc_read_raw((t_KC81710H_REQ_CH)trig.req_ch, &bufdata);
//	if (ret) return ret;
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_COMMON_ADC1, &bufdata);
	if (ret) return ret;
	chip->thm[3] = kc81710h_thm_intemp_cal((int16_t)bufdata);

	trig.req_ch = KC81710H_REQ_CH_THM3;
//	SET_TEST_LED_ON();
	ret = kc81710h_vadc_trig_req(trig, 1);
//	SET_TEST_LED_OFF();
	if (ret) return ret;
//	ret = kc81710h_vadc_read_raw((t_KC81710H_REQ_CH)trig.req_ch, &bufdata);
//	if (ret) return ret;
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_COMMON_ADC1, &bufdata);
	if (ret) return ret;
	chip->thm[3] = kc81710h_thm_0degc1_mv_cal((int16_t)bufdata);

	// 7. cell balance
	ret = kc81710h_cb_read(&chip->cb_status);
	if (ret) return ret;

	// 8. cell open
//	ret = kc81710h_co_read(&chip->co_status);
//	if (ret) return ret;

#if DEBUG
	// This is used to print all data defined in data structure.
	printf("Cell Numbers = %d \r\n", CELL_NUMBER);

	// Print tmp
	printf("Temperature0 = %d 0.1C\r\n", p_dfe_device->external_temperature[0]);
	printf("Temperature1 = %d 0.1C\r\n", p_dfe_device->external_temperature[1]);

	// Print current
	printf("CADC Current = %d mA\r\n", p_dfe_device->cadc_current);
	// Print status
	printf("Event STR0 = 0x%4.4X\r\n", p_dfe_device->status0);
	printf("Event STR1 = 0x%4.4X\r\n", p_dfe_device->status1);
	printf("Event STR2 = 0x%4.4X\r\n", p_dfe_device->status2);
	printf("Event STR3 = 0x%4.4X\r\n", p_dfe_device->status3);
	printf("Event STR4 = 0x%4.4X\r\n", p_dfe_device->status4);
#endif

	return ret;
}

/****************************************************************************
 * Description:
 *		read S17H Status
 * Parameters:
 *      chip:  KC81710H chip handle
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_read_status(t_KC81710H_CHIP *chip)
{
    uint8_t ret = 0;
    uint16_t bufdata = 0;
    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_STATUS0, &bufdata);
    if (ret) return ret;
    chip->status[0] = bufdata;

    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_STATUS1, &bufdata);
    if (ret) return ret;
    chip->status[1] = bufdata;

    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_STATUS2, &bufdata);
    if (ret) return ret;
    chip->status[2] = bufdata;

    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_STATUS3, &bufdata);
    if (ret) return ret;
    chip->status[3] = bufdata;

    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_STATUS4, &bufdata);
    if (ret) return ret;
    chip->status[4] = bufdata & (KC81710H_STATUS4_PCHG_EN_MSK | KC81710H_STATUS4_PDSG_EN_MSK | KC81710H_STATUS4_CHG_EN_MSK | KC81710H_STATUS4_DSG_EN_MSK);

    return ret;
}

/****************************************************************************
 * Description:
 *		read S17H Flags
 * Parameters:
 *      chip:  KC81710H chip handle
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_read_flag(t_KC81710H_CHIP *chip)
{
    uint8_t ret = 0;
    uint16_t bufdata = 0;
    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IF0, &bufdata);
    if (ret) return ret;
    chip->int_flag[0] = bufdata;

    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IF1, &bufdata);
    if (ret) return ret;
    chip->int_flag[1] = bufdata;

    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IF2, &bufdata);
    if (ret) return ret;
    chip->int_flag[2] = bufdata;

    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IF3, &bufdata);
    if (ret) return ret;
    chip->int_flag[3] = bufdata;

    return ret;
}

/****************************************************************************
 * Description:
 *		cell_volt_mv_cal
 *      Calculate the cell voltage.
 * Parameters:
 *      rawdata: The ADC raw data.
 * Return:
 *		the cell voltage 1mV/LSB
 ****************************************************************************/
int32_t kc81710h_cell_volt_mv_cal(int16_t rawdata)
{
    return (int32_t)rawdata * KC81710H_CELL_VOLT_LSB_MV / KC81710H_CELL_VOLT_FACTOR;
}

/****************************************************************************
 * Description:
 *		thm_volt_mv_cal
 *      Calculate the external THM / GP1 voltage.
 * Parameters:
 *      rawdata: The ADC raw data.
 * Return:
 *		the voltage 1mV/LSB
 ****************************************************************************/
int32_t kc81710h_thm_volt_mv_cal(int16_t rawdata)
{
    return (int32_t)rawdata * KC81710H_THM_VOLT_LSB_MV / KC81710H_THM_VOLT_FACTOR;
}

/****************************************************************************
 * Description:
 *		current_ma_cal
 *      Calculate the current.
 * Parameters:
 *      rawdata:    The ADC raw data.
 *      r_ur:       The Rsense 1uR/LSB.
 * Return:
 *		the voltage 1mA/LSB
 ****************************************************************************/
int32_t kc81710h_current_ma_cal(int32_t rawdata, int16_t r_ur)
{
    return (int32_t)rawdata * KC81710H_ISENS_VOLT_LSB_NV / KC81710H_ISENS_VOLT_FACTOR / r_ur;
}

/****************************************************************************
 * Description:
 *		ithm_0degC1_cal
 *      Calculate the internal THM temperature.
 * Parameters:
 *      rawdata: The ADC raw data.
 * Return:
 *		the internal THM temperature 0.1degC/LSB
 ****************************************************************************/
int32_t kc81710h_thm_0degC1_cell_cal(int16_t rawdata)
{
	int32_t r, thm;
	r = (int32_t) rawdata * (KC81710H_THM_RES_LSB_KR * 1000) / KC81710H_THM_RES_FACTOR;
	thm = one_latitude_table(TEMPERATURE_CELL_NUM, temp_cell_table, r);
	return thm;
}

int32_t kc81710h_thm_0degC1_mosfet_cal(int16_t rawdata)
{
	int32_t r, thm;
	r = (int32_t) rawdata * (KC81710H_THM_RES_LSB_KR * 1000) / KC81710H_THM_RES_FACTOR;
	thm = one_latitude_table(TEMPERATURE_MOSFET_NUM, temp_mosfet_table, r);
	return thm;
}

/****************************************************************************
 * Description:
 *      Calculate the THM channel voltage.
 * Parameters:
 *      rawdata:    The ADC raw data.
 * Return:
 *		the voltage 1mV/LSB
 ****************************************************************************/
int16_t kc81710h_thm_0degc1_mv_cal(int16_t rawdata)
{
	uint32_t res;

	if (rawdata < 0)
	{
		res = 0;
	}
	else
	{
		res = rawdata;
	}
#if KC81710H_THM_DIV_RES_OHM > (65535 * 2)
	#error The following 32bit calculation may overflow.
#endif
	#define KC81710H_THM_REF_RAW	(KC81710H_THM_REF_VOLT_MV * KC81710H_THM_VOLT_FACTOR / KC81710H_THM_VOLT_LSB_MV)
	// calculate thm resistance 1ohm / lsb
	res = (uint32_t)KC81710H_THM_DIV_RES_OHM * res / (KC81710H_THM_REF_RAW - res);
	// lookup table
	return one_latitude_table(TEMPERATURE_CELL_NUM, temp_cell_table, res);
}

/****************************************************************************
 * Description:
 *      Calculate the THM channel voltage.
 * Parameters:
 *      rawdata:    The ADC raw data.
 * Return:
 *		the voltage 1mV/LSB
 ****************************************************************************/
int16_t kc81710h_thm_intemp_cal(int16_t rawdata)
{
	uint32_t res;

	if (rawdata < 0)
	{
		res = 0;
	}
	else
	{
		res = rawdata;
	}

	res *= KC81710H_THM_VOLT_LSB_MV;
	res -= (KC81710H_THM_ITMP_REF_MV * KC81710H_THM_VOLT_FACTOR);
	res *= KC81710H_THM_ITMP_FACTOR;
	res /= (KC81710H_THM_ITMP_LSB_MV * KC81710H_THM_VOLT_FACTOR);
	res += KC81710H_THM_ITMP_REF_0C1;

	return res;
}

/****************************************************************************
 * Description:
 *      Calculate the high voltage channel voltage (VCC, VPACK).
 * Parameters:
 *      rawdata:    The ADC raw data.
 * Return:
 *		the voltage 1mV/LSB
 ****************************************************************************/
int32_t kc81710h_high_volt_mv_cal(int16_t rawdata)
{
    return (int32_t)rawdata * KC81710H_HIGH_VOLT_LSB_MV / KC81710H_HIGH_VOLT_FACTOR;
}

/****************************************************************************
 * Description:
 *      Calculate the high voltage channel voltage (VDMCU/VDDA).
 * Parameters:
 *      rawdata:    The ADC raw data.
 * Return:
 *		the voltage 1mV/LSB
 ****************************************************************************/
int32_t kc81710h_low_volt_mv_cal(int16_t rawdata)
{
    return (int32_t)rawdata * KC81710H_LOW_VOLT_LSB_MV / KC81710H_LOW_VOLT_FACTOR;
}

/****************************************************************************
 * Description:
 *		kc81710h_cc_mah_cal
 *      Calculate the coulomb counter capacity.
 * Parameters:
 *      rawdata:    The ADC raw data.
 *      r_ur:       The Rsense 1uR/LSB.
 * Return:
 *		the voltage 1mAH/LSB
 ****************************************************************************/
int32_t kc81710h_cc_mah_cal(int32_t rawdata, int16_t r_ur)
{
    return (int64_t)rawdata * (int64_t)KC81710H_CC_ACC_LSB_NV / KC81710H_CC_ACC_FACTOR / r_ur;
}

/****************************************************************************
 * Description:
 *		read CMDATA current data
 * Parameters:
 *      ma: the CMDATA data pointer (1mA/lsb)
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_read_current(int32_t *ma)
{
	uint8_t ret = 0;
	int16_t tmp, tmp1;
	uint32_t tmp32;
	int32_t itmp32;

	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_CRRT_ADC2, (uint16_t *)&tmp);
	if (ret) return ret;
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_CRRT_ADC2L, (uint16_t *)&tmp1);
	if (ret) return ret;

	tmp1 &= 0x0003;
	tmp32 = (uint32_t)tmp << 2 | tmp1;
	itmp32 = kc81710h_S24toS32(tmp32);

	CalibZeroCurRun(itmp32);
	
	*ma = kc81710h_current_ma_cal(itmp32, RSENSE);

	return KC_STATUS_OK;
}

/****************************************************************************
 * Description:
 *		read CADC coulomb counter raw data
 * Parameters:
 *      cc_raw: coulomb counter raw data pointer
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_read_cc_raw(int32_t *cc_raw)
{
	uint8_t ret = 0;
	uint16_t buf[2];

	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_CC_H, &buf[0]);
	if (ret) return ret;
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_CC_L, &buf[1]);
	if (ret) return ret;

	*cc_raw = buf[1] | ((uint32_t)buf[0] << 16);

	return KC_STATUS_OK;
}

/****************************************************************************
 * Description:
 *		clear CADC coulomb counter registers
 * Parameters:
 *      cc_raw: coulomb counter raw data
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_write_cc_raw(int32_t cc_raw)
{
	uint8_t ret = 0;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_CC_H, (uint16_t)(cc_raw >> 16) & 0xFFFF);
	if (ret) return ret;
	// write again in case of none write-protect mode
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CH_CC_L, (uint16_t)cc_raw & 0xFFFF);
	if (ret) return ret;

	return KC_STATUS_OK;
}

uint8_t kc81710h_set_cc_mode(uint8_t mode)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_SW_CC_MD, ((uint16_t)mode << KC81710H_SW_CC_MD_POS) & KC81710H_SW_CC_MD_MSK);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);
	if (ret) return ret;

	return ret;
}

uint8_t kc81710h_vadc_trig_ready(void)
{
	uint8_t ret;
	uint16_t buf;

	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_ADC1_REQ, &buf);
	if (ret) return ret;
	if (buf & KC81710H_SW_ADC1_REQ_MSK)
	{
		ret = KC_STATUS_ERROR;
	}

	return ret;
}

uint8_t kc81710h_vadc_trig_wait(void)
{
	uint8_t ret;

	ret = kc81710h_vadc_trig_ready();
	if(ret != KC_STATUS_OK)
	{
		uint8_t cnt = 250;
		for(;;)
		{
			kc81710h_delay_ms(1);
			ret = kc81710h_vadc_trig_ready();
			if (ret == KC_STATUS_OK)
			{
				break;
			}
			if (cnt-- == 0)
			{
				return KC_STATUS_TIMEOUT;
			}
		}
	}

	return ret;
}

uint8_t kc81710h_vadc_trig_set(t_KC81710H_TRIG trig)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_ADC1_REQ, trig.reg);

	return ret;
}

/****************************************************************************
 * Description:
 *		Start VADC trigger request
 * Parameters:
 *      trig:           trigger parameter
 *      wait_comp:      whether waiting trigger complete
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_vadc_trig_req(t_KC81710H_TRIG trig, uint8_t wait_comp)
{
	uint8_t ret;

	// waiting previous trigger req
	ret = kc81710h_vadc_trig_wait();
	if (ret) return ret;
	// write trig
	ret = kc81710h_vadc_trig_set(trig);
	if (ret) return ret;
	// check trig flag to wait complete
	if (wait_comp)
	{
		ret = kc81710h_vadc_trig_wait();
		if (ret) return ret;
	}

	return KC_STATUS_OK;
}

/****************************************************************************
 * Description:
 *		read VADC trigger result
 * Parameters:
 *      ch:             trigger channel
 *      raw:            raw data pointer
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_vadc_read_raw(t_KC81710H_REQ_CH ch, uint16_t *raw)
{
	uint8_t ret;
	uint8_t index;
	uint16_t buf;

	if (ch == KC81710H_REQ_CH_VTS)
	{
		index = KC81710H_OP_CH_COMMON_ADC1;
	}
	else if (ch <= KC81710H_REQ_CH_CELL17)
	{
		index = KC81710H_OP_CH_CELL01 - KC81710H_REQ_CH_CELL01 + ch;
	}
	else if (ch <= KC81710H_REQ_CH_THM1)
	{
		index = KC81710H_OP_CH_TS0R - KC81710H_REQ_CH_THM0 + ch;
	}
	else if (ch == KC81710H_REQ_CH_THM2)
	{
		index = KC81710H_OP_CH_TS2R;
	}
	else if (ch == KC81710H_REQ_CH_VCC)
	{
		index = KC81710H_OP_CH_VCC_ADC1;
	}
	else
	{
		index = KC81710H_OP_CH_COMMON_ADC1;
	}

	ret = kc81710h_read_word(KC81710H_I2C_ADDR, index, &buf);
	if (ret) return ret;

	*raw = buf;

	return KC_STATUS_OK;
}

/****************************************************************************
 * Description:
 *		Set KC81710H cell balance chanel
 * Parameters:
 *		cb: 		cell balance configuration
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_cb_set(uint32_t cb)
{
	uint8_t ret;
	uint16_t buf;

	buf = cb & 0x0001U;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_SWCB2, buf);
	if (ret) return ret;
	buf = (cb >> 1) & 0xFFFFU;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_SWCB1, buf);
	if (ret) return ret;

	return ret;
}

/****************************************************************************
 * Description:
 *		read KC81710H cell balance chanel
 * Parameters:
 *		cb: 		cell balance configuration
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_cb_read(uint32_t* cb)
{
    uint8_t ret;
    uint16_t buf1, buf2;

    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CB_STATUS0, &buf1);
    if (ret) return ret;
    ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_CB_STATUS1, &buf2);
    if (ret) return ret;

    *cb = (uint32_t)(((uint32_t)buf1 << 1) | (buf2 & 0x0001));

    return KC_STATUS_OK;
}

/****************************************************************************
 * Description:
 *		Set KC81710H sw cell balance enable
 * Parameters:
 *      handle:         KC81710H driver handle
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_swcb_enable(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_set_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_SWCBEN, KC81710H_SW_CB_EN_MSK);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

/****************************************************************************
 * Description:
 *		Set KC81710H sw cell balance disable
 * Parameters:
 *      handle:         KC81710H driver handle
 * Return:
 *		Error code
 ****************************************************************************/
uint8_t kc81710h_swcb_disable(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_clear_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_SWCBEN, KC81710H_SW_CB_EN_MSK);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_co_config_set(uint16_t bits)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CO_CFG, bits);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_co_read(uint32_t* co)
{
	uint8_t ret;
	uint16_t buf1, buf2;

	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_COCH1, &buf1);
	if (ret) return ret;
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_COCH2, &buf2);
	if (ret) return ret;

	*co = (uint32_t)(((uint32_t)buf1 << 1) | (buf2 & 0x0001));

	return KC_STATUS_OK;
}

uint8_t kc81710h_sw_release_enable(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_set_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_SWCBEN, KC81710H_SW_RLS_MSK);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_sw_config_set(uint16_t bits)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_set_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_SWCFG, bits);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_sw_config_clr(uint16_t bits)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_clear_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_SWCFG, bits);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_fuse_blow_req(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_FUSECMD, KC81710H_FUSE_BLOW_REQ_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_fuse_blow_release(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_FUSECMD, KC81710H_FUSE_BLOW_DOWN_FLAG_CLEAR);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_fuse_config(uint16_t bits)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_FUSE_OFF_EN, bits);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_chg_fail_release(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_CHG_FAIL_RLS, KC81710H_CHG_FAIL_RLS_DATA);

	return ret;
}

uint8_t kc81710h_dsg_fail_release(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_DSG_FAIL_RLS, KC81710H_DSG_FAIL_RLS_DATA);

	return ret;
}

uint8_t kc81710h_mosfet_set(uint16_t bits)
{
	uint8_t ret;

	bits &= (KC81710H_SW_CHG_EN_MSK | KC81710H_SW_DSG_EN_MSK);
	ret = kc81710h_set_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_SWFET, bits);

	return ret;
}

uint8_t kc81710h_mosfet_clr(uint16_t bits)
{
	uint8_t ret;

	bits &= (KC81710H_SW_CHG_EN_MSK | KC81710H_SW_DSG_EN_MSK);
	ret = kc81710h_clear_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_SWFET, bits);

	return ret;
}

uint8_t kc81710h_pdsg_timeout_set(uint16_t ticks)
{
	uint8_t ret;
	uint16_t bufdata;

	// ticks is N*16ms
	bufdata = (ticks << KC81710H_PDSG_TIME_POS) & KC81710H_PDSG_TIME_MSK;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PDSGTM, bufdata);

	return ret;
}

uint8_t kc81710h_pdsg_config_set(uint16_t bits)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_set_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_PDSGCFG, bits);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_pdsg_config_clr(uint16_t bits)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_clear_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_PDSGCFG, bits);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_vmcu2_control_down(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_VMCU2_CTRL, KC81710H_SW_VMCU2_DOWN_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_vmcu2_control_work(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_VMCU2_CTRL, 0);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_sleep_control_set(uint16_t cfg)
{
	uint8_t ret;
	uint16_t bufdata = 0;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;

	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_SWCFG, &bufdata);
	if (ret) return ret;
	bufdata &= ~KC81710H_SW_SLEEP_CTRL_MSK;
	bufdata |= (cfg << KC81710H_SW_SLEEP_CTRL_POS);
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_SWCFG, bufdata);
	if (ret) return ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_power_sleep(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_SLPWKUP_CMD, KC81710H_SW_SLEEP_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_power_wakeup(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_SLPWKUP_CMD, KC81710H_SW_WAKEUP_PATTERN);

	return ret;
}

uint8_t kc81710h_power_shutdown(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_STDN_CMD, KC81710H_SW_SHUTDOWN_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_ts_config(uint16_t ts2cfg, uint16_t ts3cfg)
{
	uint8_t ret;
	uint16_t bufdata = 0;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;

	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_TS_CFG, &bufdata);
	if (ret) return ret;
	bufdata &= ~(KC81710H_SW_TS2_CFG_MSK | KC81710H_TS3_CFG_MSK);
	bufdata |= (ts2cfg << KC81710H_SW_TS2_CFG_POS);
	bufdata |= (ts3cfg << KC81710H_TS3_CFG_POS);
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_TS_CFG, bufdata);
	if (ret) return ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_pchg_config(uint16_t bits)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PCHG_CFG, bits);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_deep_sleep_prepare(void)
{
	uint8_t ret;
	uint16_t bufdata = 0;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;

	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_SWCFG, &bufdata);
	if (ret) return ret;

	bufdata &= ~KC81710H_SW_SLEEP_CTRL_MSK;
	bufdata |= (KC81710H_SW_SLEEP_DEEP << KC81710H_SW_SLEEP_CTRL_POS);

	bufdata &= ~(KC81710H_SW_WKUP_EN_SEL_MSK | KC81710H_SW_ELKON_WKUP_EN_MSK | KC81710H_SW_LDON_WKUP_EN_MSK);
	bufdata |= KC81710H_SW_WKUP_EN_SEL_MSK;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_SWCFG, bufdata);
	if (ret) return ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_deep_sleep_enter(void)
{
	uint8_t ret;
	uint16_t bufdata = 0;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_TEST_EN, 0xDFEE);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_EFUSE_CTRL, 0x0001);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_EFUSE_CTRL + 1, 0x00CC);
	if (ret) return ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;

	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_SWCFG, &bufdata);
	if (ret) return ret;

	bufdata |= KC81710H_SW_WKUP_EN_SEL_MSK | KC81710H_SW_LDON_WKUP_EN_MSK;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_SWCFG, bufdata);
	if (ret) return ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_deep_sleep_restore(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_EFUSE_CTRL + 1, 0x0000);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_EFUSE_CTRL, 0x0000);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_TEST_EN, 0x0000);

	return ret;
}

uint8_t kc81710h_auto_scan_stop_set(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_set_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_AUTO_SCAN_STOP, KC81710H_AUTO_SCAN_STOP_DATA);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_auto_scan_stop_clr(void)
{
	uint8_t ret;

	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, KC81710H_PWD_UNLOCK_PATTERN);
	if (ret) return ret;
	ret = kc81710h_set_reg_bits(KC81710H_I2C_ADDR, KC81710H_OP_AUTO_SCAN_STOP, 0);
	if (ret) return ret;
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_PWD, 0);

	return ret;
}

uint8_t kc81710h_sw_check_all_req(uint8_t wait_comp)
{
	uint8_t ret;
	uint16_t bufdata;
	uint16_t mask = KC81710H_IF3_SW_CHECKALL_DONE_MSK | KC81710H_IF3_SW_CCRTCHK_DONE_MSK;
	// clear flag
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_IF3, mask);
	if (ret) return ret;
	ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IF3, &bufdata);
	if (ret) return ret;
	if(bufdata & mask)
	{
		return KC_STATUS_ERROR;
	}
	// write check all request
	ret = kc81710h_write_word(KC81710H_I2C_ADDR, KC81710H_OP_SW_CHKALL_REQ, KC81710H_SW_CHKALL_REQ_DATA);
	if (ret) return ret;
	// wait for doing
	if(wait_comp)
	{
		uint8_t cnt = 250;
		for(;;)
		{
			kc81710h_delay_ms(1);
			ret = kc81710h_read_word(KC81710H_I2C_ADDR, KC81710H_OP_IF3, &bufdata);
			if (ret) return ret;
			if((bufdata & mask) == mask)
			{
				break;
			}
			if (cnt-- == 0)
			{
				return KC_STATUS_TIMEOUT;
			}
		}
	}

	return ret;
}

#if 0
/*****************************************************************************
 * Description:
 *		cal_others
 *       calculate the sample result except external thm
 * Parameters:
 *	    CH: The channels refer to CH_*
 *	    raw: the raw data
 * Return:
 *		The calculated result.
 *****************************************************************************/
int32_t cal_others(uint8_t CH, int16_t raw)
{
    int32_t res;
    if (CH <= OP_CH_CELL08)
        res = cell_volt_mv_cal(raw);
    else
        res = thm_0degC1_cal(raw);
    return res;
}

uint8_t kc81710h_scan(uint8_t CH, int32_t *buf)
{
    uint8_t ret = 0;
    int16_t tmp, tmp1;
	uint32_t tmp32;
	int32_t itmp32;

	  if (CH == OP_CH_CRRT0)
		{
			ret = vadc_cscan_raw(OP_CH_CRRT0, &tmp);
			ret = vadc_cscan_raw(OP_CH_CRRT1, &tmp1);

			tmp1 &= 0x0003;
			tmp32 = (uint32_t)tmp << 2 | tmp1;

	  	itmp32 = S24toS32(tmp32);
      *buf = current_ma_cal(itmp32, RSENSE);
		}
		else
		{
			ret = vadc_cscan_raw(CH, &tmp);
			*buf = cal_others(CH, tmp);
		}
    return !!ret;
}

#endif

