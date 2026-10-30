/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : BatterCheck.c
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2017年1月6日 星期五
  最近修改   :
  功能描述   : 电芯、电量检测C源文件
  函数列表   :
  修改历史   :
  1.日    期   : 2017年1月6日 星期五
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/
#define _ADCM_C_

#include "AdcM.h"

/*******************************************************************************
         						ADC1检测通道
********************************************************************************/
const uint8_t ADC1_CHANNEL_TABLE[ADC1_CHL_NUM] =
{
	ADC_PACK_VOL, ADC_BAT_VOL, ADC_OPAM_CUR	
};

/*******************************************************************************
         						ADC2检测通道
********************************************************************************/
const uint8_t ADC2_CHANNEL_TABLE[ADC2_CHL_NUM] =
{
	ADC_NTC_CHG, ADC_NTC1
};

/*******************************************************************************
         						ADC3检测通道
********************************************************************************/
const uint8_t ADC3_CHANNEL_TABLE[ADC3_CHL_NUM] =
{
	ADC_NTC_DSG
};

/*******************************************************************************/

volatile ADC_Data_t AdcData;

/******************************************************************************
 函 数 名  : ADC_InitInstance

 功能描述  : ADC模块配置:规则序列、每个通道采样10次并保存在缓存中。

 输入参数  : AdcInstance;  RegChannel:通道组； 

 返 回 值  :无
 ******************************************************************************/
void ADC_InitInstance(ADC_Module* AdcInstance,const uint8_t RegChannel[], uint8_t ChlNum)
{	
	uint8_t i;
	ADC_InitType ADC_InitStructure;

	/* ADC configuration */
	ADC_InitStructure.WorkMode       = ADC_WORKMODE_INDEPENDENT;
	ADC_InitStructure.MultiChEn      = ENABLE;
	ADC_InitStructure.ContinueConvEn = ENABLE;
	ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
	ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
	ADC_InitStructure.ChsNumber      = ChlNum;
	ADC_Init(AdcInstance, &ADC_InitStructure);
	
	/* ADC1 regular channel configuration */
	for(i = 0; i < ChlNum; i++)
	{
		ADC_ConfigRegularChannel(AdcInstance, RegChannel[i], 1 + i, ADC_SAMP_TIME_239CYCLES5);
	}

	/* Disable regular discontinuous mode */
	ADC_EnableDiscMode(AdcInstance, DISABLE);
	/* Enable ADC external trigger conversion */
	ADC_EnableExternalTrigConv(AdcInstance, ENABLE);
	/* Clear ADC_FLAG_ENDC End of conversion flag */
	ADC_ClearFlag(AdcInstance, ADC_FLAG_ENDC);
	ADC_ClearFlag(AdcInstance, ADC_FLAG_STR);
	/* Enable EOC interrupt */
	ADC_ConfigInt(AdcInstance, ADC_INT_ENDC, DISABLE);
	/* Enable ADC DMA */
	ADC_EnableDMA(AdcInstance, ENABLE);
	/* Enable ADC */
	ADC_Enable(AdcInstance, ENABLE);

	/* wait ADC is ready to use */
	while(!ADC_GetFlagStatusNew(AdcInstance, ADC_FLAG_RDY));

	/* wait ADC is Power down ready */
	//while(ADC_GetFlagStatusNew(AdcInstance, ADC_FLAG_PD_RDY));

	/* Start ADC1 calibration */
	ADC_StartCalibration(AdcInstance);

	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(AdcInstance));

    ADC_EnableSoftwareStartConv(AdcInstance, ENABLE);
}

