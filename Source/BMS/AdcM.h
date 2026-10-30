#ifndef __ADC_M_H__
#define __ADC_M_H__
#include "McuHal.h"

#ifdef _ADCM_C_
	#define GLOBAL
#else
	#define GLOBAL extern
#endif

//ADC1中的采集通道
#define ADC_PACK_VOL	(ADC1_Channel_09_PC3)	// PC3 P+_AD
#define ADC_BAT_VOL		(ADC1_Channel_11_PA2)	// PA2 B+_AD
#define ADC_OPAM_CUR	(ADC1_Channel_03_PA6)	// PA6 MCU_OPAM_CUR

//ADC2中的采集通道
#define ADC_NTC_CHG		(ADC2_Channel_01_PA4)	// PA4 NTC3 CHG-MOS NTC
#define ADC_NTC1 		(ADC2_Channel_12_PC5)	// PC5 NTC1

//ADC3中的采集通道
#define ADC_NTC_DSG		(ADC3_Channel_12_PB0)	// PB0 NTC2 DSG-MOS NTC

////////////////////////////////////////////////////////////
#define ADC1_CHL_NUM        (3)
#define ADC2_CHL_NUM        (2)
#define ADC3_CHL_NUM        (1)
#define ADC_CONVERT_TIME    (6)

#define ADC_VALUE_MAX       4096
#define ADC_VOLT_VDDA		3300

#define CH_PACK_VOL			(0)
#define CH_BAT_VOL			(1)
#define CH_OPAM_CUR			(2)
#define CH_NTC_CHG			(3)
#define CH_NTC1	    		(4)
#define CH_NTC_DSG			(5)

typedef struct
{
	uint16_t RawAdc1[ADC1_CHL_NUM * ADC_CONVERT_TIME];  //原始AD值
	uint16_t RawAdc2[ADC2_CHL_NUM * ADC_CONVERT_TIME];
	uint16_t RawAdc3[ADC3_CHL_NUM * ADC_CONVERT_TIME];
	uint16_t AvgAdc1[ADC1_CHL_NUM];	 //平均AD值
	uint16_t AvgAdc2[ADC2_CHL_NUM];
	uint16_t AvgAdc3[ADC3_CHL_NUM];

	uint16_t ADVal[ADC1_CHL_NUM + ADC2_CHL_NUM + ADC3_CHL_NUM];

}ADC_Data_t;

GLOBAL volatile ADC_Data_t AdcData;



GLOBAL void ADCInit(void);
GLOBAL void ADCDeInit(void);
GLOBAL void AdcM_Calc(void);
GLOBAL void AdcM_Ctrl(void);

#undef GLOBAL
#endif /* __ADCCONVERT_H__ */

