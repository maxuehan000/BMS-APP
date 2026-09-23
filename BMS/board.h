#ifndef __BOARD_H__
#define __BOARD_H__

#ifdef __MAIN_DEF__
#define MainDef 
#else
#define MainDef extern 
#endif
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#include "n32l40x_conf.h" 
#include "Sysdefs.h"
#include "user.h"
#include "McuHal_N32L40X.h"
#include "queue.h" 
//#include "uart.h"
#include "key.h"
#include "BmsCtrl.h"
//#include "W25X40.h"
//#include "es_oz77218.h"
#include "SoftWareCRC.h"  
#include "candrv.h" 
#include "multican.h"
#include "battercheck.h"
#include "discharge.h"
#include "charge.h"
#include "balance.h"
#include "PowerCtrl.h" 
#include "Calib.h"
#include "Interface.h" 
//#include "Rtc.h"
#ifdef AFE_MODEULE
#include "AfeModule.h"
#endif
#include "dataflash.h"
#include "soc.h"
//#include "BmsMisc.h"

#ifdef CAN_OTA_SL
#include "CanOtaSL.h"
#endif

#ifdef BOOTLOADER
#include "gotoboot.h"
#include "UpgradeCan.h"
#endif 
#ifdef CAN_SW_PROTOCOL
#include "SW_CAN_PROTOCOL.h"
#endif 
#ifdef CAN_AND_PROTOCOL
#include "AND_CAN_Protocol.h"
#endif 

//#include "EfLog.h" 

#include "CanBW.h" 
#include "UartBW.h"

#define CHG_NO_DIEDOE
#define BW_UART

#define USE_MCU_RTC
//#define USE_CHIP_RTC


#define I2C_AFE_INDEX			1

#define ADBIT                   4096
#define VDD_DESIGN_VOL          3300

#define PACK_VOL_GAIN			1027/27

#define CHARGER_VOL_GAIN		101
#define BAT_VOL_GAIN			1027/27

#define OPA_CUR_GAIN			129/2	// =AD/4096*3300/25/0.5mR  #25倍 采样电阻:0.5mR

#define ADC_NTC_NUM				3
#define AFE_NTC_NUM				1



// AD 通道定义
#define ADC_CELL_NTC1			ADC_CH_9
#define ADC_DMOS_NTC 			ADC_CH_10
#define ADC_CMOS_NTC			ADC_CH_5
#define ADC_DET_IN             	ADC_CH_3
//#define ADC_DET_IN2            ADC_CH_10
#define ADC_CHARGER             ADC_CH_11
#define ADC_BAT_VOL				ADC_CH_6
#define ADC_CCR_AD				ADC_CH_7

#define CELL_NTC1_Port			GPIOB
#define CELL_NTC1_Pin			GPIO_PIN_0
#define DMOS_NTC_Port			GPIOB
#define DMOS_NTC_Pin			GPIO_PIN_1
#define CMOS_NTC_Port			GPIOA
#define CMOS_NTC_Pin			GPIO_PIN_4
#define CAGR_VOL_Port			GPIOC
#define CAGR_VOL_Pin			GPIO_PIN_0
#define BAT_VOL_Port			GPIOA
#define BAT_VOL_Pin				GPIO_PIN_5
#define DET_IN_AD_Port				GPIOA
#define DET_IN_AD_Pin					GPIO_PIN_2
//#define DET_IN_AD2_Port					GPIOB
//#define DET_IN_AD2_Pin					GPIO_PIN_1
#define OPA_CUR_Port			GPIOA
#define OPA_CUR_Pin				GPIO_PIN_6

#define OPAMP_VP_Port			GPIOA
#define OPAMP_VP_Pin			GPIO_PIN_7
#define OPAMP_VN_Port			GPIOC
#define OPAMP_VN_Pin			GPIO_PIN_5


