[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-2e0aaae1b6195c2367325f4f02e2d04e9abb55f0b24a779b69b11b9e10269abc.svg)](https://classroom.github.com/online_ide?assignment_repo_id=19508713&assignment_repo_type=AssignmentRepo)
# Lab04: Comunicación UART en PIC18F45K22

## Integrantes
[Juan David Rojas Diaz](https://github.com/JuanRojas15)

[Carlos Andres Rojas Diaz](https://github.com/CarlosRojas05)

## Documentación

### UART: comunicación serie asíncrona
UART (Universal Asynchronous Receiver/Transmitter) es un protocolo serie sencillo que permite el intercambio de datos bit a bit sin señal de reloj común. En UART no hay línea de reloj compartida; en su lugar, emisor y receptor acuerdan previamente una tasa de baudios (velocidad). Los datos se envían en paquetes con un bit de inicio, varios bits de datos (comúnmente 8), opcionalmente un bit de paridad y uno o más bits de parada. Este formato asegura sincronización en cada byte sin emplear reloj externo. Un esquema típico es “8N1” (8 bits de datos, sin paridad, 1 bit de parada). La UART es full dúplex (transmisión y recepción independientes) usando solo dos líneas (TX, RX) más tierra común.

### Comunicación síncrona vs asíncrona
- **Síncrona (SPI, I²C):** Existe una línea de reloj separada para alinear los datos.  
  - SPI usa al menos cuatro líneas (CLK, MOSI, MISO, SS) y permite velocidades muy altas con full dúplex simultáneo.  
  - I²C usa dos líneas (SCL, SDA) compartidas por múltiples dispositivos con direccionamiento interno.
- **Asíncrona (UART):** Solo necesita las líneas de datos TX y RX. Emplea tramas con bits de inicio/parada para sincronización, pero requiere que ambos extremos trabajen a la misma velocidad (típicamente dentro de ±10 % en baudios). Ideal para comunicaciones punto a punto de baja o mediana velocidad.

### Módulo EUSART del PIC18F45K22 en modo UART
El PIC18F45K22 incorpora un módulo EUSART que soporta modos síncrono y asíncrono. En modo UART (asíncrono) se utilizan estos registros principales:
- **TXREG:** Buffer de transmisión donde la CPU escribe el dato a enviar.  
- **RCREG:** Buffer de recepción donde se almacena cada byte recibido.  
- **SPBRG / SPBRGH:** Configuran el divisor del generador de baudios, controlando la velocidad de comunicación.  
- **TXSTA (Transmit Status and Control):** Controla el transmisor. Bits clave: TXEN (habilita TX), SYNC (0 = asíncrono), BRGH (alta/baja velocidad) y TRMT (indica TXREG vacío).  
- **RCSTA (Receive Status and Control):** Controla el receptor. Bits clave: SPEN (habilita puerto serie), CREN (recepción continua), FERR (error de framing) y OERR (overrun).  
- **BAUDCON:** Control avanzado del generador de baudios. Incluye BRG16 (modo 16 bits) y WUE (wake-up).

**Secuencia de inicialización típica**  
1. Calcular y cargar SPBRG/SPBRGH según Fosc, BRGH y BRG16.  
2. Poner SYNC = 0 para modo asíncrono.  
3. Habilitar puerto serie con SPEN.  
4. Habilitar transmisor (TXEN = 1) y receptor (CREN = 1).

### Buffers y registro de desplazamiento interno
El EUSART dispone de un registro de desplazamiento interno (TSR para transmisión, RSR para recepción) aparte de TXREG y RCREG:
- Al escribir en TXREG, si el TSR está libre, el dato se transfiere al TSR y comienza el envío bit a bit por la línea TX.  
- Mientras el TSR desplaza un byte, TXREG puede cargarse con el siguiente, lo que permite flujo continuo de datos.  
- En recepción, el RSR captura un byte completo y lo pasa a RCREG, activando una bandera de recepción. RCREG tiene doble buffer, lo que evita pérdida de datos si se lee con cierta frecuencia.

### Manejo por interrupciones
El EUSART genera flags e interrupciones para eventos de transmisión y recepción:
- **TXIF:** Indica que TXREG está vacío y listo para un nuevo dato.  
- **RCIF:** Indica que RCREG contiene un dato recibido.  
Para usar interrupciones, se habilitan TXIE/RCIE y luego las interrupciones globales. La rutina de servicio puede leer RCREG o cargar TXREG de forma automática al ocurrir cada evento.

###
Este repositorio incluye tres archivos para implementar comunicación UART en el PIC18F45K22:  
- `main.c` → Punto de entrada y bucle principal.  
- `uart.c`  → Definición de funciones de inicialización y envío.  
- `uart.h`  → Declaración de las funciones UART.

---

## 1) main.c

```c
#include <xc.h>
#include "uart.h"

#pragma config FOSC = INTIO67  // Oscilador interno
#pragma config WDTEN = OFF     // Watchdog Timer apagado
#pragma config LVP = OFF       // Low Voltage Programming off

void main(void) {
    OSCCON = 0b01110000;  // Configura el oscilador interno a 16 MHz
    UART_Init();          // Inicializa la UART (baudrate, pines, modo asíncrono) :contentReference[oaicite:0]{index=0}

    while (1) {
        UART_WriteString("Hola, UART funcionando!\r\n");  // Envía la cadena por UART :contentReference[oaicite:1]{index=1}
        __delay_ms(1000);                               // Pausa 1 s antes de repetir
    }
}
```
### Explicación de `main.c`

1. **Inclusión de librerias**  
   - `<xc.h>` aporta las definiciones de registros y bits específicos del PIC18F45K22.  
   - `"uart.h"` declara las funciones del módulo UART.

2. **Pragmas de configuración**  
   - `FOSC = INTIO67`: usa el oscilador interno y libera RA6/RA7 como pines de I/O.  
   - `WDTEN = OFF`: desactiva el Watchdog Timer para evitar reinicios automáticos.  
   - `LVP = OFF`: deshabilita programación en bajo voltaje y libera RB3.

3. **Ajuste de frecuencia**  
   - `OSCCON = 0b01110000;` fija el oscilador interno en 16 MHz. Esto es clave para que el cálculo del baud rate sea correcto.

4. **Inicialización de UART**  
   - `UART_Init();` configura el módulo EUSART en modo asíncrono a 9600 bps, habilita TX/RX e interrupciones.

5. **Bucle principal**  
   - Dentro de `while(1)` se envía la cadena `"Hola, UART funcionando!\r\n"` cada segundo.  
   - `__delay_ms(1000);` genera una pausa de 1000 ms usando la definición `_XTAL_FREQ` de 16 MHz.
     
## 2) uart.c

```c
#include "uart.h"
#include <stdio.h>


void UART_Init(void) {
    TRISC6 = 0; // TX como salida
    TRISC7 = 1; // RX como entrada

    SPBRG1 = 25; // Baudrate 9600 para Fosc = 16MHz
    TXSTA1bits.BRGH = 0; // Baja velocidad (BRGH = 0)
    BAUDCON1bits.BRG16 = 0; // 8 bits del generador de baudrate

    RCSTA1bits.SPEN = 1; // Habilita el módulo serial
    TXSTA1bits.SYNC = 0; // Modo asíncrono
    TXSTA1bits.TXEN = 1; // Habilita transmisión
    RCSTA1bits.CREN = 1; // Habilita recepción

    
    PIE1bits.RC1IE = 1;   // Habilita interrupción por recepción UART
    PIR1bits.RC1IF = 0;   // Limpia bandera
    INTCONbits.PEIE = 1;  // Habilita interrupciones periféricas
    INTCONbits.GIE = 1;   // Habilita interrupciones globales
}

void UART_WriteChar(char data) {
    while (!TXSTA1bits.TRMT); // Espera a que se vacíe el buffer de transmisión
    TXREG1 = data;
}

void UART_WriteString(const char* str) {
    while (*str) {
        UART_WriteChar(*str++);
    }
}

```
### Explicación de `uart.c`

1. **UART_Init()**  
   - Configura los pines RC6/TX (salida) y RC7/RX (entrada).
   - Ajusta el divisor de baudios para 9600 bps según la frecuencia de 16 MHz.
   - Habilita el módulo EUSART en modo asíncrono, así como el transmisor y receptor
   - Activa interrupciones para recepción de datos.

2. **UART_WriteChar(char data)**  
   - Espera hasta que el hardware haya terminado de enviar el carácter anterior.
   - Carga el nuevo byte en el registro de transmisión para enviarlo. 

3. **UART_WriteString(const char* str)**  
   - Recorre la cadena hasta el carácter nulo '\0'.
   - Envía cada letra mediante llamadas sucesivas a UART_WriteChar().

## 2) uart.h

```c
#ifndef UART_H
#define UART_H

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 16000000UL    // Frecuencia del oscilador interno

void UART_Init(void);
void UART_WriteChar(char data);
void UART_WriteString(const char* str);
void UART_WriteUInt(uint16_t value);
void UART_WriteVoltage(uint16_t adcValue);

#endif // UART_H

```
### Explicación de `uart.h`

1. **UART_Init()**  
   - Incluye <xc.h> para acceso a registros del PIC y <stdint.h> para tipos estándar.
   - Define _XTAL_FREQ para que las funciones de retardo (__delay_ms) sepan la frecuencia usada.
   - Declara las funciones de inicialización y transmisión de datos por UART.
   - Incluye prototipos adicionales (UART_WriteUInt, UART_WriteVoltage) para futuras ampliaciones.





## Implmentación


