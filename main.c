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
	
	// Configurar 	
	
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