// IO 功能口定义
/*
#define UART1_TX_Port        			GPIOA
#define UART1_TX_Pin		 			GPIO_PIN_9
#define UART1_TX_AF			 			GPIO_AF4_USART1
#define UART1_RX_Port        			GPIOA
#define UART1_RX_Pin         			GPIO_PIN_10
#define UART1_RX_AF			 			GPIO_AF4_USART1
*/
#define UART3_TX_Port        			GPIOC
#define UART3_TX_Pin		 			GPIO_PIN_10
#define UART3_TX_AF			 			GPIO_AF5_USART3
//#define UART3_RX_Port        			GPIOC
//#define UART3_RX_Pin         			GPIO_PIN_11
//#define UART3_RX_AF			 			GPIO_AF5_USART3
/*
#define RTC_SCL_Port					GPIOC
#define RTC_SCL_Pin						GPIO_PIN_0
#define RTC_SDA_Port					GPIOC
#define RTC_SDA_Pin						GPIO_PIN_1
#define RTC_INT_Port					GPIOC
#define RTC_INT_Pin						GPIO_PIN_2
*/
#define AFE_EFET_Port					GPIOC
#define AFE_EFET_Pin					GPIO_PIN_7
#define AFE_ALTN_Port					GPIOC
#define AFE_ALTN_Pin					GPIO_PIN_8
//#define AFE_RST_Port					GPIOC
//#define AFE_RST_Pin					    GPIO_PIN_6
#define AFE_SPI_CS_Port					GPIOB
#define AFE_SPI_CS_Pin					GPIO_PIN_6
#define AFE_SPI_CLK_Pin					GPIO_PIN_3
#define AFE_SPI_MISO_Pin				GPIO_PIN_4
#define AFE_SPI_MOSI_Pin				GPIO_PIN_5
#define AFE_SPI_Port						GPIOB
#define AFE_SPI_CLK_AF					GPIO_AF0_SPI2
#define AFE_SPI_MISO_AF					GPIO_AF0_SPI2
#define AFE_SPI_MOSI_AF					GPIO_AF0_SPI2

#define FUSE_BLOW_Port					GPIOA
#define FUSE_BLOW_Pin						GPIO_PIN_8

/*
#define W25_SPI_CS_Port					GPIOB
#define W25_SPI_CS_Pin					GPIO_PIN_6
#define W25_SPI_CLK_Pin					GPIO_PIN_3
#define W25_SPI_MISO_Pin				GPIO_PIN_4
#define W25_SPI_MOSI_Pin				GPIO_PIN_5
#define W25_SPI_Port					GPIOB
#define W25_SPI_CLK_AF					GPIO_AF1_SPI1
#define W25_SPI_MISO_AF					GPIO_AF1_SPI1
#define W25_SPI_MOSI_AF					GPIO_AF0_SPI1
#define W25_WRITE_Port					GPIOB
#define W25_WRITE_Pin					GPIO_PIN_7
*/
/*
#define CP_EN_Port						GPIOB
#define CP_EN_Pin						GPIO_PIN_8

#define CHG_MCU_Port					GPIOB
#define CHG_MCU_Pin						GPIO_PIN_9
*/
// 5V for CAN periph

#define PWR_5V_Port						GPIOA
#define PWR_5V_Pin						GPIO_PIN_10

#define EN_5V_Port						GPIOA
#define EN_5V_Pin							GPIO_PIN_9
//#define CAN_VIO_Port					GPIOA
//#define CAN_VIO_Pin						GPIO_PIN_8
#define CAN_RX_Port						GPIOA
#define CAN_RX_Pin						GPIO_PIN_11
#define CAN_TX_Port						GPIOA
#define CAN_TX_Pin						GPIO_PIN_12
//#define ECTRL_Port						GPIOC
//#define ECTRL_Pin							GPIO_PIN_7
//#define TYPEC_EN_Port					GPIOB
//#define TYPEC_EN_Pin					GPIO_PIN_2
#define TYPEC_SCL_Port					GPIOB
#define TYPEC_SCL_Pin					GPIO_PIN_10
#define TYPEC_SDA_Port					GPIOB
#define TYPEC_SDA_Pin					GPIO_PIN_11

//#define RTC_SCL_Port					GPIOC
//#define RTC_SCL_Pin						GPIO_PIN_0
//#define RTC_SDA_Port					GPIOC
//#define RTC_SDA_Pin						GPIO_PIN_1
//#define RTC_INT_Port					GPIOC
//#define RTC_INT_Pin						GPIO_PIN_2
#define AFE_SCL_Port					GPIOB
#define AFE_SCL_Pin						GPIO_PIN_10
#define AFE_SDA_Port					GPIOB
#define AFE_SDA_Pin						GPIO_PIN_11
#define AFE_INT_Port					GPIOC
#define AFE_INT_Pin						GPIO_PIN_8

