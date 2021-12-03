#include "LPC17xx.h"

#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include "Board_Joystick.h"             // ::Board Support:Joystick
#include "Board_LED.h"
#include "Board_Buttons.h"              // ::Board Support:Buttons

uint32_t coreClock_1 = 0;                       /* Variables to store core clock values */
uint32_t coreClock_2 = 0;

volatile uint32_t msTicks = 0; 

int pow_int(int x, int k){
	int result = 1;
	for(int i = 0; i < k; i++){
		result *= x;
	}
	return result;
}

void SysTick_Handler(void)  {                               /* SysTick interrupt Handler. */
  msTicks++;     
	/* See startup file startup_LPC17xx.s for SysTick vector */ 
}

void wait(unsigned t){
	msTicks = 0;
	while(msTicks < t){
	 __WFI();
	}

}

void printString(char s[], int n ){
	for(int i = 0; i < n; i++){
		while( (LPC_UART0->LSR & (1 << 5) ) != 0){
			;
		}
		LPC_UART0->THR = s[i];
	}
}


int main (void)  {
  uint32_t returnCode;
  
  returnCode = SysTick_Config(SystemCoreClock / 1000);      /* Configure SysTick to generate an interrupt every millisecond */
  
  if (returnCode != 0)  {                                   /* Check return code for errors */
    // Error Handling 
  }
	

	
	LPC_GPIO0->FIODIR = 0xF;
	LPC_GPIO2->FIODIR = 0xFF;


	LPC_UART0->LCR = (1 << 7 ) | ( 3 << 0);
	LPC_UART0->DLL = 13;
	LPC_UART0->DLM = 0;
	LPC_UART0->LCR = 3;
	LPC_UART0->FDR= 1 | (15 << 4);
	
	PIN_Configure(0, 2, 1, 2, 0);  
	PIN_Configure(0, 3, 1, 2, 0); 
	
	LPC_UART0->THR = 'A';
	char s[] = " hello 1234567890123456789";
	printString(s, sizeof(s)/sizeof(char));
	
  while(1){
		
		for(int i = 0xF; i >= 0x0; i--){
			LPC_GPIO0->FIOPIN = i;
			wait(10);
			printString(s, sizeof(s)/sizeof(char));
			//for(long j = 0; j < 1000000; j++);
		}
	
		
		for(int i = 0xFF; i >= 0x0; i--){
			LPC_GPIO2->FIOPIN = i;
			wait(10);
			printString(s, sizeof(s)/sizeof(char));
			//for(long j = 0; j < 1000000; j++);
		}
		
		//LPC_GPIO(2).FIODIR = 0xF;
		
	}
}
