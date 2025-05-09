#ifndef PINOUT_H
#define PINOUT_H

// Funciones para manipular los bits
#define setBit(port, bit) (port |= (1 << bit))
#define clrBit(port, bit) (port &= ~(1 << bit))
#define toggleBit(port, bit) (port ^= (1 << bit))

// Puertos y bits de los motores
#define P_EN1 PORTL
#define B_EN1 5
#define P_BK1 PORTL
#define B_BK1 6
#define P_EN2 PORTK
#define B_EN2 2
#define P_DI2 PORTK
#define B_DI2 3

// Puertos y bits de los sensores opticos
#define P_SO2 PORTL
#define B_SO2 0
#define P_SO3 PORTL
#define B_SO3 1
#define P_SO4 PORTD
#define B_SO4 2
#define P_SO5 PORTD
#define B_SO5 3

// Puertos y bits de los sensores mecanicos
#define P_SW1 PORTD
#define B_SW1 0
#define P_SW2 PORTD
#define B_SW2 1
#define PIN_SW2 PIND
#define P_SW3 PORTK
#define B_SW3 4

// Puertos y bits de los LEDs
#define P_L1 PORTL
#define B_L1 7
#define P_L2 PORTL
#define B_L2 4
#define P_L3 PORTK
#define B_L3 0
#define P_L4 PORTK
#define B_L4 1

#endif  // PINOUT_H
