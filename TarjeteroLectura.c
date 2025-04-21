/*
 * GccApplication1.c
 *
 * Created: 15/04/2025 10:25:15
 * Author : celia
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define SIZE 64 //Hay que cambiarlo a 28
#define GLITCH 10000  //Ajustable con el osciloscopio

volatile uint8_t vector_bit[SIZE];
volatile uint8_t indice=0; //indice para recorrer el vector


volatile uint8_t estado =0; //control de flancos: 0=primera subida, 1=bajada, 2=subida
volatile uint16_t t_inicio_negro=0;
volatile uint16_t t_fin_negro=0;
volatile uint16_t t_fin_bit=0;


volatile uint8_t inicio_lectura=0;
volatile uint8_t fin_lectura=0;

volatile uint16_t t_ultimo_bit=0;

char tarjeta[SIZE/4+1]; //Cadena resultante de la tarjeta


char tabla_codigos[15]={ //Tabla para la decodificacion de bits a caracteres
	'0','1','2','3','4','5','6','7','8','9','-','+','?','?',':'
};


//Funcion que convierte los bits de vector bit en la cadena de caracteres
void convertidor_bits_a_numero(volatile uint8_t *vector, char *tarjeta, uint8_t num_bits){
	uint8_t num_caracteres=num_bits/4; //indica el num de caracteres siendo este el total de agrupaciones de 4 bits (debe ser 7)

	//Construcion del numero decimal del grupo de 4 bits
	for(uint8_t i=0; i<num_caracteres;i++){
		uint8_t codigo=0;

		for(uint8_t j=0; j<4; j++){
			codigo<<=1;						//Desplazamiento del numero a la izquierda una posicion
			codigo|= vector[i*4+j] & 0x01;	//Anade el bit asegurando 0 o 1
		}

		tarjeta[i]= tabla_codigos[codigo]; //Se fuarda el caracter correspondiente al codigo de 4 bits
	}

	tarjeta[num_caracteres]='\0'; //Finaliza la cadena de caracteres de la tarjeta

}

void inicio_tarjetero() {
	cli();
	
	//Interrupcion que detecta subida o bajada de flanco, de momento no la usamos
	DDRB &= ~(1 << PB4);
	//Activar resistencia pull-up si no funciona PORTB |= (1<< PB4)
	PCMSK0 |= (1 << PCINT4);
	PCICR |= (1 << PCIE0);
	
	//Interrupcion del timer
	DDRD &= ~(1 << PD4);
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

		if(!inicio_lectura){ //Inicio de lectura
			inicio_lectura=1;
			indice=0;
		}

	} else if (estado == 1){ // duracion del bit negro
		t_fin_negro= ICR1;
		estado =2;
	} else if (estado == 2){
		t_fin_bit=ICR1;
		
		uint16_t duracion_negro = t_fin_negro - t_inicio_negro;
		uint16_t duracion_total = t_fin_bit - t_inicio_negro;
		uint16_t t_entre_bits = t_inicio_negro - t_ultimo_bit;

		// Si el pulso negro dura mas de 2/3 del total ? bit = 1, si no ? bit = 0
		uint8_t bit = (duracion_negro * 3 > duracion_total * 2) ? 1 : 0;

		//Verifica si el tiempo entre birs es razonable
		if(t_ultimo_bit==0 || t_entre_bits < GLITCH){
			if (indice < SIZE) {
				vector_bit[indice++] = bit;
				if(indice==SIZE){
					fin_lectura=1;
				}
			} 
		} else {
			indice=0;
		}
		
		t_ultimo_bit = t_inicio_negro;
		t_inicio_negro = t_fin_bit;
		
		TCCR1B &= ~(1 << ICES1);
		estado =1;
	}
}

int main(void)
{
	inicio_tarjetero();

    /* Replace with your application code */
    while (1) 
    {
		//Comprueba si la tarjeta se ha leido completamente
		if(inicio_lectura && fin_lectura){
			convertidor_bits_a_numero(vector_bit, tarjeta, SIZE);

			//Reset para nueva lectura
			inicio_lectura=0;
			fin_lectura=0;
			indice=0;
			estado=0;
			t_ultimo_bit=0;
		}
    }
}

