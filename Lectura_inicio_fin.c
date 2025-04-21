//Lectura inicio y fin de tarjeta

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t leyendo = 0;
volatile uint16_t timeout = 0;
volatile uint16_t contador_flancos = 0;

void setup(){
    // Configuro pin como entrada (opcional)
    DDRD &= ~(1 << PD4);

    // Configuro Input Capture
    TCCR1A = 0;                        // Modo normal
    TCCR1B &= ~(1 << ICES1);          // Flanco de bajada
    TCCR1B |= (1 << CS11);            // Prescaler 8
    TIMSK1 |= (1 << ICIE1);           // Interrupcion por input capture
    TIMSK1 |= (1 << TOIE1);  // Habilita interrupción por overflow

    sei(); // Activar interrupciones globales
}

ISR(TIMER1_CAPT_vect) {
    contador_flancos++;
    timeout = 0;

    if (!leyendo) {
        leyendo = 1;
        // Inicio de tarjeta
    }
}

ISR(TIMER1_OVF_vect) {
    if (leyendo) {
        timeout++;

        if (timeout >= 4) { // 4 * 65 ms=260 ms
            leyendo = 0;
            // Fin de tarjeta
        }
    }
}

int main(void) {
    setup();

    while (1) {
    }
}
