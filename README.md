# PIC24_ADC
ADC code to be used with the PIC24FJ64GA004 microcontroller and AQM0802A-RN-GBW LCD

This code is to be used with the PIC24FJ64GA004 microcontroller and AQM0802A-RN-GBW LCD for analog to digital conversion to implement a digital voltmeter. In this case, the analog input is a voltage, and the digital output is sent to the LCD to display the value of the voltage.

The analog input is set to the AN0 pin of the PIC24 and can be set up with a potentiometer so that the input voltage can easily be changed. A timer (timer3) is set up in the code to cause an AD interrupt to happen 16 times per second. In the AD interrupt, the resulting value from the analog to digital conversion is placed into a circular buffer. Every 100ms, an interrupt occurs from timer2 where the values in the circular buffer are averaged and converted to a string to be sent to the LCD. 
