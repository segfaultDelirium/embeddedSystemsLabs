
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include "Board_Joystick.h"             // ::Board Support:Joystick
#include "Board_LED.h"
#include "Board_Buttons.h"              // ::Board Support:Buttons

int pow_int(int x, int k){
	int result = 1;
	for(int i = 0; i < k; i++){
		result *= x;
	}
	return result;
}

int main(){

	LPC_GPIO0->FIODIR = 0xF;
	LPC_GPIO2->FIODIR = 0xFF;
	
	while(1){
		
		
		for(int i = 0xF; i >= 0x0; i--){
			LPC_GPIO0->FIOPIN = i;
			for(long j = 0; j < 1000000; j++);
		}
	
		
		for(int i = 0xFF; i >= 0x0; i--){
			LPC_GPIO2->FIOPIN = i;
			for(long j = 0; j < 1000000; j++);
		}
		
		//LPC_GPIO(2).FIODIR = 0xF;
		
		
			
	}
	
                  // ::Board Support:LED
	
}
