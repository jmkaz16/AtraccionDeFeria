#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#define GLITCH 10000 // Umbral para considerar un glitch (ajustable)
#define TIEMPO_LED 50 // 0.5s → 50 ticks de 10ms

// Variables globales
volatile uint8_t estado = 0;
volatile uint16_t t_inicio = 0;
volatile uint16_t t_fin = 0;
volatile uint16_t t_ultimo_bit = 0;

volatile uint8_t encender_led = 0;
volatile uint8_t contador_led = 0;

void setup() {
	cli();

	// LED en PL7 como salida
	DDRL |= (1 << PL7);
	PORTL &= ~(1 << PL7);

	// Input capture en PD4
	DDRD &= ~(1 << PD4);

	// Configuración TIMER1: input capture, flanco subida, prescaler 8
	TCCR1A = 0;
	TCCR1B = (1 << ICES1) | (1 << CS11);
	TIMSK1 |= (1 << ICIE1); // Habilita interrupción input capture

	// Configuración TIMER2: modo CTC para temporizar 10ms
	TCCR2A = (1 << WGM21);
	TCCR2B = (1 << CS22);     // Prescaler 64 (8 MHz / 64 = 125 kHz)
	OCR2A = 1249;             // 125000 / (1249 + 1) = 100 Hz → 10ms
	TIMSK2 |= (1 << OCIE2A);  // Habilita interrupción por comparación

	sei();
}

ISR(TIMER1_CAPT_vect) {
	if (estado == 0) {
		t_inicio = ICR1;
		TCCR1B &= ~(1 << ICES1); // Cambiar a flanco de bajada
		estado = 1;
		} else if (estado == 1) {
		t_fin = ICR1;
		TCCR1B |= (1 << ICES1);  // Cambiar a flanco de subida
		estado = 2;
		} else if (estado == 2) {
		uint16_t t_actual = ICR1;
		uint16_t t_entre_bits = t_inicio - t_ultimo_bit;

		if (t_ultimo_bit != 0 && t_entre_bits > GLITCH) {
			PORTL |= (1 << PL7);     // Encender LED
			encender_led = 1;
			contador_led = 0;
		}

		t_ultimo_bit = t_inicio;
		t_inicio = t_actual;
		TCCR1B &= ~(1 << ICES1);
		estado = 1;
	}
}

ISR(TIMER2_COMPA_vect) {
	if (encender_led) {
		contador_led++;
		if (contador_led >= TIEMPO_LED) {
			PORTL &= ~(1 << PL7); // Apagar LED
			encender_led = 0;
		}
	}
}

int main(void) {
	setup();
	while (1) {
		// Todo ocurre por interrupciones
	}
}
