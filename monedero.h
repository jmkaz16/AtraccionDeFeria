#ifndef MONEDERO_H
#define MONEDERO_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "pinout.h"
#include "tarjetero.h"

#define MONEDA10_MIN 1.09
#define MONEDA10_MAX 1.20

#define MONEDA20_MIN 1.20
#define MONEDA20_MAX 1.295

#define MONEDA50_MIN 1.355
#define MONEDA50_MAX 1.47

#define MONEDA100_MIN 1.305
#define MONEDA100_MAX 1.34

extern volatile uint8_t personas_cnt;

void monederoSetup();
void monedero();
void clrMonedero();

ISR(INT1_vect);

ISR(TIMER4_CAPT_vect);
ISR(TIMER4_COMPB_vect);
ISR(TIMER5_CAPT_vect);

#endif  // MONEDERO_H