/*****************************************************************************
 函 数 名  : ADCInit
 功能描述  : AD模块初始化
 输入参数  : void
 返 回 值  :
*****************************************************************************/
void ADCInit(void)
{
//	ADC_InitType ADC_InitStructure;
	DMA_InitType DMA_InitStructure;
	
	/* DMA channel for ADC1 configuration */
	DMA_DeInit(DMA1_CH1);
	DMA_InitStructure.PeriphAddr     = (uint32_t)&ADC1->DAT;
	DMA_InitStructure.MemAddr        = (uint32_t)AdcData.RawAdc1;
	DMA_InitStructure.Direction      = DMA_DIR_PERIPH_SRC;
	DMA_InitStructure.BufSize        = (ADC1_CHL_NUM * ADC_CONVERT_TIME);
	DMA_InitStructure.PeriphInc      = DMA_PERIPH_INC_DISABLE;
	DMA_InitStructure.DMA_MemoryInc  = DMA_MEM_INC_ENABLE;
	DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_HALFWORD;
	DMA_InitStructure.MemDataSize    = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.CircularMode   = DMA_MODE_CIRCULAR;
	DMA_InitStructure.Priority       = DMA_PRIORITY_HIGH;
	DMA_InitStructure.Mem2Mem        = DMA_M2M_DISABLE;
	DMA_Init(DMA1_CH1, &DMA_InitStructure);
	DMA_RequestRemap(DMA1_REMAP_ADC1, DMA1, DMA1_CH1, ENABLE);
	/* Enable DMA channel1 */
	DMA_EnableChannel(DMA1_CH1, ENABLE);	

	/* DMA channel for ADC2 configuration */
	DMA_DeInit(DMA1_CH2);
	DMA_InitStructure.PeriphAddr     = (uint32_t)&ADC2->DAT;
	DMA_InitStructure.MemAddr        = (uint32_t)AdcData.RawAdc2;
	DMA_InitStructure.BufSize        = (ADC2_CHL_NUM * ADC_CONVERT_TIME);
	DMA_Init(DMA1_CH2, &DMA_InitStructure);
	DMA_RequestRemap(DMA1_REMAP_ADC2, DMA1, DMA1_CH2, ENABLE);
	DMA_EnableChannel(DMA1_CH2, ENABLE);

    /* DMA channel for ADC3 configuration */
	DMA_DeInit(DMA2_CH3);
	DMA_InitStructure.PeriphAddr     = (uint32_t)&ADC3->DAT;
	DMA_InitStructure.MemAddr        = (uint32_t)AdcData.RawAdc3;
	DMA_InitStructure.BufSize        = (ADC3_CHL_NUM * ADC_CONVERT_TIME);
	DMA_Init(DMA2_CH3, &DMA_InitStructure);
	DMA_RequestRemap(DMA2_REMAP_ADC3, DMA2, DMA2_CH3, ENABLE);
	DMA_EnableChannel(DMA2_CH3, ENABLE);
	
	/* ADC configuration */
	ADC_InitInstance(ADC1,ADC1_CHANNEL_TABLE,(sizeof(ADC1_CHANNEL_TABLE)/sizeof(ADC1_CHANNEL_TABLE[0])));
	ADC_InitInstance(ADC2,ADC2_CHANNEL_TABLE,(sizeof(ADC2_CHANNEL_TABLE)/sizeof(ADC2_CHANNEL_TABLE[0])));
	ADC_InitInstance(ADC3,ADC3_CHANNEL_TABLE,(sizeof(ADC3_CHANNEL_TABLE)/sizeof(ADC3_CHANNEL_TABLE[0])));
}

/*****************************************************************************
 函 数 名  : ADCDeInit
 功能描述  : AD模块恢复默认参数值
 输入参数  : void
 返 回 值  :
*****************************************************************************/
void ADCDeInit(void)
{
    ADC_DeInit(ADC1);
    ADC_DeInit(ADC2);
    ADC_DeInit(ADC3);

    ADC_Enable(ADC1, DISABLE);
    ADC_Enable(ADC2, DISABLE);
    ADC_Enable(ADC3, DISABLE);
}

void ADC_ConvertStop(void)
{
}

