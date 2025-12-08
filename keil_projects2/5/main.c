#include "RTE_Components.h"
#include CMSIS_device_header
#include <stdio.h>

int fputc(int c, FILE *f) {
	return (ITM_SendChar(c));
}


void delay (void) {
	volatile uint32_t i=600000;// delay
	while (i > 0){
		i--;
	}
}

int main(void) {
	uint32_t priGroup = 0, PreemptPriority=0, SubPriority=0;
	SystemCoreClockUpdate();
	printf("clk=%d Hz\n", SystemCoreClock);
	
	// var 4: PB0, PA1, PA3, PB4 lines
	
	// allow GPIO A,C,B work
	RCC->APB2ENR|=RCC_APB2ENR_IOPAEN|RCC_APB2ENR_IOPCEN|RCC_APB2ENR_IOPBEN; 
	
	//PC13 on output
	GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
	SET_BIT(GPIOC->CRH, GPIO_CRH_MODE13);
	
	//PB0, PA1, PA3, PB4 Input, Pull up
	GPIOA->CRL = 0;	
	SET_BIT(
		GPIOA->CRL, 
		GPIO_CRL_CNF1_1|GPIO_CRL_CNF3_1);
	SET_BIT(
		GPIOA->ODR, 
		GPIO_ODR_ODR1|GPIO_ODR_ODR3); //pu11 up on start
	
	GPIOB->CRL = 0;	
	SET_BIT(
		GPIOB->CRL, 
		GPIO_CRL_CNF0_1|GPIO_CRL_CNF4_1);
	SET_BIT(
		GPIOB->ODR, 
		GPIO_ODR_ODR0|GPIO_ODR_ODR4); //pu11 up on start
	
	// alternate mode for GPIO
	SET_BIT(
		RCC->APB2ENR, 
		RCC_APB2ENR_AFIOEN);

	// EXTI as external inputs in alternate functions IO 
	// PB0>>EXTI0, PA1>>EXTI1, PA3>>EXTI3
	AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI0_PB
		| AFIO_EXTICR1_EXTI1_PA
		| AFIO_EXTICR1_EXTI3_PA;
	// PB4>>EXTI4
	AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI4_PB; 
	
	
	// prioritetes by var: 111, 126, 140, 125
	// 4(0) 3(1) 3(0) 2(1)
	
	//NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
	// by var 4: 1 bit for sub-priority(1-0), 3 for priority (0-7)
	NVIC_SetPriorityGrouping(4);
	priGroup = NVIC_GetPriorityGrouping();
	printf("Priority Grouping=%d\r\n", priGroup);

	//NVIC_SetPriority(EXTI0_IRQn, 111);	
	NVIC_SetPriority(
		EXTI0_IRQn, 
		NVIC_EncodePriority(priGroup, 4, 0));
		
	NVIC_DecodePriority(
		NVIC_GetPriority(EXTI0_IRQn), 
		priGroup, 
		&PreemptPriority, 
		&SubPriority);
	printf(
		"EXTI0 Preempt Priority=%d \tSubPriority=%d\r\n",
		PreemptPriority,
		SubPriority) ;

	//NVIC_SetPriority(EXTI1_IRQn, 126) ;
	NVIC_SetPriority(
		EXTI1_IRQn, 
		NVIC_EncodePriority(priGroup, 3, 1));
		
	NVIC_DecodePriority(
		NVIC_GetPriority(EXTI1_IRQn), 
		priGroup, 
		&PreemptPriority, 
		&SubPriority) ;
	printf(
		"EXTI1 Preempt Priority=%d \tSubPriority=%d\r\n",
		PreemptPriority, 
		SubPriority);
	
	//NVIC_SetPriority(EXTI3_IRQn, 140);	
	NVIC_SetPriority(
		EXTI3_IRQn, 
		NVIC_EncodePriority(priGroup, 3, 0));
		
	NVIC_DecodePriority(
		NVIC_GetPriority(EXTI3_IRQn), 
		priGroup, 
		&PreemptPriority, 
		&SubPriority);		
	printf(
		"EXTI3 Preempt Priority=%d \tSubPriority=%d\r\n",
		PreemptPriority,
		SubPriority);
	
	//NVIC_SetPriority(EXTI4_IRQn, 125);
	NVIC_SetPriority(
		EXTI4_IRQn, 
		NVIC_EncodePriority(priGroup, 2, 1));
		
	NVIC_DecodePriority(
		NVIC_GetPriority(EXTI4_IRQn), 
		priGroup, 
		&PreemptPriority, 
		&SubPriority);
	printf(
		"EXTI4 Preempt Priority=%d \tSubpriority=%d\r\n", 
		PreemptPriority, 
		SubPriority) ;

	// Falling trigger event configuration
	SET_BIT(
		EXTI->FTSR, 
		EXTI_FTSR_TR0|EXTI_FTSR_TR1|
		EXTI_FTSR_TR3|EXTI_FTSR_TR4);

	// allow interruptions for external lines
	SET_BIT(
		EXTI->IMR, 
		EXTI_IMR_MR0|EXTI_IMR_MR1|
		EXTI_IMR_MR3|EXTI_IMR_MR4);
		
	NVIC_EnableIRQ(EXTI0_IRQn);	
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);
	NVIC_EnableIRQ(EXTI4_IRQn);
	SysTick_Config(0x6DDD00);
	while (1){}
}

// switch PC13 every 100ms
void SysTick_Handler(void) {
	GPIOC->ODR^=1<<13; 
}

void EXTI0_IRQHandler (void){
	EXTI->PR = EXTI_PR_PR0;
	ITM_SendChar('0'); 
	delay();
	ITM_SendChar('a');
	ITM_SendChar('\n'); 
}

void EXTI1_IRQHandler (void) {
	EXTI->PR = EXTI_PR_PR1;
	ITM_SendChar('1'); 
	delay();
	ITM_SendChar('b');
	ITM_SendChar('\n'); 
} 

void EXTI3_IRQHandler (void) {
	EXTI->PR = EXTI_PR_PR3;
	ITM_SendChar('3'); 
	delay();
	ITM_SendChar('c');
	ITM_SendChar('\n'); 
}

void EXTI4_IRQHandler(void) {
	EXTI->PR = EXTI_PR_PR4;
	ITM_SendChar('4'); 
	delay();
	ITM_SendChar('d');
	ITM_SendChar('\n'); 
}