#include "comunicacion.h"

// Configura el UART para comunicacion serial
void uartSetup() {
    cli();  // Deshabilitar interrupciones globales

    // Configurar el baud rate
    UBRR0H = (unsigned char)(UBRR_VALUE >> 8);  // Parte alta
    UBRR0L = (unsigned char)UBRR_VALUE;         // Parte baja

    // Habilitar  el flag de recepcion (RXEN0)
    UCSR0B |= (1 << RXEN0);
    // UCSR0B = (1 << RXEN0) | (1 << RXCIE0);  // Habilita receptor e interrupcion de RX

    // Configurar formato: 8 datos, sin paridad, 1 bit de parada (8N1)
    UCSR0B &= ~(1 << UCSZ02);
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);       // 8 bits de datos
    UCSR0C &= ~((1 << UPM01) | (1 << UPM00));      // Sin paridad
    UCSR0C &= ~(1 << USBS0);                       // 1 bit de parada
    UCSR0C &= ~((1 << UMSEL01) | (1 << UMSEL00));  // Asincrono

    sei();  // Habilitar interrupciones globales
}

// Funcion bloqueante para recibir un caracter por UART
char uartReceive() {
    if ((UCSR0A & (1 << RXC0)))  // Comprueba si ha un dato
    {
        return UDR0;
    } else {
        return 0;
    }
}

void decodeData(char data) {
    switch (data) {
        case 'a':
            toggleBit(P_L1, B_L1);  // Cambiar el estado del LED 1
            break;
        case 'b':
            toggleBit(P_L2, B_L2);  // Cambiar el estado del LED 2
            break;
        case 'c':
            toggleBit(P_L3, B_L3);  // Cambiar el estado del LED 3
            break;
        case 'd':
            toggleBit(P_L4, B_L4);  // Cambiar el estado del LED 4
            break;
        case 'e':
            toggleBit(P_EN2, B_EN2);  // Cambiar el estado del motor 2
            break;
        case 'f':
            toggleBit(P_DI2, B_DI2);  // Cambiar el estado de la direccion del motor 2
            break;
        default:
            // Manejar otros casos
            break;
    }
}
