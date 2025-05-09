#ifndef ATRACCION_H
#define ATRACCION_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "pinout.h"

#define T_ATRACCION_MAX 20 * 60  // Tiempo maximo de duracion de la atraccion en decimas de segundo
#define T_ATRACCION_MIN 300      // Tiempo minimo de duracion de la atraccion en decimas de segundo

#define T_ANTIRREBOTES 2  // Tiempo de antirrebotes en milisegundos

#define PERSONAS_MAX 4  // Maximo de personas en la atraccion
#define T_ESPERA 10    // Tiempo de espera en decimas de segundo (10)

#define T_SUBIDA_MIN 600   // Tiempo de subida minimo en milisegundos en la fase 0
#define T_SUBIDA_F1 750    // Tiempo de subida minimo en milisegundos en la fase 1
#define T_DIENTES 5        // Tiempo por diente en milisegundos en la fase 0
#define T_DIENTES_F1 0.5 // Tiempo por diente en milisegundos en la fase 1

extern volatile bool emergencia_flag;  // bandera de emergencia

void atraccionSetup();  // configuracion de entradas, salidas, interrupciones y temporizadores de la atraccion
void atraccion();       // funcion principal de la atraccion (consulta periodica en el main)

void setAtraccion();    // activar la atraccion
void clrAtraccion();    // desactivar la atraccion
void moverAtraccion();  // mover la atraccion

void setParpadeo(uint16_t t_encendido_ms, uint16_t t_total_ms);  // configurar tiempos de parpadeo en ms
void parpadeo();                                                 // funcion para activar parpadeo del LED 4

ISR(INT0_vect);    // ISR del pulsador de emergencia SW1 (INT0)
ISR(INT2_vect);    // ISR del sensor optico 4 (INT2)
ISR(INT3_vect);    // ISR del sensor optico 5 (INT3)
ISR(PCINT2_vect);  // ISR del sensor mecánico SW3 (PCINT20)

ISR(TIMER0_COMPA_vect);  // ISR del temporizador 0 (Timer 0) (cada 1ms)
ISR(TIMER4_COMPA_vect);  // ISR del temporizador 4 (Timer 4) (cada 100ms)

#endif  // ATRACCION_H