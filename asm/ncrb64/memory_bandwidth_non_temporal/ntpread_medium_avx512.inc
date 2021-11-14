;---------- Memory performance patterns ---------------------------------------;
; Memory access method = " Read AVX-512 (PREFETCHNTA + VMOVAPD) " ,            ;
; mode = non temporal, prefetch distance = medium ( + 512 )                    ;
; one of variants, depends on options.                                         ;
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
Pattern_NtpRead_M_AVX512:
Measurement_NtpRead_M_AVX512:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi                  ; RAX = Reload source address
mov rdx,rcx                  ; RDX = Reload length
shr rdx,3                    ; RDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_NtpRead_M_AVX512    ; Go if Length < 8 instructions
mov ebx,512                  ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtpRead_M_AVX512:
Block_NtpRead_M_AVX512:
;---------- Prefetch ----------------------------------------------------------;
; Note about 512 bytes per iteration, skip last 3 iterations means skip 1536 bytes
cmp rdx,3                             ; Skip for 3 last iterations: RBX*2+512 = 512*3 = 1536
jbe @f                                ; Skip prefetch: prevent prefetch above block size
prefetchnta [rax-64*00+rbx*2 + 512]   ; Medium prefetch distance means + 512
prefetchnta [rax-64*01+rbx*2 + 512]
prefetchnta [rax-64*02+rbx*2 + 512]
prefetchnta [rax-64*03+rbx*2 + 512]
prefetchnta [rax+64*04+rbx*2 + 512]
prefetchnta [rax+64*05+rbx*2 + 512]
prefetchnta [rax+64*06+rbx*2 + 512]
prefetchnta [rax+64*07+rbx*2 + 512]
@@:
;---------- Read --------------------------------------------------------------;
vmovapd zmm0, [rax+64*00]
vmovapd zmm1, [rax+64*01]
vmovapd zmm2, [rax+64*02]
vmovapd zmm3, [rax+64*03]
vmovapd zmm4, [rax+64*04]
vmovapd zmm5, [rax+64*05]
vmovapd zmm6, [rax+64*06]
vmovapd zmm7, [rax+64*07]
add rax,rbx                  ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_NtpRead_M_AVX512   ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtpRead_M_AVX512:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtpRead_M_AVX512:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_NtpRead_M_AVX512
mov ebx,32
;---------- Tail cycle --------------------------------------------------------;
Tail_NtpRead_M_AVX512:
vmovapd ymm0, [rax+64*00]
add rax,rbx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtpRead_M_AVX512    ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtpRead_M_AVX512:
dec rbp
jnz Measurement_NtpRead_M_AVX512  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret

