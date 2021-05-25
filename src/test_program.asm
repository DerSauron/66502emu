VIA = $D000
VIA_PA = VIA + $01
VIA_PB = VIA + $00
VIA_DIRA = VIA + $03
VIA_DIRB = VIA + $02
VIA_T1CL = VIA + $04
VIA_T1CH = VIA + $05
VIA_T1LL = VIA + $06
VIA_T1LH = VIA + $07
VIA_T2CL = VIA + $08
VIA_T2CH = VIA + $09
VIA_IFR = VIA + $0D
VIA_IER = VIA + $0E

LCD_EN = %00000100
LCD_RW = %00000010
LCD_RS = %00000001

    .org $E000

nmi_handler:
    rti

irq_handler:
    pha

    ; switch pa
    lda VIA + VIA_PA
    eor #$FF
    sta VIA + VIA_PA

    ; switch pb
    lda VIA + VIA_PB
    eor #$FF
    sta VIA + VIA_PB

    ; restart counter
    lda #$00
    sta VIA + VIA_T2CH
    pla
    rti

startup:
      ldx #$ff
      txs

      lda #%11111111 ; Set all pins on port A to output
      sta VIA_DIRA
      lda #%00000111 ; Set top 3 pins on port B to output
      sta VIA_DIRB

      lda #%00111000 ; Set 8-bit mode; 2-line display; 5x8 font
      jsr lcd_instruction
      lda #%00001110 ; Display on; cursor on; blink off
      jsr lcd_instruction
      lda #%00000110 ; Increment and shift cursor; don't shift display
      jsr lcd_instruction
      lda #$00000001 ; Clear display
      jsr lcd_instruction

      ldx #0
print:
      lda message,x
      beq loop
      jsr print_char
      inx
      jmp print

loop:
    jmp loop

message: .asciiz "Hello, world!"

lcd_wait:
      pha
      lda #%00000000  ; Port A is input
      sta VIA_DIRA
lcdbusy:
      lda #LCD_RW
      sta VIA_PB
      lda #(LCD_RW | LCD_EN)
      sta VIA_PB
      lda VIA_PA
      and #%10000000
      bne lcdbusy

      lda #LCD_RW
      sta VIA_PB
      lda #%11111111  ; Port A is output
      sta VIA_DIRA
      pla
      rts

lcd_instruction:
      jsr lcd_wait
      sta VIA_PA
      lda #0         ; Clear RS/RW/E bits
      sta VIA_PB
      lda #LCD_EN    ; Set E bit to send instruction
      sta VIA_PB
      lda #0         ; Clear RS/RW/E bits
      sta VIA_PB
      rts

print_char:
      jsr lcd_wait
      sta VIA_PA
      lda #LCD_RS         ; Set RS; Clear RW/E bits
      sta VIA_PB
      lda #(LCD_RS | LCD_EN)   ; Set E bit to send instruction
      sta VIA_PB
      lda #LCD_RS         ; Clear E bits
      sta VIA_PB
      rts


    .org $FFFA
    .word nmi_handler
    .word startup
    .word irq_handler
