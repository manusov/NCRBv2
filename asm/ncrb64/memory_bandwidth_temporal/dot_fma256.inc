;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Dot product FMA-256 (VFMADD231PD) " ,               ;
; mode = temporal                                                              ;
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
Pattern_Dot_FMA256:
Measurement_Dot_FMA256:
;---------- Prepare big cycle, clear YMM registers ----------------------------;
vzeroall
;---------- Offsets optimization ----------------------------------------------;
; Set pointer to middle of 256-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea rax,[rsi+128]    ; RAX = Reload source address
lea rbx,[rdi+128]    ; RBX = Reload destination address
mov rdx,rcx          ; RDX = Reload length
shr rdx,3            ; RDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_Dot_FMA256  ; Go if Length < 8 instructions
mov r8d,256          ; R8 = Addend, this operation also clear bits R8[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Dot_FMA256:
Block_Dot_FMA256:
vmovapd ymm0,[rax-32*04]
vmovapd ymm1,[rax-32*03]
vmovapd ymm2,[rax-32*02]
vmovapd ymm3,[rax-32*01]
vmovapd ymm4,[rax+32*00]
vmovapd ymm5,[rax+32*01]
vmovapd ymm6,[rax+32*02]
vmovapd ymm7,[rax+32*03]
vfmadd231pd ymm8 ,ymm0,[rbx-32*04]
vfmadd231pd ymm9 ,ymm1,[rbx-32*03]
vfmadd231pd ymm10,ymm2,[rbx-32*02]
vfmadd231pd ymm11,ymm3,[rbx-32*01]
vfmadd231pd ymm12,ymm4,[rbx+32*00]
vfmadd231pd ymm13,ymm5,[rbx+32*01]
vfmadd231pd ymm14,ymm6,[rbx+32*02]
vfmadd231pd ymm15,ymm7,[rbx+32*03]
add rax,r8            ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8            ; Modify destination address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Dot_FMA256  ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Dot_FMA256:
;---------- Horizontal additions for main part, result is vector --------------;
vhaddpd ymm0,ymm8,ymm9
vhaddpd ymm1,ymm10,ymm11
vhaddpd ymm2,ymm12,ymm13
vhaddpd ymm3,ymm14,ymm15
vextractf128 xmm4,ymm0,1
vextractf128 xmm5,ymm1,1
vextractf128 xmm6,ymm2,1
vextractf128 xmm7,ymm3,1
vaddpd xmm0,xmm0,xmm4
vaddpd xmm1,xmm1,xmm5
vaddpd xmm2,xmm2,xmm6
vaddpd xmm3,xmm3,xmm7
vaddpd xmm8,xmm0,xmm1
vaddpd xmm9,xmm2,xmm3
vaddpd xmm8,xmm8,xmm9
;---------- Prepare tail cycle ------------------------------------------------;
Small_Dot_FMA256:
mov edx,ecx
and edx,00000111b       ; ECX = Extract tail length
jz Measure_Dot_FMA256
mov r8d,32
;---------- Tail cycle --------------------------------------------------------;
Tail_Dot_FMA256:
vmovapd ymm0,[rax-32*04]
vfmadd231pd ymm8,ymm0,[rbx-32*04]
add rax,r8           ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8           ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Dot_FMA256  ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Horizontal additions for tail, result is scalar -------------------;
vextractf128 xmm9,ymm8,1
vaddpd xmm8,xmm8,xmm9
vhaddpd xmm8,xmm8,xmm8
;---------- Measurement cycle -------------------------------------------------;
Measure_Dot_FMA256:
dec rbp
jnz Measurement_Dot_FMA256  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
