/*analog to digital conversion */

#include<stdio.h> 
#include<AT89x051.h>
#include<intrins.h>
#include<string.h>

//#define SERIAL_MODE

static void wait (unsigned int time);
static unsigned char ReadInstrReg (void);
static void WriteInstrReg (unsigned char Instr);
static unsigned char ReadDataReg (void);
static void WriteDataReg (unsigned char val);
static void serial_init(void);

sbit P20_PIN = P3^0; 				// RS Pin for P3.0 for 89c51
sbit P21_PIN = P3^1; 				// RW Pin for P3.1	for 89c51
sbit P22_PIN = P3^7; 				// EN Pin for P3.7 for 89c51

#define LCD_DATA	P1			  	// P1 for 89c51

#define LINE1_ADDR 0x00 			// Start of line 1 in the DD-Ram
#define LINE2_ADDR 0x40 			// Start of line 2 in the DD-Ram

/* Display-Commands */
#define CLEAR_DISPLAY 		0x01 	// Clear entire display and set Display Data Address to 0
#define DD_RAM_PTR 			0x80 	// Address Display Data RAM pointer
#define DISP_INIT 			0x38 	// 8 bit 2 lines
#define INC_MODE 			0x06 	// Display Data RAM pointer incremented after write
#define DISP_ON				0x0C	// Display On, Blink Off

sbit BUSY = P3^2;					/* Busy flag of 7135 */
sbit CLK  = P3^4;
unsigned int count;

void main(void)
{
	#ifdef SERIAL_MODE
	serial_init();
	printf("TEMP. MESUREMENT\r\n");
	#else
	WriteInstrReg(0x30);
	wait(4000);

	WriteInstrReg(0x30);
	wait(100);
	
	WriteInstrReg(0x30);
//	wait(100);

	WriteInstrReg(DISP_INIT);			 //0x38
	WriteInstrReg(INC_MODE);			 //0x06
	WriteInstrReg(DISP_ON);				 //0x0C
	WriteInstrReg(CLEAR_DISPLAY);		 //0x01
	WriteInstrReg(DD_RAM_PTR);			 //0x80
	printf("TEMP. MESUREMENT"); 
	#endif

	TMOD |= 0x0D;
	TR0 = 1;
	while(1)
	{
		TH0 = 0;
		TL0 = 0;
		while(!BUSY);
		while(BUSY);
		count = TH0;
		count <<= 8;
		count |= TL0;

		#ifndef	SERIAL_MODE
		/* Single Line 16x1 LCD  */
//		WriteInstrReg(DD_RAM_PTR);
//		printf("VALUE : \n%-8.0u", count-10001);

		/* Double Line 16x2 LCD  */
		WriteInstrReg(DD_RAM_PTR | LINE2_ADDR);
		printf("VALUE : %-8.0u", count-10001);
		#else
		printf("VALUE : %-5.0u\r", count-10001);
		#endif
	}
}
#ifdef SERIAL_MODE
void serial_init(void)
{
   	SCON = 0x50;
   	TMOD = 0x20;
   	TH1  = 0xFD;
	TL1  = 0xFD;
   	TR1  = 1;
   	TI   = 1;
}

#else

/*
* ReadInstrReg: Read from Instruction Register of LCD Display Device
*/

static unsigned char ReadInstrReg (void) {
	unsigned char Instr;
	LCD_DATA = 0xff; 				// DATA PORT is input
	P20_PIN = 0; 					// select instruction register
	P21_PIN = 1; 					// read operation
	P22_PIN = 1; 					// give operation start signal
	_nop_ (); _nop_ (); 			// wait
	Instr = LCD_DATA; 				// read Instruction
	P22_PIN = 0;
	return(Instr);
}

/*
* WriteInstrReg: Write to Instruction Register of LCD Display Device
*/
static void WriteInstrReg (unsigned char Instr) {
	while(ReadInstrReg() & 0x80);
	P20_PIN = 0; 				// select instruction register
	P21_PIN = 0; 				// write operation
	P22_PIN = 1; 				// give operation start signal
	_nop_ (); _nop_ (); 		// wait
	LCD_DATA = Instr; 			// write instruction
	P22_PIN = 0;
	LCD_DATA = 0xff;			// DATA_PORT is input [prevent I/O-Port from damage]
}

/*
* ReadDataReg: Read from Data Register of LCD Display Device
Application Note
Interface and Simulation of a LCD Text Display APNT_161
Page 3 of 6 Revision date: 26-Jun-01
*/

static unsigned char ReadDataReg (void) {
	unsigned char val;
	while(ReadInstrReg() & 0x80);
	LCD_DATA = 0xff; 				// DATA_PORT is input
	P20_PIN = 1; 					// select data register
	P21_PIN = 1; 					// read-operation
	P22_PIN = 1; 					// give operation start signal
	_nop_ (); _nop_ (); 			// wait
	val = LCD_DATA; 				// read Instruction
	P22_PIN = 0;
	return (val);
}

/*
* WriteDataReg: Write to Data Register of LCD Display Device
*/

static void WriteDataReg (unsigned char val) {
	while(ReadInstrReg() & 0x80);
	P20_PIN = 1; 					// select data register
	P21_PIN = 0; 					// write operation
	P22_PIN = 1; 					// give operation start signal
	_nop_ (); _nop_ (); 			// wait
	LCD_DATA = val; 				// write value
	P22_PIN = 0;
	LCD_DATA = 0xff; 				// DATA_PORT is input [prevent I/O-Port from demage]
}

char putchar (char c) {
	unsigned char line;
	if (c == '\n') {
		line = ReadInstrReg ();
		if (line & LINE2_ADDR) { 						// is already in line 2
			WriteInstrReg (LINE1_ADDR | DD_RAM_PTR); 	// place to start of line 1
		}
		else {
			WriteInstrReg (LINE2_ADDR | DD_RAM_PTR); 	// place to start of line 2
		}
	}
	else {
		WriteDataReg (c);
	}
	return (c);
}

/*
* Waits for some time so the 7066 can do the last command
*/

void wait (unsigned int time) {
	while(time--);
}

/*
* Clear_Display: Write 0x20 to Display RAM
*/
static void Clear_Display (void) {
	WriteInstrReg (CLEAR_DISPLAY);
	wait (1);
}
#endif
