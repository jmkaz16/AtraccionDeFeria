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
uint16_t last_encendido = 0;
uint16_t last_total = 0;

volatile uint64_t millis100_cnt = 0;  // Contador de 100ms

// ISR del timer 4 (millis100)
ISR(TIMER4_COMPA_vect) {
    millis100_cnt++;
}

uint64_t millis100() {
    return millis100_cnt;
}

void parpadeo() {
    //if (millis100() - last_total >= t_total) {
        //last_total = millis100();
    //} else {
        //if (millis100() - last_encendido >= t_encendido) {
            //last_encendido = millis100();
            //clrBit(P_L4, B_L4);  // Apagar LED 4
        //} else {
			//setBit(P_L4, B_L4);  // Encender LED 4
        //}
    //}
	// Si t_encendido == t_total, mantener encendido siempre
	if (t_encendido >= t_total) {
		setBit(P_L4, B_L4);
		return;
	}

	// Si se completó un ciclo, reiniciamos los tiempos
	if (millis100() - last_total >= t_total) {
		last_total = millis100();
		last_encendido = millis100();
	}

	// Determinar si estamos dentro del tiempo de encendido o apagado
	if (millis100() - last_encendido < t_encendido) {
		setBit(P_L4, B_L4);  // Encender LED
		} else {
		clrBit(P_L4, B_L4);  // Apagar LED
	}
}

void setParpadeo(uint16_t t_encendido_ms, uint16_t t_total_ms) {
    t_encendido = t_encendido_ms/100;  // Convertir a decimas de segundo
    t_total = t_total_ms/100;          // Convertir a decimas de segundo
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

    // Configurar interrupciones PCINT
    PCICR |= (1 << PCIE0);

    // Habilitar mascaras sensores opticos
    PCMSK0 |= (1 << B_SO3);
    PCMSK0 |= (1 << B_SO4);

    // Configurar interrupcion INT0 (flanco de subida)
    EICRA |= (1 << 2 * B_SW1) | (1 << (2 * B_SW1 + 1));

    // Habilitar mascara sensor mecanico
    EIMSK |= (1 << B_SW1);

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
    setup();                  // Configuracion inicial
    setParpadeo(10, 100);  // Encender 500ms y apagar 10000ms
    while (1) {
        parpadeo();  // Llamar a la funcion de parpadeo
		
    }
}

