;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Copy AVX-256 (VMOVNTDQA + VMOVNTPD) " ,             ;
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
Pattern_NtRCopy_AVX256:
Measurement_NtRCopy_AVX256:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 512-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea rax,[rsi+256]        ; RAX = Reload source address
lea rbx,[rdi+256]        ; RBX = Reload destination address
mov rdx,rcx              ; RDX = Reload length
shr rdx,4                ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_NtRCopy_AVX256  ; Go if Length < 16 instructions
mov r8d,512              ; R8 = Addend, this operation also clear bits R8[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtRCopy_AVX256:
Block_NtRCopy_AVX256:
vmovntdqa ymm0,[rax-32*08]
vmovntpd [rbx-32*08],ymm0
vmovntdqa ymm1,[rax-32*07]
vmovntpd [rbx-32*07],ymm1
vmovntdqa ymm2,[rax-32*06]
vmovntpd [rbx-32*06],ymm2
vmovntdqa ymm3,[rax-32*05]
vmovntpd [rbx-32*05],ymm3
vmovntdqa ymm4,[rax-32*04]
vmovntpd [rbx-32*04],ymm4
vmovntdqa ymm5,[rax-32*03]
vmovntpd [rbx-32*03],ymm5
vmovntdqa ymm6,[rax-32*02]
vmovntpd [rbx-32*02],ymm6
vmovntdqa ymm7,[rax-32*01]
vmovntpd [rbx-32*01],ymm7
vmovntdqa ymm8,[rax+32*00]
vmovntpd [rbx+32*00],ymm8
vmovntdqa ymm9,[rax+32*01]
vmovntpd [rbx+32*01],ymm9
vmovntdqa ymm10,[rax+32*02]
vmovntpd [rbx+32*02],ymm10
vmovntdqa ymm11,[rax+32*03]
vmovntpd [rbx+32*03],ymm11
vmovntdqa ymm12,[rax+32*04]
vmovntpd [rbx+32*04],ymm12
vmovntdqa ymm13,[rax+32*05]
vmovntpd [rbx+32*05],ymm13
vmovntdqa ymm14,[rax+32*06]
vmovntpd [rbx+32*06],ymm14
vmovntdqa ymm15,[rax+32*07]
vmovntpd [rbx+32*07],ymm15
add rax,r8                   ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8                   ; Modify destination address, addend=register (not constant) for optimization!
dec rdx
jnz Block_NtRCopy_AVX256     ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtRCopy_AVX256:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtRCopy_AVX256:
mov edx,ecx                    
and edx,00001111b            ; ECX = Extract tail length
jz Measure_NtRCopy_AVX256
mov r8d,32
;---------- Tail cycle --------------------------------------------------------;
Tail_NtRCopy_AVX256:
vmovntdqa ymm0,[rax-32*08]
vmovntpd [rbx-32*08],ymm0
add rax,r8                   ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8                   ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtRCopy_AVX256      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtRCopy_AVX256:
dec rbp
jnz Measurement_NtRCopy_AVX256  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
