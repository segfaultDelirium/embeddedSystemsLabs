#include "LPC17xx.h"

#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include "Board_Joystick.h"             // ::Board Support:Joystick
#include "Board_LED.h"
#include "Board_Buttons.h"              // ::Board Support:Buttons

uint32_t coreClock_1 = 0;                       /* Variables to store core clock values */
uint32_t coreClock_2 = 0;

volatile uint32_t msTicks = 0; 

_Bool rtcTickTakflag = false;
_Bool diodeFlag = false;
_Bool blink4TimesFlag = false;

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
		while( !(LPC_UART0->LSR & (1 << 5) ) ){
			;
		}
		LPC_UART0->THR = s[i];
	}
}

void printInt(int i){
	while( !(LPC_UART0->LSR & (1 << 5) ) ){
		;
	}
	LPC_UART0->THR = i;
}

void printChar(char c){
	while( !(LPC_UART0->LSR & (1 << 5) ) ){
		;
	}
	LPC_UART0->THR = c;
}

char receiveChar(){
	char res = 'a';
	res = LPC_UART0->RBR;
	return res;
}

void TIMER0_IRQHandler(void)  {                               /* TIMER0 interrupt Handler. */
	char s[] = "ping";
	printString(s, sizeof(s)/sizeof(char));
	LPC_TIM0->IR = (1 << 0);
}

void RTC_IRQHandler(void)  {/* RTC interrupt Handler. */
	if(rtcTickTakflag){
		char s[10] = "Tick";
		rtcTickTakflag = false;
		printString(s, sizeof(s)/sizeof(char));
	}else{
		char s[10] = "Tak";
		rtcTickTakflag = true;
		printString(s, sizeof(s)/sizeof(char));
	}
	LPC_RTC->ILR = 1;
}

void EINT0_IRQHandler(void)  {                               /* TIMER0 interrupt Handler. */
	blink4TimesFlag = true;
	if(diodeFlag){
		LED_Off(3);
		diodeFlag = false;
	}else{
		LED_On(3);
		diodeFlag = true;
	}
	
	char s[] = "Click";
	printString(s, sizeof(s)/sizeof(char));
	LPC_SC->EXTINT = 1;
}

int main (void)  {
	LED_Initialize();
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
	
	//Buttons_Initialize();
	
	
	LPC_TIM0->PR = 12500000;
	LPC_TIM0->MR0 = 1;
	LPC_TIM0->MCR = (3 << 0);
	NVIC_EnableIRQ(TIMER0_IRQn);
	LPC_TIM0->TCR = 1;
	
	//RTC 
	LPC_RTC->CCR = 1;
	LPC_RTC->ILR = 0; 
	LPC_RTC->CIIR = 1;// sekunda
	NVIC_EnableIRQ(RTC_IRQn);
	
	
	// key2 signal interrupt
	PIN_Configure(2, 10, 1, 0, 0); 
	LPC_SC->EXTMODE = 1;
	LPC_SC->EXTPOLAR = 0;
	LPC_SC->EXTINT = 1;
	
	NVIC_EnableIRQ(EINT0_IRQn);
	
	
	
  while(1){
		
		if(blink4TimesFlag){
			for(int i = 0; i < 4; i++){
				LED_On(2);
				wait(1000);
				LED_Off(2);
				wait(1000);
			}
			blink4TimesFlag = false;
		}
		
		char resChar = receiveChar();
		printChar(resChar + 2);
		
		__WFI();
		/*
		int availableButtons = Buttons_GetCount();
		printChar(availableButtons + '0');
		*/
		
		/*
		int buttonState = Buttons_GetState();
		printChar(buttonState + '0');
		if(buttonState == 2){
			printString(s, sizeof(s)/sizeof(char));
		}
		else if(buttonState == 1){	
		}
		*/
		/*
		for(int i = 0xFF; i >= 0x0; i--){
			LPC_GPIO2->FIOPIN = i;
			wait(10);
			printString(s, sizeof(s)/sizeof(char));
			//for(long j = 0; j < 1000000; j++);
		}
		*/
		
		//LPC_GPIO(2).FIODIR = 0xF;
		
	}
}
