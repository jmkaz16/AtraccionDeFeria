.global setupAsm

#include <avr/io.h>
#include "pinout.h"


setupAsm:
    cli

; TARJETERO SETUP

; Configuro el LED 1 como salida. En PORTL el bit 7
	lds r16, DDRL
	ori r16, 0b10000000
	sts DDRL, r16

; LED 1 apagado inicialmente
	lds r16, P_L1
	andi r16, 0b01111111
	sts P_L1, r16
 
; ATRACCION SETUP

; Configuro el LED 3. En PORTK el bit 0 
	lds r16, DDRK
	ori r16, 0b00000001
	sts DDRK, r16

; LED 3 apagado inicialmente
	lds r16, P_L3
	andi r16, 0b11111110
	sts P_L3, r16

; Configuro el LED 4. En PORTK el bit 1
	lds r16, DDRK
	ori r16, 0b00000010
	sts DDRK, r16

; LED 4 apagado inicialmente
	lds r16, P_L4
	andi r16, 0b11111101
	sts P_L4, r16

; Configuro las se�ales del motor M2 como salidas. EN2 en PORTK el bit 2 y DI2 en PORTK el bit 3
	lds r16, DDRK
	ori r16, 0b00000100
	sts DDRK, r16

	lds r16, DDRK
	ori r16, 0b00001000
	sts DDRK, r16

    sei
    ret
