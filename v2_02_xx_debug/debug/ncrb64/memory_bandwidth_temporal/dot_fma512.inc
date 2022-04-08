;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Dot product FMA-512 (VFMADD231PD) " ,               ;
; mode = temporal.                                                             ;
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
Pattern_Dot_FMA512:
Measurement_Dot_FMA512:
;---------- Prepare big cycle -------------------------------------------------;
vzeroall
mov rax,rsi          ; RAX = Reload source address
mov rbx,rdi          ; RBX = Reload destination address
mov rdx,rcx          ; RDX = Reload length
shr rdx,3            ; RDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_Dot_FMA512  ; Go if Length < 8 instructions
mov r8d,512          ; R8 = Addend, this operation also clear bits R8[63-32]
;---------- Big cycle ---------------------------------------------------------l
DumpStart_Dot_FMA512:
Block_Dot_FMA512:
vmovapd zmm0,[rax+64*00]
vmovapd zmm1,[rax+64*01]
vmovapd zmm2,[rax+64*02]
vmovapd zmm3,[rax+64*03]
vmovapd zmm4,[rax+64*04]
vmovapd zmm5,[rax+64*05]
vmovapd zmm6,[rax+64*06]
vmovapd zmm7,[rax+64*07]
vfmadd231pd zmm8 ,zmm0,[rbx+64*00]
vfmadd231pd zmm9 ,zmm1,[rbx+64*01]
vfmadd231pd zmm10,zmm2,[rbx+64*02]
vfmadd231pd zmm11,zmm3,[rbx+64*03]
vfmadd231pd zmm12,zmm4,[rbx+64*04]
vfmadd231pd zmm13,zmm5,[rbx+64*05]
vfmadd231pd zmm14,zmm6,[rbx+64*06]
vfmadd231pd zmm15,zmm7,[rbx+64*07]
add rax,r8            ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8            ; Modify destination address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Dot_FMA512  ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Dot_FMA512:
;---------- Horizontal additions for main part, result is vector --------------;
; 512
vaddpd zmm8,zmm8,zmm12
vaddpd zmm9,zmm9,zmm13
vaddpd zmm10,zmm10,zmm14 
vaddpd zmm11,zmm11,zmm15
vaddpd zmm0,zmm8,zmm10
vaddpd zmm1,zmm9,zmm11
vaddpd zmm0,zmm0,zmm1
vextractf64x4 ymm1,zmm0,1
; 256
vaddpd ymm0,ymm0,ymm1
; 128
vextractf128 xmm2,ymm0,1
vaddpd xmm0,xmm0,xmm2
vhaddpd xmm1,xmm0,xmm0
vxorpd zmm0,zmm0,zmm0
vaddsd xmm0,xmm0,xmm1
;---------- Prepare tail cycle ------------------------------------------------;
Small_Dot_FMA512:
mov edx,ecx
and edx,00000111b      ; ECX = Extract tail length
jz Measure_Dot_FMA512
mov r8d,64
;---------- Tail cycle --------------------------------------------------------;
Tail_Dot_FMA512:
vmovapd zmm1,[rax+64*00]
vfmadd231pd zmm0,zmm1,[rbx+64*00]
add rax,r8           ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8           ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Dot_FMA512  ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Horizontal additions for tail, result is scalar -------------------;
; 512
vextractf64x4 ymm1,zmm0,1
vaddpd ymm0,ymm0,ymm1
; 256
vextractf128 xmm2,ymm0,1
; 128
vaddpd xmm0,xmm0,xmm2
vhaddpd xmm0,xmm0,xmm0
;---------- Measurement cycle -------------------------------------------------l
Measure_Dot_FMA512:
dec rbp
jnz Measurement_Dot_FMA512  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
