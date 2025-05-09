#include "monedero.h"

// volatile long int tiempo_total = 0;  // guarda el tiempo total desde el reset para que milis() acceda a ella

uint8_t Valor_PINB = 0b00000000;  // "valor previo" del PINB

// Variables para la interrupcion del SO_2 (es importante que sean globales)
volatile long int tA_aux = 0;  // variable intermedia para ayudar a calcular tA // g_ta_temporizador_ms
volatile bool SO2_flanco = 0;  // me dice si estoy en un flanco "par" o "impar"

// Variables para la interrupcion del SO_3 (es importante que sean globales)
volatile long int tB_aux = 0;      // variable intermedia para ayudar a calcular tA
volatile bool SO3_flanco = false;  // me dice si estoy en un flanco "par" o "impar"

volatile long int tA = 0;
volatile long int tB = 0;
volatile float r = 0;

volatile float dinero = 0;  // guardamos el valor actualizado de ESA persona pagando (se reinicia al validar a ESA persona)
volatile bool valido = 0;   // si la moneda que entra esta en un rango aceptable de "r", valido valdra "1"; "0" en caso contrario

// Variables del "monedero" main
uint32_t tiempo_referencia_L2 = 0;  // temporizador auxiliar para ver cuanto tiempo lleva encendido el led L2, ya que queremos que este encendido solo 1000 ms
bool enable_L2 = 0;                 // similar a "SO2_flanco": es una "bandera"

uint32_t tiempo_referencia_abierto_SW2 = 0;     //*Modificar*// // contador de cuantos milisegundos lleva abierto el Switch
uint32_t tiempo_referencia_cerrandose_SW2 = 0;  //*Modificar*//

long int overflows_timers = 0;

bool bandera_SW2 = 0;            // Bandera que me permite frenarse el motor cuando toco SW2 y que impide el frenado el resto del codigo
uint32_t tiempo_actual_SW2 = 0;  //*Modificar*//
uint32_t tiempo_actual_L2 = 0;
volatile int ciclo = 0;

// void suma1aMilis(){
//	tiempo_total++;
// }

// long int milis(){
//	return tiempo_total;
// }

// ISR(TIMER3_COMPA_vect){
//	suma1aMilis();
// }

ISR(INT1_vect) {
    P_BK1 |= (1 << B_BK1);
    bandera_SW2 = 1;
}

ISR(TIMER4_CAPT_vect) {
    overflows_timers = 0;
    tA_aux = ICR4;
}

ISR(TIMER5_CAPT_vect) {
    if (TCCR5B & (1 << ICES5)) {
        P_L2 |= (1 << B_L2);

        if (overflows_timers == 0) {
            tA = (ICR5 - tA_aux);
        } else {
            tA = (65536 - tA_aux) + ICR5 + 65536 * (overflows_timers - 1);
        }
        tB_aux = ICR5;

        P_L2 |= (1 << B_L2);
        overflows_timers = 0;

        TCCR5B &= ~(1 << ICES5);
    }

    else {
        P_L2 &= ~(1 << B_L2);
        if (overflows_timers == 0) {
            tB = (ICR5 - tB_aux);
        } else {  // cambiar a else
            tB = (65536 - tB_aux) + ICR5 + 65536 * (overflows_timers - 1);
        }

        TCCR5B |= (1 << ICES5);

        r = (float)tB / (float)tA;  // cambiar r a float y conversion de tipo tambien

        if ((r > MONEDA100_MIN) && (r < MONEDA100_MAX)) {  // 1<r<1.1
            dinero = dinero + 1;

        } else if ((r > MONEDA50_MIN) && (r < MONEDA50_MAX)) {  // 1.15<r<1.25
            dinero = dinero + 0.5;

        } else if ((r > MONEDA20_MIN) && (r < MONEDA20_MAX)) {  // 0.85<r<0.95
            dinero = dinero + 0.2;

        } else if ((r > MONEDA10_MIN) && (r < MONEDA10_MAX)) {  // 0.45<r<0.55
            dinero = dinero + 0.1;

        } else {
            P_BK1 &= ~(1 << B_BK1);
            tiempo_referencia_cerrandose_SW2 = millis();
        }
    }
}

ISR(TIMER4_COMPB_vect) {
    overflows_timers++;
}
void clearMonedero(){
	P_L2 &= ~(1 << B_L2);
	P_BK1 |= (1 << B_BK1);
	TIMSK4 &=~(1 << ICIE4);
	TIMSK5 &=~(1 << ICIE5);
	
}

void monedero() {
    if (dinero >= 1.20) {
        dinero = 0;
        tiempo_referencia_L2 = millis();
        enable_L2 = 1;
        personas_cnt++;
    }
    if (enable_L2 == 1) {
        tiempo_actual_L2 = millis();
        if (tiempo_actual_L2 - tiempo_referencia_L2 < 1000) {
            P_L2 |= (1 << B_L2);
        } else {
            P_L2 &= ~(1 << B_L2);
            enable_L2 = 0;
        }
    }

    tiempo_actual_SW2 = millis();
    if ((((PIN_SW2 >> B_SW2) & 1) == 1) && (tiempo_actual_SW2 - tiempo_referencia_abierto_SW2 > 1000)) {
        P_BK1 &= ~(1 << B_BK1);
    }

    if ((((PIN_SW2 >> B_SW2) & 1) == 0) && (bandera_SW2 == 1) && (tiempo_actual_SW2 - tiempo_referencia_cerrandose_SW2 > 1000)) {
        P_BK1 |= (1 << B_BK1);  // deshabilitar motor poniendo a 1 el freno

        bandera_SW2 = 0;
        tiempo_referencia_abierto_SW2 = millis();
    }
}

