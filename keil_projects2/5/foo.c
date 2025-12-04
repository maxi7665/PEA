#include "RTE_Components.h"
#include CMSIS_device_header
#include <stdio.h>

//int fputc(int c, FILE *f) {
//		return (ITM_SendChar(c));
//}


//int main() {	
//	uint32_t cnt = 0;
//	
//	// check default clock frequency
//	SystemCoreClockUpdate();
//	ITM_SendChar('\n');
//	printf("Start clk=%d Hz\n", SystemCoreClock);
//	
//	// enable HSE
//	SET_BIT(RCC -> APB2ENR, RCC_APB2ENR_IOPAEN);
//	
//	GPIOA -> CRL = 0;
//	
//	SET_BIT(
//		GPIOA -> CRL,
//		GPIO_CRL_CNF2_1|GPIO_CRL_CNF3_1|
//		GPIO_CRL_CNF4_1|GPIO_CRL_CNF6_1);
//	
//	SET_BIT(
//		GPIOA -> ODR,
//		GPIO_ODR_ODR2|GPIO_ODR_ODR3|
//		GPIO_ODR_ODR4|GPIO_ODR_ODR6);
//		
//	while(1) {
//			if((GPIOA->IDR & 0x04)==0) {
//				printf("%9i press PA2\n",cnt++);
//			}
//			if((GPIOA->IDR & 0x08)==0) {
//				printf("%9i press PA3\n",cnt++);
//			}
//			if((GPIOA->IDR & 0x10)==0) {
//				printf("%9i press PA4\n",cnt++);
//			}
//			if((GPIOA->IDR & 0x40)==0) {
//				printf("%9i press PA6\n",cnt++);
//			}	
//	}
//	
//	for(;;){}
//	return 0;
//}