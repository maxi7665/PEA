#include "RTE_Components.h"
#include CMSIS_device_header
#include <stdio.h>

int fputc(int c, FILE *f) {
		return (ITM_SendChar(c));
}


int main() {	
	volatile uint32_t StartUpCounter = 0, HseStatus = 0;
	
	// check default clock frequency
	SystemCoreClockUpdate();
	ITM_SendChar('\n');
	printf("Start clk=%d Hz\n", SystemCoreClock);
	
	// enable HSE
	SET_BIT(RCC -> CR, RCC_CR_HSEON);
	
	do {
		HseStatus = RCC -> CR & RCC_CR_HSERDY;
		StartUpCounter++;
	} while ((HseStatus == 0) && (StartUpCounter != 0x5000));
	
	// check if hse ready
	if ((RCC->CR & RCC_CR_HSERDY) != RESET)
	{
		// set up FLASH - commands prefetch latency
		// 000: Zero wait state, if 0 < HCLK <= 24 MHz -> FLASH_ACR_LATENCY_0
		// 001: One wait state, if 24 MHz < HCLK <= 48MHz ->  FLASH_ACR_LATENCY_1
		// 010: Two wait state, if 48 MHz < HCLK <= 72MHz ->  FLASH_ACR_LATENCY_2
		//   0: Prefetch is disabled
		//	 1: Prefetch is enabled -> FLASH_ACR_PRFTBE
		FLASH -> ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_0;

		// HCLK = SYSCLK / ...
		// SYSCLK divided by 2 -> RCC_CFCR_HPRE_DIV2
		RCC->CFGR |= RCC_CFGR_HPRE_DIV2; // AHB Pre = 2 by var
		//RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

		// set up PLL to 12MHz = 8 MHz (HSI) / 2 * 3

		// disable PLL for configuration
		CLEAR_BIT(RCC -> CR, RCC_CR_PLLON);

		// Bit 16 PLLSRC: -> RCC_CFGR_PLLSRC
		//   0: HSI/2 selected as PLL input clock 
		//   1: HSE selected as PLL input clock
		// Bits 21:18 PLLMUL: -> RCC_CFGR_PLLMUL
		//   0111: PLI input clock x 3 -> RCC_CFGR_PLLMUL3
		// Bit 17 PLLXTPRE: -> RCC_CFGR_PLLXTPRE
		//   0: HSE clock not divided
		//   1: HSE clock divided by 2
		// PLL configuration: PLLCLK = HSI / 2 * 3 = 12 MHz 
		RCC -> CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);		
		RCC -> CFGR |= (RCC_CFGR_PLLSRC_HSI_Div2 | RCC_CFGR_PLLMULL3);

		// enable PLL: PLLON
		// O: PLL OFF;
		// 1: PLL ON
		SET_BIT(RCC -> CR, RCC_CR_PLLON) ;
		// wait for PLL READY state
		while ((RCC->CR & RCC_CR_PLLRDY) == 0) {}
		// select PLL as source for SYSCLK
		RCC->CFGR &= ~(RCC_CFGR_SW);
		RCC->CFGR |= RCC_CFGR_SW_PLL;
		// wait for setting PLL as source for SYSCLK
		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}
	}
	else
	{
		// HSE is not ready
		while(1){}
	}
	
	SystemCoreClockUpdate() ; // check new frequency
	printf("After configuration clk=%d Hz\n", SystemCoreClock);
	// setup Main Clock Output (MCO) to HSE
	SET_BIT(RCC -> CFGR, RCC_CFGR_MCO_HSE);
	
	SET_BIT(RCC -> APB2ENR, RCC_APB2ENR_IOPAEN) ; //allow GPIOA work
	// Reset PA8 config
	GPIOA->CRH &= ~(GPIO_CRH_MODE8 | GPIO_CRH_CNF8) ;
	//MODE: output with max freq 50 MHz
	//CNF: Alternate function output Push-pull
	SET_BIT (GPIOA->CRH, GPIO_CRH_MODE8 | GPIO_CRH_CNF8_1);
	
	// enable GPIOC PB5
	SET_BIT (RCC->APB2ENR, RCC_APB2ENR_IOPBEN);
	// setup PB5
	GPIOB->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5);
	SET_BIT(GPIOB->CRL, GPIO_CRL_MODE5);// High speed
	while(1){ 
		// set 1 on PB5
		GPIOB->BSRR = GPIO_BSRR_BS5;
		// reset bit PB5
		GPIOB->BRR = GPIO_BSRR_BS5;		
	}
	
	for(;;){}
	return 0;
}