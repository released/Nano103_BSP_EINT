/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 15/12/27 1:12p $
 * @brief    A project template for Nano103 MCU.
 *
 * @note
 * Copyright (C) 2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "Nano103.h"


//#define USE_GPIO_IRQ
#define USE_EINT0_IRQ

//#define USE_GPIO_OUTPUT
//#define USE_GPIO_INPUT

void GPABC_IRQHandler(void)
{

    uint32_t reg;
	
    printf("%s \r\n",__FUNCTION__);
    if (PB->INTSRC & BIT8)
    {
        PB->INTSRC = BIT8;
        printf("PB.8 GPIO INT occurred. \n");
    }
    else
    {    /* Unexpected interrupt. Just clear all PORTA, PORTB, PORTC interrupts */
        /* Un-expected interrupt. Just clear all PORTA, PORTB, PORTC interrupts */
        reg = PA->INTSRC;
        PA->INTSRC = reg;
        reg = PB->INTSRC;
        PB->INTSRC = reg;
        reg = PC->INTSRC;
        PC->INTSRC = reg;
        printf("Unexpected interrupts. \n");
    }
}


void EINT0_IRQHandler(void)
{
    printf("%s \r\n",__FUNCTION__);
    if(GPIO_GET_INT_FLAG(PB, BIT8))
    {
        GPIO_CLR_INT_FLAG(PB, BIT8);
		printf("PB8 EINT0 occurred. \r\n");
    }

	
}



void GPIO_Config(void)
{


	#if defined (USE_GPIO_OUTPUT)
    GPIO_SetMode(PB, BIT8, GPIO_PMD_OUTPUT);
	#endif

	#if defined (USE_GPIO_INPUT)	
    GPIO_SetMode(PB, BIT8, GPIO_PMD_INPUT);
	#endif

	#if defined (USE_EINT0_IRQ)		
    GPIO_SetMode(PB, BIT8, GPIO_PMD_INPUT);	
	
    GPIO_EnableEINT0(PB, 8, GPIO_INT_FALLING);
    NVIC_EnableIRQ(EINT0_IRQn);
	
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCLKSRC_IRC10K, GPIO_DBCLKSEL_1024);	
    GPIO_ENABLE_DEBOUNCE(PB, BIT8);
	#endif

	#if defined (USE_GPIO_IRQ)		
    GPIO_SetMode(PB, BIT8, GPIO_PMD_INPUT);	
	
    GPIO_EnableInt(PB, 8, GPIO_INT_FALLING);
    NVIC_EnableIRQ(GPABC_IRQn);
	
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCLKSRC_HCLK, GPIO_DBCLKSEL_1);	
    GPIO_ENABLE_DEBOUNCE(PB, BIT8);
	#endif

}


void TMR3_IRQHandler(void)
{
	static uint32_t LOG = 0;
	static uint16_t CNT = 0;
	
    if(TIMER_GetIntFlag(TIMER3) == 1)
    {
        TIMER_ClearIntFlag(TIMER3);
	
		if (CNT++ >= 1000)
		{		
			CNT = 0;
        	printf("%s : %4d\r\n",__FUNCTION__,LOG++);

			#if defined (USE_GPIO_OUTPUT)
			PB8 ^= 1;
			#endif	
			
		}		
    }
}



void TIMER3_Init(void)
{
    TIMER_Open(TIMER3, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER3);
    NVIC_EnableIRQ(TMR3_IRQn);	
    TIMER_Start(TIMER3);
}



void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);

	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetHCLKFreq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPLLClockFreq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK0Freq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());
	printf("\r\n\r\n");
}


void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRC0EN_Msk);
    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
	
    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRC0STB_Msk);
    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);	
	
    /* Switch HCLK clock source to HXT */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC0,CLK_HCLK_CLK_DIVIDER(1));

    CLK_EnableModuleClock(TMR3_MODULE);	
    CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL2_TMR3SEL_HIRC, 0);

    /* Enable UART clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART clock source from HXT */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_UART0_CLK_DIVIDER(1));

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();


    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk | SYS_GPB_MFPL_PB1MFP_Msk);
    SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB0MFP_UART0_RXD | SYS_GPB_MFPL_PB1MFP_UART0_TXD);

	#if defined (USE_EINT0_IRQ)
    SYS->GPB_MFPH &= ~( SYS_GPB_MFPH_PB8MFP_Msk );
    SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_INT0);
	#endif
	
    /* Lock protected registers */
    SYS_LockReg();
}

/*
 * This is a template project for Nano103 series MCU. Users could based on this project to create their
 * own application without worry about the IAR/Keil project settings.
 *
 * This template application uses external crystal as HCLK source and configures UART0 to print out
 * "Hello World", users may need to do extra system configuration based on their system design.
 */

int main()
{

    SYS_Init();

    UART0_Init();
	TIMER3_Init();

	GPIO_Config();


    /* Got no where to go, just loop forever */
    while(1)
    {
//		#if defined (USE_GPIO_INPUT)
//		if (PB8)
//		{
//			printf("input toggle\r\n");
//		}
//		#endif

    }

}

/*** (C) COPYRIGHT 2015 Nuvoton Technology Corp. ***/
