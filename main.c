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

volatile long int tiempo_total=0; // guarda el tiempo total desde el reset para que milis() acceda a ella

uint8_t Valor_PINB=0b00000000; // "valor previo" del PINB 

// Variables para la interrupcion del SO_2 (es importante que sean globales)
volatile long int tA_aux=0; // variable intermedia para ayudar a calcular tA // g_ta_temporizador_ms
volatile bool SO2_flanco=0; // me dice si estoy en un flanco "par" o "impar"

// Variables para la interrupcion del SO_3 (es importante que sean globales)
volatile long int tB_aux=0; // variable intermedia para ayudar a calcular tA
volatile bool SO3_flanco=false; // me dice si estoy en un flanco "par" o "impar"

volatile long int tA=0;
volatile long int tB=0;
volatile double r = 0;

volatile float dinero = 0; // guardamos el valor actualizado de ESA persona pagando (se reinicia al validar a ESA persona)
volatile bool valido = 0;  // si la moneda que entra esta en un rango aceptable de "r", valido valdra "1"; "0" en caso contrario

// Variables del "monedero" main
long int tiempo_referencia_L2 = 0; // temporizador auxiliar para ver cuanto tiempo lleva encendido el led L2, ya que queremos que este encendido solo 1000 ms
bool enable_L2 = 0;	   // similar a "SO2_flanco": es una "bandera"
int personas = 0;  //**IMPORTANTE** Hablarla con todos, para que la usen en Integracion

long int tiempo_referencia_abierto_SW2 = 0;			//*Modificar*// // contador de cuantos milisegundos lleva abierto el Switch
long int tiempo_referencia_cerrandose_SW2 = 0;		//*Modificar*//

//ya no se usan ¿?
//volatile long int VALOR_DE_PRUEBA = 0;	//*Modificar*// // este es el numero de milisegundos que queremos que este "abierto" el Switch 2, y se medira a ojo; no deberan caer mas monedas en ese tiempo, MINIMO
//volatile long int tLed = 0;				//*Modificar*//
//volatile long int milisNoFunciona = 0;	//*Modificar*//
//volatile bool ledEncendido = false;		//*Modificar*//
//volatile int HeEntradoEnElIf = 0;		//*Modificar*//

long int overflows_timers = 0;

bool bandera_SW2 = 0;			//*Modificar*// //Mirar en qué punto tiene que cambiarse a 0!!! // Bandera que me permite frenarse el motor cuando toco SW2 y que impide el frenado el resto del codigo
volatile long int tiempo_actual_SW2 = 0;	//*Modificar*//
volatile int ciclo = 0;

void suma1aMilis(){
	tiempo_total++;
}

long int milis(){
	return tiempo_total;
}

ISR(TIMER3_COMPA_vect){
	suma1aMilis();
}

ISR(INT1_vect){//activar por flanco de bajada
	P_BK1 |= ( 1 << B_BK1);
	tiempo_referencia_abierto_SW2=milis();
	bandera_SW2 = 1; // ***REVISAR***
}


ISR(TIMER4_CAPT_vect){
	//P_L2 |= ( 1 << B_L2);
	overflows_timers=0;
	tA_aux = ICR4; // IGUAL ES PORQUE COGE EL OCRA, Y AHORA ESTA CONFIGURADO EL B ¿?
}


