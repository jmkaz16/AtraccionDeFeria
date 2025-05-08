#ifndef TARJETERO_H
#define TARJETERO_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <string.h>

// #define SIZE 64 //Hay que cambiarlo a 28
// #define NUM_CARACTERES 7
// #define MAX_TARJETAS 10

extern volatile uint8_t personas_cnt;

uint32_t millis();

void tarjeteroSetup();
uint8_t tarjetaValida(const char* t);
void gestionarTarjeta();
void compararTarjeta(const char* usuario, const char* leida);
void conversorBits2Numero(volatile uint8_t* vector, char* tarjeta);
void procesarTarjeta();

ISR(TIMER1_CAPT_vect);
ISR(TIMER1_OVF_vect);
ISR(TIMER3_COMPA_vect);

#endif  // TARJETERO_H