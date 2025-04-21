//Codigo encender LED y parpadeo LED

#include <avr/io.h>
#include <avr/interrupt.h>


volatile uint8_t contador_ms=0;

void setup(){
   
    //configuro LED: puerto L pin PL7
    DDRL|= (1<<PL7); //salida
    PORTL &= ~(1<<PL7); //apagado al ppio

    cli();

    //configuro TIMER
    TCCR2A |= (1<<WGM21); //modo CTC
    TCCR2B |= (1<<CS22); //Prescalado 64
    OCR2A=1249; //10 ms
    TIMSK2 |= OCIE2A; //mascara interrupcion

    sei();

}

void encenderLED(){
    //Enciendo el led indefinidamente
    PORTL |= (1<<PL7);
}

ISR(TIM2_COMPA_vect){ //sucede cada 10ms
    contador_ms++;
}

void parpadeoLED(){
    //TOGGLE cada 50 ms
    if(contador_ms % 5 == 0){ 
        if(PORTL & (1<<PL7)){ //Lee el pin y hace TOGGLE
            PORTL &= ~(1<<PL7); 
        } else {
            PORTL |= (1<<PL7);
        }
    }
}

int main(){
    setup();
    encenderLED();

    while(1){
    //parpadeoLED(); 
    }
    

}