ISR(TIMER5_CAPT_vect){
	
	//P_L2 &= ~( 1 << B_L2);
	
	
	if (TCCR5B & (1 << ICES5)){ //if (SO3_flanco==false)
		
		P_L2 |= ( 1 << B_L2);
		
		// hay que mirar si esto funciona bien; igual que nos pasa con "milis()"
		
		
		if (overflows_timers==0){
			tA = (ICR5 - tA_aux);
		}
		if (overflows_timers!=0) {
			tA = (65536 - tA_aux) + ICR5 + 65536*(overflows_timers-1);
		}
		tB_aux = ICR5;
		
		SO3_flanco = true;
		P_L2 |= ( 1 << B_L2);
		
		
		/*
		TIMSK5 &= ~(1 << ICIE5);
		TCCR5B &= ~(1 << ICES5); // Cambiar a que se active la ISR por flanco de Bajada
		TIFR5 |= (1 << ICF5);          // Limpia la bandera de captura
		TIMSK5 |= (1 << ICIE5);
		*/
		
		overflows_timers = 0;
		
		TCCR5B &= ~(1 << ICES5);
	}

	
	else { //if (SO3_flanco == true)
		
		P_L2 &= ~( 1 << B_L2);
		
		
		SO3_flanco = false;
		
		
		if (overflows_timers==0){
			tB = (ICR5 - tB_aux);
		}
		if (overflows_timers!=0) {
			tB = (65536 - tB_aux) + ICR5 + 65536*(overflows_timers-1);
		}
		
		//
		/*
		TIMSK5 &= ~(1 << ICIE5);
		TCCR5B |= (1 << ICES5); // Cambiar a que se active la ISR por flanco de Subida
		TIFR5 |= (1 << ICF5);          // Limpia la bandera de captura
		TIMSK5 |= (1 << ICIE5);
		*/
		TCCR5B |= (1 << ICES5);
		
		r=(double)tB/(double)tA;
		
		if ((r>1) && (r<1.1)){ // 1<r<1.1
			dinero=dinero+1;
			P_BK1 &= ~( 1 << B_BK1);
			//valido=1;
			
		}
		
		if ((r>1.15) && (r<1.25)){ // 1.15<r<1.25
			dinero=dinero+0.5;
			P_BK1 &= ~( 1 << B_BK1);
			//valido=1;
		}
		
		if ((r>0.85) && (r<0.95)){ // 0.85<r<0.95
			dinero=dinero+0.2;
			P_BK1 &= ~( 1 << B_BK1);
			//valido=1;
		}
		
		if ((r>0.65) && (r<0.75)){ // 0.65<r<0.75
			dinero=dinero+0.1;
			P_BK1 &= ~( 1 << B_BK1);
			//valido=1;
		}
		
	}
}


ISR(TIMER4_COMPB_vect){
	overflows_timers++;
}



/*

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

*/

/*
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
*/	

/*
ISR(PCINT0_vect){
	compruebaPCINT0();
}
*/

void Monedero(){
	if (dinero >= 1.20){
		dinero = 0;
		tiempo_referencia_L2 = milis();
		enable_L2 = 1;
		personas++;
	} 
	if (enable_L2==1){
		if (tiempo_referencia_L2 - milis() < 1000){
			P_L2 |= ( 1 << B_L2);
		}
		else {
			P_L2 &= ~( 1 << B_L2);
			enable_L2 = 0;
		}
	}
	
	// FORMA ANTERIOR, CON CONSULTA PERIODICA
	/* 
	if (valido==1){
		P_BK1 &= ~( 1 << B_BK1); // habilitar motor poniendo a 0 el freno
		if ((((PIND & (1 << B_SW2)) >> B_SW2))==1){
			P_BK1 |= ( 1 << B_BK1); // deshabilitar motor poniendo a 1 el freno
			tiempo_referencia_abierto_SW2 = milis();
			valido = 0;
		}
	}
	if (tiempo_referencia_abierto_SW2 - milis () > VALOR_DE_PRUEBA ) {// Este valor habra que medirlo "a ojo"
		P_BK1 &= ~( 1 << B_BK1); // habilitar motor poniendo a 0 el freno
		if ((((PIND & (1 << B_EN1)) >> B_EN1))==0){
			P_BK1 |= ( 1 << B_BK1); // deshabilitar motor poniendo a 1 el freno
			tiempo_referencia_abierto_SW2 = 0;
		}
	}
	*/
	tiempo_actual_SW2 = milis();
	if ((((PIN_SW2 >> B_SW2) & 1) == 1)  && (tiempo_actual_SW2-tiempo_referencia_abierto_SW2>1000)){
		P_BK1 &= ~( 1 << B_BK1);	
		
		tiempo_referencia_cerrandose_SW2=milis();
		
	}
	
	
	if ((((PIN_SW2 >> B_SW2) & 1) == 0) && (bandera_SW2==1) && (tiempo_actual_SW2- tiempo_referencia_cerrandose_SW2 >1000) ) { //REVISAR SI ASI MIRO SI YA SE HA DESACTIVADO el switch Y VER SI PONER UN TIEMPO PARA DEJAR QUE SE FRENE
		// FALTA AÑADIR ALGO DEL TIPO "&& X-MILIS()>TIEMPO_QUE_QUIERO_QUE_ESTE_ABIERTO/CERRADO"
		P_BK1 |=( 1 << B_BK1); // deshabilitar motor poniendo a 1 el freno
		
		bandera_SW2 = 0; //CONFIRMAR QUE ES AQUÍ!
	}
	
}

