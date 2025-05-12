#include "tarjetero.h"

volatile uint32_t tiempos[NUM_TIEMPOS];  // vector que guarda los tiempos de los flancos detectados
volatile uint8_t flanco_index = 0;       // indice para guardar el numero de flancos
volatile uint8_t captura_completa = 0;   // 1 cuando ha leido una tarjeta
volatile uint32_t desbordamiento = 0;    // cuenta de los overflows del timer
volatile uint32_t tiempo = 0;            // tiempo en el que detecta cada flanco
volatile uint32_t tiempo_total = 0;      // contador de  milisegundos que devuelve millis()

volatile uint8_t led_activo = 0;           // indica si el led debe encederse o no (1 = encendido, 0 = apagado)
volatile uint32_t led_tiempo_inicio = 0;   // guarda el instante en el que se encedio el LED
volatile uint32_t led_duracion_total = 0;  // duracion total que el LED debe estar encendido o parpadeando
volatile uint32_t led_parpadeo = 0;        // indica si el LED debe parpadear o no (1 = parpadeo cada 100 ms, 0 = sin parpadeo)

volatile uint8_t valores[NUM_BITS] = {0};  // vector que guarda los bits de la tarjeta

char tarjeta[NUM_CARACTERES];  // codigo de caracteres de la tarjeta

char usuarios[MAX_TARJETAS][NUM_CARACTERES] = {":+097-"};  // tarjetas guardadas
volatile uint8_t num_usuarios = 1;                         // numero de tarjetas guardadas inicialmente

char tabla_codigos[INDEX_TABLA] = {  // tabla para la decodificacion de bits a caracteres
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '?', '?', '-', '?', '+', ':'};

// Configuracion de entradas, salidas, interrupciones y temporizadores de tarjetero
void tarjeteroSetup() {
    cli();

    // LED en B_L1 como salida
    // DDRL |= (1 << B_L1);
    // PORTL &= ~(1<<PL7);

    // PD4 como entrada para input capture
    DDRD &= ~(1 << PD4);

    // Configuracion TIMER1
    TCCR1A = 0;
    TCCR1B = (1 << ICES1) | (1 << CS11);  // flanco de subida inicialmente, prescaler 8
    TIMSK1 |= (1 << ICIE1);               // habilitar interrupcion input capture
    TIMSK1 |= (1 << TOIE1);               // habilitar interrupcion por desbordamiento

    // Configuracion TIMER 3 para millis
    //  Timer 3 en modo CTC (Top OCRnA)
    TCCR3A &= ~(1 << WGM30);
    TCCR3A &= ~(1 << WGM31);
    TCCR3B |= (1 << WGM32);
    TCCR3B &= ~(1 << WGM33);
    TIMSK3 |= (1 << OCIE3A);

    // Mascara para no tener preescalado
    TCCR3B = (TCCR3B | (1 << CS30));
    TCCR3B = (TCCR3B & (~(1 << CS31)));
    TCCR3B = (TCCR3B & (~(1 << CS32)));

    OCR3A = 8000;

    sei();
}

// ISR del input capture del TIMER1
ISR(TIMER1_CAPT_vect) {
    if (flanco_index < (NUM_TIEMPOS - 2)) {
        tiempo = ((uint32_t)desbordamiento << 16) | ICR1;
        tiempos[flanco_index] = tiempo;  // guarda el instante del flanco
        flanco_index++;
        TCCR1B ^= (1 << ICES1);  // parpadea el LED, hay que cambiarlo
        // P_L1 ^= (1 << B_L1);            // Cambiar flanco (subida/bajada), hay que cambiarlo
    }
    if (flanco_index >= (NUM_TIEMPOS - 2)) {
        captura_completa = 1;     // indica que ya tenemos una tarjeta leida
        TIMSK1 &= ~(1 << ICIE1);  // desactivar interrupcion para procesar tranquilo
    }
}

// ISR del desbordamiento del TIMER1
ISR(TIMER1_OVF_vect) {
    desbordamiento++;
}

// ISR del TIMER3 para el millis
ISR(TIMER3_COMPA_vect) {
    tiempo_total++;
}

// Funcion que devuelve el tiempo en milisegundos desde que se inicializo el timer
uint32_t millis() {
    return tiempo_total;
}

