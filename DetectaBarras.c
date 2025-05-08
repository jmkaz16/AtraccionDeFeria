//Codigo encender LED y parpadeo LED

#include <avr/io.h>
#include <avr/interrupt.h>


volatile uint8_t contador=0;

void setup(){
	
	//configuro LED: puerto L pin PL7
	DDRL|= (1<<PL7); //salida
	PORTL &= ~(1<<PL7); //apagado al ppio

	cli();
	//Configuracion TIMER2 (led)
	TCCR2A = (1 << WGM21);       // Modo CTC
	TCCR2B = (1 << CS22);        // Prescaler 64 (8 MHz / 64 = 125 kHz)
	OCR2A = 255;                // 10 ms ; 125000 / ( 1249 + 1) = 100 Hz
	TIMSK2 |= (1 << OCIE2A);     // Habilita interrupciOn por comparacion

	sei();

}

void encenderLED(){
	//Enciendo el led indefinidamente
	PORTL |= (1<<PL7);
}

void contador_ms(){
	contador++;
}

ISR(TIMER2_COMPA_vect){ //sucede cada 2ms
	contador_ms();
}



void parpadeoLED(){
	//TOGGLE cada 100ms
	if(contador >= 50){
		if(PORTL & (1<<PL7)){ //Lee el pin y hace TOGGLE
			PORTL &= ~(1<<PL7);
			contador=0;
			} else {
			PORTL |= (1<<PL7);
			contador=0;
		}
	}

}

int main(){
	setup();

	while(1){
		parpadeoLED();
	}
	

}