void monederoSetup() {
    // Deshabilitar interrupciones
    cli();

    // Configurar LEDs como salida ( "1" )
    DDRL |= (1 << B_L2);

    // Configurar Sensores Opticos como Entradas ( "0" )

    DDRL &= ~(1 << B_SO2);
    DDRL &= ~(1 << B_SO3);

    // Me guardo el estado de los puertos
    Valor_PINB = PINB;  // (me interesan B_SO2, B_SO3)

    // Configurar motores como salida
    DDRL |= (1 << B_EN1);
    DDRL |= (1 << B_BK1);

    // Activamos el ENABLE, ya que en todo el transcurso esta encendido; nunca se apaga
    P_EN1 |= (1 << B_EN1);
    // Inicializamos a 1 el freno dinamico
    P_BK1 |= (1 << B_BK1);

    // Configurar interrupciones PCINT
    PCICR |= (1 << PCIE0);

    // Habilitar mascaras sensores opticos
    PCMSK0 |= (1 << PCINT2);  // PCMSK0 |= (1 << B_SO2); // B_SO2 o PCINT2? o da igual?
    PCMSK0 |= (1 << PCINT3);  // PCMSK0 |= (1 << B_SO3); // PCINT3?

    // Configurar interrupcion INT1 (flanco de subida)
    // EICRA |= (1 << 2*B_SW2) | (1 << (2*B_SW2+1))	;			// CONFIRMAR PRIMERO!!!
    EICRA &= ~(1 << 2 * 1);
    EICRA |= (1 << (2 * 1 + 1));

    // Habilitar mascara sensor mecanico
    EIMSK |= (1 << B_SW2);

    // Timer 3 en modo CTC (Top OCRnA)
    // TCCR3A &= ~(1 << WGM30);
    // TCCR3A &= ~(1 << WGM31);
    // TCCR3B |= (1 << WGM32);
    // TCCR3B &= ~(1 << WGM33);

    // Timer 4 y 5 tambien en modo CTC (Top OCRnA)
    TCCR4A &= ~(1 << WGM40);
    TCCR4A &= ~(1 << WGM41);
    TCCR4B &= ~(1 << WGM42);
    TCCR4B &= ~(1 << WGM43);

    TCCR5A &= ~(1 << WGM50);
    TCCR5A &= ~(1 << WGM51);
    TCCR5B |= (1 << WGM52);
    TCCR5B &= ~(1 << WGM53);

    // Mascara para no tener preescalado
    // TCCR3B = (TCCR3B | (1 << CS30));
    // TCCR3B = (TCCR3B & (~(1 << CS31)));
    // TCCR3B = (TCCR3B & (~(1 << CS32)));

    // Mascara para no tener preescalado tampoco en los timers 4 y 5
    TCCR4B = (TCCR4B | (1 << CS40));
    TCCR4B = (TCCR4B & (~(1 << CS41)));
    TCCR4B = (TCCR4B & (~(1 << CS42)));

    TCCR5B = (TCCR5B | (1 << CS50));
    TCCR5B = (TCCR5B & (~(1 << CS51)));
    TCCR5B = (TCCR5B & (~(1 << CS52)));

    // Sera el TOP, y vale 8000 porque 125ns*8000=1ms (para la funcion "milis()" )
    // OCR3A = 8000;

    OCR4B = 65535;
    OCR5A = 65535;

    TIMSK3 |= (1 << OCIE3A);

    TIMSK4 |= (1 << OCIE4B);

    // habilitar input capture e inicializar

    TCCR4B |= (1 << ICES4);  // pongo un 1, es decir, se activa ante flanco de Subida
    TCCR5B |= (1 << ICES5);  //

    // mascara del ISR
    TIMSK4 |= (1 << ICIE4);
    TIMSK5 |= (1 << ICIE5);

    // configurar interrupcion int 1 para que detecte por flanco de subida

    EICRA |= (1 << ISC11);
    EICRA |= (1 << ISC10);
    EIMSK |= (1 << INT1);

    P_BK1 &= ~(1 << B_BK1);

    sei();
}

// HASTA AHORA:
// FUNCIONA MILIS() PERO TENGO QUE ACCEDER A ELLO A TRAVES DE UNA VARIABLE (RARO)

// MODIFICACION: AHORA SIEMPRE ACTIVO EL "ENABLE", CUANDO QUIERO QUE SE MUEVA EL MOTOR, APAGO EL BK; CUANDO QUIERO QUE SE PARE EL MOTOR, ENCIENDO BK

// COSAS RARAS:
//	 Poniendo break points, nos sale a veces el ICR con valor negativo
//	 No guarda los valores de una interrupcion a otra a menos que ponga break points (por lo tanto, en el codigo final, no va a calcularse nunca?)
//	 Problema de "no entra en el "else" la siguiente vez

// ORDENAR SETUP!!!