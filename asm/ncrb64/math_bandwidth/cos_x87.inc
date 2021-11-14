;---------- Mathematics calculations performance pattern ----------------------;
; Operation = x87 cosine , parameter = bandwidth.                              ;
;                                                                              ;
; INPUT:   RSI = Block #1 pointer (64-bit flat) , not used for math scenario   ;
;          RDI = Block #2 pointer (64-bit flat) , not used for math scenario   ;
;          R8  = Block #3 pointer (64-bit flat) , used for load data vector    ;
;          RCX = Block length, units = instructions                            ;
;          RBP = Number of measurement repeats                                 ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;          Registers corrupted, but must save R14, R15                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
Pattern_Cos_X87:
finit
Measurement_Cos_X87:
;---------- Prepare big cycle (without tail) ----------------------------------;
fld qword [r8]
mov rdx,rcx       ; RDX = Reload length
shr rdx,2         ; RDX = Block length, convert from INSTRUCTIONS to 4xINSTRUCTION units
jz Small_Cos_X87  ; Go if Length < 4 instructions
Block_Cos_X87:
fld st0
fcos
ffree st0
fincstp
fld st0
fcos
ffree st0
fincstp
fld st0
fcos
ffree st0
fincstp
fld st0
fcos
ffree st0
fincstp
dec rdx
jnz Block_Cos_X87    ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
;---------- Prepare tail cycle ------------------------------------------------;
Small_Cos_X87:
mov edx,ecx
and edx,00001111b    ; ECX = Extract tail length
jz Measure_Cos_X87
;---------- Tail cycle --------------------------------------------------------;
Tail_Cos_X87:
fld st0
fcos
ffree st0
fincstp
dec edx           
jnz Tail_Cos_X87     ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Cos_X87:
ffree st0
fincstp
dec rbp
jnz Measurement_Cos_X87   ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
finit
ret
