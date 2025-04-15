/*
 * GccApplication1.c
 *
 * Created: 15/04/2025 10:25:15
 * Author : celia
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define SIZE 28

volatile uint8_t vector_bit[SIZE];
volatile uint8_t indice=0;


volatile uint8_t estado =0;
volatile uint16_t t_inicio_negro=0;
volatile uint16_t t_fin_negro=0;
volatile uint16_t t_fin_bit=0;

void inicio_tarjetero() {
	cli();
	
	//Interrupcion que detecta subida o bajada de flanco, de momento no la usamos
	DDRB &= ~(1 << PB4);
	//Activar resistencia pull-up si no funciona PORTB |= (1<< PB4)
	PCMSK0 |= (1 << PCINT4);
	PCICR |= (1 << PCIE0);
	
	//Interrupcion del timer
	DDRD &= ~(1 << PD6);
	//Activar pull-up si da error PORTD|= (1 << PD6)
	
	TCCR1A = 0;
	TCCR1B = (1 << ICES1) | (1 << CS11);
	TIMSK1 |= (1 << ICIE1);
	
	sei();
}

ISR(TIMER1_CAPT_vect){
	//Empieza el bit
	if (estado == 0){
		t_inicio_negro= ICR1;
		TCCR1B &= ~(1 << ICES1);
		estado = 1;
	} else if (estado == 1){ // duracion del bit negro
		t_fin_negro= ICR1;
		estado =2;
	} else if (estado == 2){
		t_fin_bit=ICR1;
		
		uint16_t duracion_negro = t_fin_negro - t_inicio_negro;
		uint16_t duracion_total = t_fin_bit - t_inicio_negro;

		// Si el pulso negro dura más de 2/3 del total ? bit = 1, si no ? bit = 0
		uint8_t bit = (duracion_negro * 3 > duracion_total * 2) ? 1 : 0;

		if (indice < SIZE) {
			vector_bit[indice++] = bit;
		}
		
		t_inicio_negro = t_fin_bit;
		
		TCCR1B &= ~(1 << ICES1);
		estado =1;
	}
}

int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
    }
}

