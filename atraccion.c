#include "atraccion.h"

uint16_t t_total = 0;                // tiempo total de parpadeo en decimas de segundo
uint16_t t_encendido = 0;            // tiempo de parpadeo en decimas de segundo
volatile uint16_t parpadeo_cnt = 0;  // contador de parpadeo en decimas de segundo

volatile uint8_t personas_cnt = 0;  // contador de personas en la atraccion

volatile uint16_t t_sw3_cnt = 0;  // contador de tiempo para antirrebotes del sw3 en decimas de segundo
volatile uint8_t estado_sw3 = 1;
volatile uint8_t last_estado_sw3 = 1;  // variable para almacenar el estado anterior del sensor mecanico SW3

volatile uint16_t t_s04_cnt = 0;  // contador de tiempo para el sensor optico 4 en milisegundos
volatile uint16_t t_s05_cnt = 0;  // contador de tiempo para el sensor optico 5 en milisegundos

volatile uint8_t timer4_cnt = 0;  // contador de tiempo para sustituir el timer 4 de 100ms

bool espera_flag = false;          // bandera de contador de espera
volatile uint16_t espera_cnt = 0;  // contador de tiempo espera en decimas de segundo

bool atraccion_flag = false;          // bandera de movimiento de la atraccion
uint16_t t_atraccion = 0;             // tiempo de duracion de la atraccion en decimas de segundo
volatile uint16_t atraccion_cnt = 0;  // contador de duracion de la atraccion en decimas de segundo

volatile uint16_t dientes_cnt = 0;   // contador de dientes del engranaje
volatile uint16_t t_subida_cnt = 0;  // contador de tiempo de subida en milisegundos
uint16_t t_subida = 0;               // tiempo de subida en milisegundos

// Configuracion de entradas, salidas, interrupciones y temporizadores de la atraccion
void atraccionSetup() {
    // Deshabilitar interrupciones
    cli();

    /*// Configurar LEDs como salida
    DDRK |= (1 << B_L3);
    DDRK |= (1 << B_L4);

    // Configurar motores como salida
    DDRK |= (1 << B_EN2);
    DDRK |= (1 << B_DI2);*/

    // Configurar interrupcion INT0 (flanco de bajada)
    EICRA &= ~(1 << 2 * B_SW1);
    EICRA |= (1 << (2 * B_SW1 + 1));

    // Habilitar mascara sensor mecanico SW1
    EIMSK |= (1 << B_SW1);

    // Configurar interrupcion INT2 e INT3 (flanco de bajada)
    EICRA &= ~(1 << (2 * B_SO4));
    EICRA |= (1 << (2 * B_SO4 + 1));
    EICRA &= ~(1 << (2 * B_SO5));
    EICRA |= (1 << (2 * B_SO5 + 1));

    // Habilitar mascara sensores opticos
    EIMSK |= (1 << B_SO4);
    EIMSK |= (1 << B_SO5);

    // Configurar interrupcion PCINT20
    PCICR |= (1 << PCIE2);

    // Habilitar mascara sensor mecanico SW3
    PCMSK2 = 0;
    PCMSK2 |= (1 << PCINT20);

    // Timer 0 en modo CTC (Top OCR0A)
    TCCR0A &= ~(1 << WGM00);
    TCCR0A |= (1 << WGM01);
    TCCR0B &= ~(1 << WGM02);

    // Configurar mascara para preescalado de 64
    TCCR0B |= (1 << CS00);
    TCCR0B |= (1 << CS01);
    TCCR0B &= ~(1 << CS02);

    // Modificar TOP
    OCR0A = 125 - 1;  // 125ns*125*64=1ms

    // Configurar el registro de interrupcion
    TIMSK0 |= (1 << OCIE0A);  // habilitar interrupcion por OCRA

    // Configurar semilla para el generador de numeros aleatorios
    srand(time(NULL));

    // Inicializar el estado del sensor mecanico SW3
    last_estado_sw3 = PINK & (1 << B_SW3);

    // Parpadeo inicial
    setParpadeo(500, 10000);

    // Habilitar interrupciones
    sei();
}