void ADC_ConvertStart(void)
{
    ADC_EnableSoftwareStartConv(ADC1, ENABLE);
    ADC_EnableSoftwareStartConv(ADC2, ENABLE);
    ADC_EnableSoftwareStartConv(ADC3, ENABLE);
}

/*****************************************************************************
 函 数 名  : ADC_GetAdVal
 功能描述  : AD读取函数，采10次，去最大、最小值，取8次平均
 输入参数  : AD通道
 返 回 值  : AD平均值 
*****************************************************************************/
void ADC_GetAdVal(ADC_Module* ADCx, uint8_t chl_num)
{
	uint8_t i;
	uint8_t index;	
    uint16_t tempValue = 0;
    uint16_t Ad_MaxValue = 0;
    uint16_t Ad_MinValue = 0;
    uint32_t Ad_SumValue = 0;
	volatile uint16_t *pRawAdc;
	volatile uint16_t *pAvgAdc;
	
	if(ADCx == ADC1)
	{
		pRawAdc = &AdcData.RawAdc1[0];
		pAvgAdc = &AdcData.AvgAdc1[0];
	}
	else if(ADCx == ADC2)
	{
		pRawAdc = &AdcData.RawAdc2[0];
		pAvgAdc = &AdcData.AvgAdc2[0];
	}
	else if(ADCx == ADC3)
	{
		pRawAdc = &AdcData.RawAdc3[0];
		pAvgAdc = &AdcData.AvgAdc3[0];
	}
	else
	{
		return;
	}
	
	for(index = 0; index < chl_num; index++)
	{
		Ad_MinValue = ADC_VALUE_MAX;
		Ad_MaxValue = 0;
		Ad_SumValue = 0;

		for(i = 0; i < ADC_CONVERT_TIME; i++)
		{
			tempValue = pRawAdc[i * chl_num + index];			

			if(tempValue > Ad_MaxValue)
			{
				Ad_MaxValue = tempValue;
			}

			if(tempValue < Ad_MinValue)
			{
				Ad_MinValue = tempValue;
			}

			Ad_SumValue += tempValue;
		}

		Ad_SumValue = Ad_SumValue - Ad_MaxValue - Ad_MinValue;
		pAvgAdc[index] = Ad_SumValue / (ADC_CONVERT_TIME - 2);
	}
}
/*****************************************************************************
 函 数 名  : AdcM_Calc
 功能描述  : AdcM模块值获取
 输入参数  : void
 返 回 值  :
*****************************************************************************/
////////////////////////////////////////////////////////////
void AdcM_Calc(void)
{
	ADC_GetAdVal(ADC1, ADC1_CHL_NUM);
	ADC_GetAdVal(ADC2, ADC2_CHL_NUM);
	ADC_GetAdVal(ADC3, ADC3_CHL_NUM);

	//AD值按照索引表重新排列。
	//ADC1中的采集通道:
	AdcData.ADVal[CH_PACK_VOL] = AdcData.AvgAdc1[0];
	AdcData.ADVal[CH_BAT_VOL] = AdcData.AvgAdc1[1];
	AdcData.ADVal[CH_OPAM_CUR] = AdcData.AvgAdc1[2];

	//ADC2中的采集通道:	
	AdcData.ADVal[CH_NTC_CHG] = AdcData.AvgAdc2[0];
	AdcData.ADVal[CH_NTC1] = AdcData.AvgAdc2[1];
	
	//ADC3中的采集通道:	
	AdcData.ADVal[CH_NTC_DSG] = AdcData.AvgAdc3[0];
}

/*****************************************************************************
 函 数 名  : AdcM_Ctrl
 功能描述  : Adc模块控制
 输入参数  :
 返 回 值  :
*****************************************************************************/
void AdcM_Ctrl(void)
{
    // Tick: 10ms
    if(!TimerAdcFlag)
    {
        return;
    }
    TimerAdcFlag = 0;
		
	AdcM_Calc();
}

/************************ (C) Blueway Electronics Co. *****END OF FILE*********/

