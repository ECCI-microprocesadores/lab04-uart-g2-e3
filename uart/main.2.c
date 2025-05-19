#include <xc.h>
#include "uart.h"
#include "adc.h"
#include <stdio.h>

#pragma config FOSC = INTIO67  // Oscilador interno
#pragma config WDTEN = OFF     // Watchdog Timer apagado
#pragma config LVP = OFF       // Low Voltage Programming off

void main(void) {
    OSCCON = 0b01110000;  // Oscilador interno a 16MHz
    UART_Init();          // Inicializa UART (9600bps)
    ADC_Init();           // Inicializa ADC

    uint16_t valor;
    while (1) {
        valor = ADC_Read();        // 0–1023
        printf("%u\r\n", valor);   // convierte a ASCII y envía por UART
        __delay_ms(1000);
    }
}