// Funcion principal de la atraccion (consulta periodica en el main)
void atraccion() {
    if (personas_cnt >= PERSONAS_MAX) {
        setBit(P_L3, B_L3);  // encender LED 3
        espera_flag = true;  // activar bandera de espera
        if (espera_cnt >= T_ESPERA) {
            espera_flag = false;  // desactivar bandera de espera
            espera_cnt = 0;       // reiniciar contador de espera
            personas_cnt -= PERSONAS_MAX;
            clrBit(P_L3, B_L3);  // apagar LED 3
            setAtraccion();      // llamar a la funcion de atraccion
        }
    }
    moverAtraccion();  // llamar a la funcion de movimiento de la atraccion
}

// Activar la atraccion
void setAtraccion() {
    t_atraccion = (rand() % (T_ATRACCION_MAX - T_ATRACCION_MIN)) + T_ATRACCION_MIN;  // tiempo de duracion de la atraccion
    t_subida = T_SUBIDA_MIN;                                                         // reiniciar tiempo de subida
    atraccion_flag = true;                                                           // activar bandera de movimiento de la atraccion
    setParpadeo(500, 500);                                                           // mantener el LED 4 encendido
    cli();                                                                           // deshabilitar interrupciones globales
    EIMSK |= (1 << B_SO4) | (1 << B_SO5);                                            // habilitar mascara de interrupcion por sensores opticos
    sei();                                                                           // habilitar interrupciones globales
}

// Desactivar la atraccion
void clrAtraccion() {
    clrBit(P_EN2, B_EN2);                     // apagar motor
    EIMSK &= ~((1 << B_SO4) | (1 << B_SO5));  // deshabilitar mascara de interrupcion por sensores opticos
    atraccion_flag = false;                   // desactivar bandera de movimiento de la atraccion
    atraccion_cnt = 0;                        // reiniciar contador de duracion de la atraccion
    setParpadeo(500, 10000);                  // encender 500ms y apagar 10000ms
}

// Mover la atraccion
void moverAtraccion() {
    if (atraccion_flag) {
        setBit(P_EN2, B_EN2);  // encender motor
        clrBit(P_L3, B_L3);
        if (t_subida < T_SUBIDA_F1) {
            if (t_subida_cnt >= t_subida) {
                PINK |= (1 << B_DI2);                 // cambiar sentido de giro del motor (hace toggle)
                t_subida += dientes_cnt * T_DIENTES;  // incrementar tiempo de subida
                cli();                                // deshabilitar interrupciones globales
                EIMSK &= ~(1 << B_SO4);               // deshabilitar mascara de interrupcion para el sensor optico 4 (dientes)
                sei();                                // habilitar interrupciones globales
                dientes_cnt = 0;                      // reiniciar contador de dientes
                t_subida_cnt = 0;                     // reiniciar contador de tiempo de subida
            }
        } else {  // if(t_subida < t_subida_f2)
            if (t_subida_cnt >= t_subida - 150) {
                PINK |= (1 << B_DI2);  // cambiar sentido de giro del motor (hace toggle)
                t_subida += dientes_cnt * T_DIENTES_F1;
                cli();                   // deshabilitar interrupciones globales
                EIMSK &= ~(1 << B_SO4);  // deshabilitar mascara de interrupcion para el sensor optico 4 (dientes)
                sei();                   // habilitar interrupciones globales
                dientes_cnt = 0;         // reiniciar contador de dientes
                t_subida_cnt = 0;        // reiniciar contador de tiempo de subida
            }
        }
    } else {
        clrBit(P_EN2, B_EN2);  // para asegurarse que el motor se para
    }
}

