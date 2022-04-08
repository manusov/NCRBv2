;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Copy SSE-128 (MOVAPS) " , mode = temporal.          ;
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
Pattern_Copy_SSE128:
Measurement_Copy_SSE128:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 256-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte

lea rax,[rsi+128]     ; RAX = Reload source address
lea rbx,[rdi+128]     ; RBX = Reload destination address
mov rdx,rcx           ; RDX = Reload length
shr rdx,4             ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Copy_SSE128  ; Go if Length < 16 instructions
mov r8d,256           ; R8 = Addend, this operation also clear bits R8[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Copy_SSE128:
Block_Copy_SSE128:
movaps xmm0,[rax-16*08]
movaps [rbx-16*08],xmm0
movaps xmm1,[rax-16*07]
movaps [rbx-16*07],xmm1
movaps xmm2,[rax-16*06]
movaps [rbx-16*06],xmm2
movaps xmm3,[rax-16*05]
movaps [rbx-16*05],xmm3
movaps xmm4,[rax-16*04]
movaps [rbx-16*04],xmm4
movaps xmm5,[rax-16*03]
movaps [rbx-16*03],xmm5
movaps xmm6,[rax-16*02]
movaps [rbx-16*02],xmm6
movaps xmm7,[rax-16*01]
movaps [rbx-16*01],xmm7
movaps xmm8,[rax+16*00]
movaps [rbx+16*00],xmm8
movaps xmm9,[rax+16*01]
movaps [rbx+16*01],xmm9
movaps xmm10,[rax+16*02]
movaps [rbx+16*02],xmm10
movaps xmm11,[rax+16*03]
movaps [rbx+16*03],xmm11
movaps xmm12,[rax+16*04]
movaps [rbx+16*04],xmm12
movaps xmm13,[rax+16*05]
movaps [rbx+16*05],xmm13
movaps xmm14,[rax+16*06]
movaps [rbx+16*06],xmm14
movaps xmm15,[rax+16*07]
movaps [rbx+16*07],xmm15
add rax,r8                ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8                ; Modify destination address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Copy_SSE128     ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Copy_SSE128:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Copy_SSE128:
mov edx,ecx
and edx,00001111b         ; ECX = Extract tail length
jz Measure_Copy_SSE128
mov r8d,16
;---------- Tail cycle --------------------------------------------------------;
Tail_Copy_SSE128:
movaps xmm0,[rax-16*08]
movaps [rbx-16*08],xmm0
add rax,r8                ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8                ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Copy_SSE128      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Copy_SSE128:
dec rbp
jnz Measurement_Copy_SSE128  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
