/*
 * GccApplication1.c
 *
 * Created: 15/04/2025 10:25:15
 * Author : celia
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

//Otros

#define SIZE 64 //Hay que cambiarlo a 28
#define GLITCH 10000  //Ajustable con el osciloscopio
#define NUM_CARACTERES 7
#define MAX_TARJETAS 10

char tarjeta[SIZE/4+1]; //Cadena resultante de la tarjeta

char tabla_codigos[15]={ //Tabla para la decodificacion de bits a caracteres
	'0','1','2','3','4','5','6','7','8','9','?','-','+','?','?',':'
};

char tarjetas_validas[MAX_TARJETAS][NUM_CARACTERES+1];
bool habilitar_ISRled=false;

volatile uint8_t vector_bit[SIZE];
volatile uint8_t indice=0; //indice para recorrer el vector

//LED
volatile uint8_t tipo_led = 0;
volatile uint16_t contador_led = 0;

//Interrupcion lectura de bits
volatile uint8_t estado =0; //control de flancos: 0=primera subida, 1=bajada, 2=subida
volatile uint16_t t_inicio_negro=0;
volatile uint16_t t_fin_negro=0;
volatile uint16_t t_fin_bit=0;

//Decodificacion 
volatile uint8_t inicio_lectura=0;
volatile uint8_t fin_lectura=0;


//Glitch
volatile uint16_t t_ultimo_bit=0;

//Validacion tarjetas
uint8_t tarjetas_guardadas=0;


void setup() {
	cli();
	//Configuracion del LED como salida
	DDRL |= (1<<PL7);
	PORTL &= ~(1<<PL7); //Seguridad para tenerlo apagado al principio

	//Configuracion TIMER2 (led)
	TCCR2A = (1 << WGM21);       // Modo CTC
	TCCR2B = (1 << CS22);        // Prescaler 64 (8 MHz / 64 = 125 kHz)
	OCR2A = 1249;                // 10 ms ; 125000 / ( 1249 + 1) = 100 Hz
	TIMSK2 |= (1 << OCIE2A);     // Habilita interrupciOn por comparacion

	//Interrupcion que detecta subida o bajada de flanco, de momento no la usamos
	DDRB &= ~(1 << PB4);
	//Activar resistencia pull-up si no funciona PORTB |= (1<< PB4)
	PCMSK0 |= (1 << PCINT4);
	PCICR |= (1 << PCIE0);
	
	//IConfiguracion TIMER1
	DDRD &= ~(1 << PD4);
	//Activar pull-up si da error PORTD|= (1 << PD4)
	
	TCCR1A = 0;
	TCCR1B = (1 << ICES1) | (1 << CS11);
	TIMSK1 |= (1 << ICIE1);
	
	sei();
}


//Funcion tarjeta leida completamente
void tarjeta_leida(){
	if(inicio_lectura && fin_lectura){
		habilitar_ISRled=true;
		convertidor_bits_a_numero(vector_bit, tarjeta);
		uint8_t resultado = gestionar_tarjeta(tarjeta);

		switch (resultado) {
			case 0: // Invalida
				PORTL |= (1 << PL7);
				tipo_led = 0;
				contador_led = 0;
				break;
			case 1: // Registrada
				PORTL |= (1 << PL7);
				tipo_led = 1;
				contador_led = 0;
				break;
			case 2: // Nueva
				PORTL |= (1 << PL7);
				tipo_led = 2;
				contador_led = 0;
				break;
				
			
		}

		//Reset para nueva lectura
		inicio_lectura=0;
		fin_lectura=0;
		indice=0;
		estado=0;
		t_ultimo_bit=0;
	}
}


uint8_t gestionar_tarjeta(char *tarjeta){
		// Validar formato
		for (uint8_t i = 0; i < NUM_CARACTERES; i++) {
			if (tarjeta[i] == '\0' || tarjeta[i] == '?') {
				return 0; // Formato invalido
			}
		}

		if (tarjeta[NUM_CARACTERES] != '\0') return 0; // Longitud incorrecta porque sea muy larga
	
		// Comprobar si ya esta
		for (uint8_t i = 0; i < tarjetas_guardadas; i++) {
			if (strcmp(tarjetas_validas[i], tarjeta) == 0) {
				return 1; // Valida y ya registrada
			}
		}
	
		// Anadir si no esta y hay espacio
		if (tarjetas_guardadas < MAX_TARJETAS) {
			strcpy(tarjetas_validas[tarjetas_guardadas], tarjeta);
			tarjetas_guardadas++;
			return 2; // Valida y anadida
		}
	
		return 0; // No se anade por limite alcanzado de tarjetas que se pueden registrar
	}
	
//Funcion que convierte los bits de vector bit en la cadena de caracteres
void convertidor_bits_a_numero(volatile uint8_t *vector, char *tarjeta){

	//Construcion del numero decimal del grupo de 4 bits
	for(uint8_t i=0; i<NUM_CARACTERES;i++){
		uint8_t codigo=0;

		for(uint8_t j=0; j<4; j++){
			codigo<<=1;						//Desplazamiento del numero a la izquierda una posicion
			codigo|= vector[i*4+j] & 0x01;	//Anade el bit asegurando 0 o 1
		}

		tarjeta[i]= tabla_codigos[codigo]; //Se guarda el caracter correspondiente al codigo de 4 bits
	}

	tarjeta[NUM_CARACTERES]='\0'; //Finaliza la cadena de caracteres de la tarjeta

}

//Interrupcion lectura de bits
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
			inicio_lectura=0;
			indice=0;
		}
		
		t_ultimo_bit = t_inicio_negro;
		t_inicio_negro = t_fin_bit;
		
		TCCR1B &= ~(1 << ICES1);
		estado =1;
	}
}

//Interrupcion para el LED

ISR(TIMER2_COMPA_vect) {
	if(!habilitar_ISRled) return;
		contador_led++;

		if (tipo_led == 1 && contador_led >= 100) { // 1 segundo: usuario registrado
			PORTL &= ~(1 << PL7);
			habilitar_ISRled=false;
			contador_led = 0;
			return;
		}
		else if (tipo_led == 2 && contador_led >= 200) { // 2 segundos: nuevo usuario
			PORTL &= ~(1 << PL7);
			contador_led = 0;
			habilitar_ISRled=false;
			return;
		}
		else if (tipo_led == 0) { //parpadea durante 2 segundos: error
			if (contador_led <= 200) {
				if (contador_led % 10 == 0) {
					if (PORTL & (1 << PL7)) {
						PORTL &= ~(1 << PL7);
					} else {
						PORTL |= (1 << PL7);
					}
				} 
			}else {
				PORTL &= ~(1 << PL7);
				contador_led = 0; //lo mismo se puede quitar
				habilitar_ISRled=false;
				return;
			}
		}
	}


int main(void)
{
	inicio_tarjetero();

    /* Replace with your application code */
    while (1) 
    {
		tarjeta_leida();
	}
		
}

