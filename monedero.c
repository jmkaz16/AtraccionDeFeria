#include "monedero.h"

// Variables para la interrupcion del SO_2 (es importante que sean globales)
volatile long int tA_aux = 0;  // variable intermedia para ayudar a calcular tA

// Variables para la interrupcion del SO_3 (es importante que sean globales)
volatile long int tB_aux = 0;  // variable intermedia para ayudar a calcular tB

volatile long int tA = 0;
volatile long int tB = 0;
volatile float r = 0;  // se va a calcular r como tB/tA (forma propuesta en el guion)

volatile float dinero = 0;  // guardamos el valor actualizado de ESA persona pagando (se reinicia al validar a ESA persona)

// Variables del "monedero" main
uint32_t tiempo_referencia_L2 = 0;  // temporizador auxiliar para ver cuanto tiempo lleva encendido el led L2, ya que queremos que este encendido solo 1000 ms
bool enable_L2 = 0;                 // bandera para controlar el encendido de led L2

uint32_t tiempo_referencia_abierto_SW2 = 0;     // contador de cuantos milisegundos lleva (cerrado) el Switch
uint32_t tiempo_referencia_cerrandose_SW2 = 0;  // contador de cuantos milisegundos lleva (cerrandose) el Switch

long int overflows_timers = 0;  // contador que ayuda a calcular bien tA y tB en caso de overflow en timers 4 o 5

bool bandera_SW2 = 0;            // Bandera que me permite frenarse el motor cuando toco SW2 y que impide el frenado el resto del codigo
uint32_t tiempo_actual_SW2 = 0;  // Variable que uso para controlar el tiempo referente a SW2 (ya que hacer referencia a "millis()" directamente dentro de un "if" no funciona
uint32_t tiempo_actual_L2 = 0;   // Variable que uso para controlar el tiempo referente a L2 (ya que hacer referencia a "millis()" directamente dentro de un "if" no funciona

// Interrupcion que se activa al tocar la barrera el sensor mecanico SW2
ISR(INT1_vect) {
    P_BK1 |= (1 << B_BK1);  // paro el motor (activando a 1 el freno)
    bandera_SW2 = 1;        // modifico aqui esta bandera que ayuda a controlar el motor en el flujo principal por consulta periodica
}

ISR(TIMER4_CAPT_vect) {    // SO2 detecta la moneda cayendo (se activa por flanco de subida);
    overflows_timers = 0;  // reseteo el contador
    tA_aux = ICR4;         //  me guardo el tiempo actual
}

ISR(TIMER5_CAPT_vect) {           // SO3 detecta la moneda cayendo
    if (TCCR5B & (1 << ICES5)) {  // aqui entrara cuando se haya interrumpido por flanco de subida (la "primera vez" entra por aqui)

        if (overflows_timers == 0) {  // si no ha habido overflows desde que empezo a caer la moneda, se calcula tA de forma simple
            tA = (ICR5 - tA_aux);     // tA sera el tiempo actual, menos el tiempo que me guarde en la interrupcion de SO2 (tal y como muestra el guion(
        } else {                      // si ha habido overflows, se calcula de esta otra manera
            tA = (65536 - tA_aux) + ICR5 + 65536 * (overflows_timers - 1);
        }

        tB_aux = ICR5;  // me guardo este nuevo tiempo para posteriormente calcular tB

        P_L2 |= (1 << B_L2);   // enciendo el led un instante para ser conscientes de que se ha detectado una moneda cayendo
        overflows_timers = 0;  // reseteo el contador

        TCCR5B &= ~(1 << ICES5);  // SO3 pasa a ser activada por flanco de bajada
    }

    else {  // aqui entrara cuando se haya interrumpido por flanco de bajada (la "segunda vez" entra por aqui)

        P_L2 &= ~(1 << B_L2);  // apago el led (el que se uso para ser conscientes de que se detecto una moneda cayendo)

        if (overflows_timers == 0) {  // calculamos tB de igual forma que tA
            tB = (ICR5 - tB_aux);
        } else {
            tB = (65536 - tB_aux) + ICR5 + 65536 * (overflows_timers - 1);
        }

        TCCR5B |= (1 << ICES5);  // SO3 pasa a ser activada por flanco de subida (de nuevo)

        r = (float)tB / (float)tA;  // calculo de r

        if ((r > MONEDA100_MIN) && (r < MONEDA100_MAX)) {  // moneda de 1�
            dinero = dinero + 1;

        } else if ((r > MONEDA50_MIN) && (r < MONEDA50_MAX)) {  // moneda de 50 cts
            dinero = dinero + 0.5;

        } else if ((r > MONEDA20_MIN) && (r < MONEDA20_MAX)) {  // moneda de 20 cts
            dinero = dinero + 0.2;

        } else if ((r > MONEDA10_MIN) && (r < MONEDA10_MAX)) {  // moneda de 10 cts
            dinero = dinero + 0.1;

        } else {                     // en caso de que no haya sido ninguna de las monedas anteriores
            P_BK1 &= ~(1 << B_BK1);  // empiezo a cerrar la barrera (quitando el freno)
            tiempo_referencia_cerrandose_SW2 = millis();
        }
    }
}

