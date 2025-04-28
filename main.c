/*
 * AtraccionDeFeria.c
 *
 * Created: 30/03/2025 18:34:49
 * Author : juanm & cloti
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#include "pinout.h"

#define setBit(port, bit) (port |= (1 << bit))
#define clrBit(port, bit) (port &= ~(1 << bit))

uint16_t t_total = 0;      // Tiempo total de parpadeo en decimas de segundo
uint16_t t_encendido = 0;  // Tiempo de parpadeo en decimas de segundo
volatile uint16_t parpadeo_cnt = 0;

// ISR del timer 4 (cadan 100ms)
ISR(TIMER4_COMPA_vect) {
    parpadeo_cnt++;
}

void parpadeo() {
    if (parpadeo_cnt < t_total) {
        if (parpadeo_cnt < t_encendido) {
            setBit(P_L4, B_L4);  // Encender LED 4
        } else {
            clrBit(P_L4, B_L4);  // Apagar LED 4
        }
    } else {
        parpadeo_cnt = 0;
    }
}

// Configurar tiempos de parpadeo en ms
void setParpadeo(uint16_t t_encendido_ms, uint16_t t_total_ms) {
    t_encendido = t_encendido_ms / 100;  // Convertir a decimas de segundo
    t_total = t_total_ms / 100;          // Convertir a decimas de segundo
}

void setup() {
    // Deshabilitar interrupciones
    cli();

    // Configurar LEDs como salida
    DDRK |= (1 << B_L3);
    DDRK |= (1 << B_L4);

    // Configurar motores como salida
    DDRK |= (1 << B_EN2);
    DDRK |= (1 << B_DI2);

    // Configurar interrupcion INT0 (flanco de subida)
    EICRA |= (1 << 2 * B_SW1) | (1 << (2 * B_SW1 + 1));

    // Habilitar mascara sensor mecanico
    EIMSK |= (1 << B_SW1);

    // Configurar interrupcion INT2 e INT3 (flanco de subida)
    EICRA |= (1 << 2 * B_SO4) | (1 << (2 * B_SO4 + 1));
    EICRA |= (1 << 2 * B_SO5) | (1 << (2 * B_SO5 + 1));

    // Habilitar mascara sensores opticos
    EIMSK |= (1 << B_SO4);
    EIMSK |= (1 << B_SO5);

    // Timer 4 en modo CTC (Top OCRnA)
    TCCR4A &= ~(1 << WGM40);
    TCCR4A &= ~(1 << WGM41);
    TCCR4B |= (1 << WGM42);
    TCCR4B &= ~(1 << WGM43);

    // Configurar mascara para preescalado de 64
    TCCR4B |= (1 << CS40);
    TCCR4B |= (1 << CS41);
    TCCR4B &= ~(1 << CS42);

    // Modificar TOP
    OCR4A = 12499;  // 125ns*12500*64=100ms

    // Configurar el registro de interrupcion
    TIMSK4 |= (1 << OCIE4A);  // Habilitar interrupcion por OCRA

    // Habilitar interrupciones
    sei();
}

int main(void) {
    setup();                 // Configuracion inicial
    setParpadeo(500, 1000);  // Encender 500ms y apagar 10000ms
    while (1) {
        parpadeo();  // Llamar a la funcion de parpadeo
    }
}
