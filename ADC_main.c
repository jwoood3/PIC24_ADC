/*
 * File:   woodx552_lab6_main_v001.c
 * Author: jeremywood
 *
 * Created on April 17, 2019, 1:41 PM
 */


#include "xc.h"
//#include "circularBuffer.h"
#include <p24Fxxxx.h>
#include <string.h>
#include <stdio.h> 
#include <math.h>
#include "woodx552_lab6_asmlib.h"

// PIC24FJ64GA002 Configuration Bit Settings
// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1          // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF        // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF          // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF           // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF        // JTAG Port Enable (JTAG port is disabled)

// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config POSCMOD = NONE           // Primary Oscillator Select (Primary oscillator disabled. 
					// Primary Oscillator refers to an external osc connected to the OSC1 and OSC2 pins)
#pragma config I2C1SEL = PRI       // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF       // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON       // OSC2/CLKO/RC15 functions as port I/O (RC15)
#pragma config FCKSM = CSECME      // Clock Switching and Monitor (Clock switching is enabled, 
                                       // Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL      // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

#define len1 16

//const int size = 1024; 
volatile long unsigned int buffer[len1]; //this could be 1024 but we changed it to 16
//volatile long unsigned int length = sizeof(buffer); 
volatile long unsigned int index = 0; 
volatile long unsigned int adValue = 0;

void setup(void) 
{
    CLKDIVbits.RCDIV = 0;
    AD1PCFG = 0x9ffe; //AN0 is set to analog (needs analog data info from potentiometer)
    TRISAbits.TRISA0 = 1; //AN0 is set to an input (takes input from potentiometer)
    
    //T1CON = 0x0030; //100ms
    //PR1 = 6249; 
    //TMR1 = 0; 
    //IFS0bits.T1IF = 0; 
    //T3CONbits.TON = 1; 
    //IEC0bits.T1IE = 0; 
    
    //setting up a 100ms timer
    T2CON = 0x0030; //1:256 prescaler
    PR2 = 6249; 
    IPC1bits.T2IP = 0b0011;
    TMR2 = 0; 
    IFS0bits.T2IF = 0; 
    //T3CONbits.TON = 1; 
    IEC0bits.T2IE = 1; 
    T2CONbits.TON = 1;
    
    //setting up a 62.5 ms timer
    T3CON = 0x0020; //1:64 prescaler 
    PR3 = 15624; //
    IPC2bits.T3IP = 0b0100;
    TMR3 = 0; 
    IFS0bits.T3IF = 0; 
    T3CONbits.TON = 1; 
    IEC0bits.T3IE = 0; 
    
    AD1CON1 = 0;
    AD1CON2 = 0;
    AD1CON3 = 0;
    AD1CHS = 0; 

    AD1CSSL = 0; 
    
    IFS0bits.AD1IF = 0;
    IEC0bits.AD1IE = 1; 
     
    AD1CON1bits.ADON = 0; //Make sure the A/D converter is off during setup
    AD1CON1bits.DONE = 0; //Conversion has not yet started
    AD1CON1bits.SAMP = 0; //A/D sample-and-hold amplifier is holding
    //AD1CON1bits.ASAM = 1; 
    
    AD1CON2bits.VCFG = 0b000; //Vr+ = AVdd, Vr- = AVss
    AD1CON2bits.SMPI = 0b0000; //Interrupts at the completion of conversion for each sample/convert sequence
    
    AD1CON3bits.ADRC = 0; //Clock derived from system clock
    AD1CON3bits.SAMC = 0b10100; //Auto-sample time = 20
    AD1CON3bits.ADCS = 0b00000001; //2*Tcy
    
    AD1CON1bits.ASAM = 1; //sampling begins immediately after last conversion completes; SAMP bit is automatically set
    AD1CON1bits.SSRC = 0b010; //using with timer 3
    
   // AD1CON1bits.SAMP = 0;
    AD1CON1bits.ADON = 1; //Turn on the A/D converter
  
   
}




