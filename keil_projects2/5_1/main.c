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

void EXTI_Config(void);

EXTI_InitTypeDef   EXTI_InitStructure;
GPIO_InitTypeDef   GPIO_InitStructure;
NVIC_InitTypeDef   NVIC_InitStructure;

int main()
{
	uint32_t cnt = 0;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_ClocksTypeDef RCC_ClockFreq;
	RCC_GetClocksFreq(&RCC_ClockFreq);
	printf(
		"SYSCLK=%dHz, HCLK=%dHz, PCLK=%dHz, PLCK2=%dHz",
		RCC_ClockFreq.SYSCLK_Frequency,
		RCC_ClockFreq.HCLK_Frequency,
		RCC_ClockFreq.PCLK1_Frequency,
		RCC_ClockFreq.PCLK2_Frequency);
	
	if (SysTick_Config(0x6DDD00))
  { 
    /* Capture error */ 
    while (1);
  }
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// allow GPIO A,C,B work
	//RCC->APB2ENR|=RCC_APB2ENR_IOPAEN|RCC_APB2ENR_IOPCEN|RCC_APB2ENR_IOPBEN; 
	
	//PC13 on output
	//GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
	//SET_BIT(GPIOC->CRH, GPIO_CRH_MODE13);
	
	//while(1){}
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = 0;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_4;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* Enable AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	EXTI_Config();
	
//	while(1) {
//			if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0)==0) {
//				printf("%9i press PB0\n", cnt++);
//			}
//			if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)==0) {
//				printf("%9i press PA1\n", cnt++);
//			}
//			if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3)==0) {
//				printf("%9i press PA3\n", cnt++);
//			}
//			if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4)==0) {
//				printf("%9i press PB4\n", cnt++);
//			}
//	}
	
	while(1){}
	return 0;
}


// configuring EXTI
void EXTI_Config(void)
{
  /* Connect EXTI Lines to pins */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);

  /* Configure EXTI lines */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line3 | EXTI_Line4;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);


  /* Enable and set EXTI0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_Init(&NVIC_InitStructure);
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