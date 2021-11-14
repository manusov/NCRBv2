;---------- Mathematics calculations performance pattern ----------------------;
; Operation = AVX512 vector sqrt , parameter = bandwidth.                      ;
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
Pattern_Sqrt_AVX512:
Measurement_Sqrt_AVX512:
;---------- Prepare big cycle (without tail) ----------------------------------;
vmovapd zmm7,[esi]
mov edx,ecx           ; EDX = Reload length
shr edx,4             ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Sqrt_AVX512  ; Go if Length < 16 instructions
;---------- Big cycle ---------------------------------------------------------;
Block_Sqrt_AVX512:
vsqrtpd zmm0,zmm7
vsqrtpd zmm1,zmm7
vsqrtpd zmm2,zmm7
vsqrtpd zmm3,zmm7
vsqrtpd zmm4,zmm7
vsqrtpd zmm5,zmm7
vsqrtpd zmm6,zmm7
vsqrtpd zmm0,zmm7
vsqrtpd zmm1,zmm7
vsqrtpd zmm2,zmm7
vsqrtpd zmm3,zmm7
vsqrtpd zmm4,zmm7
vsqrtpd zmm5,zmm7
vsqrtpd zmm6,zmm7
vsqrtpd zmm0,zmm7
vsqrtpd zmm1,zmm7
vaddpd zmm7,zmm7,zmm7
dec edx
jnz Block_Sqrt_AVX512        ; Cycle for block data transfer
;---------- Prepare tail cycle ------------------------------------------------;
Small_Sqrt_AVX512:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_Sqrt_AVX512
;---------- Tail cycle --------------------------------------------------------;
Tail_Sqrt_AVX512:
vsqrtpd zmm0,zmm7
dec edx           
jnz Tail_Sqrt_AVX512         ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Sqrt_AVX512:
sub ebp,1
sbb ebx,0
mov eax,ebp
or eax,ebx
jnz Measurement_Sqrt_AVX512  ; Cycle for measurement, repeat same operation
ret
