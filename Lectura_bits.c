#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define GLITCH 10000

volatile uint8_t flanco = 0;
volatile uint16_t tiempos[48];  // Guarda cada instante de flanco
volatile uint8_t flanco_index = 0;
volatile uint8_t captura_completa = 0;

volatile uint8_t valores[24] = {0};

void setup() {
	cli();

	// LED en PL7 como salida
	DDRL |= (1 << PL7);
	PORTL &= ~(1 << PL7);

	// PD4 como entrada para input capture
	DDRD &= ~(1 << PD4);

	// Configuración TIMER1
	TCCR1A = 0;
	TCCR1B = (1 << ICES1) | (1 << CS11); // Flanco de subida inicialmente, prescaler 8
	TIMSK1 |= (1 << ICIE1); // Habilitar interrupción input capture

	sei();
}

ISR(TIMER1_CAPT_vect) {
	if (flanco_index < 48) {
		tiempos[flanco_index] = ICR1;  // Guarda el instante del flanco
		flanco_index++;
		TCCR1B ^= (1 << ICES1);  //parpadea el LED
		PORTL ^= (1 << PL7);            // Cambiar flanco (subida/bajada)
	}
	if (flanco_index >= 48) {
		captura_completa = 1;  // Señala que ya tenemos una tarjeta leída
		TIMSK1 &= ~(1 << ICIE1); // Desactivar interrupción para procesar tranquilo
	}
}

void procesar_tarjeta() {
	if (!captura_completa) return;

	for (uint8_t i = 0; i < 24; i++) {
		uint16_t dur_negro = tiempos[2*i+1] - tiempos[2*i];  // tiempo entre subida-bajada
		uint16_t dur_total = tiempos[2*i+2] - tiempos[2*i];  // tiempo total subida-subida siguiente

		if (dur_total < GLITCH) continue; // ignorar glitches

		valores[i] = (dur_negro * 3 > dur_total * 2) ? 1 : 0;
	}

	// Opcional: encender LED si quieres visualizarlo
	
	for (uint8_t i = 0; i < 24; i++) {
		if (valores[i] == 1) PORTL |= (1 << PL7);
		else PORTL &= ~(1 << PL7);
	}

	// Reset para próxima tarjeta
	flanco_index = 0;
	captura_completa = 0; //mirarlo
	TIMSK1 |= (1 << ICIE1);  // Rehabilitar interrupción
}

int main(void) {
	setup();

	while (1) {
		procesar_tarjeta();
	}
}