#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>


volatile uint8_t flanco = 0;
volatile uint32_t tiempos[54];  // Guarda cada instante de flanco
volatile uint8_t flanco_index = 0;
volatile uint8_t captura_completa = 0;
volatile uint32_t desbordamiento=0;
volatile uint32_t tiempo=0;

volatile uint8_t valores[24] = {0};

char tarjeta[6];

char usuarios[6]=":+097-"; //Cadena resultante de la tarjeta


char tabla_codigos[16]={ //Tabla para la decodificacion de bits a caracteres
	'0','1','2','3','4','5','6','7','8','9','?','?','-','?','+',':'
};

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
	TIMSK1 |= (1<<TOIE1); //Habilitar interrupcion por desbordamiento
	sei();
}

ISR(TIMER1_CAPT_vect) {
	if (flanco_index < 52) {
		tiempo= ((uint32_t)desbordamiento << 16) | ICR1;
		tiempos[flanco_index] = tiempo;  // Guarda el instante del flanco
		flanco_index++;
		TCCR1B ^= (1 << ICES1);  //parpadea el LED, hay que cambiarlo
		PORTL ^= (1 << PL7);            // Cambiar flanco (subida/bajada), hay que cambiarlo
	}
	if (flanco_index >= 52) {
		captura_completa = 1;  // Señala que ya tenemos una tarjeta leída
		TIMSK1 &= ~(1 << ICIE1); // Desactivar interrupción para procesar tranquilo
	}
}

ISR(TIMER1_OVF_vect){
	desbordamiento++;
}

void procesar_tarjeta() {
	if (!captura_completa) return;

	for (uint8_t i = 2, j=0; i < 26; i++, j++) { //Empezamos desde 4 para ignorar el glitch y la primera barra
		uint32_t dur_negro = tiempos[2*i] - tiempos[2*i-1];  // tiempo negro
		uint32_t dur_total = tiempos[2*i+1] - tiempos[2*i-1];  // tiempo total bit
		uint32_t dur_blanco = dur_total - dur_negro;
		// Si el pulso negro dura mas de 2/3 del total ? bit = 1, si no ? bit = 0

		valores[j] = (dur_negro * 3 > dur_total * 2) ? 0 : 1; //creo que esto esta al reves
		
		
	}

	// Opcional: encender LED si quieres visualizarlo
	
	// 	for (uint8_t i = 0; i < 24; i++) {
	// 		if (valores[i] == 1) PORTL |= (1 << PL7);
	// 		else PORTL &= ~(1 << PL7);
	// 	}
	
	convertidor_bits_a_numero(valores, tarjeta);
	comparar_tarjeta(usuarios, tarjeta);

	// Reset para próxima tarjeta
	flanco_index = 0;
	captura_completa = 0; //mirarlo
	desbordamiento=0;
	TCNT1=0;
	TIMSK1 |= (1 << ICIE1);  // Rehabilitar interrupción
}

//Funcion que convierte los bits de vector bit en la cadena de caracteres
void convertidor_bits_a_numero(volatile uint8_t *vector, char *tarjeta){

	//Construcion del numero decimal del grupo de 4 bits
	for(uint8_t i=0; i<6; i++){
		uint8_t codigo=0;

		for(uint8_t j=0; j<4; j++){
			codigo<<=1;						//Desplazamiento del numero a la izquierda una posicion
			codigo|= vector[i*4+j] & 0x01;	//Anade el bit asegurando 0 o 1
		}

		tarjeta[i]= tabla_codigos[codigo]; //Se guarda el caracter correspondiente al codigo de 4 bits
	}

	tarjeta[6]='\0'; //Finaliza la cadena de caracteres de la tarjeta

}

void comparar_tarjeta(const char* usuario, const char* leida) {
	if (strcmp(usuario, leida) == 1)
	PORTL |= (1 << PL7);   // Encender LED si coinciden
	else
	PORTL &= ~(1 << PL7);  // Apagar LED si no coinciden
}

int main(void) {
	setup();
	

	while (1) {
		procesar_tarjeta();
	}
}