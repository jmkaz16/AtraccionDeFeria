#ifndef TARJETEROLECTURA_H
#define TARJETEROLECTURA_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>


// #define SIZE 64 //Hay que cambiarlo a 28
// #define NUM_CARACTERES 7
// #define MAX_TARJETAS 10

void setup();
uint8_t tarjeta_valida(const char* t);
void gestionar_tarjeta();
void comparar_tarjeta(const char* usuario, const char* leida);
void convertidor_bits_a_numero(volatile uint8_t *vector, char *tarjeta);
void procesar_tarjeta();
ISR(TIMER1_CAPT_vect);
ISR(TIMER1_OVF_vect);
ISR(TIMER2_COMPA_vect);


#endif //TARJETEROLECTURA_H