#define LED8_CTRL_Port					GPIOB
#define LED8_CTRL_Pin					GPIO_PIN_15
#define LED7_CTRL_Port					GPIOB
#define LED7_CTRL_Pin					GPIO_PIN_13
#define LED6_CTRL_Port					GPIOB
#define LED6_CTRL_Pin					GPIO_PIN_12
#define LED5_CTRL_Port					GPIOA
#define LED5_CTRL_Pin					GPIO_PIN_3
#define LED4_CTRL_Port					GPIOA
#define LED4_CTRL_Pin					GPIO_PIN_1
#define LED3_CTRL_Port					GPIOA
#define LED3_CTRL_Pin					GPIO_PIN_0
#define LED2_CTRL_Port					GPIOC
#define LED2_CTRL_Pin					GPIO_PIN_3
#define LED1_CTRL_Port					GPIOC
#define LED1_CTRL_Pin					GPIO_PIN_2
//#define LEDT_CTRL_Port					GPIOB
//#define LEDT_CTRL_Pin					GPIO_PIN_5
//#define LEDRG_CTRL_Port					GPIOA
//#define LEDRG_CTRL_Pin					GPIO_PIN_3


#define NTC_PWR_Port					GPIOC
#define NTC_PWR_Pin						GPIO_PIN_9
#define BAT_PWR_Port					GPIOC
#define BAT_PWR_Pin						GPIO_PIN_11

#define OPA_BIAS_Port					GPIOC
#define OPA_BIAS_Pin					GPIO_PIN_4

//#define CHG_IN_Port        				GPIOB
//#define CHG_IN_Pin		 				GPIO_PIN_0
#define KEY_LED_Port					GPIOC
#define KEY_LED_Pin						GPIO_PIN_6
#define VLOCK_IN_Port					GPIOB
#define VLOCK_IN_Pin					GPIO_PIN_9
#define DET_IN_PWR_Port					GPIOB
#define DET_IN_PWR_Pin					GPIO_PIN_14



#define CAN_IN_Port						GPIOC
#define CAN_IN_Pin						GPIO_PIN_1

#define CHG_OV2_Port					GPIOC
#define CHG_OV2_Pin						GPIO_PIN_15

#define KEY_LED_PRESS()             	(GPIO_ReadInputDataBit(KEY_LED_Port, KEY_LED_Pin)==0)
// #define DET_IN()               			(GPIO_ReadInputDataBit(DET_IN_Port, DET_IN_Pin)==0)
#define DET_IN()               			0
#define VLOCK_IN()               		(GPIO_ReadInputDataBit(VLOCK_IN_Port, VLOCK_IN_Pin)==0)
#define CHG_IN()               			0 //(GPIO_ReadInputDataBit(CHG_IN_Port, CHG_IN_Pin)==0)
#define CAN_IN()               			(GPIO_ReadInputDataBit(CAN_IN_Port, CAN_IN_Pin)==0)

//#define TYPEC_SDA_LEVEL()				(GPIO_ReadInputDataBit(TYPEC_SDA_Port, TYPEC_SDA_Pin))
//#define TYPEC_SCL_LEVEL()				(GPIO_ReadInputDataBit(TYPEC_SCL_Port, TYPEC_SCL_Pin))

#define AFE_ALTN_VALID()				(GPIO_ReadInputDataBit(AFE_ALTN_Port, AFE_ALTN_Pin)==0)
#define AFE_RST_ON()					{GPIO_SetBits(AFE_RST_Port, AFE_RST_Pin);}
#define AFE_RST_OFF()					{GPIO_ResetBits(AFE_RST_Port, AFE_RST_Pin);}
#define AFE_EFET_ON()					{GPIO_SetBits(AFE_EFET_Port, AFE_EFET_Pin);}
#define AFE_EFET_OFF()					{GPIO_ResetBits(AFE_EFET_Port, AFE_EFET_Pin);}
#define AFE_SPI_NSS()					{GPIO_ResetBits(AFE_SPI_CS_Port, AFE_SPI_CS_Pin);}
#define AFE_SPI_PUS()					{GPIO_SetBits(AFE_SPI_CS_Port, AFE_SPI_CS_Pin);}

