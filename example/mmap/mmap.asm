org $0000

.Start
   ; Read Memory
   ld a, ($C0FF)
   ld a, ($C100)
   ld a, ($C180)
   ld a, ($C1FF)
   ld a, ($C200)

   ; Write Memory
   ld ($C1FF), a
   ld ($C200), a
   ld ($C280), a
   ld ($C2FF), a
   ld ($C300), a
   ret

