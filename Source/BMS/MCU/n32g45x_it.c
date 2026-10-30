/*****************************************************************************
 * Copyright (c) 2019, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file n32l40x_it.c
 * @author Nations
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */

#include "n32g45x.h"
#include "n32g45x_it.h"
#include "board.h"
#include "McuHal_N32G45X.h"
#include "PowerCtrl.h"
#include "User.h"
#include "DataFlash.h"
#include "UartBW.h"
#include "RGB.h"

void FailSafePOR(void)
{
}

#define TICK_1S_CNT				1000
#define CAN_CNT_MAX_T1S			5000
#define RTC_CNT_MAX_T1S			6
#define UART_CNT_MAX_T1S		2000	// for 9600 baudrate
static volatile u32 Tick1msIntCnt = 0;
static volatile u32 CanIntCnt = 0;
static volatile u32 RtcIntCnt = 0;
static volatile u32 UartIntCnt = 0;


/** @addtogroup N32L40x_StdPeriph_Template
 * @{
 */

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 */
void NMI_Handler(void)
{
	#ifdef STL_VERBOSE
    printf("\n\r >>>>>>NMI_Handler\n\r");
	#endif	/* STL_VERBOSE */
	while(1)
	{
		FailSafePOR();
	}
}

/**
 * @brief  This function handles Hard Fault exception.
 */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	#ifdef STL_VERBOSE
    printf("\n\r >>>>>>HardFault_Handler\n\r");
    #endif  /* STL_VERBOSE */
	while(1)
	{
		FailSafePOR();
	}
}

/**
 * @brief  This function handles Memory Manage exception.
 */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	#ifdef STL_VERBOSE
    printf("\n\r >>>>>>MemManage_Handler\n\r");
	#endif	/* STL_VERBOSE */
	while(1)
	{
		FailSafePOR();
	}
}

/**
 * @brief  This function handles Bus Fault exception.
 */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	#ifdef STL_VERBOSE
    printf("\n\r >>>>>>BusFault_Handler\n\r");
	#endif	/* STL_VERBOSE */
	while(1)
	{
		FailSafePOR();
	}
}

/**
 * @brief  This function handles Usage Fault exception.
 */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	#ifdef STL_VERBOSE
    printf("\n\r >>>>>>UsageFault_Handler\n\r");
	#endif	/* STL_VERBOSE */
	while(1)
	{
		FailSafePOR();
	}
}

/**
 * @brief  This function handles SVCall exception.
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles Debug Monitor exception.
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 */
//extern volatile u8 SysTickTimer;
void SysTick_Handler(void)
{
	static u8 RgbTick10ms = 0;
	SysTickTimer++;
	if (++RgbTick10ms >= TIMEBASE_LOOP)   /* 10ms ¶¨Ê±Ë¢ÐÂ RGB */
	{
		RgbTick10ms = 0;
		RGBPwmUp();
	}
}

/**
 * @brief  This function handles DMA interrupt request defined in main.h .
 */
void DMA_IRQ_HANDLER(void)
{
}

void COMP_1_2_IRQHandler(void)
{
	if(RESET != COMP_GetIntStsOneComp(COMP2))
	{
//		ProtectShortCallback();
//		EXTI_ClrITPendBit(EXTI_LINE22);
	}
}

/**
 * @brief  This function handles USART3 global interrupt request.
 */
void USART3_IRQHandler(void)
{
	u8 rx_data;
	UartIntCnt++;
	if(USART_GetIntStatus(USART3, USART_INT_RXDNE) != RESET)
	{
		rx_data = USART_ReceiveData(USART3);
		UartReceive(rx_data);
	}
	if(USART_GetIntStatus(USART3, USART_INT_IDLEF) != RESET)
	{
		USART_ClrFlag(USART3, USART_FLAG_IDLEF);
	}
	if(USART_GetIntStatus(USART3, USART_INT_TXC) != RESET)
	{
		UartBmsSend();
		USART_ClrFlag(USART3, USART_FLAG_TXC);
	}
//	USART_ClrFlag(USART3, USART_FLAG_OREF);
//	USART_ClrFlag(USART3, USART_FLAG_NEF);
//	USART_ClrFlag(USART3, USART_FLAG_FEF);
//	USART_ClrFlag(USART3, USART_FLAG_PEF);
}

/**
 * @brief  This function handles UART4 global interrupt request.
 */