void FuncionaLed(){
	P_L2 |= ( 1 << B_L2);
	P_L2 |= ( 1 << B_L2);
	P_L2 |= ( 1 << B_L2);
	P_L2 |= ( 1 << B_L2);
	//P_L2 &= ~( 1 << B_L2);
	/*
	milisNoFunciona=milis();
	if (milisNoFunciona>10000){
		// P_L2 |= ( 1 << B_L2);
		P_L2 &= ~( 1 << B_L2);
	} else if (milisNoFunciona>5000) {
		P_L2 |= ( 1 << B_L2);
		HeEntradoEnElIf = 1;
	}
	*/
	/*
		milisNoFunciona=milis();
		if ((!ledEncendido)&& ((milisNoFunciona - tLed) > 500)) {
			P_L2 |= (1 << B_L2);      // Encender LED
			tLed = milis();           // Guardar tiempo de encendido
			ledEncendido = true;      // Marcar que está encendido
		}
		else {
			if (milisNoFunciona - tLed > 500){ // Comprobar si han pasado 50 ms
				P_L2 &= ~(1 << B_L2); // Apagar LED
				ledEncendido = false; // Resetear flag para permitir otro ciclo
				tLed = milis();
			}
		}
	*/
	
}

void EncenderLed(){
	 P_L2 |= ( 1 << B_L2);
	 //P_L2 &= ~( 1 << B_L2);
}

void FuncionaMotor(){ //int tiempo_motor = 0;

//tiempo_motor = milis();

//((Valor_PINB & (1 << B_SO2)) >> B_SO2)
// ((PIN_SW2 >> B_SW2) & 1) == 1
	if((((PIN_SW2 >> B_SW2) & 1) == 0) /*|| (tiempo_motor - 2000 > 0)*/ ){
		P_BK1 |= ( 1 << B_BK1);
		P_L2 &= ~(1 << B_L2);
	} else {
		P_BK1 &= ~( 1 << B_BK1);
		P_L2 |= (1 << B_L2);
		
	}
	
}
	

