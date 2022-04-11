;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Write AVX-256 (VMOVAPD) " , mode = temporal.        ;
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
Pattern_Write_AVX256:
vzeroall             ; Clear YMM0-YMM15 for write data
Measurement_Write_AVX256:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 512-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea rax,[rsi+256]      ; RAX = Reload source address
mov rdx,rcx            ; RDX = Reload length
shr rdx,4              ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Write_AVX256  ; Go if Length < 16 instructions
mov ebx,512            ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Write_AVX256:
Block_Write_AVX256:
vmovapd [rax-32*08],ymm0
vmovapd [rax-32*07],ymm1
vmovapd [rax-32*06],ymm2
vmovapd [rax-32*05],ymm3
vmovapd [rax-32*04],ymm4
vmovapd [rax-32*03],ymm5
vmovapd [rax-32*02],ymm6
vmovapd [rax-32*01],ymm7
vmovapd [rax+32*00],ymm8
vmovapd [rax+32*01],ymm9
vmovapd [rax+32*02],ymm10
vmovapd [rax+32*03],ymm11
vmovapd [rax+32*04],ymm12
vmovapd [rax+32*05],ymm13
vmovapd [rax+32*06],ymm14
vmovapd [rax+32*07],ymm15
add rax,rbx                ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Write_AVX256     ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Write_AVX256:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Write_AVX256:
mov edx,ecx
and edx,00001111b          ; ECX = Extract tail length
jz Measure_Write_AVX256
mov ebx,32
;---------- Tail cycle --------------------------------------------------------;
Tail_Write_AVX256:
vmovapd [rax-32*08],ymm0
add rax,rbx                ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Write_AVX256      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Write_AVX256:
dec rbp
jnz Measurement_Write_AVX256  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
