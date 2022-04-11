;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Copy AVX-512 (VMOVNTDQA + VMOVNTPD) " ,             ;
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
Pattern_NtRCopy_AVX512:
Measurement_NtRCopy_AVX512:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi              ; RAX = Reload source address
mov rbx,rdi              ; RBX = Reload destination address
mov rdx,rcx              ; RDX = Reload length
shr rdx,3                ; RDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_NtRCopy_AVX512  ; Go if Length < 8 instructions
mov r8d,512              ; R8 = Addend, this operation also clear bits R8[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtRCopy_AVX512:
Block_NtRCopy_AVX512:
vmovntdqa zmm0,[rax+64*00]
vmovntpd [rbx+64*00],zmm0
vmovntdqa zmm1,[rax+64*01]
vmovntpd [rbx+64*01],zmm1
vmovntdqa zmm2,[rax+64*02]
vmovntpd [rbx+64*02],zmm2
vmovntdqa zmm3,[rax+64*03]
vmovntpd [rbx+64*03],zmm3
vmovntdqa zmm4,[rax+64*04]
vmovntpd [rbx+64*04],zmm4
vmovntdqa zmm5,[rax+64*05]
vmovntpd [rbx+64*05],zmm5
vmovntdqa zmm6,[rax+64*06]
vmovntpd [rbx+64*06],zmm6
vmovntdqa zmm7,[rax+64*07]
vmovntpd [rbx+64*07],zmm7
add rax,r8                 ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8                 ; Modify destination address, addend=register (not constant) for optimization!
dec rdx
jnz Block_NtRCopy_AVX512   ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtRCopy_AVX512:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtRCopy_AVX512:
mov edx,ecx
and edx,00000111b          ; ECX = Extract tail length
jz Measure_NtRCopy_AVX512
mov r8d,64
;---------- Tail cycle --------------------------------------------------------;
Tail_NtRCopy_AVX512:
vmovntdqa zmm8,[rax+64*00]
vmovntpd [rbx+64*00],zmm8
add rax,r8                 ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8                 ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtRCopy_AVX512    ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtRCopy_AVX512:
dec rbp
jnz Measurement_NtRCopy_AVX512  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
