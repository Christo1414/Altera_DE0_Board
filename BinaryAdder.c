/* LED Binary Calculator on Nios-II DE0 Board
*
* This program reads the values from the slider switch data register and
* adds/subtracts from the value read from the LED data register.
*
* Seven segment display prints decimal representation of LED register
*
* pushbutton0 adds value switch and LED data registers.
* pushbutton1 subtracts switch and LED data registers.
* pushbutton2 sets LED data register to 0.
*/

#define FALSE 0
#define TRUE !(FALSE)

#define ZERO 0b00111111
#define ONE 0b00000110
#define TWO 0b01011011
#define THREE 0b01001111
#define FOUR 0b01100110
#define FIVE 0b01101101
#define SIX 0b01111101
#define SEVEN 0b00000111
#define EIGHT 0b01111111
#define NINE 0b01100111
int digits[10] = {ZERO,ONE,TWO,THREE,FOUR,FIVE,SIX,SEVEN,EIGHT,NINE};



void resetIO(volatile int* LED_ptr, volatile int* KEY_ptr, volatile int* DISP_ptr);
int readEDGE(volatile int* KEY_ptr, volatile int* LED_ptr, volatile int* SWITCH_ptr);
void updateDISP(volatile int* DISP_ptr, volatile int* LED_ptr);


int main (void){

	volatile int* LED_ptr = (int*) 0x10000010; //LED address
	volatile int* SWITCH_ptr = (int*) 0x10000040; //SWITCH address
	volatile int* KEY_ptr = (int*) 0x10000050; // Pushbutton data register
	volatile int* DISP_ptr = (int*) 0x10000020; // DISP data register

	int LED_CHANGED;

	resetIO(LED_ptr, KEY_ptr, DISP_ptr);

	while(1){
		LED_CHANGED = readEDGE(KEY_ptr, LED_ptr, SWITCH_ptr);
		if(LED_CHANGED){ //update seven-segment with value
			updateDISP(DISP_ptr, LED_ptr);
		}
	}
}

void resetIO(volatile int* LED_ptr, volatile int* KEY_ptr, volatile int* DISP_ptr){
	// turn off LED, clear EDGE capture register
	*(LED_ptr) = 0;
	*(KEY_ptr+0x3) = 0xf;
	*(DISP_ptr) = 0;
}

int readEDGE(volatile int* KEY_ptr, volatile int* LED_ptr, volatile int* SWITCH_ptr){
	int EDGE = *(KEY_ptr+0x3);
	int SWITCH_value = *(SWITCH_ptr);
	int LED_value = *(LED_ptr);

	if(EDGE > 0x0){
		// read 0x1 0x2 and 0x4 each to determine if pressed
		// write 0x7 to edge capture register to clear all bits
		if(EDGE & 0x1){ // read value from switches and add to LED sum
			if((LED_value + SWITCH_value) >= 0b1111111111){ //reached max value
				*(LED_ptr) = 0b1111111111;
			}else{
				*(LED_ptr) = LED_value + SWITCH_value;
			}
		}

		if(EDGE & 0x2){ // read value from switches and subtract from LED sum
			if((LED_value - SWITCH_value) < 0){ //disallow negative numbers
				*(LED_ptr) = 0;
			}else{
				*(LED_ptr) = LED_value - SWITCH_value;
			}
		}

		if(EDGE & 0x4){
			*(LED_ptr) = 0;
		}
		*(KEY_ptr+0x3) = 0xf; // clears EDGE register
		return TRUE;

	}
	else{
		return FALSE;
	}
}

void updateDISP(volatile int* DISP_ptr, volatile int* LED_ptr){
	// determine how many digits needs to be displayed

	int LED_value = *(LED_ptr);
	int value_masked = LED_value &0b1111111111;

	if(value_masked < 10){ // update HEX0
		*(DISP_ptr) = digits[value_masked];
	}
	else if(value_masked < 100){ // update HEX1 HEX0
		int digit_0 = value_masked % 10;
		int digit_1 = (value_masked - digit_0)/10;

		*(DISP_ptr) = digits[digit_0] + (digits[digit_1]<<8);
	}
	else if(value_masked < 1000){ // update HEX2 HEX1 HEX0
		int digit_0 = value_masked % 10;
		int digit_1 = ((value_masked - digit_0) % 100)/10;
		int digit_2 = (value_masked - digit_0 - digit_1*10)/100;

		*(DISP_ptr) = digits[digit_0] + (digits[digit_1]<<8) + (digits[digit_2]<<16);
	}
	else{ // update HEX3 HEX2 HEX1 HEX0
		int digit_0 = value_masked % 10;
		int digit_1 = ((value_masked - digit_0) % 100)/10;
		int digit_2 = ((value_masked - digit_0 - digit_1*10) % 1000)/100;
		int digit_3 = (value_masked - digit_0 - digit_1*10 - digit_2*100)/1000;

		*(DISP_ptr) = digits[digit_0] + (digits[digit_1]<<8) + (digits[digit_2]<<16) + (digits[digit_3]<<24);

	}

}
