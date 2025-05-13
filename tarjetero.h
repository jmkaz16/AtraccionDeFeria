#ifndef TARJETERO_H
#define TARJETERO_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <string.h>

#include "pinout.h"

#define NUM_TIEMPOS 54    // numero de flancos a leer en la tarjeta
#define NUM_BITS 24       // numero de bits en la tarjeta
#define NUM_CARACTERES 6  // numero de caracteres en la tarjeta
#define MAX_TARJETAS 20   // numero maximo de tarjetas a guardar
#define INDEX_TABLA 16    // numero de caracteres en la tabla de codigos

extern volatile uint8_t personas_cnt;

void tarjeteroSetup();                                               // configuracion de entradas, salidas, interrupciones y temporizadores de tarjetero
ISR(TIMER1_CAPT_vect);                                               // ISR del input capture del TIMER1
ISR(TIMER1_OVF_vect);                                                // ISR del desbordamiento del TIMER1
ISR(TIMER3_COMPA_vect);                                              // ISR del TIMER3 para el millis
uint32_t millis();                                                   // funcion que devuelve el tiempo en milisegundos desde que se inicializo el timer
void procesarTarjeta();                                              // funcion que procesa la tarjeta leida
void conversorBits2Numero(volatile uint8_t* vector, char* tarjeta);  // funcion que convierte los bits de vector bit en la cadena de caracteres
uint8_t tarjetaValida(const char* tarjeta_valida);                   // funcion que verifica si la tarjeta es valida
void gestionarTarjeta();                                             // funcion que gestiona la tarjeta leida y la compara con la lista de usuarios
void actualizarLED();												 // funcion que se llama periodicamente en el main y que gestiona el estado del led: apagado, encendido o parpadeando
uint8_t compararTarjetas(const char* t1, const char* t2);			 // funcion que recorre el vector de usuarios para ver si la nueva tarjeta esta ya registrada o no							
void copiarTarjeta(char* destino, const char* origen);				 // funcion que copia la nueva tarjeta en el vector usuarios porque es nueva
#endif  // TARJETERO_H