#include "LPC17xx.h"
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO

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

volatile uint32_t msTicks = 0; 
void SysTick_Handler(void);
void printCalibrationPicture(void);
void wait(unsigned t);
void printString(char s[], int n );
void printOnLED(char string[], int len);
void printCalibrationX(int xPos, int yPos, int color);

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
	
	/*
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
	
	LPC_GPIOINT->IO0IntClr = (1 << 19);
	LPC_GPIOINT->IO0IntEnF = (1 << 19);
	NVIC_EnableIRQ(EINT3_IRQn);
	*/
	LPC_GPIOINT->IO0IntClr = 1 << 19;
}




int main (void)  {
	// SysTick part
  uint32_t returnCode;
  returnCode = SysTick_Config(SystemCoreClock / 1000);      /* Configure SysTick to generate an interrupt every millisecond */
  if (returnCode != 0)  {                                   /* Check return code for errors */
    // Error Handling 
  }
	//////////////////////
	// UART PART
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
	////////////////////////////////
	// GPIOINT for touch LCD
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	PIN_Configure(0, 19, 0, 0, 0); 
	LPC_GPIOINT->IO0IntEnF = (1 << 19);
	NVIC_EnableIRQ(EINT3_IRQn);
	//////////////////////////
	// LCD
	lcdConfiguration();
	init_ILI9325();
	
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
	char hello[] = "helloworld";
	int len = sizeof(hello)/sizeof(char);
	printOnLED(hello, len);
	//////////////////////////
	//touch pad init
	touchpanelInit();
	printCalibrationPicture();
	
  while(1){
	
	}
}

void SysTick_Handler(void)  {                               /* SysTick interrupt Handler. */
  msTicks++;     
	/* See startup file startup_LPC17xx.s for SysTick vector */ 
}

void printCalibrationPicture(void){
	printCalibrationX(0, 0, LCDRed);
	printCalibrationX(220, 0, LCDBlue);
	printCalibrationX(220, 300, LCDCyan);
	printCalibrationX(0, 300, LCDMagenta);
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

void printCalibrationX(int xPos, int yPos, int color){
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
}