void UART4_IRQHandler(void)
{
	u8 rx_data;
	UartIntCnt++;
	if(USART_GetIntStatus(UART4, USART_INT_RXDNE) != RESET)
	{
		rx_data = USART_ReceiveData(UART4);
		UartReceive(rx_data);
	}
	if(USART_GetIntStatus(UART4, USART_INT_IDLEF) != RESET)
	{
		USART_ClrFlag(UART4, USART_FLAG_IDLEF);
	}
	if(USART_GetIntStatus(UART4, USART_INT_TXC) != RESET)
	{
		UartBmsSend();
		USART_ClrFlag(UART4, USART_FLAG_TXC);
	}
//	USART_ClrFlag(UART4, USART_FLAG_OREF);
//	USART_ClrFlag(UART4, USART_FLAG_NEF);
//	USART_ClrFlag(UART4, USART_FLAG_FEF);
//	USART_ClrFlag(UART4, USART_FLAG_PEF);
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{  
	CanIntCnt++;
	CAN_RX0_IRQHandler_CallBack();
}
void CAN1_RX1_IRQHandler(void)
{   
	CanIntCnt++;
	CAN_RX1_IRQHandler_CallBack();
}

void USB_HP_CAN1_TX_IRQHandler(void)
{
    /* CAN1 TX uses polling; clear TME flag as a defensive guard */
    CAN_ClearINTPendingBit(CAN1, CAN_INT_TME);
}

void CAN2_RX0_IRQHandler(void)
{
	CanRxMessage RxMsg;
	CanIntCnt++;
	CAN_ReceiveMessage(CAN2, CAN_FIFO0, &RxMsg);
	queue_push_back(MtRxQueue, &RxMsg);
}

void CAN2_RX1_IRQHandler(void)
{
	CanRxMessage RxMsg;
	CanIntCnt++;
	CAN_ReceiveMessage(CAN2, CAN_FIFO1, &RxMsg);
	queue_push_back(MtRxQueue, &RxMsg);
}

/**
 * @brief  This function handles ADC2.
 */
void ADC1_2_IRQHandler(void)
{
    /* Get adc2 converted value */
    if (ADC_GetIntStatus(ADC2, ADC_INT_ENDC))
    {
        ADC_ClearFlag(ADC2, ADC_FLAG_ENDC);        
    }
}

/**
 * @brief  This function handles ADC4.
 */
void ADC3_4_IRQHandler(void)
{
    /* Get adc4 converted value */
    if (ADC_GetIntStatus(ADC4, ADC_INT_ENDC))
    {
        ADC_ClearFlag(ADC4, ADC_FLAG_ENDC);        
    }
}

/**
 * @brief  This function handles EXTI0 global interrupts requests.
 */
void EXTI0_IRQHandler(void)
{
//	if(RESET != EXTI_GetITStatus(M4G_WKUP_EXIT_LINE))
//	{    	
//	//	Callback_EXTI1_IRQn();
//		PinWakeupFlag = 1;
//		EXTI_ClrITPendBit(M4G_WKUP_EXIT_LINE);  
//	}
}

/**
 * @brief  This function handles EXTI1 global interrupts requests.
 */
void EXTI1_IRQHandler(void)
{
//	if(RESET != EXTI_GetITStatus(CHG_IN_EXIT_LINE))
//	{    	
//		PinWakeupFlag = 1;
//		EXTI_ClrITPendBit(CHG_IN_EXIT_LINE);
//	}
}

/**
 * @brief  This function handles EXTI4 global interrupts requests.
 */
void EXTI4_IRQHandler(void)
{
//	if(RESET != EXTI_GetITStatus(PDSG_DOC_EXIT_LINE))
//	{    	
//		PinWakeupFlag = 1;
//		EXTI_ClrITPendBit(PDSG_DOC_EXIT_LINE);
//	}
}

/**
 * @brief  This function handles EXTI9-5 global interrupts requests.
 */
void EXTI9_5_IRQHandler(void)
{
//	if(RESET != EXTI_GetITStatus(PDSG_SC_EXIT_LINE))
//	{    	
//		PinWakeupFlag = 1;
//		EXTI_ClrITPendBit(PDSG_SC_EXIT_LINE);
//	}
}

/**
 * @brief  This function handles EXTI0 global interrupts requests.
 */
void EXTI15_10_IRQHandler(void)
{  
//	if(RESET != EXTI_GetITStatus(KEY_ON_EXIT_LINE))
//	{
//		PinWakeupFlag = 1;
//		EXTI_ClrITPendBit(KEY_ON_EXIT_LINE);  
//	}
//    
//	if(RESET != EXTI_GetITStatus(AFE_ALNT_EXIT_LINE))
//	{		
//		PinWakeupFlag = 1;
//		EXTI_ClrITPendBit(AFE_ALNT_EXIT_LINE);
//	}
//    
//	if(RESET != EXTI_GetITStatus(CAN_WKUP_EXIT_LINE))
//	{
//		PinWakeupFlag = 1;
//		EXTI_ClrITPendBit(CAN_WKUP_EXIT_LINE);
//	}
}

void RTC_WKUP_IRQHandler(void)
{
//	if (BatStatus.Bits.Sleep)
//	{
//		Tick1msIntCnt = 0;
//		CanIntCnt = 0;
//		RtcIntCnt = 0;
//		UartIntCnt = 0;
//	}
//	else
//	{
//		RtcIntCnt++;
//	}
//	if (RESET!=RTC_GetITStatus(RTC_INT_WUT) || RESET!=EXTI_GetITStatus(EXTI_LINE20))
//	{  	     
//		Rtc1STick = 1;
//	//	SYSTEM_TIME++;
//    	
//		RTC_ClrIntPendingBit(RTC_INT_WUT);
//		EXTI_ClrITPendBit(EXTI_LINE20);
//	}
}


