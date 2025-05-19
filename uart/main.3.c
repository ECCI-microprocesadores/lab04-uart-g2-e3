#include <xc.h>
#include "pwm.h"
#pragma config FOSC = INTIO67      
#pragma config PLLCFG = ON         
#pragma config WDTEN = OFF         

// -------- Main --------
    void main(void) {
    OSCCON = 0b01110000;       
    OSCTUNEbits.PLLEN = 1;     
    
    setupPWM();                    
    setupTimer0();                 

    while (1) {
        
    }
}