// Funcion que procesa la tarjeta leida
void procesarTarjeta() {
    if (!captura_completa) return;

    for (uint8_t i = 2, j = 0; i < 26; i++, j++) {                     // empezamos desde 4 para ignorar el glitch y la primera barra
        uint32_t dur_negro = tiempos[2 * i] - tiempos[2 * i - 1];      // tiempo negro
        uint32_t dur_total = tiempos[2 * i + 1] - tiempos[2 * i - 1];  // tiempo total bit

        // Si el pulso negro dura mas de 2/3 del total ? bit = 1, si no ? bit = 0

        valores[j] = (dur_negro * 3 > dur_total * 2) ? 0 : 1;  // creo que esto esta al reves
    }

    conversorBits2Numero(valores, tarjeta);
    gestionarTarjeta();

    // Reset para proxima tarjeta
    flanco_index = 0;
    captura_completa = 0;  // mirarlo
    desbordamiento = 0;
    TCNT1 = 0;
    TIMSK1 |= (1 << ICIE1);  // rehabilitar interrupcion
}

// Funcion que convierte los bits de vector bit en la cadena de caracteres
void conversorBits2Numero(volatile uint8_t* vector, char* tarjeta) {
    // Construcion del numero decimal del grupo de 4 bits
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t codigo = 0;

        for (uint8_t j = 0; j < 4; j++) {
            codigo <<= 1;                        // desplazamiento del numero a la izquierda una posicion
            codigo |= vector[i * 4 + j] & 0x01;  // anade el bit asegurando 0 o 1
        }

        tarjeta[i] = tabla_codigos[codigo];  // se guarda el caracter correspondiente al codigo de 4 bits
    }

    tarjeta[NUM_CARACTERES] = '\0';  // finaliza la cadena de caracteres de la tarjeta
}

// Funcion que comprueba si la tarjeta es valida
uint8_t tarjetaValida(const char* tarjeta_valida) {
    for (uint8_t i = 0; i < 6; i++) {
        if (tarjeta_valida[i] == '?' || tarjeta_valida[i] == '\0') {
            return 0;
        }  // menos de 6 caracteres o caracter '?'
    }
    if (tarjeta_valida[NUM_CARACTERES] != '\0') {
        return 0;  // mas de 6 caracteres
    } else {
        personas_cnt++;
        return 1;
    }
}

// Funcion que verifica si la tarjeta se encuentra en la lista de usuarios y si no la a�ade
void gestionarTarjeta() {
    uint8_t encontrada = 0;

    if (!tarjetaValida(tarjeta)) {
        // Tarjeta no valida, el LED debe parapdear durante 1s

        led_parpadeo = 1;
        led_duracion_total = 1000;  // 1000ms=1s
        led_tiempo_inicio = millis();
        led_activo = 1;

    } else {
        // Comprobamos si la tarjeta esta en la lista de usuarios registrados
        for (uint8_t i = 0; i < num_usuarios && !encontrada; i++) {
            if (strcmp(tarjeta, usuarios[i]) == 0) {
                encontrada = 1;
            }
        }
        if (encontrada) {               // tarjeta conocida, el debe encenderse durante 1s
            led_duracion_total = 1000;  // 1000ms = 1s
        } else {                        // tarjeta nueva, el led debe encenderse durante 3s
            led_duracion_total = 3000;  // 3000ms = 3s
            // Anadimos la tarjeta par que quede registrada como un usuario conocido, puesto que no estaba en la lista
            if (num_usuarios < 10) {
                strcpy(usuarios[num_usuarios], tarjeta);
                num_usuarios++;
            }
        }

        led_parpadeo = 0;
        led_tiempo_inicio = millis();
        led_activo = 1;
        setBit(P_L1, B_L1);  // encendemos LED 1
    }
}

// Funcion que gestiona el estado del LED: apagado, encendido o parpadeando
void actualizarLED() {
    if (!led_activo) return;  // no hay que hacer nada con el LED, salimos

    uint32_t t_actual = millis();

    // Si ha pasado el tiempo total de espera, apagamos el led y salimos
    if (t_actual - led_tiempo_inicio >= led_duracion_total) {
        clrBit(P_L1, B_L1);  // apagamos LED 1
        led_activo = 0;
        return;
    }

    // Modo parpadeo
    if (led_parpadeo) {
        if ((t_actual - led_tiempo_inicio) % 200 < 100) {  // // entre 200 porque es el periodo: 100ms encendido y 100ms apagado
            setBit(P_L1, B_L1);
        } else {
            clrBit(P_L1, B_L1);
        }
    }
}