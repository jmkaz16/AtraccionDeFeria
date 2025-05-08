#ifndef ATRACCION_H
#define ATRACCION_H

#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "pinout.h"

extern volatile bool emergencia_flag;  // Bandera de emergencia

void atraccion_setup();  // Configuracion de entradas, salidas, interrupciones y temporizadores de la atraccion
void atraccion();        // Funcion principal de la atraccion (consulta periodica en el main)

void setAtraccion();    // Activar la atraccion
void clrAtraccion();    // Desactivar la atraccion
void moverAtraccion();  // Mover la atraccion

void setParpadeo(uint16_t t_encendido_ms, uint16_t t_total_ms);  // Configurar tiempos de parpadeo en ms
void parpadeo();                                                 // Funcion para activar parpadeo del LED 4

#endif  // ATRACCION_H