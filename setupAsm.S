.global setupAsm
.type setupAsm, @function

setupAsm:
    cli

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

    sei
    ret
