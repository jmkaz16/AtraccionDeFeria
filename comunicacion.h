#ifndef COMUNICACION_H
#define COMUNICACION_H

#include <avr/interrupt.h>
#include <avr/io.h>

#include "pinout.h"

#define F_CPU 8000000UL  // Frecuencia del reloj del microcontrolador
#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

void uartSetup();    // Configura el UART
char uartReceive();  // Recibe un caracter por UART
void decodeData(char data);   // Decodifica el dato recibido por UART

#endif  // COMUNICACION_H