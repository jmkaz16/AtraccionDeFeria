/*
 * AtraccionDeFeria.c
 *
 * Created: 13/04/2025 17:10:27
 * Author : rodri & rafa
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "pinout.h"

void setup(){
	
	// Deshabilitar interrupciones
	cli();
	
	// Configurar LEDs como salida
	DDRL |= (1 << B_L2)	
	
	// Configurar motores como salida
	DDRL |= (1 << B_EN1)	
	DDRL |= (1 << B_BK1)
	
	// Configurar interrupciones PCINT
	PCICR |= (1 << PCIE0)
	
	// Habilitar mascaras sensores opticos
	PCMSK0 |= (1 << B_SO2) // PCINT2?
	PCMSK0 |= (1 << B_SO3) // PCINT3?
	
	// Configurar interrupcion INT0 (flanco de subida)
	EICRA |= (1 << 2*B_SW2) | (1 << 2*B_SW2+1)				// CONFIRMAR PRIMERO!!!
	
	// Habilitar mascara sensor mecanico
	EIMSK |= (1 << B_SW2)
	
	// Habilitar interrupciones
	sei();
}


int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
    }
}

