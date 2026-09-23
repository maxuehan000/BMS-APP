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

#include "n32l40x.h"
#include "N32L40x_it.h"
#include "board.h"
//#include "UartBW.h"
//#include "CanBW.h"
//#include "SysDefs.h"
//#include "gotoboot.h"

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
}

/**
 * @brief  This function handles Hard Fault exception.
 */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while(1)
	{
	}
}

/**
 * @brief  This function handles Memory Manage exception.
 */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while(1)
	{
	}
}

/**
 * @brief  This function handles Bus Fault exception.
 */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while(1)
	{
	}
}

/**
 * @brief  This function handles Usage Fault exception.
 */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while(1)
	{
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
extern volatile u8 SysTickTimer;
void SysTick_Handler(void)
{
	SysTickTimer++;
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
		EXTI_ClrITPendBit(EXTI_LINE22);
	}
}

/**
 * @brief  This function handles USART1 global interrupt request.
 */
void USART1_IRQHandler(void)
{
	if(USART_GetIntStatus(USART1, USART_INT_RXDNE) != RESET)
	{
//		UartReceive();
	}
	if(USART_GetIntStatus(USART1, USART_INT_IDLEF) != RESET)
	{
		USART_ClrFlag(USART1, USART_FLAG_IDLEF);
	}
	if(USART_GetIntStatus(USART1, USART_INT_TXC) != RESET)
	{
//		UartSend();
		USART_ClrFlag(USART1, USART_FLAG_TXC);
	}
}

/**
 * @brief  This function handles USART2 global interrupt request.
 */
void USART2_IRQHandler(void)
{
	if(USART_GetIntStatus(USART2, USART_INT_RXDNE) != RESET)
	{
//		InvRxCallback();
	}
	if(USART_GetIntStatus(USART2, USART_INT_IDLEF) != RESET)
	{
		USART_ClrFlag(USART2, USART_FLAG_IDLEF);
	}
	if(USART_GetIntStatus(USART2, USART_INT_TXC) != RESET)
	{
//		InvTxCallback();
		USART_ClrFlag(USART2, USART_FLAG_TXC);
	}
}

void USART3_IRQHandler(void)
{	
	u8 rxdata;
	
	if(USART_GetIntStatus(USART3, USART_INT_RXDNE) != RESET)
	{
		rxdata = USART_ReceiveData(USART3);
		UartReceive(rxdata);
#ifdef BOOTLOADER
		BootReceive(rxdata);
#endif 
	}
	
	if(USART_GetIntStatus(USART3, USART_INT_TXC) != RESET)
	{
		USART_ClrFlag(USART3, USART_FLAG_TXC);
		UartBmsSend();
	}
//	if(USART_GetIntStatus(USART3, USART_INT_IDLEF) != RESET)
//	{
//		USART_ClrFlag(USART3, USART_FLAG_IDLEF);
//	}
}

void CAN_RX0_IRQHandler(void)
{  

	CAN_RX0_IRQHandler_CallBack();
}
void CAN_RX1_IRQHandler(void)
{   

	CAN_RX1_IRQHandler_CallBack();
}
/**
 * @brief  This function handles ADC global interrupts requests.
 */
void ADC_IRQHandler(void)
{
	/* Clear ADC End of conversion interrupt */
	ADC_ClearIntPendingBit(ADC, ADC_INT_ENDC);
//	AdcConvertStart();
}


/**
 * @brief  This function handles EXTI0 global interrupts requests.
 */
void EXTI0_IRQHandler(void)
{
	if (RESET != EXTI_GetITStatus(CHG_IN_EXIT_LINE))
	{        
		EXTI_ClrITPendBit(CHG_IN_EXIT_LINE);    
	}    
}

/**
 * @brief  This function handles EXTI0 global interrupts requests.
 */
void EXTI1_IRQHandler(void)
{
    if(RESET != EXTI_GetITStatus(EXTI_LINE1))
    {
     //   Callback_EXTI1_IRQn();
        EXTI_ClrITPendBit(EXTI_LINE1);  
    }
}

/**
 * @brief  This function handles EXTI4 global interrupts requests.
 */
void EXTI9_5_IRQHandler(void)
{
	if (RESET != EXTI_GetITStatus(AFE_ALNT_EXIT_LINE))
	{        
		EXTI_ClrITPendBit(AFE_ALNT_EXIT_LINE);    
	}
	if (RESET != EXTI_GetITStatus(DET_IN_EXIT_LINE))
	{        
		EXTI_ClrITPendBit(DET_IN_EXIT_LINE);    
	}
}

/**
 * @brief  This function handles EXTI0 global interrupts requests.
 */
void EXTI15_10_IRQHandler(void)
{    
	if (RESET != EXTI_GetITStatus(CAN_WKUP_EXIT_LINE))
	{        
		EXTI_ClrITPendBit(CAN_WKUP_EXIT_LINE);    
	}
}

extern volatile u8 Rtc1STick;
void RTC_WKUP_IRQHandler(void)
{
//	TEST_PIN_ON();
    if (RTC_GetITStatus(RTC_INT_WUT) != RESET)
    {        
    	Rtc1STick = 1;
        RTC_ClrIntPendingBit(RTC_INT_WUT);
        EXTI_ClrITPendBit(EXTI_LINE20);
    }
//  TEST_PIN_OFF();
}


