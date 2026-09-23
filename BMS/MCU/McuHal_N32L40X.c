#include "McuHal_N32L40X.h"
#include "SysDefs.h"
#include "board.h"
#include "stdio.h"

/*****************************************************************************
 函 数 名  : Delay10us
 功能描述  : 10us延时程序
 输入参数  : u16 TimeTick  
 返 回 值  : 
*****************************************************************************/
void Delay10us(u32 uldelay)
{
    u32 i, j;
	__NOP();
    for(i = 0; i < uldelay; i++) 		// 48M:160  64M:?(O2)
	{		
        for(j = 0; j < 160; j++)
		{
			;
        }
    }
}

/*****************************************************************************
 函 数 名  : Delay10us_INT
 功能描述  : 10us延时程序-中断函数中使用
 输入参数  : u16 TimeTick  
 返 回 值  : 
*****************************************************************************/
void Delay10us_INT(u32 uldelay)
{
    u32 i, j;
	__NOP();
    for(i = 0; i < uldelay; i++) 
	{		
        for(j = 0; j < 62; j++) 	
		{
			;
        }
    }
}

void Delay1ms(u16 TimeTick)
{
	if (TimeTick<1) 
	{
		TimeTick++;
	}
	
	while(TimeTick--) 
	{ 
		HwClrWdt();
		Delay10us(99);
	} 
}

/*****************************************************************************
 函 数 名  : ClkInit
 功能描述  : 系统时钟、基准时钟TIMER0初始化
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void ClkInit(void)
{
	/* Get SystemCoreClock */    
	SystemCoreClockUpdate();
	
	/* Config 1MS SysTick  */
    if(SysTick_Config(SystemCoreClock/1000))
    {
        /* Capture error */
        while(1);
    }

	/* Enable PWR Clock */
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_PWR, ENABLE);

	// RCC_ConfigLse(RCC_LSE_ENABLE);
}

/*****************************************************************************
 函 数 名  : SysTick_Set
 功能描述  : SysTick周期和控制的设定
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
uint32_t SysTick_Set(uint32_t ticks, FunctionalState Cmd)
{
	if (Cmd == DISABLE)
	{
		SysTick->CTRL  = 0;
		return (1UL);
	}
	
	if ((ticks - 1UL) > SysTick_LOAD_RELOAD_Msk)
	{
		return (1UL);                                                   /* Reload value impossible */
	}

	SysTick->LOAD  = (uint32_t)(ticks - 1UL);                         /* set reload register */
	NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
	SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
	               SysTick_CTRL_TICKINT_Msk   |
	               SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

	return (0UL);                                                     /* Function successful */
}

/*****************************************************************************
 函 数 名  : WdtInit
 功能描述  : 看门狗初始化
 			 WdtInit(IWDG_PRESCALER_DIV128, 61)产生250MS的看门狗定时器
 			 WdtInit(IWDG_PRESCALER_DIV128, 61*4)产生1000MS的看门狗定时器
 			 软件仿真时需要禁止调用.
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void WdtInit(void)
{
	/* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteConfig(IWDG_WRITE_ENABLE);

    /* IWDG counter clock: LSI/32 */
    IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV32);
    /* Set counter reload value to obtain 1000ms IWDG TimeOut.
       Counter Reload Value = 1000ms/IWDG counter clock period
                            = 1000ms / (LSI/32)
                            = 1s / (LsiFreq/32)
                            = LsiFreq/32 */
    IWDG_CntReload(40000/32 * 2);	// LSI 40KHz-2S
    /* Reload IWDG counter */
    IWDG_ReloadKey();

	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();

    IWDG_WriteConfig(IWDG_WRITE_DISABLE);

#ifdef SWD_DEBUG    	
	DBG_ConfigPeriph(DBG_IWDG_STOP, ENABLE);
	DBG_ConfigPeriph(DBG_STOP, ENABLE);
#endif
}


void HwClrWdt(void)
{
	IWDG_ReloadKey();
}

#if 1
/*****************************************************************************
 函 数 名  : ADCInit
 功能描述  : AD模块初始化
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void ADCInit(void)
{
	ADC_InitType ADC_InitStructure;

    /* Enable ADC clocks */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);
    /* RCC_ADCHCLK_DIV4 */
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV4);
    /* Enable ADC 1M clock */
    RCC_EnableHsi(ENABLE);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSI, RCC_ADC1MCLK_DIV16);

	
     /* ADC configuration ------------------------------------------------------*/
    ADC_InitStructure.MultiChEn      = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
    ADC_InitStructure.DatAlign       = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber      = 1;
    ADC_Init(ADC, &ADC_InitStructure);

    /* Enable ADC */
    ADC_Enable(ADC, ENABLE);
    
    /* Check ADC Ready */
    while(ADC_GetFlagStatusNew(ADC,ADC_FLAG_RDY) == RESET)
    {
        ;
    }
	while(ADC_GetFlagStatusNew(ADC,ADC_FLAG_PD_RDY))
	{
        ;                   
	}
	
	/* Start ADC1 calibration */
    ADC_StartCalibration(ADC);
    /* Check the end of ADC1 calibration */
    while (ADC_GetCalibrationStatus(ADC))
    {
        ;
    }
}

#else

void ADCInit(void)
{
	ADC_InitType ADC_InitStructure;

    /* Enable ADC clocks */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);
	/* Enable ADC 1M clock */
	RCC_EnableHsi(ENABLE);
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSI, RCC_ADC1MCLK_DIV16);
    /* RCC_ADCHCLK_DIV16 */
    ADC_ConfigClk(ADC_CTRL3_CKMOD_PLL, RCC_ADCPLLCLK_DIV4);

	/* ADC configuration ------------------------------------------------------*/
	ADC_InitStructure.MultiChEn 	 = DISABLE;
	ADC_InitStructure.ContinueConvEn = DISABLE;
	ADC_InitStructure.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE;
	ADC_InitStructure.DatAlign		 = ADC_DAT_ALIGN_R;
	ADC_InitStructure.ChsNumber 	 = 1;
	ADC_Init(ADC, &ADC_InitStructure);

	/* Set injected sequencer length */
	ADC_ConfigInjectedSequencerLength(ADC, 1);
	/* ADC injected channel Configuration */
	ADC_ConfigInjectedChannel(ADC, ADC_DHL, 1, ADC_SAMP_TIME_71CYCLES5);

	/* ADC injected external trigger configuration */
	ADC_ConfigExternalTrigInjectedConv(ADC, ADC_EXT_TRIG_INJ_CONV_T3_CC4);
	/* Enable automatic injected conversion start after regular one */
	ADC_EnableAutoInjectedConv(ADC, DISABLE);
	/* Enable JEOC interrupt */
//	  ADC_ConfigInt(ADC, ADC_INT_JENDC, ENABLE);
	ADC_EnableExternalTrigInjectedConv(ADC, ENABLE);
	
	/* Configure high and low analog watchdog thresholds */
	ADC_ConfigAnalogWatchdogThresholds(ADC, 0x0FFF, 0x0FFE);
	/* Configure channel1 as the single analog watchdog guarded channel */
//	  ADC_ConfigAnalogWatchdogSingleChannel(ADC, ADC_CH_4_PA4);
	/* Enable analog watchdog on one regular channel */
	ADC_ConfigAnalogWatchdogWorkChannelType(ADC, ADC_ANALOG_WTDG_SINGLEINJEC_ENABLE);	//ADC_ANALOG_WTDG_ALLINJEC_ENABLE);//

	/* Enable AWD interrupt */
	ADC_ConfigInt(ADC, ADC_INT_JENDC, ENABLE);

	/* Enable ADC */
	ADC_Enable(ADC, ENABLE);

	/*wait ADC is ready to use*/
	while(!ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY));
		/*wait ADC is powered on*/
	while(ADC_GetFlagStatusNew(ADC, ADC_FLAG_PD_RDY));

		/* Start ADC1 calibration */
    ADC_StartCalibration(ADC);
    /* Check the end of ADC1 calibration */
    while (ADC_GetCalibrationStatus(ADC))
    {
        ;
    }

}
#endif

/*****************************************************************************
 函 数 名  : ADCDeInit
 功能描述  : AD模块恢复默认参数值
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void ADCDeInit(void)
{
    ADC_DeInit(ADC);

    ADC_Enable(ADC, DISABLE);
}

/*****************************************************************************
 函 数 名  : ADC
 功能描述  : 读取通道的具体AD值
 输入参数  : u8 channel AD通道 
 返 回 值  : 
*****************************************************************************/
__attribute__((unused)) static u16 HwADC(u8 channel)
{
    u16 dat;
	ADC_ConfigRegularChannel(ADC, channel, 1, ADC_SAMP_TIME_71CYCLES5);	// ADC_SAMP_TIME_56CYCLES5);
	/* Start ADC Software Conversion */
	ADC_EnableSoftwareStartConv(ADC,ENABLE);
	while(ADC_GetFlagStatus(ADC, ADC_FLAG_ENDC)==0){
	}
	ADC_ClearFlag(ADC, ADC_FLAG_ENDC);
	ADC_ClearFlag(ADC, ADC_FLAG_STR);
	dat=ADC_GetDat(ADC);
	return dat;
}

