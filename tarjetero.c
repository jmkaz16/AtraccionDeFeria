#include "tarjetero.h"

volatile uint8_t flanco = 0;
volatile uint32_t tiempos[54];  // Guarda cada instante de flanco
volatile uint8_t flanco_index = 0;
volatile uint8_t captura_completa = 0;
volatile uint32_t desbordamiento = 0;
volatile uint32_t tiempo = 0;
volatile uint32_t tiempo_total = 0;

volatile uint8_t valores[24] = {0};

char tarjeta[6];

char usuarios[10][6] = {":+097-"};  // Tarjetas guardadas
volatile uint8_t num_usuarios = 1;  // Numero de tarjetas guardadas inicialmente

char tabla_codigos[16] = {  // Tabla para la decodificacion de bits a caracteres
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '?', '?', '-', '?', '+', ':'};

void tarjeteroSetup() {
    cli();

    // LED en PL7 como salida
    DDRL |= (1 << PL7);
    PORTL &= ~(1 << PL7);

    // PD4 como entrada para input capture
    DDRD &= ~(1 << PD4);

    // Configuracion TIMER1
    TCCR1A = 0;
    TCCR1B = (1 << ICES1) | (1 << CS11);  // Flanco de subida inicialmente, prescaler 8
    TIMSK1 |= (1 << ICIE1);               // Habilitar interrupcion input capture
    TIMSK1 |= (1 << TOIE1);               // Habilitar interrupcion por desbordamiento

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

ISR(TIMER1_CAPT_vect) {
    if (flanco_index < 52) {
        tiempo = ((uint32_t)desbordamiento << 16) | ICR1;
        tiempos[flanco_index] = tiempo;  // Guarda el instante del flanco
        flanco_index++;
        TCCR1B ^= (1 << ICES1);  // parpadea el LED, hay que cambiarlo
        // PORTL ^= (1 << PL7);            // Cambiar flanco (subida/bajada), hay que cambiarlo
    }
    if (flanco_index >= 52) {
        captura_completa = 1;     // Senala que ya tenemos una tarjeta leida
        TIMSK1 &= ~(1 << ICIE1);  // Desactivar interrupcion para procesar tranquilo
    }
}

ISR(TIMER1_OVF_vect) {
    desbordamiento++;
}

uint32_t millis() {
    return tiempo_total;
}

ISR(TIMER3_COMPA_vect) {
    tiempo_total++;
}

void procesarTarjeta() {
    if (!captura_completa) return;

    for (uint8_t i = 2, j = 0; i < 26; i++, j++) {                     // Empezamos desde 4 para ignorar el glitch y la primera barra
        uint32_t dur_negro = tiempos[2 * i] - tiempos[2 * i - 1];      // tiempo negro
        uint32_t dur_total = tiempos[2 * i + 1] - tiempos[2 * i - 1];  // tiempo total bit

        // Si el pulso negro dura mas de 2/3 del total ? bit = 1, si no ? bit = 0

        valores[j] = (dur_negro * 3 > dur_total * 2) ? 0 : 1;  // creo que esto esta al reves
    }

    conversorBits2Numero(valores, tarjeta);
    // T(usuarios, tarjeta);
    gestionarTarjeta();

    // Reset para proxima tarjeta
    flanco_index = 0;
    captura_completa = 0;  // mirarlo
    desbordamiento = 0;
    TCNT1 = 0;
    TIMSK1 |= (1 << ICIE1);  // Rehabilitar interrupcion
}

// Funcion que convierte los bits de vector bit en la cadena de caracteres
void conversorBits2Numero(volatile uint8_t* vector, char* tarjeta) {
    // Construcion del numero decimal del grupo de 4 bits
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t codigo = 0;

        for (uint8_t j = 0; j < 4; j++) {
            codigo <<= 1;                        // Desplazamiento del numero a la izquierda una posicion
            codigo |= vector[i * 4 + j] & 0x01;  // Anade el bit asegurando 0 o 1
        }

        tarjeta[i] = tabla_codigos[codigo];  // Se guarda el caracter correspondiente al codigo de 4 bits
    }

    tarjeta[6] = '\0';  // Finaliza la cadena de caracteres de la tarjeta
}

void compararTarjeta(const char* usuario, const char* leida) {
    if (strcmp(usuario, leida) == 1)
        PORTL |= (1 << PL7);  // Encender LED si coinciden
    else
        PORTL &= ~(1 << PL7);  // Apagar LED si no coinciden
}

void gestionarTarjeta() {
    uint32_t tiempo_inicial = 0;
    uint32_t tiempo_actual = 0;
    uint32_t duracion_espera = 0;
    uint8_t encontrada = 0;

    if (!tarjetaValida(tarjeta)) {
        tiempo_inicial = millis();
        do {
            tiempo_actual = millis();
            if ((tiempo_actual - tiempo_inicial) % 200 < 100) {  // Entre 200 porque es el periodo: 100 encendido y 100 apagado
                PORTL |= (1 << PL7);                             // LED ON
            } else {
                PORTL &= ~(1 << PL7);  // LED OFF
            }
        } while (tiempo_actual - tiempo_inicial < 1000);
        PORTL &= ~(1 << PL7);  // Apagamos LED definitivamente

    } else {
        // Comprobamos si la tarjeta esta en la lista de usuarios
        for (uint8_t i = 0; i < num_usuarios && !encontrada; i++) {
            if (strcmp(tarjeta, usuarios[i]) == 0) {
                encontrada = 1;
            }
        }
        if (encontrada) {
            duracion_espera = 1000;
        } else {
            duracion_espera = 3000;
            // Anadimos la tarjeta puesto que no estaba en la lista
            if (num_usuarios < 10) {
                strcpy(usuarios[num_usuarios], tarjeta);
                num_usuarios++;
            }
        }

        tiempo_inicial = millis();
        PORTL |= (1 << PL7);  // Encendemos led
        do {
            tiempo_actual = millis();
        } while (tiempo_actual - tiempo_inicial < duracion_espera);  // Apaga tanto si encontrada como no, varian solo los segundos
        PORTL &= ~(1 << PL7);
    }
}

uint8_t tarjetaValida(const char* t) {
    for (uint8_t i = 0; i < 6; i++) {
        if (t[i] == '?' || t[i] == '\0') {
            return 0;
        }  // Menos de 6 caracteres o caracter '?'
    }
    if (t[6] != '\0') {
        return 0;  // Mas de 6 caracteres
    } else {
        personas_cnt++;
        return 1;
    }
}
