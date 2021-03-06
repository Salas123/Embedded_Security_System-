/*
 * pirMotion.c
 *
 * Created: 5/21/2019 10:20:19 AM
 * Author : azul3
 */ 

#define F_CPU 8000000UL

#include <string.h>
#include <util/delay.h>
#include "SPI_Master_H_file.h"
#include "Font.h"
//#include "usart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <bit.h>
#include <timer.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
// Function Pototype
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

// Function Implementation
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();

    return;
}




// *** KEYPAD FUNCTIONS ***

unsigned char GetKeypadKey() 
{
	//A = 1010, B = 1011, C = 1100, D = 1101, E = 1110, F = 1111

	//Check keys in col 1
 	PORTA = 0xEF; // Enable col 4 with 0, disable others with 1’s (1110 1111)
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('1'); }
	if (GetBit(PINA,1)==0) { return('4'); }
	if (GetBit(PINA,2)==0) { return('7'); }
	if (GetBit(PINA,3)==0) { return('*'); }

	// Check keys in col 2
	PORTA = 0xDF; // Enable col 5 with 0, disable others with 1’s (1101 1111)
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('2'); }
	if (GetBit(PINA,1)==0) { return('5'); }
	if (GetBit(PINA,2)==0) { return('8'); }
	if (GetBit(PINA,3)==0) { return('0'); }

	// Check keys in col 3
	PORTA = 0xBF; // Enable col 6 with 0, disable others with 1’s (1011 1111)
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('3'); }
	if (GetBit(PINA,1)==0) { return('6'); }
	if (GetBit(PINA,2)==0) { return('9'); }
	if (GetBit(PINA,3)==0) { return('#'); }

	// Check keys in col 4	
	PORTA = 0x7F; // Enable col 6 with 0, disable others with 1’s (0111 1111)
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('A'); }
	if (GetBit(PINA,1)==0) { return('B'); }
	if (GetBit(PINA,2)==0) { return('C'); }
	if (GetBit(PINA,3)==0) { return('D'); }

	return('\0'); // default value

}

char KeyPadInput()
{
	char temp = '\0';

	unsigned char x;

	x = GetKeypadKey();
	
	switch (x)
	 {
			case '\0': temp = '\0'; break;
			case '1': temp = '1'; break; // hex equivalent
			case '2': temp = '2'; break;
			case '3': temp = '3'; break;
			case '4': temp = '4'; break;
			case '5': temp = '5'; break;
			case '6': temp = '6'; break;
			case '7': temp = '7'; break;
			case '8': temp = '8'; break;
			case '9': temp = '9'; break;
			case 'A': temp = 'A'; break;
			case 'B': temp = 'B'; break;
			case 'C': temp = 'C'; break;
			case 'D': temp = 'D'; break;
			case '*': temp = 'E'; break;
			case '0': temp = '0'; break;
			case '#': temp = 'F'; break;
			default: break;

	}

	return temp;
}
// *** END OF KEYPAD FUNCTIONS ***

// ** NOKIA LCD FUNCTIONS *** 
void N5110_Cmnd(char DATA)
{
	PORTB &= ~(1<<DC);				/* make DC pin to logic zero for command operation */
	SPI_SS_Enable();				/* enable SS pin to slave selection */	
	SPI_Write(DATA);				/* send data on data register */
	PORTB |= (1<<DC);				/* make DC pin to logic high for data operation */
	SPI_SS_Disable();
}

void N5110_Data(char *DATA)
{
	PORTB |= (1<<DC);									/* make DC pin to logic high for data operation */
	SPI_SS_Enable();									/* enable SS pin to slave selection */
	int lenan = strlen(DATA);							/* measure the length of data */
	for (int g=0; g<lenan; g++)
	{
		for (int index=0; index<5; index++)
		{
			SPI_Write(ASCII[DATA[g] - 0x20][index]);	/* send the data on data register */			
		}
		SPI_Write(0x00);
	}							
	SPI_SS_Disable();									
}

void N5110_Reset()					/* reset the Display at the beginning of initialization */
{
	PORTB &= ~(1<<RST);
	_delay_ms(100);
	PORTB |= (1<<RST);
}

