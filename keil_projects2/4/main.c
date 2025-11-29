#include "RTE_Components.h"
#include CMSIS_device_header

void delay () {
	volatile uint32_t count = 4323;
	while (count--)
	__NOP();
}

int main() {	
	// PA5:640Hz, PA10:1280Hz
	
	// addresses of registers for PA5
	uint32_t* _APB2ENR = (uint32_t*) (0x40021018);
	uint32_t* _GPIOA_CRL = (uint32_t*) (0x40010800);
	uint32_t* _GPIOA_BSRR = (uint32_t*) (0x40010810);
	uint32_t* _GPIOA_BRR = (uint32_t*) (0x40010814);
	
	// enable GPIOA (2th bit)
	*_APB2ENR |= 1 << 2;
	
	// setup PA5 in configuration register low
	*_GPIOA_CRL &= 0xFF0FFFFF;
	*_GPIOA_CRL |= 0x00200000;	
	
	// setup PA10 in configuration register high
	GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
	SET_BIT(GPIOA->CRH, GPIO_CRH_MODE10_1);
	
	
	// bitmask of PA5 for BRR/BSRR
	uint32_t PA5_BIT = 1 << 5;
	
	for (;;) {
		// PA5 = 1; PA10 = 1
		* _GPIOA_BSRR = PA5_BIT;
		GPIOA->BSRR = GPIO_BSRR_BS10;
		delay();
		
		// PA5 = 1; PA10 = 0
		* _GPIOA_BSRR = PA5_BIT;
		GPIOA->BRR = GPIO_BRR_BR10;
		delay();			
		
		// PA5 = 0; PA10 = 1
		* _GPIOA_BRR = PA5_BIT;
		GPIOA->BSRR = GPIO_BSRR_BS10;
		delay();		
		
		// PA5 = 0; PA10 = 0
		* _GPIOA_BRR = PA5_BIT;
		GPIOA->BRR = GPIO_BRR_BR10;
		delay();	
	}
	return 0;
}