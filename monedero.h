#ifndef MONEDERO_H
#define MONEDERO_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "pinout.h"
#include "tarjetero.h"

extern volatile uint8_t personas_cnt;

void monederoSetup();
void monedero();

ISR(INT1_vect);

ISR(TIMER4_CAPT_vect);
ISR(TIMER4_COMPB_vect);
ISR(TIMER5_CAPT_vect);

#endif  // MONEDERO_H
