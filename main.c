/*
 * AtraccionDeFeria.c
 *
 * Created: 30/03/2025 18:34:49
 * Author : juanm & cloti
 */

#include "atraccion.h"
#include "monedero.h"
#include "tarjetero.h"

volatile bool emergencia_flag = false;  // Bandera de emergencia

void setup() {
    atraccionSetup();  // Configuracion inicial de la atraccion
                       // tarjeteroSetup();  // Configuracion inicial del tarjetero
    monederoSetup();   // Configuracion inicial del monedero
}

int main(void) {
    setup();
    setupAsm();
    while (1) {
        if (!emergencia_flag) {
            atraccion();        // Llamar a la funcion de la atraccion
            procesarTarjeta();  // Llamar a la funcion de procesar tarjeta
            monedero();         // Llamar a la funcion de monedero
        }

        parpadeo();  // Llamar a la funcion de parpadeo
    }
}
