;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Read AVX-256 (PREFETCHNTA + VMOVAPD) " ,            ;
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
Pattern_NtpRead_AVX256:
Measurement_NtpRead_AVX256:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 512-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea rax,[rsi+256]        ; RAX = Reload source address
mov rdx,rcx              ; RDX = Reload length
shr rdx,4                ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_NtpRead_AVX256  ; Go if Length < 16 instructions
mov ebx,512              ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtpRead_AVX256:
Block_NtpRead_AVX256:
;---------- Prefetch ----------------------------------------------------------; 
; note about 512 bytes per iteration, 
; skip last 2 iterations means skip 1024 bytes
cmp rdx,2                       ; Skip for 2 last iterations: RBX*2 = 512*2 = 1024
jbe @f                          ; Skip prefetch: prevent prefetch above block size
prefetchnta [rax-32*08+rbx*2]
prefetchnta [rax-32*06+rbx*2]
prefetchnta [rax-32*04+rbx*2]
prefetchnta [rax-32*02+rbx*2]
prefetchnta [rax+32*00+rbx*2]
prefetchnta [rax+32*02+rbx*2]
prefetchnta [rax+32*04+rbx*2]
prefetchnta [rax+32*06+rbx*2]
@@:
;---------- Read --------------------------------------------------------------;
vmovapd ymm0, [rax-32*08]
vmovapd ymm1, [rax-32*07]
vmovapd ymm2, [rax-32*06]
vmovapd ymm3, [rax-32*05]
vmovapd ymm4, [rax-32*04]
vmovapd ymm5, [rax-32*03]
vmovapd ymm6, [rax-32*02]
vmovapd ymm7, [rax-32*01]
vmovapd ymm8, [rax+32*00]
vmovapd ymm9, [rax+32*01]
vmovapd ymm10,[rax+32*02]
vmovapd ymm11,[rax+32*03]
vmovapd ymm12,[rax+32*04]
vmovapd ymm13,[rax+32*05]
vmovapd ymm14,[rax+32*06]
vmovapd ymm15,[rax+32*07]
add rax,rbx                ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_NtpRead_AVX256   ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtpRead_AVX256:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtpRead_AVX256:
mov edx,ecx
and edx,00001111b          ; ECX = Extract tail length
jz Measure_NtpRead_AVX256
mov ebx,32
;---------- Tail cycle --------------------------------------------------------;
Tail_NtpRead_AVX256:
vmovapd ymm0, [rax-32*08]
add rax,rbx                ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtpRead_AVX256    ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtpRead_AVX256:
dec rbp
jnz Measurement_NtpRead_AVX256  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
