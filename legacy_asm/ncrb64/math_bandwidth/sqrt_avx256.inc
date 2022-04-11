;---------- Mathematics calculations performance pattern ----------------------;
; Operation = AVX256 vector sqrt , parameter = bandwidth.                      ;
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
Pattern_Sqrt_AVX256:
Measurement_Sqrt_AVX256:
;---------- Prepare big cycle (without tail) ----------------------------------;
vmovapd ymm15,[r8]
mov rdx,rcx           ; RDX = Reload length
shr rdx,4             ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Sqrt_AVX256  ; Go if Length < 16 instructions
;---------- Big cycle ---------------------------------------------------------;
Block_Sqrt_AVX256:
vsqrtpd ymm0,ymm15
vsqrtpd ymm1,ymm15
vsqrtpd ymm2,ymm15
vsqrtpd ymm3,ymm15
vsqrtpd ymm4,ymm15
vsqrtpd ymm5,ymm15
vsqrtpd ymm6,ymm15
vsqrtpd ymm7,ymm15
vsqrtpd ymm8,ymm15
vsqrtpd ymm9,ymm15
vsqrtpd ymm10,ymm15
vsqrtpd ymm11,ymm15
vsqrtpd ymm12,ymm15
vsqrtpd ymm13,ymm15
vsqrtpd ymm14,ymm15
vsqrtpd ymm0,ymm15
vaddpd ymm15,ymm15,ymm15
dec rdx
jnz Block_Sqrt_AVX256        ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
;---------- Prepare tail cycle ------------------------------------------------;
Small_Sqrt_AVX256:
mov edx,ecx
and edx,00001111b            ; EDX = Extract tail length
jz Measure_Sqrt_AVX256
;---------- Tail cycle --------------------------------------------------------;
Tail_Sqrt_AVX256:
vsqrtpd ymm0,ymm15
dec edx           
jnz Tail_Sqrt_AVX256         ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Sqrt_AVX256:
dec rbp
jnz Measurement_Sqrt_AVX256  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret

