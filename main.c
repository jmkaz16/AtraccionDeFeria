/*
 * AtraccionDeFeria.c
 *
 * Created: 30/03/2025 18:34:49
 * Author : juanm & cloti
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "pinout.h"

#define setBit(port, bit) (port |= (1 << bit))
#define clrBit(port, bit) (port &= ~(1 << bit))

// Hay que modificar las constantes con defines para organizar el codigo

uint16_t t_total = 0;                // Tiempo total de parpadeo en decimas de segundo
uint16_t t_encendido = 0;            // Tiempo de parpadeo en decimas de segundo
volatile uint16_t parpadeo_cnt = 0;  // Contador de parpadeo en decimas de segundo

volatile uint16_t personas_cnt = 0;  // Contador de personas en la atraccion
uint8_t personas_max = 4;            // Maximo de personas en la atraccion

volatile uint16_t t_sw3_cnt = 0;  // Contador de tiempo para antirrebotes del sw3 en decimas de segundo
volatile uint8_t estado_sw3 = 1;
volatile uint8_t last_estado_sw3 = 1;  // Variable para almacenar el estado anterior del sensor mecanico SW3

volatile uint16_t t_s04_cnt = 0;  // Contador de tiempo para el sensor optico 4 en milisegundos

bool espera_flag = false;          // Bandera de contador de espera´
uint16_t t_espera = 100;           // Tiempo de espera en decimas de segundo
volatile uint16_t espera_cnt = 0;  // Contador de tiempo espera en decimas de segundo

bool atraccion_flag = false;          // Bandera de movimiento de la atraccion
uint16_t t_atraccion = 0;             // Tiempo de duracion de la atraccion en decimas de segundo
uint16_t t_atraccion_max = 20 * 60;   // Tiempo maximo de duracion de la atraccion en decimas de segundo
uint16_t t_atraccion_min = 300;       // Tiempo minimo de duracion de la atraccion en decimas de segundo
volatile uint16_t atraccion_cnt = 0;  // Contador de duracion de la atraccion en decimas de segundo

volatile uint16_t dientes_cnt = 0;   // Contador de dientes del engranaje
volatile uint16_t t_subida_cnt = 0;  // Contador de tiempo de subida en milisegundos
uint16_t t_subida = 0;               // Tiempo de subida en milisegundos
uint16_t t_subida_min = 500;         // Tiempo de subida minimo en milisegundos
uint16_t t_dientes = 2;              // Tiempo por diente en milisegundos

volatile bool emergencia_flag = false;  // Bandera de emergencia

// Configurar tiempos de parpadeo en ms (Prinmero porq se llama luego)
void setParpadeo(uint16_t t_encendido_ms, uint16_t t_total_ms) {
    t_encendido = t_encendido_ms / 100;  // Convertir a decimas de segundo
    t_total = t_total_ms / 100;          // Convertir a decimas de segundo
    parpadeo_cnt = 0;
}

// Parpadeo del LED 4
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

// Activar la atraccion
void setAtraccion() {
    t_atraccion = (rand() % (t_atraccion_max - t_atraccion_min)) + t_atraccion_min;  // Tiempo de duracion de la atraccion
    t_subida = t_subida_min;                                                         // Reiniciar tiempo de subida
    atraccion_flag = true;                                                           // Activar bandera de movimiento de la atraccion
    setParpadeo(500, 500);                                                           // Mantener el LED 4 encendido
    cli();                                                                           // Deshabilitar interrupciones globales
    // TIMSK0 |= (1 << OCIE0A);                                                         // Habilitar interrupcion por OCRA
    EIMSK |= (1 << B_SO4) | (1 << B_SO5);  // Habilitar mascara de interrupcion por sensores opticos
    sei();                                 // Habilitar interrupciones globales
}

// Desactivar la atraccion
void clrAtraccion() {
    clrBit(P_EN2, B_EN2);  // Apagar motor
    // TIMSK0 &= ~(1 << OCIE0A);                 // Deshabilitar interrupcion por OCRA del Timer 0
    EIMSK &= ~((1 << B_SO4) | (1 << B_SO5));  // Deshabilitar mascara de interrupcion por sensores opticos
    atraccion_flag = false;                   // Desactivar bandera de movimiento de la atraccion
    atraccion_cnt = 0;                        // Reiniciar contador de duracion de la atraccion
    setParpadeo(500, 10000);                  // Encender 500ms y apagar 10000ms
}

// Mover la atraccion
void moverAtraccion() {
    if (atraccion_flag) {
        setBit(P_EN2, B_EN2);  // Encender motor
        if (t_subida_cnt >= t_subida) {
            PINK |= (1 << B_DI2);                 // Cambiar sentido de giro del motor (hace toggle)
            t_subida += dientes_cnt * t_dientes;  // Incrementar tiempo de subida
            cli();                                // Deshabilitar interrupciones globales
            // TIMSK0 &= ~(1 << OCIE0A);             // Deshabilitar interrupcion por OCRA del Timer 0
            EIMSK &= ~(1 << B_SO4);  // Deshabilitar mascara de interrupcion para el sensor optico 4 (dientes)
            sei();                   // Habilitar interrupciones globales
            dientes_cnt = 0;         // Reiniciar contador de dientes
            t_subida_cnt = 0;        // Reiniciar contador de tiempo de subida
        }
    }
}

// ISR del pulsador de emergencia SW1 (INT0)
ISR(INT0_vect) {
    setParpadeo(200, 1000);  // Encender 200ms y apagar 1000ms
    // TIMSK0 &= ~(1 << OCIE0A);                                // Deshabilitar interrupcion por OCRA del Timer 0
    EIMSK &= ~((1 << B_SO4) | (1 << B_SO5) | (1 << B_SW1));  // Deshabilitar mascara de interrupcion por sensores opticos y mecanico
    emergencia_flag = true;                                  // Activar bandera de emergencia
}

// ISR del sensor optico 4 (INT2)
ISR(INT2_vect) {
    if (t_s04_cnt >= 10) {  // Si el tiempo de activacion del sensor optico 4 es mayor a 10ms
        dientes_cnt++;      // Incrementar contador de dientes
        t_s04_cnt = 0;      // Reiniciar contador de tiempo del sensor optico 4
    }
}

// ISR del sensor optico 5 (INT3)
ISR(INT3_vect) {
    if (atraccion_cnt >= t_atraccion) {
        clrAtraccion();  // Desactivar atraccion
    } else {
        // TIMSK0 |= (1 << OCIE0A);  // Habilitar interrupcion por OCRA del Timer 0
        EIMSK |= (1 << B_SO4);  // Habilitar mascara de interrupcion para el sensor optico 4 (dientes)}
    }
}

// ISR del sensor mecánico SW3
ISR(PCINT2_vect) {
    estado_sw3 = PINK & (1 << B_SW3);                           // Leer el estado del sensor mecánico SW3
    if ((estado_sw3 ^ last_estado_sw3) && (estado_sw3 == 0)) {  // Si el sensor mecánico SW3 tiene un flanco de bajada
        if (t_sw3_cnt >= 1) {
            personas_cnt++;  // Incrementar contador de personas en la atraccion
            t_sw3_cnt = 0;   // Reiniciar contador de tiempo de antirrebote
        }
    }
    last_estado_sw3 = estado_sw3;  // Guardar el estado actual del sensor mecánico SW3
}

// ISR del Timer 0 (cada 1ms)
ISR(TIMER0_COMPA_vect) {
    t_subida_cnt++;  // Incrementar contador de tiempo de subida
    t_s04_cnt++;     // Incrementar contador de tiempo del sensor optico 4
}

// ISR del Timer 4 (cada 100ms)
ISR(TIMER4_COMPA_vect) {
    parpadeo_cnt++;
    t_sw3_cnt++;
    if (espera_flag) {
        espera_cnt++;
    }
    if (atraccion_flag) {
        atraccion_cnt++;
    }
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

    // Configurar interrupcion INT0 (flanco de bajada)
    EICRA &= ~(1 << 2 * B_SW1);
    EICRA |= (1 << (2 * B_SW1 + 1));

    // Habilitar mascara sensor mecanico SW1
    EIMSK |= (1 << B_SW1);

    // Configurar interrupcion INT2 e INT3 (flanco de bajada)
    EICRA &= ~(1 << (2 * B_SO4));
    EICRA |= (1 << (2 * B_SO4 + 1));
    EICRA &= ~(1 << (2 * B_SO5));
    EICRA |= (1 << (2 * B_SO5 + 1));

    // Habilitar mascara sensores opticos
    EIMSK |= (1 << B_SO4);
    // EIMSK |= (1 << B_SO5);

    // Configurar interrupcion PCINT20
    PCICR |= (1 << PCIE2);

    // Habilitar mascara sensor mecanico SW3
    PCMSK2 = 0;
    PCMSK2 |= (1 << PCINT20);

    // Timer 0 en modo CTC (Top OCR0A)
    TCCR0A &= ~(1 << WGM00);
    TCCR0A |= (1 << WGM01);
    TCCR0B &= ~(1 << WGM02);

    // Configurar mascara para preescalado de 64
    TCCR0B |= (1 << CS00);
    TCCR0B |= (1 << CS01);
    TCCR0B &= ~(1 << CS02);

    // Modificar TOP
    OCR0A = 125 - 1;  // 125ns*125*64=1ms

    // Configurar el registro de interrupcion
    TIMSK0 |= (1 << OCIE0A);  // Habilitar interrupcion por OCRA

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
    OCR4A = 12500 - 1;  // 125ns*12500*64=100ms

    // Configurar el registro de interrupcion
    TIMSK4 |= (1 << OCIE4A);  // Habilitar interrupcion por OCRA

    // Configurar semilla para el generador de numeros aleatorios
    srand(time(NULL));

    // Inicializar el estado del sensor mecanico SW3
    last_estado_sw3 = PINK & (1 << B_SW3);

    // Parpadeo inicial
    setParpadeo(500, 10000);

    // Habilitar interrupciones
    sei();
}

int main(void) {
    setup();  // Configuracion inicial
    while (1) {
        if (!emergencia_flag) {
            if (personas_cnt >= personas_max) {
                setBit(P_L3, B_L3);  // Encender LED 3
                espera_flag = true;  // Activar bandera de espera
                if (espera_cnt >= t_espera) {
                    espera_flag = false;  // Desactivar bandera de espera
                    espera_cnt = 0;       // Reiniciar contador de espera
                    personas_cnt -= personas_max;
                    clrBit(P_L3, B_L3);  // Apagar LED 3
                    setAtraccion();      // Llamar a la funcion de atraccion
                }
            }
            moverAtraccion();  // Llamar a la funcion de movimiento de la atraccion
        }
        parpadeo();  // Llamar a la funcion de parpadeo
    }
}