/*
#define W25_SPI_NSS()					{GPIO_ResetBits(W25_SPI_CS_Port, W25_SPI_CS_Pin);}
#define W25_SPI_PUS()					{GPIO_SetBits(W25_SPI_CS_Port, W25_SPI_CS_Pin);}
#define W25X40_HW_UNLOCK()				{GPIO_SetBits(W25_WRITE_Port, W25_WRITE_Pin);}
#define W25X40_HW_LOCK()				{GPIO_ResetBits(W25_WRITE_Port, W25_WRITE_Pin);}

#define FET_PUMP_ON()					{GPIO_SetBits(CP_EN_Port, CP_EN_Pin);}
#define FET_PUMP_OFF()					{GPIO_ResetBits(CP_EN_Port, CP_EN_Pin);}

#define CHG_MCU_OFF()                   {GPIO_ResetBits(CHG_MCU_Port, CHG_MCU_Pin);}
#define CHG_MCU_ON()                    {GPIO_SetBits(CHG_MCU_Port, CHG_MCU_Pin);} 
*/
#define CHG_MOS2_ON()
#define CHG_MOS2_OFF()


#define DET_IN_POW_OFF()				{GPIO_ResetBits(DET_IN_PWR_Port, DET_IN_PWR_Pin);} 
#define DET_IN_POW_ON()					{GPIO_SetBits(DET_IN_PWR_Port, DET_IN_PWR_Pin);}

#define NTC_POW_OFF()					{GPIO_ResetBits(NTC_PWR_Port, NTC_PWR_Pin);} 
#define NTC_POW_ON()					{GPIO_SetBits(NTC_PWR_Port, NTC_PWR_Pin);}
#define BAT_POW_OFF()					{GPIO_ResetBits(BAT_PWR_Port, BAT_PWR_Pin);} 
#define BAT_POW_ON()					{GPIO_SetBits(BAT_PWR_Port, BAT_PWR_Pin);}

#define OPA_BIAS_OFF()					{GPIO_ResetBits(OPA_BIAS_Port, OPA_BIAS_Pin);}
#define OPA_BIAS_ON()					{GPIO_SetBits(OPA_BIAS_Port, OPA_BIAS_Pin);} 

#define PWR_5V_OFF()					{GPIO_ResetBits(PWR_5V_Port, PWR_5V_Pin);}
#define PWR_5V_ON()						{GPIO_SetBits(PWR_5V_Port, PWR_5V_Pin);} 
#define CAN_5V_OFF()					{GPIO_ResetBits(EN_5V_Port, EN_5V_Pin);}
#define CAN_5V_ON()						{GPIO_SetBits(EN_5V_Port, EN_5V_Pin);} 
#define CAN_VIO_OFF()					 //{GPIO_ResetBits(CAN_VIO_Port, CAN_VIO_Pin);}
#define CAN_VIO_ON()					 //{GPIO_SetBits(CAN_VIO_Port, CAN_VIO_Pin);} 


//#define ECTRL_OFF()						{GPIO_ResetBits(ECTRL_Port, ECTRL_Pin);}
//#define ECTRL_ON()						{GPIO_SetBits(ECTRL_Port, ECTRL_Pin);} 
#define FUSE_BLOW_OFF()                 {GPIO_ResetBits(FUSE_BLOW_Port, FUSE_BLOW_Pin);}
#define FUSE_BLOW_ON()                  {GPIO_SetBits(FUSE_BLOW_Port, FUSE_BLOW_Pin);}  

#if 1
#define LED_1_ON()						{GPIO_SetBits(LED1_CTRL_Port, LED1_CTRL_Pin);}
#define LED_1_OFF()						{GPIO_ResetBits(LED1_CTRL_Port, LED1_CTRL_Pin);}
#define LED_2_ON()						{GPIO_SetBits(LED2_CTRL_Port, LED2_CTRL_Pin);}
#define LED_2_OFF()						{GPIO_ResetBits(LED2_CTRL_Port, LED2_CTRL_Pin);}
#define LED_3_ON()						{GPIO_SetBits(LED3_CTRL_Port, LED3_CTRL_Pin);}
#define LED_3_OFF()						{GPIO_ResetBits(LED3_CTRL_Port, LED3_CTRL_Pin);}
#define LED_4_ON()						{GPIO_SetBits(LED4_CTRL_Port, LED4_CTRL_Pin);}
#define LED_4_OFF()						{GPIO_ResetBits(LED4_CTRL_Port, LED4_CTRL_Pin);}
#define LED_5_ON()						{GPIO_SetBits(LED5_CTRL_Port, LED5_CTRL_Pin);}
#define LED_5_OFF()						{GPIO_ResetBits(LED5_CTRL_Port, LED5_CTRL_Pin);}
#define LED_6_ON()						{GPIO_SetBits(LED6_CTRL_Port, LED6_CTRL_Pin);}
#define LED_6_OFF()						{GPIO_ResetBits(LED6_CTRL_Port, LED6_CTRL_Pin);}
#define LED_7_ON()						{GPIO_SetBits(LED7_CTRL_Port, LED7_CTRL_Pin);}
#define LED_7_OFF()						{GPIO_ResetBits(LED7_CTRL_Port, LED7_CTRL_Pin);}
#define LED_8_ON()						{GPIO_SetBits(LED8_CTRL_Port, LED8_CTRL_Pin);}
#define LED_8_OFF()						{GPIO_ResetBits(LED8_CTRL_Port, LED8_CTRL_Pin);}
#endif