void N5110_init()
{
	N5110_Reset();					/* reset the display */
	N5110_Cmnd(0x21);				/* command set in addition mode */
	N5110_Cmnd(0xC0);				/* set the voltage by sending C0 means VOP = 5V */
	N5110_Cmnd(0x07);				/* set the temp. coefficient to 3 */
	N5110_Cmnd(0x13);				/* set value of Voltage Bias System */
	N5110_Cmnd(0x20);				/* command set in basic mode */
	N5110_Cmnd(0x0C);				/* display result in normal mode */
}

void lcd_setXY(char x, char y)		/* set the column and row */
{
	N5110_Cmnd(x);
	N5110_Cmnd(y);
}

void N5110_clear()					/* clear the Display */
{
	SPI_SS_Enable();
	PORTB |= (1<<DC);
	for (int k=0; k<=503; k++)
	{
		SPI_Write(0x00);		
	}
	PORTB &= ~(1<<DC);
	SPI_SS_Disable();	
}

void N5110_image(const unsigned char *image_data)		/* clear the Display */
{
	SPI_SS_Enable();
	PORTB |= (1<<DC);
	for (int k=0; k<=503; k++)
	{
		SPI_Write(image_data[k]);
	}
	PORTB &= ~(1<<DC);
	SPI_SS_Disable();
}
// *** END OF NOKIA LCD FUNCTIONS ***



//--------Find GCD function --------------------------------------------------
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
b = c;
	}
	return 0;
}
//--------End find GCD function ----------------------------------------------

//--------Task scheduler data structure---------------------------------------
// Struct for Tasks represent a running process in our simple real-time operating system.
typedef struct _task {
	/*Tasks should have members that include: state, period,
		a measurement of elapsed time, and a function pointer.*/
	signed char state; //Task's current state
	unsigned long int period; //Task period
	unsigned long int elapsedTime; //Time elapsed since last task tick
	int (*TickFct)(int); //Task tick function
} task;

//--------End Task scheduler data structure-----------------------------------

//--------Shared Variables----------------------------------------------------
char PIRMotionDetect = 0;
unsigned char awaitKeypadPress = 0;
unsigned char PIRPause = 1;
unsigned char motionDetected = 0;
char *ptrKeypadToLCDString;
char userInputPin[5];
char correctPin[5] = "1A4B";
char readCorrectInput[5]; 
unsigned char SM2_buttonWasPressed = 0x00;
unsigned char pause = 0;
char *ptrKeyPadInputString = NULL;
unsigned char displayCounter = 0;
char readByteOfData[10];
char correct_pin[10] = "Pin:1A4B";
char inputEntry;
char *display = "Pin:" ;
unsigned char pinEnteredCorrect = 0;




