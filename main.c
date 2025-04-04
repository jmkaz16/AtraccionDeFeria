/*
 * AtraccionDeFeria.c
 *
 * Created: 30/03/2025 18:34:49
 * Author : juanm & cloti
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "pinout.h"

void setup(){
	
	// Deshabilitar interrupciones
	cli();
	
	// Configurar LEDs como salida
	DDRL |= (1 << B_L3)	
	DDRL |= (1 << B_L4)	
	
	// Configurar motores como salida
	DDRL |= (1 << B_EN2)	
	DDRL |= (1 << B_DI2)
	
	// Configurar interrupciones PCINT
	PCICR |= (1 << PCIE0)
	
	// Habilitar mascaras sensores opticos
	PCMSK0 |= (1 << B_SO3)
	PCMSK0 |= (1 << B_SO4)
	
	// Configurar interrupcion INT0 (flanco de subida)
	EICRA |= (1 << 2*B_SW1) | (1 << 2*B_SW1+1)
	
	// Habilitar mascara sensor mecanico
	EIMSK |= (1 << B_SW1)
	
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