// Metodo por el que el contador overflows_timers aumenta
ISR(TIMER4_COMPB_vect) {
    overflows_timers++;
}

// Deshabilita monedero: se apaga el led, se frena la barrera, y se deshabilitan las interrupciones
void clearMonedero() {
    P_L2 &= ~(1 << B_L2);
    P_BK1 |= (1 << B_BK1);
    TIMSK4 &= ~(1 << ICIE4);
    TIMSK5 &= ~(1 << ICIE5);
}

// Flujo principal de la funcion
void monedero() {
    if (dinero >= 1.20) {  // si el dinero pagado por 1 persona supera cierto valor, reseteamos el dinero,
        dinero = 0;        // aumentamos en 1 la cantidad de personas a la cola, y empezamos a encender el led durante 1 segundo
        tiempo_referencia_L2 = millis();
        enable_L2 = 1;
        personas_cnt++;
    }

    if (enable_L2 == 1) {  // control de encendido del led durante 1 segundo despues del pago exitoso de 1 persona
        tiempo_actual_L2 = millis();
        if (tiempo_actual_L2 - tiempo_referencia_L2 < 1000) {
            P_L2 |= (1 << B_L2);
        } else {
            P_L2 &= ~(1 << B_L2);
            enable_L2 = 0;
        }
    }

    // control de la barrera:
    // cuando se detecta una moneda no valida se empieza a cerrar
    // cuando pasa un tiempo "tiempo_referencia_cerrandose_SW2", se frena, y se queda la barrera cerrada
    // cuando pasa otro tiempo "tiempo_referencia_abierto_SW2" vuelve a ponerse en movimiento el motor
    // hasta que la barrera toque el sensor mecanico SW2, que sera cuando se encuentre en la posicion de "totalmente abierto"
    // es decir, la misma posicion en la que empezo

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
    //DDRL |= (1 << B_L2);

    // Configurar Sensores Opticos como Entradas ( "0" )

    DDRL &= ~(1 << B_SO2);
    DDRL &= ~(1 << B_SO3);

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

    // Timer 4 modo normal; 5 en modo CTC (Top OCRnA)
    TCCR4A &= ~(1 << WGM40);
    TCCR4A &= ~(1 << WGM41);
    TCCR4B &= ~(1 << WGM42);
    TCCR4B &= ~(1 << WGM43);

    TCCR5A &= ~(1 << WGM50);
    TCCR5A &= ~(1 << WGM51);
    TCCR5B |= (1 << WGM52);
    TCCR5B &= ~(1 << WGM53);

    // Mascara para no tener preescalado tampoco en los timers 4 y 5
    TCCR4B = (TCCR4B | (1 << CS40));
    TCCR4B = (TCCR4B & (~(1 << CS41)));
    TCCR4B = (TCCR4B & (~(1 << CS42)));

    TCCR5B = (TCCR5B | (1 << CS50));
    TCCR5B = (TCCR5B & (~(1 << CS51)));
    TCCR5B = (TCCR5B & (~(1 << CS52)));

    OCR4B = 65535;
    OCR5A = 65535;

    TIMSK3 |= (1 << OCIE3A);
    TIMSK4 |= (1 << OCIE4B);

    // habilitar input capture e inicializar

    TCCR4B |= (1 << ICES4);  // pongo un 1, es decir, se activa ante flanco de Subida
    TCCR5B |= (1 << ICES5);

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