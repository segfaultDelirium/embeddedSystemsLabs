#include "LPC17xx.h"
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include "Board_Joystick.h"             // ::Board Support:Joystick
#include "Board_LED.h"
#include "Board_Buttons.h"              // ::Board Support:Buttons

#include ".\lcd_lib\Open1768_LCD.h"
#include ".\lcd_lib\LCD_ILI9325.h"
#include ".\lcd_lib\asciiLib.h"
#include ".\tp_lib\TP_Open1768.h"
const int maxXTP = 4095;
const int maxYTP = 3930;
const int maxXlcd = 240;
const int maxYlcd = 320;

//OSCIL_ON


// 115200 baud rate, COM1

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



void printOnLED(char string[], int len){
	for(int k = 0; k < len; k++){
		char letter = string[k];
		unsigned char pbuffer[16];
		GetASCIICode(0, pbuffer, letter);
		for(int i = 0; i < 16; i++){
			unsigned int value = pbuffer[i];
			for(int j = 0; j < 8; j++){
					lcdWriteIndex(ADRX_RAM);
					lcdWriteData(240 - i);
					lcdWriteIndex(ADRY_RAM);
					lcdWriteData(((7 - j) + 16 * k));
					lcdWriteIndex(DATA_RAM);
				if(value & 1 << j){
					lcdWriteData(LCDWhite);
				}else{
					lcdWriteData(LCDBlack);
				}
			}
		}
	}
}

int toTPlcdPositionX(int x){
	return (maxXlcd * x/maxXTP);
}

int toTPlcdPositionY(int y){
	return (maxYlcd * y/maxYTP);
}

	// GPIOINT for touch LCD
void EINT3_IRQHandler(void)  {                              
	char s[] = "_Bang_";
	printString(s, sizeof(s)/sizeof(char));
	
	// clear pending, clear peryferium
	
	//NVIC_DisableIRQ(EINT3_IRQn);
	LPC_GPIOINT->IO0IntEnF = (0 << 19);
	LPC_GPIOINT->IO0IntClr = 1 << 19;
	
	
	float xAvg = 0;
	float yAvg = 0;
	int sampleAmount = 100;
	for(int i = 0; i < sampleAmount; i++){
		int x = 0;
		int y = 0;
		touchpanelGetXY(&y, &x);
		printString("after", sizeof(s)/sizeof(char));
		xAvg += x;
		yAvg += y;
	}
	xAvg /= sampleAmount;
	yAvg /= sampleAmount;
	int xlcdPosition = toTPlcdPositionX(xAvg);
	int ylcdPosition = toTPlcdPositionY(yAvg); //240 * yAvg/maxYTP;
	char position[30] = "";
	sprintf(position, "x=%4d, y=%4d", xlcdPosition, ylcdPosition);
	int positionLen = strlen(position);
	printOnLED(position, positionLen);
	printString(position, positionLen);
	printString("aft2", sizeof(s)/sizeof(char));
	
	
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	/*
	LPC_GPIOINT->IO0IntClr = (1 << 19);
	LPC_GPIOINT->IO0IntEnF = (1 << 19);
	NVIC_EnableIRQ(EINT3_IRQn);
	*/
	LPC_GPIOINT->IO0IntClr = 1 << 19;
}

