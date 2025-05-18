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

### Comparación con SPI e I²C
- **UART vs SPI:**  
  - SPI es síncrono full-dúplex con reloj externo y alta velocidad.  
  - UART es asíncrono, más lento, pero solo usa dos líneas y es más simple de implementar.
- **UART vs I²C:**  
  - I²C es síncrono bidireccional en dos hilos, permite múltiples dispositivos con direccionamiento y velocidades hasta varios MHz.  
  - UART es punto a punto, sin direccionamiento interno, y depende de bits de inicio/parada en cada trama.

### Aplicaciones prácticas del UART
- **Depuración y monitoreo** de datos de sensores o eventos en PC mediante convertidores USB‑serie.  
- **Módulos inalámbricos y GPS** (por ejemplo, HC‑05, ESP32 en modo AT, receptores GNSS).  
- **Sensores o periféricos** que ofrecen salida UART (RFID, módulos de telemetría).  
- **Protocolos industriales** como Modbus RTU sobre RS‑485.  
- **Bootloaders y actualizaciones de firmware** en sistemas embebidos.  
- **Educación y prototipado**, por su sencillez y soporte nativo en la mayoría de microcontroladores.


## Implmentación