// LED顺序临时调试:
#if 0
#define LED_1_ON()						{GPIO_SetBits(LED4_CTRL_Port, LED4_CTRL_Pin);}
#define LED_1_OFF()						{GPIO_ResetBits(LED4_CTRL_Port, LED4_CTRL_Pin);}
#define LED_2_ON()						{GPIO_SetBits(LED3_CTRL_Port, LED3_CTRL_Pin);}
#define LED_2_OFF()						{GPIO_ResetBits(LED3_CTRL_Port, LED3_CTRL_Pin);}
#define LED_3_ON()						{GPIO_SetBits(LED2_CTRL_Port, LED2_CTRL_Pin);}
#define LED_3_OFF()						{GPIO_ResetBits(LED2_CTRL_Port, LED2_CTRL_Pin);}
#define LED_4_ON()						{GPIO_SetBits(LED1_CTRL_Port, LED1_CTRL_Pin);}
#define LED_4_OFF()						{GPIO_ResetBits(LED1_CTRL_Port, LED1_CTRL_Pin);}
#define LED_5_ON()						{GPIO_SetBits(LED5_CTRL_Port, LED5_CTRL_Pin);}
#define LED_5_OFF()						{GPIO_ResetBits(LED5_CTRL_Port, LED5_CTRL_Pin);}
#endif


#define LED_T_ON()						//{GPIO_SetBits(LEDT_CTRL_Port, LEDT_CTRL_Pin);}
#define LED_T_OFF()						//{GPIO_ResetBits(LEDT_CTRL_Port, LEDT_CTRL_Pin);}

#define TEST_PIN_ON()					{GPIO_SetBits(GPIOA, GPIO_PIN_5);}
#define TEST_PIN_OFF()					{GPIO_ResetBits(GPIOA, GPIO_PIN_5);}

#define AFE_ALNT_EXIT_PORT_SOURCE		GPIOC_PORT_SOURCE
#define AFE_ALNT_EXIT_PIN_SOURCE		GPIO_PIN_SOURCE6
#define AFE_ALNT_EXIT_LINE				EXTI_LINE6
#define AFE_ALNT_EXIT_TRIGGER			EXTI_Trigger_Falling
#define AFE_ALNT_EXIT_IRQn				EXTI9_5_IRQn

#define DET_IN_EXIT_PORT_SOURCE			GPIOC_PORT_SOURCE
#define DET_IN_EXIT_PIN_SOURCE			GPIO_PIN_SOURCE8
#define DET_IN_EXIT_LINE				EXTI_LINE8
#define DET_IN_EXIT_TRIGGER				EXTI_Trigger_Rising_Falling
#define DET_IN_EXIT_IRQn				EXTI9_5_IRQn

#define CHG_IN_EXIT_PORT_SOURCE			GPIOB_PORT_SOURCE
#define CHG_IN_EXIT_PIN_SOURCE			GPIO_PIN_SOURCE0
#define CHG_IN_EXIT_LINE				EXTI_LINE0
#define CHG_IN_EXIT_TRIGGER				EXTI_Trigger_Rising_Falling
#define CHG_IN_EXIT_IRQn				EXTI0_IRQn

#define CAN_WKUP_EXIT_PORT_SOURCE		GPIOA_PORT_SOURCE
#define CAN_WKUP_EXIT_PIN_SOURCE		GPIO_PIN_SOURCE10
#define CAN_WKUP_EXIT_LINE				EXTI_LINE10
#define CAN_WKUP_EXIT_TRIGGER			EXTI_Trigger_Rising_Falling
#define CAN_WKUP_EXIT_IRQn				EXTI15_10_IRQn


#define AFE_SPI							SPI2
#define AFE_SPI_CLK						RCC_APB2_PERIPH_SPI2

