/*
 * AtraccionDeFeria.c
 *
 * Created: 30/03/2025 18:34:49
 * Author : juanm & cloti
 */

#include "atraccion.h"
#include "comunicacion.h"
#include "monedero.h"
#include "pinout.h"
#include "tarjetero.h"

extern void asmSetup(void);

volatile bool emergencia_flag = false;  // Bandera de emergencia
volatile bool controller_flag = false;  // Bandera para habilitar el mando de PS3

// char data='0';

void setup() {
    atraccionSetup();  // Configuracion inicial de la atraccion
    tarjeteroSetup();  // Configuracion inicial del tarjetero
    monederoSetup();   // Configuracion inicial del monedero
    uartSetup();       // Configuracion inicial del UART
    asmSetup();        // Configuracion inicial en ensamblador
}

int main(void) {
    setup();
    while (1) {
        if (!controller_flag) {
            if (!emergencia_flag) {
                atraccion();        // Llamar a la funcion de la atraccion
                procesarTarjeta();  // Llamar a la funcion de procesar tarjeta
                monedero();         // Llamar a la funcion de monedero
            }
            parpadeo();       // Llamar a la funcion de parpadeo
            actualizarLED();  // LLamar a la funcion de actualizar LED
        } else {
            decodeData(uartReceive());  // Recibir dato por UART
        }
    }
}
