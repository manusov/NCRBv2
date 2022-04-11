;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Read SSE-128 (MOVNTDQA) " ,                         ;
; mode = non temporal, one of variants, depends on options.                    ;
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
Pattern_NtRead_SSE128:
Measurement_NtRead_SSE128:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 256-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea rax,[rsi+128]       ; RAX = Reload source address
mov rdx,rcx             ; RDX = Reload length
shr rdx,4               ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_NtRead_SSE128  ; Go if Length < 16 instructions
mov ebx,256             ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------;           
; 256 bytes per cycle with 16 128-bit SSE registers
DumpStart_NtRead_SSE128:
Block_NtRead_SSE128:
movntdqa xmm0, [rax-16*08]
movntdqa xmm1, [rax-16*07]
movntdqa xmm2, [rax-16*06]
movntdqa xmm3, [rax-16*05]
movntdqa xmm4, [rax-16*04]
movntdqa xmm5, [rax-16*03]
movntdqa xmm6, [rax-16*02]
movntdqa xmm7, [rax-16*01]
movntdqa xmm8, [rax+16*00]
movntdqa xmm9, [rax+16*01]
movntdqa xmm10,[rax+16*02]
movntdqa xmm11,[rax+16*03]
movntdqa xmm12,[rax+16*04]
movntdqa xmm13,[rax+16*05]
movntdqa xmm14,[rax+16*06]
movntdqa xmm15,[rax+16*07]
add rax,rbx                 ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_NtRead_SSE128     ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtRead_SSE128:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtRead_SSE128:
mov edx,ecx
and edx,00001111b           ; ECX = Extract tail length
jz Measure_NtRead_SSE128
mov ebx,16                  ; 16 bytes per cycle with one 128-bit SSE register
;---------- Tail cycle --------------------------------------------------------;
Tail_NtRead_SSE128:
movntdqa xmm0, [rax-16*08]
add rax,rbx                    ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtRead_SSE128         ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtRead_SSE128:
dec rbp
jnz Measurement_NtRead_SSE128  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
