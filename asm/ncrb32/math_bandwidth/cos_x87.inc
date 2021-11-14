;---------- Mathematics calculations performance pattern ----------------------;
; Operation = x87 cosine , parameter = bandwidth.                              ;
;                                                                              ;
; INPUT:   ESI = Block #1 pointer (32-bit flat) , used for load data vector    ;
;          EDI = Block #2 pointer (32-bit flat) , not used for math scenario   ;
;          ECX = Block length, units = instructions                            ;
;          EBX:EBP = Number of measurement repeats, EBX=High32, EBP=Low32      ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;          Registers corrupted                                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
Pattern_Cos_X87:
finit
Measurement_Cos_X87:
;---------- Prepare big cycle (without tail) ----------------------------------;
fld qword [esi]
mov edx,ecx       ; EDX = Reload length
shr edx,2         ; EDX = Block length, convert from INSTRUCTIONS to 4xINSTRUCTION units
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
dec edx
jnz Block_Cos_X87         ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
;---------- Prepare tail cycle ------------------------------------------------;
Small_Cos_X87:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_Cos_X87
;---------- Tail cycle --------------------------------------------------------;
Tail_Cos_X87:
fld st0
fcos
ffree st0
fincstp
dec edx           
jnz Tail_Cos_X87          ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Cos_X87:
ffree st0
fincstp
sub ebp,1
sbb ebx,0
mov eax,ebp
or eax,ebx
jnz Measurement_Cos_X87   ; Cycle for measurement, repeat same operation
finit
ret
