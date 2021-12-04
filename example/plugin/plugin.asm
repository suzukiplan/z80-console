org $0000

.Start
   di
   in a, ($C0)
   in a, ($C1)
   out ($C0), a
   out ($C1), a
   ld a, 0
   ei
   ret