void printCalibrationPicture(int xPos, int yPos, int color){
	for(int i = -8; i < 8; i++){
		lcdWriteIndex(ADRX_RAM);
		lcdWriteData(xPos+i);
		lcdWriteIndex(ADRY_RAM);
		lcdWriteData(yPos);
		lcdWriteIndex(DATA_RAM);
		lcdWriteData(color);
	}
	
	for(int i = -8; i < 8; i++){
		lcdWriteIndex(ADRX_RAM);
		lcdWriteData(xPos);
		lcdWriteIndex(ADRY_RAM);
		lcdWriteData(yPos+i);
		lcdWriteIndex(DATA_RAM);
		lcdWriteData(color);
	}
	
	/*
	for(int i = 0; i < 10; i++){
		lcdWriteIndex(ADRX_RAM);
		lcdWriteData(200 + i);
		lcdWriteIndex(ADRY_RAM);
		lcdWriteData(100);
		lcdWriteIndex(DATA_RAM);
		lcdWriteData(LCDWhite);
	}
	*/
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
	
	// GPIOINT for touch LCD
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	PIN_Configure(0, 19, 0, 0, 0); 
	LPC_GPIOINT->IO0IntEnF = (1 << 19);
	NVIC_EnableIRQ(EINT3_IRQn);
	
	// LCD
	lcdConfiguration();
	int reg = lcdReadReg(OSCIL_ON);
	if(reg == 0x8989){
		wait(1000);
		char confinguration[] = "SDD1289";
		printString(confinguration, sizeof(confinguration)/sizeof(char));
	}
	else if(reg == 0x9325 || reg == 0x9328){
		wait(1000);
		char confinguration[] = "ILI9325 or ILI9328";
		printString(confinguration, sizeof(confinguration)/sizeof(char));
		init_ILI9325();
	}
	else{
		wait(1000);
		char confinguration[] = "__OTHER__";
		printString(confinguration, sizeof(confinguration)/sizeof(char));
	}
	/*
	lcdWriteReg(ADRX_RAM, 100);
	lcdWriteReg(ADRY_RAM, 100);
	lcdWriteReg(DATA_RAM, LCDRed);
	*/
	
	lcdWriteIndex(ADRX_RAM);
	lcdWriteData(100);
	lcdWriteIndex(ADRY_RAM);
	lcdWriteData(100);
	lcdWriteIndex(DATA_RAM);
	
	for(int i = 0; i < 240; i++){
		for( int j = 0; j < 320; j++){
			lcdWriteData(LCDRed);
		}
	}

	/*
	lcdWriteReg(HADRPOS_RAM_START, 100);
	lcdWriteReg(HADRPOS_RAM_END, 200);
	lcdWriteReg(VADRPOS_RAM_START, 100);
	lcdWriteReg(HADRPOS_RAM_END, 200);
	*/
	
	lcdWriteIndex(ADRX_RAM);
	lcdWriteData(100);
	lcdWriteIndex(ADRY_RAM);
	lcdWriteData(100);
	lcdWriteIndex(DATA_RAM);
	
	unsigned char pbuffer[16];
	GetASCIICode(0, pbuffer, 'a');
	
	for(int i = 0; i < 240; i++){
		for( int j = 0; j < 320; j++){
			lcdWriteData(LCDBlack);
		}
	}
	
	/* //printing single char
	for(int i = 0; i < 16; i++){
		unsigned int value = pbuffer[i];
		for(int j = 0; j < 8; j++){
			if(value & 1 << j){
				lcdWriteIndex(ADRX_RAM);
				lcdWriteData(i);
				lcdWriteIndex(ADRY_RAM);
				lcdWriteData(j);
				lcdWriteIndex(DATA_RAM);
				lcdWriteData(LCDWhite);
			}
		}
	}
	*/
	
	char hello[] = "helloworld";
	int len = sizeof(hello)/sizeof(char);
	printOnLED(hello, len);
	
	//touch pad init
	touchpanelInit();
	
	printCalibrationPicture(0, 0, LCDRed);
	printCalibrationPicture(220, 0, LCDBlue);
	printCalibrationPicture(220, 300, LCDCyan);
	printCalibrationPicture(0, 300, LCDMagenta);
	
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
		
		// touch pad functionality
		
		/*
		float xAvg = 0;
		float yAvg = 0;
		int sampleAmount = 100;
		for(int i = 0; i < sampleAmount; i++){
			int x = 0;
			int y = 0;
			touchpanelGetXY(&y, &x);
			xAvg += x;
			yAvg += y;
		}
		
		xAvg /= sampleAmount;
		yAvg /= sampleAmount;
		
		
		int xlcdPosition = toTPlcdPositionX(xAvg);
		int ylcdPosition = toTPlcdPositionY(yAvg); //240 * yAvg/maxYTP;
		
		char position[30] = "";
		sprintf(position, "x=%4d, y=%4d", xlcdPosition, ylcdPosition);
		int positionLen = strlen(position);
		
		printOnLED(position, positionLen);
		printString(position, positionLen);
		*/
		
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
