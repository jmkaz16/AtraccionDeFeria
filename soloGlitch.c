#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#define GLITCH 56000 // Umbral para considerar un glitch (ajustable)
#define TIEMPO_LED 50 // 0.5s ? 50 ticks de 10ms

// Variables globales
volatile uint8_t estado = 0;
volatile uint16_t t_flanco_anterior = 0;
volatile uint16_t t_actual = 0;


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

	//Configuracion TIMER2 (led)
	TCCR2A = (1 << WGM21);       // Modo CTC
	TCCR2B = (1 << CS22);        // Prescaler 64 (8 MHz / 64 = 125 kHz)
	OCR2A = 255;                // 2 ms ; 125000 / ( 1249 + 1) = 100 Hz
	TIMSK2 |= (1 << OCIE2A);     // Habilita interrupciOn por comparacion


	sei();
}

ISR(TIMER1_CAPT_vect) {
	t_actual = ICR1;
	uint16_t delta_t = t_actual - t_flanco_anterior;

	if ( delta_t > GLITCH) {
		// Glitch detectado
		PORTL |= (1 << PL7); // Encender LED
		encender_led = 1;
		contador_led = 0;
	}

	t_flanco_anterior = t_actual;

	// Alternamos flanco: si era subida, ahora bajada y viceversa
	if (TCCR1B & (1 << ICES1))
	TCCR1B &= ~(1 << ICES1); // bajar
	else
	TCCR1B |= (1 << ICES1); // subir
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
