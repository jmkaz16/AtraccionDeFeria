#ifndef TARJETEROLECTURA_H
#define TARJETEROLECTURA_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define SIZE 64 //Hay que cambiarlo a 28
#define GLITCH 10000  //Ajustable con el osciloscopio
#define NUM_CARACTERES 7
#define MAX_TARJETAS 10

void setup();
void tarjeta_leida();
uint8_t gestionar_tarjeta();
void convertidor_bits_a_numero(volatile uint8_t *vector, char *tarjeta);
ISR(TIMER1_CAPT_vect);
ISR(TIMER2_COMPA_vect);

#endif //TARJETEROLECTURA_H