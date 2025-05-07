/*
 * AtraccionDeFeria.c
 *
 * Created: 13/04/2025 17:10:27
 * Author : rodri & rafa
 */ 

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "pinout.h"

long int tiempo_total=0; // guarda el tiempo total desde el reset para que milis() acceda a ella

uint8_t Valor_PINB=0b00000000; // "valor previo" del PINB 

// Variables para la interrupcion del SO_2 (es importante que sean globales)
long int tA_aux=0; // variable intermedia para ayudar a calcular tA
bool SO2_flanco=0; // me dice si estoy en un flanco "par" o "impar"

// Variables para la interrupcion del SO_3 (es importante que sean globales)
long int tB_aux=0; // variable intermedia para ayudar a calcular tA
bool SO3_flanco=0; // me dice si estoy en un flanco "par" o "impar"

long int tA=0;
long int tB=0;
double r = 0;

float dinero = 0; // guardamos el valor actualizado de ESA persona pagando (se reinicia al validar a ESA persona)
bool valido = 0;  // si la moneda que entra esta en un rango aceptable de "r", valido valdra "1"; "0" en caso contrario

// Variables del "monedero" main
<<<<<<< Updated upstream
long int T_L2 = 0; // temporizador auxiliar para ver cuanto tiempo lleva encendido el led L2, ya que queremos que este encendido solo 1000 ms
bool L2_en = 0;    // similar a "SO2_flanco": es una "bandera"
int personas = 0;  //**IMPORTANTE** Hablarla con todos, para que la usen en Integracion

long int cont_SW = 0; // contador de cuantos milisegundos lleva abierto el Switch
long int VALOR_DE_PRUEBA = 0; // este es el numero de milisegundos que queremos que este "abierto" el Switch 2, y se medira a ojo; no deberan caer mas monedas en ese tiempo, MINIMO

=======
uint32_t tiempo_referencia_L2 = 0; // temporizador auxiliar para ver cuanto tiempo lleva encendido el led L2, ya que queremos que este encendido solo 1000 ms
bool enable_L2 = 0;	   // similar a "SO2_flanco": es una "bandera"
int personas = 0;  //**IMPORTANTE** Hablarla con todos, para que la usen en Integracion

uint32_t tiempo_referencia_abierto_SW2 = 0;			//*Modificar*// // contador de cuantos milisegundos lleva abierto el Switch
uint32_t tiempo_referencia_cerrandose_SW2 = 0;		//*Modificar*//
>>>>>>> Stashed changes


<<<<<<< Updated upstream
ISR(INT1_vect){
}
=======
bool bandera_SW2 = 0;			//*Modificar*// //Mirar en qué punto tiene que cambiarse a 0!!! // Bandera que me permite frenarse el motor cuando toco SW2 y que impide el frenado el resto del codigo
uint32_t tiempo_actual_SW2 = 0;	//*Modificar*//
uint32_t tiempo_actual_L2 = 0;
volatile int ciclo = 0;
>>>>>>> Stashed changes


/*
void suma1aMilis(){
	tiempo_total++;
}

long int milis(){
	return tiempo_total;
}

ISR(TIMER3_COMPA_vect){
	suma1aMilis();
}
*/



<<<<<<< Updated upstream
void interrupcion_SO2(){
	if (SO2_flanco==0){
		tA_aux = milis();
		SO2_flanco = 1;
	}
	else {
		SO2_flanco = 0;
	}
}

void interrupcion_SO3(){
	if (SO3_flanco==0){
		tA=milis()-tA_aux;
		tB_aux = milis();
		SO3_flanco = 1;
	}
	else {
		SO3_flanco = 0;
		tB=tB_aux-milis();
		r=(double)tB/(double)tA;
		
		if ((r>1) && (r<1.1)){ // 1<r<1.1
			dinero=dinero+1;
			valido=1;
		}
		
		else if ((r>1.15) && (r<1.25)){ // 1.15<r<1.25
			dinero=dinero+0.5;
			valido=1;
		}
		
		else if ((r>0.85) && (r<0.95)){ // 0.85<r<0.95
			dinero=dinero+0.2;
			valido=1;
		}
		
		else if ((r>0.65) && (r<0.75)){ // 0.65<r<0.75
			dinero=dinero+0.1;
			valido=1;
		}
		
		else {
			valido=0;
		}
	}
}