void setup(){
	
	
	// Deshabilitar interrupciones
	cli();
	
	// Configurar LEDs como salida ( "1" )
	DDRL |= (1 << B_L2);
	
	// Configurar Sensores Opticos como Entradas ( "0" )
	//DDRB &= ~(1 << B_SO2);
	//DDRB &= ~(1 << B_SO3);
	
	DDRL &= ~(1 << B_SO2);
	DDRL &= ~(1 << B_SO3);
	
	// Me guardo el estado de los puertos
	Valor_PINB = PINB; // (me interesan B_SO2, B_SO3)
	
	// Configurar motores como salida
	DDRL |= (1 << B_EN1);	
	DDRL |= (1 << B_BK1);
	
	// Activamos el freno dinamico, ya que en todo el transcurso esta encendido; nunca se apaga
	//P_BK1 |= (1 << B_BK1);
	//Activamos el ENABLE, ya que en todo el transcurso esta encendido; nunca se apaga
	P_EN1 |= (1 << B_EN1);
	//Inicializamos a 1 el freno dinamico
	P_BK1 |= (1 << B_BK1);
	
	// Configurar interrupciones PCINT
	PCICR |= (1 << PCIE0);
	
	// Habilitar mascaras sensores opticos
	PCMSK0 |= (1 << PCINT2); // PCMSK0 |= (1 << B_SO2); // B_SO2 o PCINT2? o da igual?
	PCMSK0 |= (1 << PCINT3); // PCMSK0 |= (1 << B_SO3); // PCINT3?
	
	
	// Configurar interrupcion INT1 (flanco de subida)
	//EICRA |= (1 << 2*B_SW2) | (1 << (2*B_SW2+1))	;			// CONFIRMAR PRIMERO!!!
	EICRA &= ~(1 << 2*1);
	EICRA |= (1 << (2*1+1));
	
	// Habilitar mascara sensor mecanico
	EIMSK |= (1 << B_SW2);

	// TCCR3A X X X X X X 0 0 
	// TCCR3B X X X 0 1 X X X
	// Timer 3 en modo CTC (Top OCRnA)
	TCCR3A &= ~(1 << WGM30);  
	TCCR3A &= ~(1 << WGM31);
	TCCR3B |= (1 << WGM32);
	TCCR3B &= ~(1 << WGM33);
	
	// Timer 4 y 5 tambien en modo CTC (Top OCRnA)
	TCCR4A &= ~(1 << WGM40);
	TCCR4A &= ~(1 << WGM41);
	//TCCR4B |= (1 << WGM42);
	TCCR4B &= ~(1 << WGM42);
	TCCR4B &= ~(1 << WGM43);
	
	TCCR5A &= ~(1 << WGM50);
	TCCR5A &= ~(1 << WGM51);
	TCCR5B |= (1 << WGM52);
	TCCR5B &= ~(1 << WGM53);
	
	
	// Mascara para no tener preescalado 
	TCCR3B = (TCCR3B | (1 << CS30));
	TCCR3B = (TCCR3B & (~(1 << CS31)));
	TCCR3B = (TCCR3B & (~(1 << CS32)));
	
	// Mascara para no tener preescalado tampoco en los timers 4 y 5 
	TCCR4B = (TCCR4B | (1 << CS40));
	TCCR4B = (TCCR4B & (~(1 << CS41)));
	TCCR4B = (TCCR4B & (~(1 << CS42)));
	
	TCCR5B = (TCCR5B | (1 << CS50));
	TCCR5B = (TCCR5B & (~(1 << CS51)));
	TCCR5B = (TCCR5B & (~(1 << CS52)));
	
	// Sera el TOP, y vale 8000 porque 125ns*8000=1ms (para la funcion "milis()" )
	OCR3A = 8000; // **puede que de problema por "perdida de precision", tener cuidado
	//IGUAL HAY QUE USAR "input capture"...
	
	OCR4B = 65535;
	OCR5A = 65535;
	 // Configuramos la interrupción por OCRA
	 
	 // ****DUDA****
	 // ASEGURARNOS DE QUE ES "OCIE3A", ¿por que "A"? ¿es porque estamos usando ocrA? )
	 
	TIMSK3 |= (1 << OCIE3A);
	
	// ES AQUI !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		TIMSK4 |= (1 << OCIE4B);
	// ES AQUI !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	//TIMSK5 |= (1 << OCIE5A);
	// Habilitar interrupciones
	
	// habilitar input capture e inicializar 
	
	TCCR4B |= (1 << ICES4); // pongo un 1, es decir, se activa ante flanco de Subida
	TCCR5B |= (1 << ICES5); //
	
	// mascara del ISR
	TIMSK4 |= (1 << ICIE4);
	TIMSK5 |= (1 << ICIE5);
	
	
	//P_L2 &= ~( 1 << B_L2); // inicio con el led apagado
	
	// configurar interrupcion int 1 para que detecte por flanco de subida
	
	EICRA|=(1<<ISC11);
	EICRA|=(1<<ISC10);
	EIMSK|=(1<<INT1);
	
	sei();
}


int main(void)
{
    // Llamamos a setup 1 vez
	setup();
    while (1) 
    {
		//printf("tiempo_total vale: %ld\n",tiempo_total);
		/* Monedero(); */
		//FuncionaLed();
		//FuncionaMotor();		
		//tLed=milis();
		//printf("tLed = %ld\n", tLed);
		//printf("HeEntradoEnElIf = %ld\n", HeEntradoEnElIf);
		//EncenderLed();
		//P_L2 |= ( 1 << B_L2);
		//P_L2 &= ~( 1 << B_L2);
 		ciclo++;
    }
	
}

// HASTA AHORA:
// FUNCIONA MILIS() PERO TENGO QUE ACCEDER A ELLO A TRAVES DE UNA VARIABLE (RARO)

// MODIFICACION: AHORA SIEMPRE ACTIVO EL "ENABLE", CUANDO QUIERO QUE SE MUEVA EL MOTOR, APAGO EL BK; CUANDO QUIERO QUE SE PARE EL MOTOR, ENCIENDO BK


// COSAS RARAS:
//	 Poniendo break points, nos sale a veces el ICR con valor negativo
//	 No guarda los valores de una interrupcion a otra a menos que ponga break points (por lo tanto, en el codigo final, no va a calcularse nunca?)
//	 Problema de "no entra en el "else" la siguiente vez


// ORDENAR SETUP!!!