//This function is used for the setup of the LCD
void lcd_cmd(char Package) 
{
    I2C2CONbits.SEN = 1;
    while (I2C2CONbits.SEN == 1);                //wait for SEN == 0
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = 0b01111100;             //8 bits consisting of the slave address and the R/nW bit
    while (IFS3bits.MI2C2IF == 0);    //wait for MI2C2IF to equal 1
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = 0b00000000;             //8 bits consisting of control byte
    while (IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = Package;                //8 bits consisting of the data byte
    while (IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    I2C2CONbits.PEN = 1;
    while (I2C2CONbits.PEN == 1);                //wait for PEN == 0
}

//Initializes the lcd by calling the lcd_cmd function to set various settings
//by sending different data bytes
void lcd_init(void) 
{
    
    I2C2CONbits.I2CEN = 0;
    I2C2BRG = 0x9D;         //Baud rate = 100k
    I2C2CONbits.I2CEN = 1;
    IFS3bits.MI2C2IF = 0;   //clear interrupt flag
    
    
    int i, j, k;
    for (i = 0; i < 50; i++) 
    {
        one_milli();
    }
    lcd_cmd(0b00111000);    //function set, normal instruction mode
    lcd_cmd(0b00111001);    //function set, extended instruction mode
    lcd_cmd(0b00000100);    //Entry mode set
    lcd_cmd(0b00010100);    //interval oscillator
    lcd_cmd(0b01110000);    //contrast C3-C0
    lcd_cmd(0b01011110);    //Ion, Bon, C5-C4
    lcd_cmd(0b01101100);    //follower control
    for (j = 0; j < 200; j++) 
    {
        one_milli();
    }
    lcd_cmd(0b00111000);    //function set, normal instruction mode
    lcd_cmd(0b00001100);    //display on
    lcd_cmd(0b00000001);    //clear display
    for (k = 0; k < 2; k++) 
    {
        one_milli();
    }    
}

//Sets the location of the cursor which determines where a character will be printed
void lcd_setCursor(char x, char y) 
{
    int coordinate;
    if (x < 0 || x > 7) 
    {
        x = x%8;
    }
    if (y < 0 || y > 1) 
    {
        y = y%2;
    }
    
    coordinate = 0x40*(y) + x;
    coordinate = coordinate | 0x80;
    lcd_cmd(coordinate);
    
}

//Prints the character that was sent to the lcd
void lcd_printChar(char myChar) 
{
    I2C2CONbits.SEN = 1;
    while (I2C2CONbits.SEN == 1);                //wait for SEN == 0
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = 0b01111100;             //8 bits consisting of the slave address and the R/nW bit
    while (IFS3bits.MI2C2IF == 0);    //wait for MI2C2IF to equal 1
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = 0b01000000;             //8 bits consisting of control byte
    while (IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    I2C2TRN = myChar;                //8 bits consisting of the data byte
    while (IFS3bits.MI2C2IF == 0);
    IFS3bits.MI2C2IF = 0;
    I2C2CONbits.PEN = 1;
    while (I2C2CONbits.PEN == 1);                //wait for PEN == 0
}

//Prints strings by calling printChar multiple times
void lcd_printStr(const char *s) 
{
  
    int length = strlen(s);
    
    int i, k;
    
    
    
        lcd_cmd(0b00000001);
        for (i = 0; i < length; i++) 
        {
            one_milli();
            lcd_setCursor(i,2);
            one_milli();
            lcd_printChar(s[i]);
            one_milli();
        }
        
        for (k = 0; k < 500; k++) 
        {
            one_milli();
        }
}

//ADC1 interrupt. Triggered every 62.5ms
void __attribute__((__interrupt__,__auto_psv__)) _ADC1Interrupt(void)
{
    putVal(ADC1BUF0); //calls putVal to put the value of ADC1BUF0 into the buffer
    _AD1IF = 0; 
}

//T2interrupt. Triggers every 100ms
void __attribute__((__interrupt__,__auto_psv__)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;
    //getAvg(); 
    
    adValue = getAvg(); //Gets the average of all of the buffer values and stores that in adValue
    char adStr[20]; //String used in printing to the lcd
 //
    sprintf(adStr, "%6.4f V", (3.3/1024)*adValue); //converts the floating point number to a string so it can be sent to the lcd
    
    lcd_printStr(adStr); //calls printStr to send the string to the lcd
    
}

//Puts values into the buffer (which is a circular buffer)
void putVal(int newValue)
{
    buffer[index] = newValue; //Puts the value from the A/D conversion into the buffer
    index++; //move to the next index in the buffer
    index = index % 16; 
}

//Averages all of the values in the buffer
int getAvg(void)
{
    int i; 
    int total = 0; 
    int average; 
    for(i = 0; i < len1; i++)
    {
        total = total + buffer[i];
    }
    average = total/len1; 
    return average; 
        
}

//Initializes the buffer by setting all values to 0
void initBuffer(void)
{
    int i; 
     
    for(i = 0; i < len1; i++)
    {
        buffer[i] = 0; 
    }
    
}


int main(void) 
{
    
    setup();
    lcd_init();
    initBuffer(); 
    T3CONbits.TON = 1;
    
    while(1) 
    {
      
    }
    
    return 0;
}