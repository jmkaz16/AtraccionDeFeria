#ifndef COMUNICACION_H
#define COMUNICACION_H

#include <avr/interrupt.h>
#include <avr/io.h>

#include "pinout.h"

#define F_CPU 8000000UL  // Frecuencia del reloj del microcontrolador
#define BAUD 9600
#define UBRR F_CPU / 16 / BAUD - 1

void uartSetup();    // Configura el UART
void uartReceive();  // Recibe un caracter por UART
void decodeData();   // Decodifica el dato recibido por UART

#endif  // COMUNICACION_H