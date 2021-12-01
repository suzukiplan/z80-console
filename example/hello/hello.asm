org $0000

.Start
   ld hl, SAMPLE_TEXT
   ld a, 14
   out ($0F), a
   ld a, 0
   ret

SAMPLE_TEXT:
   db "Hello, World!", $0A

