/*
 * AtraccionDeFeria.c
 *
 * Created: 30/03/2025 18:34:49
 * Author : juanm & cloti
 */

#include "atraccion.h"

volatile bool emergencia_flag = false;  // Bandera de emergencia


void setup(){
	atraccionSetup(); // Configuracion inicial de la atraccion
}

int main(void) {
    setup();
    while (1) {
        if (!emergencia_flag) {
            atraccion();  // Llamar a la funcion de la atraccion
        }
        parpadeo();  // Llamar a la funcion de parpadeo
    }
}
