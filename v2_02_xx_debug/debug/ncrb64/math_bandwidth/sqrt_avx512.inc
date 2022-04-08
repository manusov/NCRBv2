;---------- Mathematics calculations performance pattern ----------------------;
; Operation = AVX512 vector sqrt , parameter = bandwidth.                      ;
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
Pattern_Sqrt_AVX512:
Measurement_Sqrt_AVX512:
;---------- Prepare big cycle (without tail) ----------------------------------;
vmovapd zmm15,[r8]
mov rdx,rcx            ; RDX = Reload length
shr rdx,4              ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Sqrt_AVX512   ; Go if Length < 16 instructions
;---------- Big cycle ---------------------------------------------------------;
Block_Sqrt_AVX512:
vsqrtpd zmm0,zmm15
vsqrtpd zmm1,zmm15
vsqrtpd zmm2,zmm15
vsqrtpd zmm3,zmm15
vsqrtpd zmm4,zmm15
vsqrtpd zmm5,zmm15
vsqrtpd zmm6,zmm15
vsqrtpd zmm7,zmm15
vsqrtpd zmm8,zmm15
vsqrtpd zmm9,zmm15
vsqrtpd zmm10,zmm15
vsqrtpd zmm11,zmm15
vsqrtpd zmm12,zmm15
vsqrtpd zmm13,zmm15
vsqrtpd zmm14,zmm15
vsqrtpd zmm0,zmm15
vaddpd zmm15,zmm15,zmm15
dec rdx
jnz Block_Sqrt_AVX512        ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
;---------- Prepare tail cycle ------------------------------------------------;
Small_Sqrt_AVX512:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_Sqrt_AVX512
;---------- Tail cycle --------------------------------------------------------;
Tail_Sqrt_AVX512:
vsqrtpd zmm0,zmm15
dec edx           
jnz Tail_Sqrt_AVX512         ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Sqrt_AVX512:
dec rbp
jnz Measurement_Sqrt_AVX512  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
