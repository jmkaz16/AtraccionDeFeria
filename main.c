/*
 * AtraccionDeFeria.c
 *
 * Created: 13/04/2025 17:10:27
 * Author : rodri & rafa
 */ 

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "pinout.h"

long int tiempo_total=0;

ISR(INT1_vect){
}
	
ISR(PCINT0_vect){
}


void suma1aMilis(){
	tiempo_total++;
}

long int milis(){
	return tiempo_total;
}

ISR(TIMER3_COMPA_vect){
	suma1aMilis();
}

void setup(){
	
	// Deshabilitar interrupciones
	cli();
	
	// Configurar LEDs como salida
	DDRL |= (1 << B_L2);
	
	// Configurar motores como salida
	DDRL |= (1 << B_EN1);	
	DDRL |= (1 << B_BK1);
	
	// Configurar interrupciones PCINT
	PCICR |= (1 << PCIE0);
	
	// Habilitar mascaras sensores opticos
	PCMSK0 |= (1 << B_SO2); // PCINT2?
	PCMSK0 |= (1 << B_SO3); // PCINT3?
	
	// Configurar interrupcion INT0 (flanco de subida)
	EICRA |= (1 << 2*B_SW2) | (1 << (2*B_SW2+1))	;			// CONFIRMAR PRIMERO!!!
	
	// Habilitar mascara sensor mecanico
	EIMSK |= (1 << B_SW2);

	// TCCR3A X X X X X X 0 0 
	// TCCR3B X X X 0 1 X X X
	// Timer 3 en modo CTC (Top OCRnA)
	TCCR3A &= ~(1 << WGM30);  
	TCCR3A &= ~(1 << WGM31);
	TCCR3B |= (1 << WGM32);
	TCCR3B &= ~(1 << WGM33);
	
	// Mascara para no tener preescalado 
	TCCR3B = (TCCR3B | (1 << CS30));
	TCCR3B = (TCCR3B & (~(1 << CS31)));
	TCCR3B = (TCCR3B & (~(1 << CS32)));
	
	// Sera el TOP, y vale 8000 porque 125ns*8000=1ms (para la funcion "milis()" )
	OCR3A = 8000;
	
	 // Configuramos la interrupción por OCRA
	 
	 // ****DUDA****
	 // ASEGURARNOS DE QUE ES "OCIE3A", ¿por qué "A"? ¿es porque estamos usando ocrA? )
	TIMSK3 = (1 << OCIE3A);
	
	// Habilitar interrupciones
	sei();
}


int main(void)
{
    // Llamamos a setup 1 vez
	setup();
    while (1) 
    {
		printf("tiempo_total vale: %ld\n",tiempo_total);
    }
}

