;---------- Mathematics calculations performance pattern ----------------------;
; Operation = AVX256 vector sqrt , parameter = bandwidth.                      ;
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
Pattern_Sqrt_AVX256:
Measurement_Sqrt_AVX256:
;---------- Prepare big cycle (without tail) ----------------------------------;
vmovapd ymm7,[esi]
mov edx,ecx           ; EDX = Reload length
shr edx,4             ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Sqrt_AVX256  ; Go if Length < 16 instructions
;---------- Big cycle ---------------------------------------------------------;
Block_Sqrt_AVX256:
vsqrtpd ymm0,ymm7
vsqrtpd ymm1,ymm7
vsqrtpd ymm2,ymm7
vsqrtpd ymm3,ymm7
vsqrtpd ymm4,ymm7
vsqrtpd ymm5,ymm7
vsqrtpd ymm6,ymm7
vsqrtpd ymm0,ymm7
vsqrtpd ymm1,ymm7
vsqrtpd ymm2,ymm7
vsqrtpd ymm3,ymm7
vsqrtpd ymm4,ymm7
vsqrtpd ymm5,ymm7
vsqrtpd ymm6,ymm7
vsqrtpd ymm0,ymm7
vsqrtpd ymm1,ymm7
vaddpd ymm7,ymm7,ymm7
dec edx
jnz Block_Sqrt_AVX256        ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
;---------- Prepare tail cycle ------------------------------------------------;
Small_Sqrt_AVX256:
mov edx,ecx
and edx,00001111b            ; EDX = Extract tail length
jz Measure_Sqrt_AVX256
;---------- Tail cycle --------------------------------------------------------;
Tail_Sqrt_AVX256:
vsqrtpd ymm0,ymm7
dec edx           
jnz Tail_Sqrt_AVX256         ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Sqrt_AVX256:
sub ebp,1
sbb ebx,0
mov eax,ebp
or eax,ebx
jnz Measurement_Sqrt_AVX256  ; Cycle for measurement, repeat same operation
ret