#define W25_SPI							SPI1
#define W25_SPI_CLK						RCC_APB2_PERIPH_SPI1

#define SPI_NSS_SW_LOW(spi)				{AFE_SPI_NSS();}//{if (spi==SPI1) W25_SPI_NSS(); if (spi==SPI2) AFE_SPI_NSS();}
#define SPI_NSS_SW_HIGH(spi)			{AFE_SPI_PUS();}//{if (spi==SPI1) W25_SPI_PUS(); if (spi==SPI2) AFE_SPI_PUS();}


#define TEST_A13_ON()					GPIO_SetBits(GPIOA, GPIO_PIN_13);
#define TEST_A13_OFF()					GPIO_ResetBits(GPIOA, GPIO_PIN_13);
#define TEST_A14_ON()					GPIO_SetBits(GPIOA, GPIO_PIN_14);
#define TEST_A14_OFF()					GPIO_ResetBits(GPIOA, GPIO_PIN_14);

#define UartBmsSendByteIT(_data)		UartSendByteIT(_data)
#define UartBmsSendByte(_data)			UartSendByte(_data)
#define UartBmsRxEnable()				UartRxEnable()
#define UartBmsRxDisable()				UartRxDisable()


#define TMP_0C_01K					2731

#define BAT_NTC_AD(R)                                ((u16)((unsigned long long int)R*(u32)ADBIT/(unsigned long long int)(R+4700)))

#define BAT_NTC_P120                                BAT_NTC_AD(598)                                        // 120
#define BAT_NTC_P110                                BAT_NTC_AD(759)                                        // 110
#define BAT_NTC_P100                                BAT_NTC_AD(976)                                        // 100
#define BAT_NTC_P90                                 BAT_NTC_AD(1270)                                        // 90
#define BAT_NTC_P80                                 BAT_NTC_AD(1673)                                // 80
#define BAT_NTC_P70                                 BAT_NTC_AD(2233)                                // 70
#define BAT_NTC_P60                                 BAT_NTC_AD(3024)                                // 60
#define BAT_NTC_P50                                 BAT_NTC_AD(4160)                                // 50
#define BAT_NTC_P40                                 BAT_NTC_AD(5824)                                // 40
#define BAT_NTC_P30                                 BAT_NTC_AD(8309)                                // 30
#define BAT_NTC_P20                                 BAT_NTC_AD(12099)                                // 20
#define BAT_NTC_P10                                 BAT_NTC_AD(18010)                                // 10
#define BAT_NTC_P0                                  BAT_NTC_AD(27445)                                // 0
#define BAT_NTC_N10                                 BAT_NTC_AD(42889)                                // -10
#define BAT_NTC_N20                                 BAT_NTC_AD(68915)                                // -20
#define BAT_NTC_N30                                 BAT_NTC_AD(114340)                                // -30
#define BAT_NTC_N40                                 BAT_NTC_AD(197390)                                // -40



#define MOS_NTC_AD(R) 				((u16)((unsigned long long int)R*(u32)ADBIT/(unsigned long long int)(R+4700)))
#define MOS_NTC_P120				MOS_NTC_AD(601)					// 120
#define MOS_NTC_P110				MOS_NTC_AD(762)					// 110
#define MOS_NTC_P100				MOS_NTC_AD(977)					// 100
#define MOS_NTC_P90					MOS_NTC_AD(1268)				// 90
#define MOS_NTC_P80					MOS_NTC_AD(1666)				// 80
#define MOS_NTC_P70					MOS_NTC_AD(2221)				// 70
#define MOS_NTC_P60					MOS_NTC_AD(3007)				// 60
#define MOS_NTC_P50					MOS_NTC_AD(4138)				// 50
#define MOS_NTC_P40					MOS_NTC_AD(5800)				// 40
#define MOS_NTC_P30					MOS_NTC_AD(8294)				// 30
#define MOS_NTC_P20					MOS_NTC_AD(12126)				// 20
#define MOS_NTC_P10					MOS_NTC_AD(18166)				// 10
#define MOS_NTC_P0					MOS_NTC_AD(27965)				// 0
#define MOS_NTC_N10 				MOS_NTC_AD(44369)				// -10
#define MOS_NTC_N20 				MOS_NTC_AD(72818)				// -20
#define MOS_NTC_N30 				MOS_NTC_AD(124135)				// -30
#define MOS_NTC_N40 				MOS_NTC_AD(220888)				// -40


#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */

