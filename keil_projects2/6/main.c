#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_cortex.h"
#include <stdio.h>

int fputc(int c, FILE *f) {
	return (ITM_SendChar(c));
}

uint16_t TIM4cnt=0; // current counter TIM4
uint16_t TIM1cnt=0; // current counter TIM1

// TIM1 PB5 4693 
void setupTIM1() {
	// setup timer 1 (TIM1)
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);//enable timer
	LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);//count up	
	
	//44,000,000 / (938 * 10) ~ 4691Hz
	uint16_t PSC = 9;
	uint16_t ARR = 937;
	
	// set prescaler
	//TIM1CLK = PCLK2x1=44MHz
	LL_TIM_SetPrescaler(
		TIM1, 
		PSC);
		
	printf(
		"TIM1_PSC=%d\n", 
		PSC);
		
	LL_TIM_SetAutoReload(TIM1, ARR);
	printf("TIM1_ARR=%d\n", ARR);
	
	// allow timer 4 interrupt on update
	LL_TIM_EnableIT_UPDATE(TIM1);
	NVIC_SetPriority(TIM1_UP_IRQn, 0);
	NVIC_EnableIRQ(TIM1_UP_IRQn); // allow TIM1 IRQ
	LL_TIM_EnableCounter(TIM1); // include counter timer
	LL_TIM_GenerateEvent_UPDATE(TIM1); // call update programmatically
}

// TIM4 PA11 246
void setupTIM4() {
	// setup timer 4 (TIM4)
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);//enable timer
	LL_TIM_SetCounterMode(TIM4, LL_TIM_COUNTERMODE_DOWN);//count down	
	
	// 44,000,000/(423*422) ~ 246.72 Hz
	uint16_t PSC = 422;
	uint16_t ARR = 421;
	
	// set prescaler
	//TIM4CLK = PCLK1x2=44MHz
	LL_TIM_SetPrescaler(
		TIM4, 
		PSC);
		
	printf(
		"TIM4_PSC=%d\n", 
		PSC);
	
	LL_TIM_SetAutoReload(TIM4, ARR);
	printf("TIM4_ARR=%d\n", ARR);
	
	// allow timer 4 interrupt on update
	LL_TIM_EnableIT_UPDATE(TIM4);
	NVIC_SetPriority(TIM4_IRQn, 0);
	NVIC_EnableIRQ(TIM4_IRQn); // allow TIM2 IRQ
	LL_TIM_EnableCounter(TIM4); // include counter timer
	LL_TIM_GenerateEvent_UPDATE(TIM4); // call update programmatically
}

int main(void){
	printf("clk=%d Hz\n", SystemCoreClock);
	
	// config FLASH - 44 MHz SYSCLK -> one wait state
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
	
	// enable HSI
	LL_RCC_HSI_Enable();
	while (LL_RCC_HSI_IsReady() != 1){}
		
	// config PLL: 8 / 2 * 11 == 44 MHz
	LL_RCC_PLL_ConfigDomain_SYS(
		LL_RCC_PLLSOURCE_HSI_DIV_2,
		LL_RCC_PLL_MUL_11);
	LL_RCC_PLL_Enable();
	while (LL_RCC_PLL_IsReady() != 1){}	
		
	// config SYSCLK source from PLL
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL){};
		
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
	SystemCoreClockUpdate();
	printf("clk=%d Hz\n", SystemCoreClock);
		
	// variant 4: 
	// 1. TIM1 PB5 4693 
	// 2. TIM4 PA11 246
		
	// setup PB5 and PA11 output
	LL_APB2_GRP1_EnableClock(
		LL_APB2_GRP1_PERIPH_GPIOB
		| LL_APB2_GRP1_PERIPH_GPIOA);
		
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_11, LL_GPIO_MODE_OUTPUT);	
	
	// get and print clocks freq 	
	LL_RCC_ClocksTypeDef clocks;	
	LL_RCC_GetSystemClocksFreq(&clocks);	
	printf(
		"PCLK1=%d\nPCLK2=%d\n", 
		clocks.PCLK1_Frequency, 
		clocks.PCLK2_Frequency);
	
	// setup TIM1
	setupTIM1();
	
	// setup TIM4
	setupTIM4();		
	
	while(1){ 
		TIM4cnt=TIM4->CNT;
		TIM1cnt=TIM1->CNT;
	}
}

void TIM4_IRQHandler(void) {
	if(LL_TIM_IsActiveFlag_UPDATE(TIM4) == 1) {
			LL_TIM_ClearFlag_UPDATE(TIM4);
			LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_11);
	}
}

void TIM1_UP_IRQHandler(void) {
	if(LL_TIM_IsActiveFlag_UPDATE(TIM1) == 1) {
			LL_TIM_ClearFlag_UPDATE(TIM1);
			LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_5);
	}	
}