// Configurar tiempos de parpadeo en ms
void setParpadeo(uint16_t t_encendido_ms, uint16_t t_total_ms) {
    t_encendido = t_encendido_ms / 100;  // convertir a decimas de segundo
    t_total = t_total_ms / 100;          // convertir a decimas de segundo
    parpadeo_cnt = 0;
}

// Funcion para activar parpadeo del LED 4
void parpadeo() {
    if (parpadeo_cnt < t_total) {
        if (parpadeo_cnt < t_encendido) {
            setBit(P_L4, B_L4);  // encender LED 4
        } else {
            clrBit(P_L4, B_L4);  // apagar LED 4
        }
    } else {
        parpadeo_cnt = 0;
    }
}

// ISR del pulsador de emergencia SW1 (INT0)
ISR(INT0_vect) {
    clrAtraccion();
    setParpadeo(200, 1000);                                  // encender 200ms y apagar 1000ms
    EIMSK &= ~((1 << B_SO4) | (1 << B_SO5) | (1 << B_SW1));  // deshabilitar mascara de interrupcion por sensores opticos y mecanico
    clrAtraccion();                                          // desactiva la atraccion
    emergencia_flag = true;                                  // activar bandera de emergencia
}

// ISR del sensor optico 4 (INT2)
ISR(INT2_vect) {
    if (t_s04_cnt >= T_ANTIRREBOTES) {  // si el tiempo de activacion del sensor optico 4 es mayor a 10ms
        dientes_cnt++;                  // incrementar contador de dientes
        t_s04_cnt = 0;                  // reiniciar contador de tiempo del sensor optico 4                                   // incrementar el puntero del vector de dientes
    }
}

// ISR del sensor optico 5 (INT3)
ISR(INT3_vect) {
    if (t_s05_cnt >= T_ANTIRREBOTES) {  // si el tiempo de activacion del sensor optico 5 es mayor a 10ms
        if (atraccion_cnt >= t_atraccion) {
            clrAtraccion();  // desactivar atraccion
            t_s05_cnt = 0;
        } else {
            EIMSK |= (1 << B_SO4);  // habilitar mascara de interrupcion para el sensor optico 4 (dientes)
            t_s05_cnt = 0;
        }
    }
}

// ISR del sensor mecánico SW3
ISR(PCINT2_vect) {
    estado_sw3 = PINK & (1 << B_SW3);                           // leer el estado del sensor mecánico SW3
    if ((estado_sw3 ^ last_estado_sw3) && (estado_sw3 == 0)) {  // si el sensor mecánico SW3 tiene un flanco de bajada
        if (t_sw3_cnt >= T_ANTIRREBOTES) {                      // si el tiempo de activacion del sensor mecanico SW3 es mayor a 10ms
            // personas_cnt++;                                     // incrementar contador de personas en la atraccion
            if (controller_flag) {
                controller_flag = false;  // desactivar bandera de controlador
            } else {
                controller_flag = true;  // activar bandera de controlador
            }
            t_sw3_cnt = 0;  // reiniciar contador de tiempo de antirrebotes
        }
    }
    last_estado_sw3 = estado_sw3;  // guardar el estado actual del sensor mecánico SW3
}

// ISR del Timer 0 (cada 1ms)
ISR(TIMER0_COMPA_vect) {
    timer4_cnt++;    // incrementar contador de tiempo del timer 0
    t_subida_cnt++;  // incrementar contador de tiempo de subida
    t_s04_cnt++;     // incrementar contador de tiempo del sensor optico 4
    t_s05_cnt++;     // incrementar contador de tiempo del sensor optico 5
    t_sw3_cnt++;

    if (timer4_cnt >= 100 - 1) {  // si el contador de tiempo del timer 4 es mayor a 100ms
        parpadeo_cnt++;
        if (espera_flag) {
            espera_cnt++;
        }
        if (atraccion_flag) {
            atraccion_cnt++;
        }
        timer4_cnt = 0;  // reiniciar contador de tiempo del timer 4
    }
}
