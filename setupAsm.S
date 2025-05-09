.global setupAsm
.type setupAsm, @function

; Puertos y bits de los motores
.equ P_EN1, PORTL
.equ B_EN1, 5
.equ P_BK1, PORTL
.equ B_BK1, 6
.equ P_EN2, PORTK
.equ B_EN2, 2
.equ P_DI2, PORTK
.equ B_DI2, 3

; Puertos y bits de los sensores ópticos
.equ P_SO2, PORTL
.equ B_SO2, 0
.equ P_SO3, PORTL
.equ B_SO3, 1
.equ P_SO4, PORTD
.equ B_SO4, 2
.equ P_SO5, PORTD
.equ B_SO5, 3

; Puertos y bits de los sensores mecánicos
.equ P_SW1, PORTD
.equ B_SW1, 0
.equ P_SW2, PORTD
.equ B_SW2, 1
.equ PIN_SW2, PIND
.equ P_SW3, PORTK
.equ B_SW3, 4

; Puertos y bits de los LEDs
.equ P_L1, PORTL
.equ B_L1, 7
.equ P_L2, PORTL
.equ B_L2, 4
.equ P_L3, PORTK
.equ B_L3, 0
.equ P_L4, PORTK
.equ B_L4, 1

setupAsm:
    cli

	/*;TarjeteroSetup

    ; LED en PL7 como salida
    lds     r16, 0x10A       ; DDRL
    ori     r16, 0x80
    sts     0x10A, r16

    ; PORTL &= ~0x80
    lds     r16, 0x10B       ; PORTL
    andi    r16, 0x7F
    sts     0x10B, r16

    ; PD4 como entrada
    in      r16, 0x0A        ; DDRD
    andi    r16, 0xEF
    out     0x0A, r16

    ; TCCR1A = 0
    ldi     r16, 0x00
    sts     0x80, r16

    ; TCCR1B = (1 << ICES1) | (1 << CS11)
    ldi     r16, 0x42
    sts     0x81, r16

    ; TIMSK1 |= (1 << ICIE1) | (1 << TOIE1)
    lds     r16, 0x6F
    ori     r16, 0x25
    sts     0x6F, r16

    ; TCCR3A &= ~((1 << WGM30) | (1 << WGM31))
    lds     r16, 0x90
    andi    r16, 0xFC
    sts     0x90, r16

    ; TCCR3B |= (1 << WGM32); &= ~(1 << WGM33)
    lds     r16, 0x91
    ori     r16, 0x08
    andi    r16, 0xEF
    sts     0x91, r16

    ; TIMSK3 |= (1 << OCIE3A)
    lds     r16, 0x71
    ori     r16, 0x02
    sts     0x71, r16

    ; Prescaler TIMER3: CS30 = 1, CS31 = CS32 = 0
    lds     r16, 0x91
    andi    r16, 0xF8
    ori     r16, 0x01
    sts     0x91, r16

    ; OCR3A = 8000 = 0x1F40
    ldi     r16, lo8(8000)
    sts     0x98, r16        ; OCR3AL
    ldi     r16, hi8(8000)
    sts     0x99, r16        ; OCR3AH
	*/
	; AtraccionSetup

	; LEDs L3 y L4
	in  r16, DDRK
    ori r16, (1 << B_L3) | (1 << B_L4)
    out DDRK, r16

	; Motor M2
	in  r16, DDRK
    ori r16, (1 << B_EN2) | (1 << B_DI2)
    out DDRK, r16

	; INT0 a flanco de bajada
    in  r16, EICRA
    andi r16, ~(1 << (2 * B_SW1))      ; clear ISC0n0
    ori  r16, (1 << (2 * B_SW1 + 1))   ; set ISC0n1
    out EICRA, r16

	; Habilitar INT0
    in  r16, EIMSK
    ori r16, (1 << B_SW1)
    out EIMSK, r16

	; INT2 e INT3 a flanco de bajada
    in  r16, EICRA
    andi r16, ~(1 << (2 * B_SO4))
    ori  r16, (1 << (2 * B_SO4 + 1))
    andi r16, ~(1 << (2 * B_SO5))
    ori  r16, (1 << (2 * B_SO5 + 1))
    out EICRA, r16

	; Habilitar INT2 y INT3
    in  r16, EIMSK
    ori r16, (1 << B_SO4) | (1 << B_SO5)
    out EIMSK, r16

    ; Habilitar interrupción SW3 (PCINT20)
    in  r16, PCICR
    ori r16, (1 << PCIE2)
    out PCICR, r16

    ; Habilitar máscara en PCMSK2 para PCINT20
    ldi r16, (1 << PCINT20)
    out PCMSK2, r16

    ; Timer0 en modo CTC (Top = OCR0A)
    in  r16, TCCR0A
    andi r16, ~(1 << WGM00)
    ori  r16, (1 << WGM01)
    out TCCR0A, r16
    in  r16, TCCR0B
    andi r16, ~(1 << WGM02)

    ; Preescalador 64: CS01 y CS00
    ori  r16, (1 << CS01) | (1 << CS00)
    andi r16, ~(1 << CS02)
    out TCCR0B, r16

    ; OCR0A = 124
    ldi r16, 124
    out OCR0A, r16

    ; Habilitar interrupción por OCRA
    in  r16, TIMSK0
    ori r16, (1 << OCIE0A)
    out TIMSK0, r16

    sei
    ret