void compruebaPCINT0(){
	//if (bit2dePCINT0!=PORTB(0))
	// (Valor_PINB & (1 << B_SO2)) >> B_SO2
	
	if (((Valor_PINB & (1 << B_SO2)) >> B_SO2) != ((PINB & (1 << B_SO2)) >> B_SO2)){	
		interrupcion_SO2();			
	} // igual es posible que quisiesemos no hacer "else if" sino todo "if...; if...; ..."
	else if (((Valor_PINB & (1 << B_SO3)) >> B_SO3) != ((PINB & (1 << B_SO3)) >> B_SO3)) {
		interrupcion_SO3();		
	}
	
	Valor_PINB = PINB;
}
	
ISR(PCINT0_vect){
	compruebaPCINT0();
}

void Monedero(){
	if (dinero >= 1.20){
		dinero = 0;
		T_L2 = milis();
		L2_en = 1;
		personas++;
	} 
	if (L2_en==1){
		if (T_L2 - milis() < 1000){
=======
ISR(INT1_vect){//activar por flanco de bajada
	P_BK1 |= ( 1 << B_BK1);
	
	bandera_SW2 = 1; 
}


ISR(TIMER4_CAPT_vect){
	
	overflows_timers=0;
	tA_aux = ICR4; 
}


ISR(TIMER5_CAPT_vect){
	
	
	if (TCCR5B & (1 << ICES5)){ 
		
		P_L2 |= ( 1 << B_L2);
		
		
		
		
		if (overflows_timers==0){
			tA = (ICR5 - tA_aux);
		}
		else { 
			tA = (65536 - tA_aux) + ICR5 + 65536*(overflows_timers-1);
		}
		tB_aux = ICR5;
		P_L2 |= ( 1 << B_L2);
		
		overflows_timers = 0;
		
		TCCR5B &= ~(1 << ICES5);
	}

	
	else { 
		
		P_L2 &= ~( 1 << B_L2);
		
		
		
		
		
		if (overflows_timers==0){
			tB = (ICR5 - tB_aux);
		}
		else { 
			tB = (65536 - tB_aux) + ICR5 + 65536*(overflows_timers-1);
		}
		
		TCCR5B |= (1 << ICES5);
		
		r=(float)tB/(float)tA; 
		
		if ((r>1.28) && (r<1.35)){ 
			dinero=dinero+1;
			}
		
		else if ((r>1.2) && (r<1.28)){ 
			dinero=dinero+0.2;
			
		}
		
		else if ((r>1.05) && (r<1.2)){ 
			dinero=dinero+0.1;
			
		}
		else {
			P_BK1 &= ~( 1 << B_BK1);
			tiempo_referencia_cerrandose_SW2=millis();
		}
		
	}
}


ISR(TIMER4_COMPB_vect){
	overflows_timers++;
}


void Monedero(){
	if (dinero >= 1.20){
		dinero = 0;
		tiempo_referencia_L2 = millis();
		enable_L2 = 1;
		personas++;
	} 
	if (enable_L2==1){
		tiempo_actual_L2 = millis();
		if (tiempo_actual_L2 - tiempo_referencia_L2 < 1000){
>>>>>>> Stashed changes
			P_L2 |= ( 1 << B_L2);
		}
		else {
			P_L2 &= ~( 1 << B_L2);
			L2_en = 0;
		}
	}
<<<<<<< Updated upstream
	if (valido==1){
		P_EN1 |= ( 1 << B_EN1); // habilitar en_M1 = 1
		if ((((PIND & (1 << B_EN1)) >> B_EN1))==1){
			P_EN1 &= ~( 1 << B_EN1); // deshabilitar en_M1 = 0
			cont_SW = milis();
			valido = 0;
		}
	}
	if (cont_SW - milis () > VALOR_DE_PRUEBA ) {// Este valor habra que medirlo "a ojo"
		P_EN1 |= ( 1 << B_EN1); // habilitar en_M1 = 1
		if ((((PIND & (1 << B_EN1)) >> B_EN1))==0){
			P_EN1 &= ~( 1 << B_EN1); // deshabilitar en_M1 = 0
			cont_SW = 0;
		}
=======
	
	tiempo_actual_SW2 = millis();
	
	if ((((PIN_SW2 >> B_SW2) & 1) == 1)  && (tiempo_actual_SW2-tiempo_referencia_abierto_SW2>1000)){
		P_BK1 &= ~( 1 << B_BK1);	
		
		
	}
	
	
	if ((((PIN_SW2 >> B_SW2) & 1) == 0) && (bandera_SW2==1) && (tiempo_actual_SW2- tiempo_referencia_cerrandose_SW2 >1000) ) { 
		
		P_BK1 |=( 1 << B_BK1); // deshabilitar motor poniendo a 1 el freno
		
		bandera_SW2 = 0; 
		tiempo_referencia_abierto_SW2=millis();
>>>>>>> Stashed changes
	}
	
}

<<<<<<< Updated upstream
=======

	

>>>>>>> Stashed changes
void setup(){
	
	// Deshabilitar interrupciones
	cli();
	
	// Configurar LEDs como salida ( "1" )
	DDRL |= (1 << B_L2);
	
<<<<<<< Updated upstream
	// Configurar Sensores Opticos como Entradas ( "0" )
	DDRB &= ~(1 << B_SO2);
	DDRB &= ~(1 << B_SO3);
=======
	DDRL &= ~(1 << B_SO2);
	DDRL &= ~(1 << B_SO3);
>>>>>>> Stashed changes
	
	// Me guardo el estado de los puertos
	Valor_PINB = PINB; // (me interesan B_SO2, B_SO3)
	
	// Configurar motores como salida
	DDRL |= (1 << B_EN1);	
	DDRL |= (1 << B_BK1);
	
	// Activamos el freno dinamico, ya que en todo el transcurso esta encendido; nunca se apaga
	P_BK1 |= (1 << B_BK1);
	
	// Configurar interrupciones PCINT
	PCICR |= (1 << PCIE0);
	
	// Habilitar mascaras sensores opticos
	PCMSK0 |= (1 << PCINT2); // PCMSK0 |= (1 << B_SO2); // B_SO2 o PCINT2? o da igual?
	PCMSK0 |= (1 << PCINT3); // PCMSK0 |= (1 << B_SO3); // PCINT3?
	
	// Configurar interrupcion INT1 (flanco de subida)
	EICRA |= (1 << 2*B_SW2) | (1 << (2*B_SW2+1))	;			// CONFIRMAR PRIMERO!!!
	
	// Habilitar mascara sensor mecanico
	EIMSK |= (1 << B_SW2);


	//TCCR3A &= ~(1 << WGM30);  
	//TCCR3A &= ~(1 << WGM31);
	//TCCR3B |= (1 << WGM32);
	//TCCR3B &= ~(1 << WGM33);
	
<<<<<<< Updated upstream
=======
	// Timer 4 y 5 tambien en modo CTC (Top OCRnA)
	TCCR4A &= ~(1 << WGM40);
	TCCR4A &= ~(1 << WGM41);
	TCCR4B &= ~(1 << WGM42);
	TCCR4B &= ~(1 << WGM43);
	
	TCCR5A &= ~(1 << WGM50);
	TCCR5A &= ~(1 << WGM51);
	TCCR5B |= (1 << WGM52);
	TCCR5B &= ~(1 << WGM53);
	
	
>>>>>>> Stashed changes
	// Mascara para no tener preescalado 
	//TCCR3B = (TCCR3B | (1 << CS30));
	//TCCR3B = (TCCR3B & (~(1 << CS31)));
	//TCCR3B = (TCCR3B & (~(1 << CS32)));
	
	// Sera el TOP, y vale 8000 porque 125ns*8000=1ms (para la funcion "milis()" )
	//OCR3A = 8000; // **puede que de problema por "perdida de precision", tener cuidado
	
<<<<<<< Updated upstream
	 // Configuramos la interrupción por OCRA
	 
	 // ****DUDA****
	 // ASEGURARNOS DE QUE ES "OCIE3A", ¿por qué "A"? ¿es porque estamos usando ocrA? )
	TIMSK3 = (1 << OCIE3A);
	
	// Habilitar interrupciones
=======
	OCR4B = 65535;
	OCR5A = 65535;
	 
	//TIMSK3 |= (1 << OCIE3A);
	TIMSK4 |= (1 << OCIE4B);

	
	// habilitar input capture e inicializar 
	
	TCCR4B |= (1 << ICES4); // pongo un 1, es decir, se activa ante flanco de Subida
	TCCR5B |= (1 << ICES5); //
	
	// mascara del ISR
	TIMSK4 |= (1 << ICIE4);
	TIMSK5 |= (1 << ICIE5);
	
	EICRA|=(1<<ISC11);
	EICRA|=(1<<ISC10);
	EIMSK|=(1<<INT1);
	
	
	P_BK1 &= ~(1 << B_BK1);
	
	
>>>>>>> Stashed changes
	sei();
}


int main(void)
{
    // Llamamos a setup 1 vez
	setup();
    while (1) 
    {
		
		Monedero();
<<<<<<< Updated upstream
=======
 		ciclo++;
>>>>>>> Stashed changes
    }
}

