#include "adc.h"

void ADC_Init(void) {
    ADCON0 = 0x01;     
    ADCON1 = 0x0E;     
    ADCON2 = 0xA9;    
}   

unsigned int ADC_Read(void) {
 ADCON0bits.GO = 1;              
 while (ADCON0bits.GO);          
  return ((unsigned int )(ADRESH << 8) | ADRESL); 
}
