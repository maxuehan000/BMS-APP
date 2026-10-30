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

#include "n32g45x_conf.h" 
#include "Sysdefs.h"
#include "user.h"
#include "McuHal_N32G45X.h"
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

#ifdef CAN_UPGRADE_BW
#include "CanUpgradeBW.h"
#endif

//#define USE_MCU_RTC
//#define USE_CHIP_RTC

#define I2C_AFE_INDEX			SW_I2C0

#define ADBIT                   4096
#define VDD_DESIGN_VOL          3300

#define PACK_VOL_GAIN			1027/27

#define BAT_VOL_GAIN			1027/27

#define OPA_CUR_GAIN			129/2	// =AD/4096*3300/25/0.5mR  #25倍 采样电阻:0.5mR

////////////////////////////////////////////////////////
#define CAN_TX1_Port			GPIOB
#define CAN_TX1_Pin				GPIO_PIN_9
#define CAN_RX1_Port			GPIOB
#define CAN_RX1_Pin				GPIO_PIN_8

#define CAN_TX2_Port			GPIOB
#define CAN_TX2_Pin				GPIO_PIN_13
#define CAN_RX2_Port			GPIOB
#define CAN_RX2_Pin				GPIO_PIN_12

////////////////////////////////////////////////////////
#define UART4_TX_Port			GPIOC
#define UART4_TX_Pin			GPIO_PIN_10

#define UART4_RX_Port			GPIOC
#define UART4_RX_Pin			GPIO_PIN_11

////////////////////////////////////////////////////////
#define AFE_SCL_Port			GPIOB
#define AFE_SCL_Pin				GPIO_PIN_10

#define AFE_SDA_Port			GPIOB
#define AFE_SDA_Pin				GPIO_PIN_11

#define AFE_INT_Port			GPIOC
#define AFE_INT_Pin				GPIO_PIN_8

////////////////////////////////////////////////////////
#define OVP_CK_Port				GPIOC
#define OVP_CK_Pin				GPIO_PIN_0

#define LED_PWM_Port			GPIOA
#define LED_PWM_Pin				GPIO_PIN_1

#define OPA_BIAS_Port			GPIOC
#define OPA_BIAS_Pin			GPIO_PIN_4

#define PRE_DIS_Port			GPIOB
#define PRE_DIS_Pin				GPIO_PIN_1

#define NTC_PWR_Port			GPIOC
#define NTC_PWR_Pin				GPIO_PIN_9

#define FUSE_BLOW_Port			GPIOA
#define FUSE_BLOW_Pin			GPIO_PIN_8

#define COM_EN_5V_Port			GPIOA
#define COM_EN_5V_Pin			GPIO_PIN_9

#define CAN_PWR_Port			GPIOA
#define CAN_PWR_Pin				GPIO_PIN_10

#define ECTRL_Port				GPIOC
#define ECTRL_Pin				GPIO_PIN_7
////////////////////////////////////////////////////////
#define CHG_OV2_Port					GPIOC
#define CHG_OV2_Pin						GPIO_PIN_0

#define CAN_IN_Port						GPIOC
#define CAN_IN_Pin						GPIO_PIN_2

#define VLOCK_IN_Port					GPIOA					// EN INT
#define VLOCK_IN_Pin					GPIO_PIN_11

#define AFE_ALTN_Port					GPIOC
#define AFE_ALTN_Pin					GPIO_PIN_8

#define KEY_ID1_Port					GPIOB
#define KEY_ID1_Pin						GPIO_PIN_7

#define KEY_ID2_Port					GPIOB
#define KEY_ID2_Pin						GPIO_PIN_6

////////////////////////////////////////////////////////
#define KEY_LED_PRESS()             	(GPIO_ReadInputDataBit(KEY_LED_Port, KEY_LED_Pin)==0)

#define VLOCK_IN()               		(GPIO_ReadInputDataBit(VLOCK_IN_Port, VLOCK_IN_Pin)==0)

#define CAN_IN()               			(GPIO_ReadInputDataBit(CAN_IN_Port, CAN_IN_Pin)==0)

#define AFE_ALTN_VALID()				(GPIO_ReadInputDataBit(AFE_ALTN_Port, AFE_ALTN_Pin)==0)

#define KEY_ID1_IN()					(GPIO_ReadInputDataBit(KEY_ID1_Port, KEY_ID1_Pin)==0)

#define KEY_ID2_IN()					(GPIO_ReadInputDataBit(KEY_ID2_Port, KEY_ID2_Pin)==0)

#define DET_IN()               			0
#define CHG_IN()               			0 
////////////////////////////////////////////////////////
#define DET_IN_POW_OFF()				{GPIO_ResetBits(DET_IN_PWR_Port, DET_IN_PWR_Pin);} 
#define DET_IN_POW_ON()					{GPIO_SetBits(DET_IN_PWR_Port, DET_IN_PWR_Pin);}

#define NTC_POW_OFF()					{GPIO_ResetBits(NTC_PWR_Port, NTC_PWR_Pin);} 
#define NTC_POW_ON()					{GPIO_SetBits(NTC_PWR_Port, NTC_PWR_Pin);}

#define OPA_BIAS_OFF()					{GPIO_ResetBits(OPA_BIAS_Port, OPA_BIAS_Pin);}
#define OPA_BIAS_ON()					{GPIO_SetBits(OPA_BIAS_Port, OPA_BIAS_Pin);} 

#define BAT_POW_OFF()					{GPIO_ResetBits(CAN_PWR_Port, CAN_PWR_Pin);}
#define BAT_POW_ON()					{GPIO_SetBits(CAN_PWR_Port, CAN_PWR_Pin);} 

#define CAN_5V_OFF()					{GPIO_ResetBits(COM_EN_5V_Port, COM_EN_5V_Pin);}
#define CAN_5V_ON()						{GPIO_SetBits(COM_EN_5V_Port, COM_EN_5V_Pin);} 

#define CAN_VIO_OFF()					
#define CAN_VIO_ON()					

#define CHG_MOS2_ON()					
#define CHG_MOS2_OFF()					

#define AFE_EFET_OFF()					{GPIO_ResetBits(ECTRL_Port, ECTRL_Pin);}
#define AFE_EFET_ON()					{GPIO_SetBits(ECTRL_Port, ECTRL_Pin);} 

#define FUSE_BLOW_OFF()                 {GPIO_ResetBits(FUSE_BLOW_Port, FUSE_BLOW_Pin);}
#define FUSE_BLOW_ON()                  {GPIO_SetBits(FUSE_BLOW_Port, FUSE_BLOW_Pin);}  
////////////////////////////////////////////////////////
#if 1
#define LED_1_ON()						0
#define LED_1_OFF()						0
#define LED_2_ON()						0
#define LED_2_OFF()						0
#define LED_3_ON()						0
#define LED_3_OFF()						0
#define LED_4_ON()						0
#define LED_4_OFF()						0
#define LED_5_ON()						0
#define LED_5_OFF()						0
#define LED_6_ON()						0
#define LED_6_OFF()						0
#define LED_7_ON()						0
#define LED_7_OFF()						0
#define LED_8_ON()						0
#define LED_8_OFF()						0
#endif

#define LED_T_ON()						0
#define LED_T_OFF()						0

#define TEST_PIN_ON()					0
#define TEST_PIN_OFF()					0

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

#define UartBmsSendByteIT(_data)		UartSendByteIT(UART4, _data)
#define UartBmsSendByte(_data)			UartSendByte(UART4,_data)

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