/*****************************************************************************
 函 数 名  : ADRead
 功能描述  : AD读取函数，采10次，去最大、最小值，取8次平均
 输入参数  : u8 channel  AD通道
 返 回 值  : 读取的AD值
*****************************************************************************/
u16 ADRead(u8 channel)
{
    u16 value = 0;
	u8 i = 0;
	u16 Ad_MaxValue = 0;
	u16 Ad_MinValue = 0;
	u16 Ad_Sum = 0;	

	// 采样10次,抛弃最大值和最小值,剩余8次求平均.
	Ad_MaxValue = 0;
	Ad_MinValue = 0x1000;
	Ad_Sum = 0;

	ADC_ConfigRegularChannel(ADC, channel, 1, ADC_SAMP_TIME_71CYCLES5);
	ADC_ClearFlag(ADC, ADC_FLAG_ENDC);
	ADC_ClearFlag(ADC, ADC_FLAG_STR);

	for (i=0; i<10; i++)
	{	
		/* Start ADC Software Conversion */
		ADC_EnableSoftwareStartConv(ADC, ENABLE);
		while(ADC_GetFlagStatus(ADC, ADC_FLAG_ENDC) == 0);
		ADC_ClearFlag(ADC, ADC_FLAG_ENDC);
		ADC_ClearFlag(ADC, ADC_FLAG_STR);
		
		value = ADC_GetDat(ADC);
		if (value > Ad_MaxValue)
		{
			Ad_MaxValue = value;
		}
		if (value < Ad_MinValue)
		{
			Ad_MinValue = value;
		}
		Ad_Sum += value;
	}
	Ad_Sum = Ad_Sum - Ad_MaxValue - Ad_MinValue;
    value = Ad_Sum/8;
		
    return value;
}

/*****************************************************************************
 函 数 名  : PortInit
 功能描述  : IO口配置初始化
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void PortInit(void)
{	
	GPIO_InitType GPIO_InitStructure;		
    GPIO_InitStruct(&GPIO_InitStructure);	

    /* Enable GPIO clocks */
    RCC_EnableAPB2PeriphClk( RCC_APB2_PERIPH_GPIOA\
    						|RCC_APB2_PERIPH_GPIOB\
    						|RCC_APB2_PERIPH_GPIOC\
    						|RCC_APB2_PERIPH_GPIOD\
    						|RCC_APB2_PERIPH_AFIO,  ENABLE );
    FUSE_BLOW_OFF();
    NTC_POW_ON();
    PWR_5V_ON();
    OPA_BIAS_ON();
    CAN_5V_ON();
	  BAT_POW_ON();
		/* Configure analog */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
    GPIO_InitStructure.Pin       = CELL_NTC1_Pin;
    GPIO_InitPeripheral(CELL_NTC1_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = DMOS_NTC_Pin;
    GPIO_InitPeripheral(DMOS_NTC_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = CMOS_NTC_Pin;
    GPIO_InitPeripheral(CMOS_NTC_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = CAGR_VOL_Pin;
    GPIO_InitPeripheral(CAGR_VOL_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = BAT_VOL_Pin;
    GPIO_InitPeripheral(BAT_VOL_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = OPA_CUR_Pin;
    GPIO_InitPeripheral(OPA_CUR_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = OPAMP_VP_Pin;
    GPIO_InitPeripheral(OPAMP_VP_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = OPAMP_VN_Pin;
    GPIO_InitPeripheral(OPAMP_VN_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = DET_IN_AD_Pin;
		GPIO_InitPeripheral(DET_IN_AD_Port, &GPIO_InitStructure);	
//    GPIO_InitStructure.Pin       = DET_IN_AD2_Pin;
//	GPIO_InitPeripheral(DET_IN_AD2_Port, &GPIO_InitStructure);	
	
    /* Configure output PP */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
    GPIO_InitStructure.Pin       = OPA_BIAS_Pin;
    GPIO_InitPeripheral(OPA_BIAS_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = AFE_SPI_CS_Pin;
    GPIO_InitPeripheral(AFE_SPI_CS_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = AFE_EFET_Pin;
    GPIO_InitPeripheral(AFE_EFET_Port, &GPIO_InitStructure);    
//    GPIO_InitStructure.Pin       = CAN_VIO_Pin;
//    GPIO_InitPeripheral(CAN_VIO_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = EN_5V_Pin;
    GPIO_InitPeripheral(EN_5V_Port, &GPIO_InitStructure);
//	GPIO_InitStructure.Pin       = LEDT_CTRL_Pin;
//    GPIO_InitPeripheral(LEDT_CTRL_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = NTC_PWR_Pin;
    GPIO_InitPeripheral(NTC_PWR_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       =  PWR_5V_Pin;
    GPIO_InitPeripheral(PWR_5V_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       =  BAT_PWR_Pin;
    GPIO_InitPeripheral(BAT_PWR_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = FUSE_BLOW_Pin;
    GPIO_InitPeripheral(FUSE_BLOW_Port, &GPIO_InitStructure);
		GPIO_InitStructure.Pin       = LED1_CTRL_Pin;
    GPIO_InitPeripheral(LED1_CTRL_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = LED2_CTRL_Pin;
    GPIO_InitPeripheral(LED2_CTRL_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = LED3_CTRL_Pin;
    GPIO_InitPeripheral(LED3_CTRL_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = LED4_CTRL_Pin;
    GPIO_InitPeripheral(LED4_CTRL_Port, &GPIO_InitStructure); 
    GPIO_InitStructure.Pin       = LED5_CTRL_Pin;
    GPIO_InitPeripheral(LED5_CTRL_Port, &GPIO_InitStructure); 
    GPIO_InitStructure.Pin       = LED6_CTRL_Pin;
    GPIO_InitPeripheral(LED6_CTRL_Port, &GPIO_InitStructure); 
    GPIO_InitStructure.Pin       = LED7_CTRL_Pin;
    GPIO_InitPeripheral(LED7_CTRL_Port, &GPIO_InitStructure); 
    GPIO_InitStructure.Pin       = LED8_CTRL_Pin;
    GPIO_InitPeripheral(LED8_CTRL_Port, &GPIO_InitStructure); 
		
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
		GPIO_InitStructure.Pin       = DET_IN_PWR_Pin;
		GPIO_InitPeripheral(DET_IN_PWR_Port, &GPIO_InitStructure); 
		GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;   
	
    /* unused pin set default PP output */
//		GPIO_InitStructure.Pin       = GPIO_PIN_4 | GPIO_PIN_15;
//    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
//    GPIO_InitStructure.Pin       = GPIO_PIN_8;
//    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
//		GPIO_InitStructure.Pin       = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_0\
//									|GPIO_PIN_6|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
//    GPIO_InitPeripheral(GPIOC, &GPIO_InitStructure);
//    GPIO_InitStructure.Pin       = GPIO_PIN_2;
//    GPIO_InitPeripheral(GPIOD, &GPIO_InitStructure);
    
    /* Configure input float */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
    GPIO_InitStructure.Pin       = AFE_ALTN_Pin;
    GPIO_InitPeripheral(AFE_ALTN_Port, &GPIO_InitStructure);

		/* Configure input pull up */
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
//	GPIO_InitStructure.Pin       = CHG_IN_Pin;
//    GPIO_InitPeripheral(CHG_IN_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = CAN_IN_Pin;
    GPIO_InitPeripheral(CAN_IN_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = CHG_OV2_Pin;
		GPIO_InitPeripheral(CHG_OV2_Port, &GPIO_InitStructure);
		//按键IO信号输入
		GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	// GPIO_InitStructure.Pin       = DET_IN_Pin;
    // GPIO_InitPeripheral(DET_IN_Port, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
		GPIO_InitStructure.Pin       = KEY_LED_Pin;
    GPIO_InitPeripheral(KEY_LED_Port, &GPIO_InitStructure);
		GPIO_InitStructure.Pin       = VLOCK_IN_Pin;
    GPIO_InitPeripheral(VLOCK_IN_Port, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
//	GPIO_InitStructure.Pin       = TYPEC_SCL_Pin;
//    GPIO_InitPeripheral(TYPEC_SCL_Port, &GPIO_InitStructure);
//    GPIO_InitStructure.Pin       = TYPEC_SDA_Pin;
//    GPIO_InitPeripheral(TYPEC_SDA_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = AFE_SCL_Pin;
    GPIO_InitPeripheral(AFE_SCL_Port, &GPIO_InitStructure);
    GPIO_InitStructure.Pin       = AFE_SDA_Pin;
    GPIO_InitPeripheral(AFE_SDA_Port, &GPIO_InitStructure);    
		/* Configure SPI pins: SCK and MOSI as Alternate Function Push Pull */
		 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		 GPIO_InitStructure.Pin        = AFE_SPI_CLK_Pin;    
		 GPIO_InitStructure.GPIO_Alternate = AFE_SPI_CLK_AF;  
		 GPIO_InitPeripheral(AFE_SPI_Port, &GPIO_InitStructure);
		 GPIO_InitStructure.Pin        = AFE_SPI_MOSI_Pin;    
		 GPIO_InitStructure.GPIO_Alternate = AFE_SPI_MOSI_AF;           
		 GPIO_InitPeripheral(AFE_SPI_Port, &GPIO_InitStructure);
		 GPIO_InitStructure.Pin = AFE_SPI_MISO_Pin;     
		 GPIO_InitStructure.GPIO_Alternate = AFE_SPI_MISO_AF;
		 GPIO_InitPeripheral(AFE_SPI_Port, &GPIO_InitStructure);
	
		/* Configure MISO pin as Input Floating  */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;	
		GPIO_InitStructure.Pin = AFE_SPI_MISO_Pin;     
		GPIO_InitStructure.GPIO_Alternate = AFE_SPI_MISO_AF;
		GPIO_InitPeripheral(AFE_SPI_Port, &GPIO_InitStructure);


	/* Configure CAN RX */
    GPIO_InitStructure.Pin       = CAN_RX_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
    GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF1_CAN;
    GPIO_InitPeripheral(CAN_RX_Port, &GPIO_InitStructure);
    /* Configure CAN TX */
    GPIO_InitStructure.Pin        = CAN_TX_Pin;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitPeripheral(CAN_TX_Port, &GPIO_InitStructure);
	
#ifndef SWD_DEBUG
    /* SWD interface set as IO */
	// GPIO_InitStructure.Pin       = GPIO_PIN_13|GPIO_PIN_14;
	// GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
#endif	

    /* Configure USART RX */
    // GPIO_InitStructure.Pin       = UART3_RX_Pin;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    // GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
    // GPIO_InitStructure.GPIO_Alternate = UART3_RX_AF;
    // GPIO_InitPeripheral(UART3_RX_Port, &GPIO_InitStructure);
    /* Configure USART TX */
//    GPIO_InitStructure.Pin        = UART3_TX_Pin;
//    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
//    GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
//    GPIO_InitStructure.GPIO_Alternate = UART3_TX_AF;
//    GPIO_InitPeripheral(UART3_TX_Port, &GPIO_InitStructure);

		GPIO_InitStructure.Pin            = UART3_TX_Pin;
    GPIO_InitStructure.GPIO_Pull 	  = GPIO_Pull_Up;
		GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Alternate = UART3_TX_AF;
    GPIO_InitPeripheral(UART3_TX_Port, &GPIO_InitStructure);

    Delay1ms(5);
//    CAN_VIO_ON();
    DET_IN_POW_ON();    
}

/*****************************************************************************
 函 数 名  : PwmSetFreq
 功能描述  : PWM 频率设置
 输入参数  : u16 nFre  
 返 回 值  : 
*****************************************************************************/
void PwmSetFreq(u16 prescaler,u16 period)
{
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.Period    = period;		//665;
    TIM_TimeBaseStructure.Prescaler = prescaler;			// PrescalerValue;
    TIM_TimeBaseStructure.ClkDiv    = 0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(TIM3, &TIM_TimeBaseStructure);
}

/*****************************************************************************
 函 数 名  : PwmSetDuty
 功能描述  : PWM 占空比设置
 输入参数  : u16 nDuty  
 返 回 值  : 
*****************************************************************************/
void PwmSetDuty(u16 nDuty)
{     
	OCInitType TIM_OCInitStructure;
	/* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.OcMode      = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
    TIM_OCInitStructure.Pulse       = nDuty;
    TIM_OCInitStructure.OcPolarity  = TIM_OC_POLARITY_HIGH;

    TIM_InitOc1(TIM3, &TIM_OCInitStructure);

    TIM_ConfigOc1Preload(TIM3, TIM_OC_PRE_LOAD_ENABLE);
}

/*****************************************************************************
 函 数 名  : PwmInit
 功能描述  : PWM初始化
// PB0: dirty sensor output, T3C3P
 PB1: Brush1 control, T1C3N
 PB14:Brush2 control, T1C2N
 PB10:Pump control, T2C3P
 PB15:BLDC PWM, T9C4P
 PC8: dirty sensor output, T8C3P
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void PwmInit(void)
{
	OCInitType TIM_OCInitStructure;
	GPIO_InitType GPIO_InitStructure;
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
		
    GPIO_InitStruct(&GPIO_InitStructure);	
    TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
	TIM_InitOcStruct(&TIM_OCInitStructure);
	
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM2|RCC_APB1_PERIPH_TIM3|RCC_APB1_PERIPH_TIM9, ENABLE);
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_TIM1|RCC_APB2_PERIPH_TIM8, ENABLE);
	
	/* GPIO AF: PA7 set as T1 CH1N, PB1 set as T3 CH3, PB8 set as T8 CH1 */
    GPIO_InitStructure.Pin        = GPIO_PIN_1;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF5_TIM1;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.Pin        = GPIO_PIN_14;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF2_TIM1;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.Pin        = GPIO_PIN_10;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF2_TIM2;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.Pin        = GPIO_PIN_15;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF1_TIM9;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.Pin        = GPIO_PIN_8;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF6_TIM8;
    GPIO_InitPeripheral(GPIOC, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_14);
//    BRUSH1_PWM_OFF();


    /* TIMx configuration */  
    TIM_TimeBaseStructure.ClkDiv    = 0x0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

    TIM_TimeBaseStructure.Prescaler = 0;
    TIM_TimeBaseStructure.Period    = SystemCoreClock/2000-1;	// BRUSH PWM: 2K
    TIM_InitTimeBase(TIM1, &TIM_TimeBaseStructure);

    TIM_TimeBaseStructure.Prescaler = 0;	
	TIM_TimeBaseStructure.Period	= SystemCoreClock/1000-1;	// BLDC PWM: 1K		
	TIM_InitTimeBase(TIM9, &TIM_TimeBaseStructure);

    TIM_TimeBaseStructure.Prescaler = SystemCoreClock/11200 - 1;
    TIM_TimeBaseStructure.Period    = 11200/7-1;				// PUMP PWM: 7-8-20
    TIM_InitTimeBase(TIM2, &TIM_TimeBaseStructure);	
    
    TIM_TimeBaseStructure.Prescaler = 0;
    TIM_TimeBaseStructure.Period    = SystemCoreClock/1000-1;	// dirty PWM: 1K
    TIM_InitTimeBase(TIM8, &TIM_TimeBaseStructure);	    


    TIM_TimeBaseStructure.Prescaler = 0;
    TIM_TimeBaseStructure.Period    = SystemCoreClock/1000-1;	// trig PWM: 1K
    TIM_InitTimeBase(TIM3, &TIM_TimeBaseStructure);	    

    /* PWM1 Mode configuration */
    TIM_OCInitStructure.OcMode       = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.Pulse        = 0; 
    
    /* brush1/2 T1C3N,T1C1N */
    TIM_OCInitStructure.OutputState  = TIM_OUTPUT_STATE_DISABLE;
    TIM_OCInitStructure.OutputNState = TIM_OUTPUT_NSTATE_ENABLE;       
//    TIM_OCInitStructure.OcPolarity   = TIM_OC_POLARITY_HIGH;    
    TIM_OCInitStructure.OcNPolarity  = TIM_OCN_POLARITY_LOW;    
//    TIM_OCInitStructure.OcIdleState  = TIM_OC_IDLE_STATE_SET;    
    TIM_OCInitStructure.OcNIdleState = TIM_OC_IDLE_STATE_SET;
    TIM_InitOc3(TIM1, &TIM_OCInitStructure);
    TIM_InitOc2(TIM1, &TIM_OCInitStructure);
    TIM_ConfigOc3Preload(TIM1, TIM_OC_PRE_LOAD_ENABLE);
    TIM_ConfigOc2Preload(TIM1, TIM_OC_PRE_LOAD_ENABLE);

    /* BLDC pwm T9C4P */
	TIM_OCInitStructure.OutputState  = TIM_OUTPUT_STATE_ENABLE;
	TIM_OCInitStructure.OutputNState = TIM_OUTPUT_NSTATE_DISABLE;	 
	TIM_OCInitStructure.OcPolarity   = TIM_OC_POLARITY_HIGH;	 
//	TIM_OCInitStructure.OcNPolarity  = TIM_OCN_POLARITY_HIGH;	 
	TIM_OCInitStructure.OcIdleState  = TIM_OC_IDLE_STATE_RESET;
//	TIM_OCInitStructure.OcNIdleState = TIM_OC_IDLE_STATE_RESET;
	TIM_InitOc4(TIM9, &TIM_OCInitStructure);
	TIM_ConfigOc4Preload(TIM9, TIM_OC_PRE_LOAD_ENABLE);

	TIM_OCInitStructure.OutputState  = TIM_OUTPUT_STATE_ENABLE;
	TIM_OCInitStructure.OutputNState = TIM_OUTPUT_NSTATE_DISABLE;	 
	TIM_OCInitStructure.OcPolarity	 = TIM_OC_POLARITY_HIGH;	 
//	TIM_OCInitStructure.OcNPolarity  = TIM_OCN_POLARITY_HIGH;	 
	TIM_OCInitStructure.OcIdleState  = TIM_OC_IDLE_STATE_RESET;
//	TIM_OCInitStructure.OcNIdleState = TIM_OC_IDLE_STATE_RESET;
	TIM_OCInitStructure.Pulse		 = 200;
	TIM_InitOc4(TIM3, &TIM_OCInitStructure);
	TIM_ConfigOc4Preload(TIM3, TIM_OC_PRE_LOAD_ENABLE);
	TIM_OCInitStructure.Pulse        = 0;

	/* pump T2C3P */
	TIM_OCInitStructure.OutputState  = TIM_OUTPUT_STATE_ENABLE;
	TIM_OCInitStructure.OutputNState = TIM_OUTPUT_NSTATE_DISABLE;	 
	TIM_OCInitStructure.OcPolarity   = TIM_OC_POLARITY_LOW;	 
//	TIM_OCInitStructure.OcNPolarity  = TIM_OCN_POLARITY_HIGH;	 
	TIM_OCInitStructure.OcIdleState  = TIM_OC_IDLE_STATE_SET;
//	TIM_OCInitStructure.OcNIdleState = TIM_OC_IDLE_STATE_RESET;
	TIM_InitOc3(TIM2, &TIM_OCInitStructure);
	TIM_ConfigOc3Preload(TIM2, TIM_OC_PRE_LOAD_ENABLE);

	TIM_InitOc1(TIM2, &TIM_OCInitStructure);

    /* dirty pwm T8C3P */
	TIM_OCInitStructure.OutputState  = TIM_OUTPUT_STATE_ENABLE;
	TIM_OCInitStructure.OutputNState = TIM_OUTPUT_NSTATE_DISABLE;	 
	TIM_OCInitStructure.OcPolarity   = TIM_OC_POLARITY_HIGH;	 
//	TIM_OCInitStructure.OcNPolarity  = TIM_OCN_POLARITY_HIGH;	 
	TIM_OCInitStructure.OcIdleState  = TIM_OC_IDLE_STATE_RESET;
//	TIM_OCInitStructure.OcNIdleState = TIM_OC_IDLE_STATE_RESET;
	TIM_InitOc3(TIM8, &TIM_OCInitStructure);
	TIM_ConfigOc3Preload(TIM8, TIM_OC_PRE_LOAD_ENABLE);	

	TIM_Enable(TIM3, ENABLE);

	
	TIM_Enable(TIM1, ENABLE);
	TIM_Enable(TIM2, ENABLE);
	TIM_Enable(TIM8, ENABLE);
	TIM_Enable(TIM9, ENABLE);
	TIM_EnableCtrlPwmOutputs(TIM1, ENABLE);
	TIM_EnableCtrlPwmOutputs(TIM8, ENABLE);	
}

/*****************************************************************************
 函 数 名  : UartInit
 功能描述  : Uart初始化
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void UartOpen(USART_Module* USARTn, u32 BaudRate)
{
	USART_InitType USART_InitStructure;

	 /* USART configuration ------------------------------------------------------*/
	USART_InitStructure.BaudRate			= BaudRate;
	USART_InitStructure.WordLength			= USART_WL_8B;
	USART_InitStructure.StopBits			= USART_STPB_1;
	USART_InitStructure.Parity				= USART_PE_NO;
	USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
	USART_InitStructure.Mode				= USART_MODE_RX | USART_MODE_TX;

	/* Configure USART */
	USART_Init(USARTn, &USART_InitStructure);

	/* Enable USART Receive and Transmit interrupts */
	USART_ConfigInt(USARTn, USART_INT_RXDNE, ENABLE);
	USART_ConfigInt(USARTn, USART_INT_TXC, ENABLE);

	/* Enable the USART */
	USART_Enable(USARTn, ENABLE);
	
	/* Enable USARTy Half Duplex Mode*/
   USART_EnableHalfDuplex(USARTn, ENABLE);
}

void UartInit(void)
{  
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_USART3, ENABLE);
	UartOpen(USART3, 115200);
	UartRxEnable();
}

/*****************************************************************************
 函 数 名  : UartDeInit
 功能描述  : Uart恢复默认初始化值
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void UartDeInit(void)
{
	;
}

/*****************************************************************************
 函 数 名  : UartRxDisable
 功能描述  : 单线通讯时，RX禁止
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void UartRxDisable(void)
{
	USART_ConfigInt(USART3, USART_INT_RXDNE, DISABLE);
//	GPIOC->PMODE = ((GPIOC->PMODE) & (0xFFCFFFFF)) | (0x02 << (10*2));		// 切回复用功能
	USART_ClrIntPendingBit(USART3, USART_INT_TXC);
	USART_ConfigInt(USART3, USART_INT_TXC, ENABLE);
}

void UartRxEnable(void)
{
	USART_ConfigInt(USART3, USART_INT_RXDNE, ENABLE);
	USART_ConfigInt(USART3, USART_INT_TXC, DISABLE);

//	GPIOC->PMODE = ((GPIOC->PMODE) & (0xFFCFFFFF)) | (0x00 << (10*2));		// 切回通用输出
//	GPIO_SetBits(UART3_TX_Port,UART3_TX_Pin);
}

/*****************************************************************************
 函 数 名  : UartSendByte
 功能描述  : Uart 发送
 输入参数  : u8 tx_data  
 返 回 值  : 
*****************************************************************************/
void UartSendByte(u8 tx_data) 
{  
//	while (USART_GetFlagStatus(USART3, USART_FLAG_TXDE) != RESET);
	USART_SendData(USART3, tx_data);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXDE) == RESET);
}

/*****************************************************************************
 函 数 名  : UartSendByteIT
 功能描述  : Uart 发送 --> 不等待发送完成标志
 输入参数  : u8 tx_data  
 返 回 值  : 
*****************************************************************************/
void UartSendByteIT(u8 tx_data) 
{  
	USART_SendData(USART3, tx_data);
}

/*****************************************************************************
 函 数 名  : UartReceiveByte
 功能描述  : Uart 接收
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
u8 UartReceiveByte(void)
{
	return USART_ReceiveData(USART3);   
}

/*****************************************************************************
 函 数 名  : SpiInit
 功能描述  : Spi0初始化
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void SpiInit(void)
{
	SPI_InitType SPI_InitStructure;
	SPI_InitStruct(&SPI_InitStructure);
	
	RCC_EnableAPB2PeriphClk(AFE_SPI_CLK, ENABLE);
	SPI_Enable(AFE_SPI, DISABLE);

	SPI_InitStructure.DataDirection = SPI_DIR_DOUBLELINE_FULLDUPLEX;
	SPI_InitStructure.SpiMode		= SPI_MODE_MASTER;
	SPI_InitStructure.DataLen 	    = SPI_DATA_SIZE_8BITS;
	SPI_InitStructure.CLKPOL		= SPI_CLKPOL_HIGH;
	SPI_InitStructure.CLKPHA		= SPI_CLKPHA_SECOND_EDGE;	  
	SPI_InitStructure.NSS 		    = SPI_NSS_SOFT;	 
	SPI_InitStructure.BaudRatePres  = SPI_BR_PRESCALER_64;		// 32M/64=500K 
	SPI_InitStructure.FirstBit	    = SPI_FB_MSB;    
	SPI_InitStructure.CRCPoly	    = 7;    
	SPI_Init(AFE_SPI, &SPI_InitStructure);	
	SPI_Enable(AFE_SPI, ENABLE);
}

void SpiDeInit(void)
{
	SPI_Enable(AFE_SPI, DISABLE);
	RCC_EnableAPB2PeriphClk(AFE_SPI_CLK, DISABLE);
}

#define SPI_DELAY		2000
u8 SpiTransmitBuffer(SPI_Module* SpiDev, u8 NumSend, u8 *Sbuf,u8 NumRec, u8 *Rbuf)
{
	u8 i;
	u8 ret = 0;
	u16 cnt;
	
	SPI_NSS_SW_LOW(SpiDev);	
	/* SPI通讯优化新增代码 */
	if (SET == SPI_I2S_GetStatus(SpiDev, SPI_I2S_RNE_FLAG))
	{
		*Rbuf = SPI_I2S_ReceiveData(SpiDev);
	}
	for(i=0; i<NumSend; i++)
	{
		cnt = 0;
		do
		{	
			cnt++;
			ret = SPI_I2S_GetStatus(SpiDev, SPI_I2S_TE_FLAG);
		}
		while(ret==RESET && cnt<SPI_DELAY);
		SPI_I2S_TransmitData(SpiDev, *(Sbuf+i));

		cnt = 0;
		do
		{
			cnt++;
			ret = SPI_I2S_GetStatus(SpiDev, SPI_I2S_RNE_FLAG);
		}
		while(ret==RESET && cnt<SPI_DELAY);
		*(Rbuf+i) = SPI_I2S_ReceiveData(SpiDev);

		if ((*(Rbuf+i)!=*(Sbuf+i)) || (cnt==SPI_DELAY))
		{
			SPI_NSS_SW_HIGH(SpiDev);
			return 1;
		}
	}

	for(i=0; i<NumRec; i++)
	{
		cnt = 0;
		do
		{
			cnt++;
			ret = SPI_I2S_GetStatus(SpiDev, SPI_I2S_TE_FLAG);
		}
		while(ret==RESET && cnt<SPI_DELAY);
		SPI_I2S_TransmitData(SpiDev, 0xFF);

		cnt = 0;
		do
		{
			cnt++;
			ret = SPI_I2S_GetStatus(SpiDev, SPI_I2S_RNE_FLAG);
		}while(ret==RESET && cnt<SPI_DELAY);
		*(Rbuf+NumSend+i) = SPI_I2S_ReceiveData(SpiDev);
	}

	SPI_NSS_SW_HIGH(SpiDev);
	
	return 0;
}

#ifdef USE_MCU_RTC
void HwRtcInit(void)
{
	EXTI_InitType EXTI_InitStructure;
	NVIC_InitType NVIC_InitStructure;
	RTC_TimeType  RtcTimeInit;
	RTC_DateType  RtcDateInit;

	PWR_BackupAccessEnable(ENABLE);
	
	/* Enable the LSI OSC */
	RCC_EnableLsi(ENABLE);
	// RCC_ConfigLse(RCC_LSE_ENABLE);
	
	while (RCC_GetFlagStatus(RCC_LDCTRL_FLAG_LSERD) == RESET);
	RCC_ConfigRtcClk(RCC_RTCCLK_SRC_LSE);
	
	RCC_EnableRtcClk(ENABLE);

	/* Wait for RTC registers synchronization */
  	RTC_WaitForSynchro();

	RTC_EnableWakeUp(DISABLE);

	RTC_SetWakeUpCounter((1UL<<15)/4); 						/* LSE:32.768KHz(2^15)/4/x : 1S  */
	Delay10us(5);				
	RTC_ConfigWakeUpClock(RTC_WKUPCLK_RTCCLK_DIV4);
	
#if 1
	EXTI_ClrITPendBit(EXTI_LINE20);
	EXTI_InitStructure.EXTI_Line = EXTI_LINE20;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitPeripheral(&EXTI_InitStructure);
	/* Enable the RTC WakeUp Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel					 = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd				 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

	/* Enable the RTC Wakeup Interrupt */
	RTC_ConfigInt(RTC_INT_WUT, ENABLE);
	
	RTC_EnableWakeUp(ENABLE);

	RTC_GetDate(RTC_FORMAT_BIN, &RtcDateInit);
	if (RtcDateInit.Year < 24)
	{
		RtcDateInit.Year = 24;
		RtcDateInit.Month = 6;
		RtcDateInit.Date = 24;
		RtcDateInit.WeekDay = 1;
		
		RtcTimeInit.H12 = RTC_AM_H12;		// MCU RTC is default 24H format
		RtcTimeInit.Hours = 8;
		RtcTimeInit.Minutes = 30;
		RtcTimeInit.Seconds = 30;

		RTC_ConfigTime(RTC_FORMAT_BIN, &RtcTimeInit);
		RTC_SetDate(RTC_FORMAT_BIN, &RtcDateInit);
	}

	PWR_BackupAccessEnable(DISABLE);
}

void RtcTimeSet(RTC_TIME_S *RTC_Time)
{
	RTC_TimeType  RtcTimeInit;
	RTC_DateType  RtcDateInit;

	RTC_GetDate(RTC_FORMAT_BIN, &RtcDateInit);
	RTC_GetTime(RTC_FORMAT_BIN, &RtcTimeInit);

	if (RTC_Time->year >= 24)
	{
		RtcDateInit.Year = RTC_Time->year;
	}
	if (RTC_Time->month <= 12)
	{
		RtcDateInit.Month = RTC_Time->month;
	}
	if (RTC_Time->day <= 31)
	{
		RtcDateInit.Date = RTC_Time->day;
	}
	if (RTC_Time->week < 7)
	{
		RtcDateInit.WeekDay = RTC_Time->week;
	}
	if (RTC_Time->hour_12_24 == 1)
	{
		RtcTimeInit.Hours = RTC_Time->hour;
	}
	else
	{
		RtcTimeInit.Hours = RTC_Time->hour;
		if (RTC_Time->ampm == 1)
		{
			RtcTimeInit.Hours += 12;	
		}
	}
	if (RTC_Time->minute < 60)
	{
		RtcTimeInit.Minutes = RTC_Time->minute;
	}
	if (RTC_Time->second < 60)
	{
		RtcTimeInit.Seconds = RTC_Time->second;
	}
		
	RTC_ConfigTime(RTC_FORMAT_BIN, &RtcTimeInit);
	RTC_SetDate(RTC_FORMAT_BIN, &RtcDateInit);
}

void RtcTimeRead(RTC_TIME_S *RTC_Time)
{
	RTC_TimeType  RtcTimeInit;
	RTC_DateType  RtcDateInit;

	RTC_GetDate(RTC_FORMAT_BIN, &RtcDateInit);
	RTC_GetTime(RTC_FORMAT_BIN, &RtcTimeInit);

	RTC_Time->year = RtcDateInit.Year;
	RTC_Time->month = RtcDateInit.Month;
	RTC_Time->day = RtcDateInit.Date;
	RTC_Time->week = RtcDateInit.WeekDay;

	RTC_Time->hour_12_24 = 1;
	RTC_Time->ampm = (RtcTimeInit.H12>0) ? 1: 0;
	RTC_Time->hour = RtcTimeInit.Hours;
	RTC_Time->minute = RtcTimeInit.Minutes;
	RTC_Time->second = RtcTimeInit.Seconds;
}
#endif

/*****************************************************************************
 函 数 名  : HwIntDisable
 功能描述  : 中断关闭
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void HwIntDisable(void)
{
	// disable interrupts
	__disable_irq();
}

/*****************************************************************************
 函 数 名  : HwIntEnable
 功能描述  : 中断开启
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void HwIntEnable(void)
{
	__enable_irq();
}

/*****************************************************************************
 函 数 名  : HwMcuReset
 功能描述  : MCU软件复位
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void HwMcuReset(void)
{
	__disable_irq();
	NVIC_SystemReset();
}

/*****************************************************************************
 函 数 名  : IntPriorityInit
 功能描述  : 中断优先级设置
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void HwCompOpamInit(void)
{
#if 0
    COMP_InitType COMP_Initial;    /*Initial comp*/
    COMP_StructInit(&COMP_Initial);  

    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_COMP, ENABLE);
    COMP_SetRefScl(3, 1,  3, 1);			// enable Vref(6bit:64)

    COMP_Initial.InpSel     = COMP2_CTRL_INPSEL_PA11;  			// brush BC+  
    COMP_Initial.InmSel     = COMP2_CTRL_INMSEL_VREF_VC2;    
    COMP_Initial.SampWindow = 10;    
    COMP_Initial.Thresh     = 6;    
    COMP_Init(COMP2, &COMP_Initial);
    COMP_Initial.InpSel     = COMP1_CTRL_INPSEL_PB3;  			// brush BC+  
    COMP_Initial.InmSel     = COMP1_CTRL_INMSEL_VREF_VC1;    
    COMP_Initial.SampWindow = 10;    
    COMP_Initial.Thresh     = 6; 
    COMP_Init(COMP1, &COMP_Initial);
    COMP_SetIntEn(3);		// IT enable comp1/2
    COMP_Enable(COMP1, ENABLE);
    COMP_Enable(COMP2, ENABLE);
#endif

#if 1
	OPAMP_InitType OPAMP_Initial;    
    OPAMP_StructInit(&OPAMP_Initial);
    
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_OPAMP, ENABLE); 
    
    OPAMP_Initial.Mod   = OPAMP_CS_EXT_OPAMP;
//    OPAMP_Initial.Gain  = OPAMP_CS_PGA_GAIN_16;
//    OPAMP_Init(OPAMP1, &OPAMP_Initial);
    OPAMP_Init(OPAMP2, &OPAMP_Initial);
    
    OPAMP_SetVpSel(OPAMP2, OPAMP2_CS_VPSEL_PA7);
    OPAMP_SetVmSel(OPAMP2, OPAMP2_CS_VMSEL_PC5);
	OPAMP_SetVpSecondSel(OPAMP2, OPAMP2_CS_VPSSEL_NC);
	OPAMP_SetVmSecondSel(OPAMP2, OPAMP2_CS_VMSSEL_FLOAT);

    OPAMP_Enable(OPAMP2, ENABLE);
#endif
}

/*****************************************************************************
 函 数 名  : HwDataInit
 功能描述  : 数据记录初始化
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void HwDataInit(void)
{    

}

/*****************************************************************************
 函 数 名  : HwDataUnlock
 功能描述  : 写数据前需执行解锁
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void HwDataUnlock(void)
{ 
	__disable_irq();
	FLASH_Unlock();	
}

/*****************************************************************************
 函 数 名  : HwDataLock
 功能描述  : 写数据完成后需执行上锁保护
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void HwDataLock(void)
{ 
	FLASH_Lock();	
	__enable_irq();
}

u8 HwDataRead(u32 Addr, u32 *pData, u16 Len)
{
	u16 i;
	volatile u32* AddrPt;

	if ((Addr&0x0000003) != 0)
	{
		return ERROR;				// request address align 4
	}

	if ((Addr+Len*4) > FLASH_DATA_SIZE)
	{
		return ERROR;				// request data area right
	}

	AddrPt = (u32 *)(FLASH_DATA_BASE_ADDR + Addr);
	for (i=0; i<Len; i++)
	{
		*pData = *AddrPt;
		pData++;
		AddrPt++;
	}

	return SUCCESS;
}

u8 HwDataReadRandom(u32 Addr, u32 *pData, u16 Len)
{
	u16 i;
	volatile u32* AddrPt;

	if ((Addr&0x0000003) != 0)
	{
		return ERROR;				// request address align 4
	}

	AddrPt = (u32 *)Addr;

	for (i=0; i<Len; i++)
	{
		*pData = *AddrPt;
		pData++;
		AddrPt++;
	}

	return SUCCESS;
}

u8 HwDataPageWrite(u32 Addr, u32 *pData, u16 Len)
{
	u8 ret, trycnt;
	u16 i;	
	u32 ProAddr;

	/* check input info */
	if ((Addr&(FLASH_PAGE_SIZE-1)) != 0)
	{
		return ERROR;				// request address if page first address
	}

	if ((Addr+Len*4) > FLASH_DATA_SIZE)
	{
		return ERROR;				// request data area right
	}

	ProAddr = FLASH_DATA_BASE_ADDR + Addr;

	/* do erase */
	trycnt = 0;
	do
	{
		ret = FLASH_EraseOnePage(ProAddr);
	}
	while((ret!=FLASH_COMPL) && (trycnt++<5));
	
	if (ret != FLASH_COMPL)
		return ERROR;

	/* do program */
	for (i=0; i<Len; i++)
	{
		trycnt = 0;
		do
		{
			ret = FLASH_ProgramWord(ProAddr, *pData);
			
			__NOP();__NOP();__NOP();__NOP();
			
			if (*(__IO u32*)ProAddr != *pData)
			{
				ret = FLASH_ERR_EV;
			}
		}while((ret!=FLASH_COMPL) && (trycnt++<5));
		
		if (ret != FLASH_COMPL)
			return ERROR;
		
		ProAddr += 4;
		pData++;
	}
	
	return SUCCESS;
}

static u32 FlashPGBuf[FLASH_PAGE_SIZE/4];
u8 HwDataPageWriteRandom(u32 Addr, u32 *pData, u16 Len)
{
    u8 ret, trycnt;
    u16 i, num;	
    u32 ProAddr, PageAddr;

#if 0
    /* check input info */
    if ((Addr&(FLASH_PAGE_SIZE/2-1)) != 0)      // 首地址应为页起始地址or半页地址
    {
        return ERROR;				// request address if page first address
    }
#endif

#if 1
    if ((Len*4) > FLASH_PAGE_SIZE)			
    {
        return ERROR;				// request data area right
    }
#endif

#if 1
	/* check same data to return */
	ret = 0;
    for (i=0; i<Len; i++)
    {
        if (*(__IO u32*)(Addr+i*4) != *(pData+i))
        {
        	ret = 1;
            break;
        }
    }
    if (ret == 0)
    {
    	return SUCCESS;
    }
#endif

	/* update buf data */
	PageAddr = Addr&(~((u32)FLASH_PAGE_SIZE-1));
	for (i=0; i<FLASH_PAGE_SIZE/4; i++)
	{
		FlashPGBuf[i] = *(__IO u32*)(PageAddr+i*4);
	}
	i = (Addr-PageAddr)/4;
	num = i+Len;
	for (; i<num; i++)
	{
		FlashPGBuf[i] = *pData;
		pData++;
	}

    /* do erase */
    trycnt = 0;
    do
    {
        ret = FLASH_EraseOnePage(PageAddr);
    }
    while((ret!=FLASH_COMPL) && (trycnt++<5));
    
    if (ret != FLASH_COMPL)
		return ERROR;    

    /* do program */
    ProAddr = PageAddr;
    for (i=0; i<FLASH_PAGE_SIZE/4; i++)
    {
        trycnt = 0;
        do
        {
            ret = FLASH_ProgramWord(ProAddr, FlashPGBuf[i]);
            
            __NOP();__NOP();__NOP();__NOP();
            
            if (*(__IO u32*)ProAddr != FlashPGBuf[i])
            {
                ret = FLASH_ERR_EV;
            }
        }while((ret!=FLASH_COMPL) && (trycnt++<5));
        
        if (ret != FLASH_COMPL)
            return ERROR;
        
        ProAddr += 4;
    }
    
    return SUCCESS;
}

/*****************************************************************************
 函 数 名  : McuIntConfig
 功能描述  : 工作状态中断配置
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void McuIntConfig(FunctionalState Cmd)
{
	NVIC_InitType NVIC_InitStructure;   
#if 0
	EXTI_InitType EXTI_InitStructure;

    GPIO_ConfigEXTILine(DSG_SC_EXIT_PORT_SOURCE, DSG_SC_EXIT_PIN_SOURCE);
   
    
    /*Configure SC EXTI line*/
    EXTI_InitStructure.EXTI_Line    = DSG_SC_EXIT_LINE;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 	// EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

	/*set EXTI_LINE21-22 for compareIRQ */
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_Line = EXTI_LINE21;    	
    EXTI_InitPeripheral(&EXTI_InitStructure); 

    EXTI_ClrITPendBit(EXTI_LINE21);
#endif

#if 0		// replace with SysTick_Config()
    /*Set systick interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel                   = SysTick_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority           = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if 1
	NVIC_InitStructure.NVIC_IRQChannel					 = CAN_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd				 = Cmd;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel					 = CAN_RX1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd				 = Cmd;
	NVIC_Init(&NVIC_InitStructure);
#endif

	NVIC_InitStructure.NVIC_IRQChannel					 = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 0x2;
	NVIC_InitStructure.NVIC_IRQChannelCmd				 = Cmd;
	NVIC_Init(&NVIC_InitStructure);
}

void McuGpioLpConfig(FunctionalState cmdstate)
{
	GPIO_InitType GPIO_InitStructure;
	GPIO_InitStruct(&GPIO_InitStructure);

	if (cmdstate == ENABLE)
	{
		GPIO_ResetBits(CAN_RX_Port, CAN_RX_Pin);
		GPIO_ResetBits(CAN_TX_Port, CAN_TX_Pin);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	    GPIO_InitStructure.GPIO_Pull = GPIO_No_Pull;
	    GPIO_InitStructure.GPIO_Alternate = GPIO_NO_AF;
	    GPIO_InitStructure.Pin       = CAN_RX_Pin;
	    GPIO_InitPeripheral(CAN_RX_Port, &GPIO_InitStructure);
	    GPIO_InitStructure.Pin       = CAN_TX_Pin;
	    GPIO_InitPeripheral(CAN_TX_Port, &GPIO_InitStructure);
	#if 1
	    /* SWD interface set as IO */
	    GPIO_ResetBits(GPIOA, GPIO_PIN_13);
		GPIO_ResetBits(GPIOA, GPIO_PIN_14);
	    GPIO_InitStructure.Pin       = GPIO_PIN_13|GPIO_PIN_14;
	    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
	#endif
		#if 1
		GPIO_SetBits(AFE_SPI_Port, AFE_SPI_CLK_Pin);
		GPIO_ResetBits(AFE_SPI_Port, AFE_SPI_MOSI_Pin);
		GPIO_ResetBits(AFE_SPI_Port, AFE_SPI_MISO_Pin);
	    GPIO_InitStructure.Pin       = AFE_SPI_CLK_Pin|AFE_SPI_MOSI_Pin|AFE_SPI_MISO_Pin;
	    GPIO_InitPeripheral(AFE_SPI_Port, &GPIO_InitStructure);
	#endif

	}
	else
	{
		/* Configure CAN RX */
	    GPIO_InitStructure.Pin       = CAN_RX_Pin;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
	    GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;
	    GPIO_InitStructure.GPIO_Alternate = GPIO_AF1_CAN;
	    GPIO_InitPeripheral(CAN_RX_Port, &GPIO_InitStructure);
	    /* Configure CAN TX */
	    GPIO_InitStructure.Pin        = CAN_TX_Pin;
	    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	    GPIO_InitPeripheral(CAN_TX_Port, &GPIO_InitStructure);

	#if 1 //ndef SWD_DEBUG
		/* SWD interface recover */
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
	    GPIO_InitStructure.GPIO_Alternate = GPIO_AF0_SW_JTAG;
	    GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Up;	    
	    GPIO_InitStructure.Pin       = GPIO_PIN_13;
	    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
	    GPIO_InitStructure.GPIO_Pull = GPIO_Pull_Down;	    
	    GPIO_InitStructure.Pin       = GPIO_PIN_14;
	    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);
	#endif
	
		/* Configure SPI pins: SCK and MOSI as Alternate Function Push Pull */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.Pin        = AFE_SPI_CLK_Pin;    
		GPIO_InitStructure.GPIO_Alternate = AFE_SPI_CLK_AF;  
		GPIO_InitPeripheral(AFE_SPI_Port, &GPIO_InitStructure);
		GPIO_InitStructure.Pin        = AFE_SPI_MOSI_Pin;    
		GPIO_InitStructure.GPIO_Alternate = AFE_SPI_MOSI_AF;           
		GPIO_InitPeripheral(AFE_SPI_Port, &GPIO_InitStructure);
		GPIO_InitStructure.Pin = AFE_SPI_MISO_Pin;     
		GPIO_InitStructure.GPIO_Alternate = AFE_SPI_MISO_AF;
		GPIO_InitPeripheral(AFE_SPI_Port, &GPIO_InitStructure);
	}
}

void McuClkLpConfig(FunctionalState cmdstate)
{
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_USART3, cmdstate);
//	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_CAN, cmdstate);
	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_OPAMP, cmdstate); 
}

void McuWakeupConfig(FunctionalState cmdstate)
{
	/***********************************************
	1.AFE_ALNT_Pin(PB7) fall INT;
	2.R485_WKUP_Pin(PA8) fall INT;	
	3.KEY_EN_Pin(PA12) fall/rise INT;
	***********************************************/
	EXTI_InitType EXTI_InitStructure;
	NVIC_InitType NVIC_InitStructure;	

	/* AFE_ALNT_Pin(PC6) fall INT */
	EXTI_ClrITPendBit(AFE_ALNT_EXIT_LINE);
	GPIO_ConfigEXTILine(AFE_ALNT_EXIT_PORT_SOURCE, AFE_ALNT_EXIT_PIN_SOURCE); 
	EXTI_InitStructure.EXTI_Line    = AFE_ALNT_EXIT_LINE;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = AFE_ALNT_EXIT_TRIGGER; 
	EXTI_InitStructure.EXTI_LineCmd = cmdstate;
	EXTI_InitPeripheral(&EXTI_InitStructure);	
#if 0	
	/* CAN_WKUP_Pin(PA10) fall INT */
	EXTI_ClrITPendBit(CAN_WKUP_EXIT_LINE);
	GPIO_ConfigEXTILine(CAN_WKUP_EXIT_PORT_SOURCE, CAN_WKUP_EXIT_PIN_SOURCE); 
	EXTI_InitStructure.EXTI_Line    = CAN_WKUP_EXIT_LINE;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = CAN_WKUP_EXIT_TRIGGER; 
	EXTI_InitStructure.EXTI_LineCmd = cmdstate;
	EXTI_InitPeripheral(&EXTI_InitStructure);	
#endif
	/* DET_IN_Pin(PC8) fall/rise INT */
	EXTI_ClrITPendBit(DET_IN_EXIT_LINE);
	GPIO_ConfigEXTILine(DET_IN_EXIT_PORT_SOURCE, DET_IN_EXIT_PIN_SOURCE); 
	EXTI_InitStructure.EXTI_Line    = DET_IN_EXIT_LINE;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = DET_IN_EXIT_TRIGGER; 
	EXTI_InitStructure.EXTI_LineCmd = cmdstate;
	EXTI_InitPeripheral(&EXTI_InitStructure);
	/* CHG_IN_Pin(PB0) fall/rise INT */
	EXTI_ClrITPendBit(CHG_IN_EXIT_LINE);
	GPIO_ConfigEXTILine(CHG_IN_EXIT_PORT_SOURCE, CHG_IN_EXIT_PIN_SOURCE); 
	EXTI_InitStructure.EXTI_Line    = CHG_IN_EXIT_LINE;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = CHG_IN_EXIT_TRIGGER; 
	EXTI_InitStructure.EXTI_LineCmd = cmdstate;
	EXTI_InitPeripheral(&EXTI_InitStructure);

	/*Set AFE_ALNT_Pin interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel                   = AFE_ALNT_EXIT_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = cmdstate;
    NVIC_Init(&NVIC_InitStructure);
#if 0
    /*Set CAN_WKUP_Pin interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel                   = CAN_WKUP_EXIT_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = cmdstate;
    NVIC_Init(&NVIC_InitStructure);
#endif
    /*Set DET_IN_Pin interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel                   = DET_IN_EXIT_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = cmdstate;
    NVIC_Init(&NVIC_InitStructure);
    /*Set CHG_IN_Pin interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel                   = CHG_IN_EXIT_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = cmdstate;
    NVIC_Init(&NVIC_InitStructure);
}

/*****************************************************************************
 函 数 名  : HwSleepEnter
 功能描述  : 休眠前准备工作处理
 输入参数  : void
 返 回 值  :
*****************************************************************************/
void HwSleepEnter(void)
{
	McuIntConfig(DISABLE);	
	
	ADCDeInit();
	
	SpiDeInit();
	
//	CAN_DeInit(CAN);
//	OPAMP_DeInit();

//	McuGpioLpConfig(ENABLE);

//	McuClkLpConfig(DISABLE);
		
	McuWakeupConfig(ENABLE);

	SysTick_Set(SystemCoreClock/1000, DISABLE);
}

/*****************************************************************************
 函 数 名  : HwSleepExit
 功能描述  : 退出休眠
 输入参数  : void
 返 回 值  :
*****************************************************************************/
void HwSleepExit(void)
{	
	McuWakeupConfig(DISABLE);

//	McuGpioLpConfig(DISABLE);

	McuClkLpConfig(ENABLE);

	ADCInit();
	HwCompOpamInit();
	SpiInit();

	SysTick_Set(SystemCoreClock/1000, ENABLE);
	
	McuIntConfig(ENABLE);	
}

/*****************************************************************************
 函 数 名  : HwMcuInit
 功能描述  : MCU硬件模块初始化总入口
 输入参数  : void  
 返 回 值  : 
*****************************************************************************/
void HwMcuInit(void)
{
//	HwIntDisable();
	ClkInit();							
//	WdtInit();							
	HwDataInit();
	PortInit();	
	ADCInit();	
//	PwmInit();
	//SpiInit();
//	Timer0Init();
	I2CInit();
#ifdef BW_UART
	UartInit();
#endif
	//HwRtcInit();
	HwCompOpamInit();
	McuIntConfig(ENABLE);
//	HwIntEnable();	
}


#define SOFTWARE_IIC
#ifdef SOFTWARE_IIC

#define I2C0_SCL_PORT               TYPEC_SCL_Port
#define I2C0_SCL_PIN                TYPEC_SCL_Pin
#define I2C0_SDA_PORT               TYPEC_SDA_Port
#define I2C0_SDA_PIN                TYPEC_SDA_Pin

#define I2C1_SCL_PORT               AFE_SCL_Port
#define I2C1_SCL_PIN                AFE_SCL_Pin
#define I2C1_SDA_PORT               AFE_SDA_Port
#define I2C1_SDA_PIN                AFE_SDA_Pin

#define I2C_RX	1	 				/*!< Reception direction */
#define I2C_TX	0					/*!< Transmission direction */

u8 ack;


void SET_SCL_HIGH(u8 DevI2C)
{
	if (DevI2C == SW_I2C0)
	{
		GPIO_SetBits(I2C0_SCL_PORT, I2C0_SCL_PIN);
	}
	else if (DevI2C == SW_I2C1)
	{
		GPIO_SetBits(I2C1_SCL_PORT, I2C1_SCL_PIN);
	}
}
void SET_SCL_LOW(u8 DevI2C)
{
	if (DevI2C == SW_I2C0)
	{
		GPIO_ResetBits(I2C0_SCL_PORT, I2C0_SCL_PIN);
	}
	else if (DevI2C == SW_I2C1)
	{
		GPIO_ResetBits(I2C1_SCL_PORT, I2C1_SCL_PIN);
	}
}
void SET_SDA_HIGH(u8 DevI2C)
{
	if (DevI2C == SW_I2C0)
	{
		GPIO_SetBits(I2C0_SDA_PORT, I2C0_SDA_PIN);
	}
	else if (DevI2C == SW_I2C1)
	{
		GPIO_SetBits(I2C1_SDA_PORT, I2C1_SDA_PIN);
	}
}
void SET_SDA_LOW(u8 DevI2C)
{
	if (DevI2C == SW_I2C0)
	{
		GPIO_ResetBits(I2C0_SDA_PORT, I2C0_SDA_PIN);
	}
	else
	{
		GPIO_ResetBits(I2C1_SDA_PORT, I2C1_SDA_PIN);
	}
}
u8 GET_SCL(u8 DevI2C)
{
	if (DevI2C == SW_I2C0)
	{
		return GPIO_ReadInputDataBit(I2C0_SCL_PORT, I2C0_SCL_PIN);
	}
	else if (DevI2C == SW_I2C1)
	{
		return GPIO_ReadInputDataBit(I2C1_SCL_PORT, I2C1_SCL_PIN);
	}
	return 0;
}
u8 GET_SDA(u8 DevI2C)
{
	if (DevI2C == SW_I2C0)
	{
		return GPIO_ReadInputDataBit(I2C0_SDA_PORT, I2C0_SDA_PIN);
	}
	else if (DevI2C == SW_I2C1)
	{
		return GPIO_ReadInputDataBit(I2C1_SDA_PORT, I2C1_SDA_PIN);
	}
	return 0;
}
void SET_SCL_IN(u8 DevI2C)
{
	SET_SCL_HIGH(DevI2C);
}
void SET_SCL_OUT(u8 DevI2C)
{
}
void SET_SDA_IN(u8 DevI2C)
{
	SET_SDA_HIGH(DevI2C);
}
void SET_SDA_OUT(u8 DevI2C)
{
}


#define I2C_DELAY_UNIT		42

void I2C_Delay(u8 cnt)			//set I2C SCL speed 设置通信速度
{
    u16 i = cnt;

    while(i)
	{
        i--;
    }
}

/*****************************************************************************
 函 数 名  : I2C_SoftInit
 功能描述  : 模拟I2C I2C初始化

 输入参数  :
 返 回 值  :
*****************************************************************************/
void I2C_SoftInit(void)
{
	SET_SDA_HIGH(SW_I2C0);
	SET_SCL_HIGH(SW_I2C0);
	
	SET_SDA_HIGH(SW_I2C1);
	SET_SCL_HIGH(SW_I2C1);
}

/*****************************************************************************
 函 数 名  : I2C_SoftStart
 功能描述  : 模拟I2C起始
 			标准的I2C协议I2C起始条件：SCL为高时，SDA由高变低
 输入参数  :
 返 回 值  :
*****************************************************************************/
static void I2C_SoftStart(u8 DevI2C)
{
	SET_SDA_HIGH(DevI2C);
	SET_SDA_OUT(DevI2C);
	SET_SCL_HIGH(DevI2C);
	I2C_Delay(2*I2C_DELAY_UNIT);
	SET_SDA_LOW(DevI2C);
	//标准的I2C协议I2C起始条件：SCL为高时，SDA由高变低
	I2C_Delay(2*I2C_DELAY_UNIT);
	SET_SCL_LOW(DevI2C);//钳住I2C总线，准备发送或接收数据
}

/*****************************************************************************
 函 数 名  : I2C_SoftStop
 功能描述  : 模拟I2C停止
 			标准的I2C协议I2C停止条件：SCL为高时，SDA由低变高
 输入参数  :
 返 回 值  :
*****************************************************************************/
static void I2C_SoftStop(u8 DevI2C)
{
	SET_SCL_LOW(DevI2C);
	SET_SDA_LOW(DevI2C);
	SET_SDA_OUT(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SCL_HIGH(DevI2C);
	I2C_Delay(2*I2C_DELAY_UNIT);
	SET_SDA_HIGH(DevI2C);
	//标准的I2C协议I2C停止条件：SCL为高时，SDA由低变高
	I2C_Delay(2*I2C_DELAY_UNIT);
}

/*****************************************************************************
 函 数 名  : I2C_SoftAck
 功能描述  : 模拟I2C的应答
			根据标准的I2C协议，从I2C从器件读1个byte后
			也就是在第9个CLK的时候，I2C主设备的SDA要变低
			表示I2C主器件已经接收完一个字节byte，I2C从器件
			继续发送数据
 输入参数  :
 返 回 值  :
*****************************************************************************/
static void I2C_SoftAck(u8 DevI2C)
{
	SET_SCL_LOW(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SDA_LOW(DevI2C);
	SET_SDA_OUT(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SCL_HIGH(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SCL_LOW(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SDA_HIGH(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SCL_HIGH(DevI2C);
}

 /*****************************************************************************
 函 数 名  : I2C_SoftNack
 功能描述  : 模拟I2C的不应答
 输入参数  :
 返 回 值  :
*****************************************************************************/
static void I2C_SoftNack(u8 DevI2C)
{
	SET_SCL_LOW(DevI2C);
	SET_SDA_HIGH(DevI2C);
	SET_SDA_OUT(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SCL_HIGH(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SCL_LOW(DevI2C);
}

 /*****************************************************************************
 函 数 名  : I2C_SoftSend
 功能描述  : 模拟I2C主器件向从器件写一个字节数据
 输入参数  :
 			Byte: 需写入的单字节数据
 返 回 值  :
 			0: 写成功		1: 写失败
*****************************************************************************/
static u8 I2C_SoftSend(u8 DevI2C, u8 Byte)
{
	u8 bit;

	SET_SDA_OUT(DevI2C);
	SET_SCL_LOW(DevI2C);
	for (bit=0; bit<8; bit++)
	{
		if ((Byte&0x80)>>7)
		{
			SET_SDA_HIGH(DevI2C);
		}
		else
		{
			SET_SDA_LOW(DevI2C);
		}

		Byte<<=1;
		I2C_Delay(1*I2C_DELAY_UNIT/2);
		SET_SCL_HIGH(DevI2C);
		I2C_Delay(1*I2C_DELAY_UNIT);
		SET_SCL_LOW(DevI2C);
		I2C_Delay(1*I2C_DELAY_UNIT/2);
	}

  	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SDA_IN(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SCL_HIGH(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	if (0 != GET_SDA(DevI2C))
	{
		SET_SCL_HIGH(DevI2C);
		return 1;
	}
	else
	{
		SET_SCL_LOW(DevI2C);		//用ASK=1为有应答信号
		I2C_Delay(1*I2C_DELAY_UNIT);
		return 0;
	}
}

/*****************************************************************************
 函 数 名  : I2C_SoftRead
 功能描述  : 模拟I2C主器件在从器件读一个字节数据
 输入参数  :
 返 回 值  :
 			Receive_Byte: 主器件从从器件里读取的单字节数据
*****************************************************************************/
static u8 I2C_SoftRead(u8 DevI2C)
{
	u8 bit,Receive_Byte = 0;

	SET_SDA_IN(DevI2C);

	// 首Bit数据读取，需考虑SCL时否被释放(由低转高)
	SET_SCL_IN(DevI2C);
	for(u16 i=0; i<5000; i++)
	{
		if (0 != GET_SCL(DevI2C))
		{
			break;
		}
		else
		{
			I2C_Delay(1*I2C_DELAY_UNIT);
		}
	}
	SET_SCL_HIGH(DevI2C);
	I2C_Delay(1*I2C_DELAY_UNIT);
	SET_SCL_OUT(DevI2C);

	Receive_Byte <<= 1;//把上一时刻的值左移一位
	if(0 != GET_SDA(DevI2C))
	{
		Receive_Byte |= 0x01;
	}

	// 余下7个Bit数据读取
	for(bit=0; bit<7; bit++)
	{
		SET_SCL_LOW(DevI2C);
		I2C_Delay(2*I2C_DELAY_UNIT);
		SET_SCL_HIGH(DevI2C);
		Receive_Byte <<= 1;//把上一时刻的值左移一位
		if(0 != GET_SDA(DevI2C))
		{
			Receive_Byte |= 0x01;
		}
		I2C_Delay(1*I2C_DELAY_UNIT);
	}

	return Receive_Byte;
}

/*****************************************************************************
 函 数 名  : I2C_ReadBuffer
 功能描述  : I2C通用读取接口
 输入参数  :
 			addr: 从器件地址
	 		cmdCode: 命令符地址
	 		pBuf: 字节读取返回数组
	 		length:读取长度
 返 回 值  :
 			0: 成功		1: 失败
*****************************************************************************/
u8 I2C_ReadBuffer(u8 DevI2C, u8 addr, u8 cmdCode, u8 *pBuf, u8 length)
{
	u8 ret;

	I2C_SoftStart(DevI2C);
	ret = I2C_SoftSend(DevI2C, addr|I2C_TX);
	if (ret)
	{
		return 1;
	}

	ret = I2C_SoftSend(DevI2C, cmdCode);
	if (ret)
	{
		return 1;
	}

	I2C_SoftStart(DevI2C);
	ret = I2C_SoftSend(DevI2C, addr|I2C_RX);
	if (ret)
	{
		return 1;
	}

	while(length)
	{
		*pBuf++ = I2C_SoftRead(DevI2C);
		length-- ;
		if (length)
		{
			I2C_SoftAck(DevI2C);
		}
		else
		{
			I2C_SoftNack(DevI2C);
		}
	}

	I2C_SoftStop(DevI2C);
	return 0;
}

/*****************************************************************************
 函 数 名  : I2C_WriteBuffer
 功能描述  : I2C通用写接口
 输入参数  :
 			addr: 从器件地址
	 		cmdCode: 命令符地址
	 		*pBuf: 需写字节数组地址
	 		length: 数组长度
 返 回 值  :
 			0: 成功		1: 失败
*****************************************************************************/
u8 I2C_WriteBuffer(u8 DevI2C, u8 addr, u8 cmdCode, u8 *pBuf, u8 length)
{
	u8 ret;

	I2C_SoftStart(DevI2C);
	ret = I2C_SoftSend(DevI2C, addr|I2C_TX);
	if (ret)
	{
		return 1;
	}

	ret = I2C_SoftSend(DevI2C, cmdCode);
	if (ret)
	{
		return 1;
	}

	while(length)
	{
		ret = I2C_SoftSend(DevI2C, *pBuf++);
		if (ret)
		{
			return 1;
		}

		length-- ;
	}

	I2C_SoftStop(DevI2C);
	return 0;
}

/*****************************************************************************
 函 数 名  : I2CInit
 功能描述  : I2C初始化
 输入参数  : void
 返 回 值  :
*****************************************************************************/
void I2CInit(void)
{
	I2C_SoftInit();
}
void HwStopMode(void)
{
	PWR_EnterSTOP2Mode(PWR_STOPENTRY_WFI, PWR_CTRL3_RAM1RET|PWR_CTRL3_RAM2RET);
	__NOP(); __NOP(); __NOP(); __NOP(); __NOP();    			// 退出休眠时增加nop延时后再进行喂狗
}
#else

void I2CInit(void)
{
}

#endif

int fputc(int ch, FILE* f)
{
    USART_SendData(USART3, (uint8_t)ch);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXDE) == RESET)
        ;

    return (ch);
}

