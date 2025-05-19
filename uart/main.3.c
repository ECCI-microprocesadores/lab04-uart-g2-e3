#include <xc.h>
#include "uart.h"
#include "adc.h"
#include <stdio.h>

#pragma config FOSC = INTIO67  
#pragma config WDTEN = OFF     
#pragma config LVP = OFF       

void main(void) {
    OSCCON = 0b01110000;  
    UART_Init();          
    ADC_Init();           

    uint16_t valor, mV;
    uint8_t volts, deci;

    while (1) {
        valor = ADC_Read();                
        // convierto a milivoltios
        mV = (valor * 5000UL) / 1023;      

        // separo parte entera y decimal (un dígito)
        volts = mV / 1000;                 // ej. 5000/1000 = 5
        deci  = (mV % 1000) / 100;         // ej. (5000%1000)/100 = 0
        printf("Voltaje: %u.%u V\r\n", volts, deci);

        __delay_ms(1000);
    }
}
