#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define GLITCH 10000

volatile uint8_t estado = 0;
volatile uint16_t t_inicio = 0;
volatile uint16_t t_medio = 0;
volatile uint16_t t_fin = 0;
volatile uint16_t t_ultimo_bit = 0;

void setup() {
	cli();

	// LED en PL7 como salida
	DDRL |= (1 << PL7);
	PORTL &= ~(1 << PL7); // LED apagado al inicio

	// PD4 como entrada para input capture
	DDRD &= ~(1 << PD4);

	// Configuración TIMER1 para input capture
	TCCR1A = 0;
	TCCR1B = (1 << ICES1) | (1 << CS11); // flanco subida, prescaler 8
	TIMSK1 |= (1 << ICIE1); // Habilitar interrupción por captura

	sei();
}

ISR(TIMER1_CAPT_vect) {
	if (estado == 0) {
		t_inicio = ICR1;
		TCCR1B &= ~(1 << ICES1); // flanco bajada
		estado = 1;
		} else if (estado == 1) {
		t_medio = ICR1;
		TCCR1B |= (1 << ICES1); // flanco subida
		estado = 2;
		} else if (estado == 2) {
		t_fin = ICR1;
		uint16_t dur_negro = t_medio - t_inicio;
		uint16_t dur_total = t_fin - t_inicio;
		uint16_t entre_bits = t_inicio - t_ultimo_bit;

		if (t_ultimo_bit == 0 || entre_bits < GLITCH) {
			uint8_t bit = (dur_negro * 3 > dur_total * 2) ? 1 : 0;

			if (bit == 1) {
				PORTL |= (1 << PL7);  // Encender LED
				} else {
				PORTL &= ~(1 << PL7); // Apagar LED
			}
		}
		
		t_ultimo_bit = t_inicio;
		t_inicio = t_fin;
		TCCR1B &= ~(1 << ICES1); // volver a esperar flanco bajada
		estado = 1;
	}
}

int main(void) {
	setup();
	while (1) {
		// Todo ocurre en la interrupción
	}
}
