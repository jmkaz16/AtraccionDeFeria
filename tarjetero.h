#ifndef TARJETERO_H
#define TARJETERO_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <string.h>

#include "pinout.h"

#define NUM_TIEMPOS 54
#define NUM_BITS 24
#define NUM_CARACTERES 6
#define MAX_TARJETAS 10
#define INDEX_TABLA 16

extern volatile uint8_t personas_cnt;

void tarjeteroSetup();
ISR(TIMER1_CAPT_vect);
ISR(TIMER1_OVF_vect);
ISR(TIMER3_COMPA_vect);
uint32_t millis();
void procesarTarjeta();
void conversorBits2Numero(volatile uint8_t* vector, char* tarjeta);
uint8_t tarjetaValida(const char* t);
void gestionarTarjeta();

#endif  // TARJETERO_H