void append(char* s, char c) 
{
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

//--------End Shared Variables------------------------------------------------

//--------User defined FSMs---------------------------------------------------
//Enumeration of states.
// Monitors button connected to PA0. 
// When button is pressed, shared variable "pause" is toggled.
void readPIR()
{
	PIRMotionDetect = PINC & 0x10;
}


unsigned char PIRcounter = 0;
unsigned char PIRreset = 0;

enum SM1_States{SM1_AwaitToReadPIR, SM1_BeginToReadPIR, SM1_StopReadingPIR, SM1_ResetDelay};
int SMTick1(int state) 
{
	//State machine transitions
	switch (state) 
	{
		case SM1_AwaitToReadPIR:
		{
			if (PIRPause == 0)
				state = SM1_BeginToReadPIR;

			break;
		}

		case SM1_BeginToReadPIR:
		{
			if (PIRMotionDetect == 0x10)
			{
				motionDetected = 1;
				state = SM1_StopReadingPIR;
				PIRPause = 1;
			}
			break;
		}
		
		case SM1_StopReadingPIR:
		{
			if (PIRreset == 1)
			{
				state = SM1_ResetDelay;
				PIRreset = 0;
			}
			break;
		}

		case SM1_ResetDelay:
		{
			if(PIRcounter == 50)
			{
				state = SM1_AwaitToReadPIR;
				PIRcounter = 0;
			}

			break;
		}

		default: state = SM1_AwaitToReadPIR; break;
	}

	//State machine actions
	switch(state)
	{
		case SM1_AwaitToReadPIR: break;
		case SM1_BeginToReadPIR:readPIR(); break;
		case SM1_StopReadingPIR: break;
		case SM1_ResetDelay: PIRcounter++; break;
		default: break;
	}


	return state;
}


//--------End Shared Variables------------------------------------------------


unsigned char keypadCounter = 0;
unsigned char entryCursor = 0;
unsigned char unlockKeypad = 0;
//Enumeration of states.
enum SM2_States { SM2_init, SM2_awaitPress, SM2_setOutPressVar, SM2_lockKeypad};

int SMTick2(int state)
{

		//State machine transitions
		switch (state) 
		{
			case SM2_init:
			{	
				if (motionDetected != 0)
					state = SM2_awaitPress;
				
				break;
			}

			case SM2_awaitPress:
			{
				if (inputEntry != '\0')
				{
					state = SM2_setOutPressVar;
					append(display, inputEntry);
					SM2_buttonWasPressed = 0x01;
					entryCursor++;

				}
				break;
			}

			case SM2_setOutPressVar:
			{		
					if (keypadCounter == 20 && entryCursor != 4)
					{
						keypadCounter= 0;
						state = SM2_init;
						inputEntry = '\0';
						SM2_buttonWasPressed = 0x00;
					}
					

					if (entryCursor == 4)
					{
						state = SM2_lockKeypad;
						unlockKeypad = 1;
					}

				break;
			}


			case SM2_lockKeypad:
			{
				if (unlockKeypad == 0)
				{
					state = SM2_init;
				}
				break;
			}

			default: state = SM2_init; break;
		}

			//State machine actions
		switch(state) 
		{
			case SM2_init:	break;

			case SM2_awaitPress: inputEntry = KeyPadInput(); break; //toggle LED

			case SM2_setOutPressVar: keypadCounter++; break;

			case SM2_lockKeypad: break;

			default: break;
		}

	return state;

}

unsigned char doorCounter = 0;
unsigned char doorRotor = 0;

// //Enumeration of states.
enum SM3_States {SM3_doorClosed, SM3_doorOpenedDelay, SM3_doorOpened, SM3_doorCloseDelay};

// Combine blinking LED outputs from SM2 and SM3, and output on PORTB
int SMTick3(int state) 
{
	// Local Variables

	//State machine transitions
	switch (state)
	{
		case SM3_doorClosed:
		{ 
			if (pinEnteredCorrect == 1)
			{
				

				state = SM3_doorOpened;
				OCR1A = 300;	/* Set servo shaft at -90° position */
			}
				
			break;
		}

		case SM3_doorOpenedDelay:
		{
			if (doorCounter == 30)
			{
				// TCNT1 = 0;		/* Set timer1 count zero */
				// ICR1 = 2499;		/* Set TOP count for timer1 in ICR1 register */

				//  Set Fast PWM, TOP in ICR1, Clear OC1A on compare match, clk/64 
				// TCCR1A = (1<<WGM11)|(1<<COM1A1);
				// TCCR1B = (1<<WGM12)|(1<<WGM13)|(1<<CS10)|(1<<CS11);

				state = SM3_doorCloseDelay;
			}

			break;

		}

		case SM3_doorCloseDelay:
		{
			if (doorCounter == 60)
			{
				state = SM3_doorClosed;
				pinEnteredCorrect = 0;
			}
			break;
		}

		default: state = SM3_doorClosed; break;
	}

	//State machine actions
	switch(state) 
	{
		case SM3_doorClosed:
		{ 
			break;
		}

		case SM3_doorOpenedDelay: doorCounter++; break;

		case SM3_doorOpened:
		{
			doorCounter++;
			break;
		}
		case SM3_doorCloseDelay: doorCounter++; break;

		default: break;

	}

	return state;
	
}





unsigned char SM4_output = 0x00;

//Enumeration of states.
enum SM4_States { SM4_initDisplay, SM4_awaitMotion, SM4_displayMotion, SM4_awaitPress, SM4_displayEntry1, SM4_awaitPress2,
 SM4_displayEntry2, SM4_awaitPress3, SM4_displayEntry3, SM4_awaitPress4, SM4_displayEntry4, SM4_preRead, SM4_read,
 SM4_postReadCorrect, SM4_postReadIncorrect, SM4_preResetSys, SM4_resetSys};

// Monitors button connected to PA0. 
// When button is pressed, shared variable "pause" is toggled.
int SMTick4(int state) {	

	//State machine transitions
	switch (state)
	{
		case SM4_initDisplay: 
		{
			N5110_init();
			N5110_clear();
			lcd_setXY(0x40, 0x40);
			N5110_Data("Scanning...");
			state = SM4_awaitMotion;
			PIRPause = 0;
			SM4_output = 0x02;

			break;
		}

		case SM4_awaitMotion:
		{
			if (motionDetected != 0)
			{
				N5110_init();
				N5110_clear();
				lcd_setXY(0x40, 0x40);
				N5110_Data("Motion Detected!");
				SM4_output = 0x01;
				state = SM4_displayMotion;
			}
			break;
		}

		case SM4_displayMotion:
		{
			if (displayCounter == 6 )
			{
				N5110_init();
				N5110_clear();
				lcd_setXY(0x40, 0x40);
				N5110_Data(display);
				state = SM4_displayEntry1;
				displayCounter = 0;
				SM4_output = 0x00;
				state = SM4_awaitPress;
			}
			break;
		}

		case SM4_awaitPress:
		{
			if (SM2_buttonWasPressed == 0x01)
			{
				N5110_init();
				N5110_clear();
				lcd_setXY(0x40, 0x40);
				N5110_Data(display);
				state = SM4_displayEntry1;
			}

			break;
		}

		case SM4_displayEntry1:
		{
			state = SM4_awaitPress2;
			break;
		}

		case SM4_awaitPress2:
		{
			if (SM2_buttonWasPressed == 0x01)
			{
				N5110_init();
				N5110_clear();
				lcd_setXY(0x40, 0x40);
				N5110_Data(display);
				state = SM4_displayEntry2;
			}

			break;
		}

		case SM4_displayEntry2:
		{
			state = SM4_awaitPress3;
			break;
		}

		case SM4_awaitPress3:
		{
			if (SM2_buttonWasPressed == 0x01)
			{
				N5110_init();
				N5110_clear();
				lcd_setXY(0x40, 0x40);
				N5110_Data(display);
				state = SM4_displayEntry3;
			}

			break;
		}
		case SM4_displayEntry3:
		{
			state = SM4_awaitPress4;
			break;
		}

		case SM4_awaitPress4:
		{
			if (SM2_buttonWasPressed == 0x01)
			{
				N5110_init();
				N5110_clear();
				lcd_setXY(0x40, 0x40);
				N5110_Data(display);
				state = SM4_displayEntry4;
			}

			break;
		}

		case SM4_displayEntry4:
		{
			state = SM4_preRead;
			break;
		}


		case SM4_preRead:
		{
			state = SM4_read;
			break;
		}

		case SM4_read: 
		{
			if (strcmp(display, ptrKeyPadInputString) == 0)
			{
				N5110_init();
				N5110_clear();
				lcd_setXY(0x40, 0x40);
				N5110_Data("Correct Pin!");
				state = SM4_postReadCorrect;
				pinEnteredCorrect = 1;
				memset(readByteOfData, 0, sizeof readByteOfData);
			}
			else
			{
				N5110_init();
				N5110_clear();
				lcd_setXY(0x40, 0x40);
				N5110_Data("Incorrect Pin!");
				state = SM4_postReadIncorrect;
				memset(readByteOfData, 0, sizeof readByteOfData);
			}

			break;

		} 

		case SM4_postReadCorrect:
		{
			if (displayCounter == 30)
			{
				displayCounter = 0;
				state = SM4_preResetSys;

			} 

			break;

		}

		case SM4_postReadIncorrect:
		{
			if (displayCounter == 60)
			{
				displayCounter = 0;
				state = SM4_preResetSys;

			} 
			break;
		}
		case SM4_preResetSys:
		{
			state = SM4_resetSys;
			break;
		} 


		case SM4_resetSys:
		{
			unlockKeypad = 0;
			motionDetected = 0;
			PIRPause = 1;
			PIRreset = 1;
			pinEnteredCorrect = 0;
			N5110_clear();
			wdt_init();
			state = SM4_initDisplay;

			break;
		}

		default: state = SM4_initDisplay; break;
	}

	switch(state) // Actions
	{
		case SM4_initDisplay: break;
		case SM4_awaitMotion: break;
		case SM4_displayMotion: displayCounter++; break;
		case SM4_awaitPress: break;
		case SM4_displayEntry1: break;
		case SM4_awaitPress2: break;
		case SM4_displayEntry2:break;
		case SM4_awaitPress3: break;
		case SM4_displayEntry3: break;
		case SM4_awaitPress4: break;
		case SM4_displayEntry4: break;
		case SM4_preRead:
		{ 
			eeprom_read_block((void*)readByteOfData, 0, strlen(correct_pin)); 
			ptrKeyPadInputString = readByteOfData;
			break;
		}
		case SM4_read: break;
		case SM4_postReadCorrect:
		{ 
			displayCounter++;
		 	SM4_output = (SM4_output == 0x00) ? 0x02: 0x00;
		 	break;
		 }
		case SM4_postReadIncorrect:
		{ 
			displayCounter++;
		 	SM4_output = (SM4_output == 0x00) ? 0x01: 0x00;
		 	break;
		 }
		case SM4_preResetSys: break;
		case SM4_resetSys: break;
		default: break;
	}

	PORTC = (SM4_output & 0x03);

	return state;
}



// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main(void)
{
// Set Data Direction Registers
// Buttons PORTA[0-7], set AVR PORTA to pull down logic
DDRA = 0xF0; PORTA = 0x0F; // PC7..4 outputs init 0s, PC3..0 inputs init 1s
DDRB = 0xFF; PORTB = 0x00; // PORTB set to output, outputs init 0s
DDRC = 0x0F; PORTC = 0x10;
DDRD = 0xF0; PORTD = 0x0F;


// writing to EEPROM mem
memset(readByteOfData, 0, 10);
eeprom_busy_wait();
eeprom_write_block(correct_pin, 0, strlen(correct_pin));

// initializing Nokia LCD
SPI_Init();
N5110_init();


TCNT1 = 0;		/* Set timer1 count zero */
ICR1 = 2499;		/* Set TOP count for timer1 in ICR1 register */

/* Set Fast PWM, TOP in ICR1, Clear OC1A on compare match, clk/64 */
TCCR1A = (1<<WGM11)|(1<<COM1A1);
TCCR1B = (1<<WGM12)|(1<<WGM13)|(1<<CS10)|(1<<CS11);

// Period for the tasks
unsigned long int SMTick1_calc = 50;
unsigned long int SMTick2_calc = 10;
unsigned long int SMTick3_calc = 10;
unsigned long int SMTick4_calc = 100;

//Calculating GCD
unsigned long int tmpGCD = 1;
tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
tmpGCD = findGCD(tmpGCD, SMTick3_calc);
tmpGCD = findGCD(tmpGCD, SMTick4_calc);

//Greatest common divisor for all tasks or smallest time unit for tasks.
unsigned long int GCD = tmpGCD;

//Recalculate GCD periods for scheduler
unsigned long int SMTick1_period = SMTick1_calc/GCD;
unsigned long int SMTick2_period = SMTick2_calc/GCD;
unsigned long int SMTick3_period = SMTick3_calc/GCD;
unsigned long int SMTick4_period = SMTick4_calc/GCD;

//Declare an array of tasks 
static task task1, task2, task3, task4;
task *tasks[] = {&task1, &task2, &task3, &task4};
const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

// Task 1
task1.state = -1;//Task initial state.
task1.period = SMTick1_period;//Task Period.
task1.elapsedTime = SMTick1_period;//Task current elapsed time.
task1.TickFct = &SMTick1;//Function pointer for the tick.

//Task 2
task2.state = -1;//Task initial state.
task2.period = SMTick2_period;//Task Period.
task2.elapsedTime = SMTick2_period;//Task current elapsed time.
task2.TickFct = &SMTick2;//Function pointer for the tick.

//Task 3
task3.state = -1;//Task initial state.
task3.period = SMTick3_period;//Task Period.
task3.elapsedTime = SMTick3_period; // Task current elasped time.
task3.TickFct = &SMTick3; // Function pointer for the tick.

// Task 4
task4.state = -1;//Task initial state.
task4.period = SMTick4_period;//Task Period.
task4.elapsedTime = SMTick4_period; // Task current elasped time.
task4.TickFct = &SMTick4; // Function pointer for the tick.

// Set the timer and turn it on
TimerSet(GCD);
TimerOn();

unsigned short i; // Scheduler for-loop iterator
while(1) {
	// Scheduler code
	for ( i = 0; i < numTasks; i++ ) {
		// Task is ready to tick
		if ( tasks[i]->elapsedTime == tasks[i]->period ) {
			// Setting next state for task
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			// Reset the elapsed time for next tick.
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 1;
	}
	while(!TimerFlag);
	TimerFlag = 0;
}

// Error: Program should not exit!
return 0;
}


