//Codigo para guardar los bits de la tarjeta en un vector

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define MAX_BITS 24 //se puede cambiar

volatile uint8_t bits[MAX_BITS];
volatile uint8_t bit_index = 0;

volatile uint8_t estado = 0;
volatile uint16_t t_inicio_negro = 0;
volatile uint16_t t_fin_negro = 0;
volatile uint16_t t_fin_bit= 0;

uint8_t vector_bits[MAX_BITS/4];  // conjunto de 4 bits ya convertido a 0/1

void setup() {
    cli()
	DDRB &= ~(1 << PB0);  // ICP1 (PB0) como entrada
	TCCR1A = 0;
	TCCR1B = (1 << ICES1) | (1 << CS11);  // Subida, prescaler 8
	TIMSK1 |= (1 << ICIE1);               // Habilita interrupcion input capture
    sei()
}

ISR(TIMER1_CAPT_vect) {

	if (estado == 0) {
		t_inicio_negro= ICR1;
		TCCR1B &= ~(1 << ICES1);  // Esperar bajada
		estado = 1;
	}
	else if (estado == 1) {
		t_fin_negro = ICR1;
		TCCR1B |= (1 << ICES1);  // Esperar subida
		estado = 2;
	}
	else if (estado == 2) {
		t_fin_bit = ICR1;

		uint16_t duracion_negro = t_fin_negro - t_inicio_negro;
		uint16_t duracion_bit = t_fin_bit - t_inicio_negro;
        uint8_t bit = (duracion_negro * 3 > duracion_bit * 2) ? 1 : 0;

		if (bit_index < MAX_BITS) {
			bits[bit_index++] = bit;
		}

		estado = 0;
	}
}

void procesar_bits() {
	for (uint8_t i = 0; i < MAX_BITS/4; i++) {
		uint8_t val = 0;
		for (uint8_t j = 0; j < 4; j++) {
			val <<= 1;
			val |= bits[i * 4 + j] & 0x01;
		}
		valores[i] = val;
	}
}

int main(void) {
	setup();

	while (1) {
		if (bit_index >= MAX_BITS) {
			procesar_bits();
			bit_index = 0;
		}
	}
}