.global setupAsm

#include <avr/io.h>
#include "pinout.h"


setupAsm:
    cli

 ; Test LED L3 (PK0) 

    /*; Configurar PK0 como salida ? DDRK |= (1 << 0)
    lds r16, DDRK        ; DDRK
    ori r16, 0b00000001        ; bit 0
    sts DDRK, r16        ; guardar

    ; Encender LED L3 ? PORTK |= (1 << 0)
    lds r16, PORTK        ; PORTK
    ori r16, 0b00000001        ; bit 0
    sts PORTK, r16        ; guardar*/

;Atraccion Setup

;Configuro el LED 1 como salida. En PORTL el bit 7
	lds r16, DDRL
	ori r16, 0b10000000
	sts DDRL, r16

;LED 1 apagado inicialmente
	lds r16, P_L1
	andi r16, 0b01111111
	sts P_L1, r16
   


    sei
    ret
