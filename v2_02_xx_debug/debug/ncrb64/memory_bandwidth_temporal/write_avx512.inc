;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Write AVX-512 (VMOVAPD) " , mode = temporal.        ;
;                                                                              ;
; INPUT:   RSI = Block #1 pointer (64-bit flat)                                ;
;          RDI = Block #2 pointer (64-bit flat)                                ;
;          R8  = Block #3 pointer (64-bit flat) , reserved and not used yet    ;
;                For Read, Write, Modify use RSI as Source or Destination      ;
;                For Copy use RSI = Source , RDI = Destination                 ;
;          RCX = Block length, units = instructions                            ;
;          RBP = Number of measurement repeats                                 ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;          Registers corrupted, but must save R14, R15                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
Pattern_Write_AVX512:
vzeroall               ; Clear ZMM0-ZMM15 for write data, note ZMM16-ZMM31 not changed by VZEROALL
Measurement_Write_AVX512:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi            ; RAX = Reload source address
mov rdx,rcx            ; RDX = Reload length
shr rdx,3              ; RDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_Write_AVX512  ; Go if Length < 8 instructions
mov ebx,512            ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Write_AVX512:
Block_Write_AVX512:
vmovapd [rax+64*00],zmm0
vmovapd [rax+64*01],zmm1
vmovapd [rax+64*02],zmm2
vmovapd [rax+64*03],zmm3
vmovapd [rax+64*04],zmm4
vmovapd [rax+64*05],zmm5
vmovapd [rax+64*06],zmm6
vmovapd [rax+64*07],zmm7
add rax,rbx                  ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Write_AVX512       ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Write_AVX512:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Write_AVX512:
mov edx,ecx
and edx,00000111b            ; ECX = Extract tail length
jz Measure_Write_AVX512
mov ebx,64
;---------- Tail cycle --------------------------------------------------------;
Tail_Write_AVX512:
vmovapd [rax+64*00],zmm8
add rax,rbx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Write_AVX512        ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Write_AVX512:
dec rbp
jnz Measurement_Write_AVX512  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
