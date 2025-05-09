; setupAsm.S

.global setupAsm
.type setupAsm, @function

setupAsm:
    cli

;Timer para millis() 
    ; Configuración TIMER3 modo CTC
    in     r16, TCCR3A
    andi   r16, ~((1 << WGM30) | (1 << WGM31))
    out    TCCR3A, r16

    in     r16, TCCR3B
    ori    r16, (1 << WGM32)
    andi   r16, ~(1 << WGM33)
    out    TCCR3B, r16

    ; TIMSK3 |= (1 << OCIE3A)
    in     r16, TIMSK3
    ori    r16, (1 << OCIE3A)
    out    TIMSK3, r16

    ; Prescaler = 1 para TIMER3
    in     r16, TCCR3B
    ori    r16, (1 << CS30)
    andi   r16, ~((1 << CS31) | (1 << CS32))
    out    TCCR3B, r16

    ; OCR3A = 8000 → 0x1F40
    ldi    r16, low(8000)
    out    OCR3AL, r16
    ldi    r16, high(8000)
    out    OCR3AH, r16

;Configuracion de monedero
    ; LED en PL7 como salida
    in     r16, DDRL
    ori    r16, (1 << 7)
    out    DDRL, r16

    in     r16, PORTL
    andi   r16, ~(1 << 7)
    out    PORTL, r16

    ; PD4 como entrada (Input Capture)
    in     r16, DDRD
    andi   r16, ~(1 << 4)
    out    DDRD, r16

    ; TCCR1A = 0
    ldi    r16, 0x00
    out    TCCR1A, r16

    ; TCCR1B = (1 << ICES1) | (1 << CS11)
    ldi    r16, (1 << ICES1) | (1 << CS11)
    out    TCCR1B, r16

    ; TIMSK1 |= (1 << ICIE1) | (1 << TOIE1)
    in     r16, TIMSK1
    ori    r16, (1 << ICIE1) | (1 << TOIE1)
    out    TIMSK1, r16



    sei
    ret
