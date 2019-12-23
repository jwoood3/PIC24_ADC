# PIC24_ADC
ADC code to be used with the PIC24FJ64GA004 microcontroller, AQM0802A-RN-GBW LCD, and ST7032 controller chip.

This code is to be used with the PIC24FJ64GA004 microcontroller and AQM0802A-RN-GBW LCD for analog to digital conversion to implement a digital voltmeter. In this case, the analog input is a voltage, and the digital output is sent to the LCD to display the value of the voltage.

The analog input is set to the AN0 pin of the PIC24 and can be set up with a potentiometer so that the input voltage can easily be changed. A timer (timer3) is set up in the code to cause an AD interrupt to happen 16 times per second. In the AD interrupt, the resulting value from the analog to digital conversion is placed into a circular buffer. Every 100ms, an interrupt occurs from timer2 where the values in the circular buffer are averaged and converted to a string to be sent to the LCD using I2C communication protocol. The following functions in the code allow the data to be sent, along with the interrupt from timer2. The functions and how they work are as follows:

void lcd_init(void)  
This function initializes the lcd so that it is ready to receive the character data to display strings and characters. The contrast is adjustable through the variable called contrast. All of the setup happens by calling the lcd_cmd function to send the information for the proper settings to the lcd.

void lcd_setCursor(char x, char y)  
This function sets where the character will be printed on the LCD. If y%2 = 0, it will be printed on the top line, if y% is 1, it will be printed on the bottom line. If x is between 0 and 7, it will be printed in the corresponding column, where 0 is the first column and 7 is the last column. If x is not in that range, it will print in the x%8 column.

void lcd_printChar(char my Char)  
This function sends the character data to the lcd . It does this by sending a start bit, waiting for an acknowledge, sending the slave address and waiting for and acknowledge, sending the control byte and waiting for an acknowledge, sending the data byte (the number representing the character) and waiting for an acknowledge, and finally sending the stop bit to indicate that the process is complete.

void lcd_printStr(const char *s)  
This function prints out a string of characters by calling printChar for each character for the length of the string, and printing out each character in the column after the previous character was printed. This function also moves the string across the lcd and wraps back around to the front when it reaches the end. This is achieved by continuously printing the string and incrementing the column position of each character every time the string is printed. Because our setCursor function accounts for x values outside of the range of 0 to 7 by wrapping back around using the mod operator, the string will wrap back around to the front of the lcd when it reaches the end.

void lcd_cmd(char Package)  
This is used for the lcd_init function, and sends the setup data to the lcd. It does this in the same way that lcd_printChar works, except when the data byte is sent, instead of being data for a character to be printed, it is data to select the desired settings for the lcd.

void setup(void)  
This sets up the clock divider, sets the pins to digital, and sets pin RA0 to an output.









