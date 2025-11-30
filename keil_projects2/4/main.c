#include "RTE_Components.h"
#include CMSIS_device_header
#include <stdio.h>


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
		FLASH -> ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
		
		// HCLK = SYSCLK / ...
		// 1011: SYSCLK divided by 16 -> RCC_CFCR_HPRE_DIV16
		RCC->CFGR |= RCC_CFGR_HPRE_DIV16; // AHB Pre = 16 by var
		
		// set up PLL to 72MHz = 8 MHz (HSE) * 9
	}
	else
	{
			// HSE is not ready
			while(1){}
	}
	
	for(;;){}
	return 0;
}