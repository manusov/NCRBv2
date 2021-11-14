;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Write SSE-128 (MOVAPS) " , mode = temporal.         ;
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
Pattern_Write_SSE128:
xorps xmm0,xmm0        ; Clear XMM0-XMM15 for write data
xorps xmm1,xmm1
xorps xmm2,xmm2
xorps xmm3,xmm3
xorps xmm4,xmm4
xorps xmm5,xmm5
xorps xmm6,xmm6
xorps xmm7,xmm7
xorps xmm8,xmm8
xorps xmm9,xmm9
xorps xmm10,xmm10
xorps xmm11,xmm11
xorps xmm12,xmm12
xorps xmm13,xmm13
xorps xmm14,xmm14
xorps xmm15,xmm15
Measurement_Write_SSE128:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 256-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea rax,[rsi+128]      ; RAX = Reload source address
mov rdx,rcx            ; RDX = Reload length
shr rdx,4              ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Write_SSE128  ; Go if Length < 16 instructions
mov ebx,256            ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Write_SSE128:
Block_Write_SSE128:
movaps [rax-16*08],xmm0
movaps [rax-16*07],xmm1
movaps [rax-16*06],xmm2
movaps [rax-16*05],xmm3
movaps [rax-16*04],xmm4
movaps [rax-16*03],xmm5
movaps [rax-16*02],xmm6
movaps [rax-16*01],xmm7
movaps [rax+16*00],xmm8
movaps [rax+16*01],xmm9
movaps [rax+16*02],xmm10
movaps [rax+16*03],xmm11
movaps [rax+16*04],xmm12
movaps [rax+16*05],xmm13
movaps [rax+16*06],xmm14
movaps [rax+16*07],xmm15
add rax,rbx                ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Write_SSE128     ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Write_SSE128:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Write_SSE128:
mov edx,ecx
and edx,00001111b          ; ECX = Extract tail length
jz Measure_Write_SSE128
mov ebx,16
;---------- Tail cycle --------------------------------------------------------;
Tail_Write_SSE128:
movaps [rax-16*08],xmm0
add rax,rbx                ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Write_SSE128      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Write_SSE128:
dec rbp
jnz Measurement_Write_SSE128  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
