;---------- Memory performance patterns ---------------------------------------;
; Memory access method = " Read AVX-512 (PREFETCHNTA + VMOVAPD) " ,            ;
; mode = non temporal, prefetch distance = medium ( + 512 )                    ;
; one of variants, depends on options.                                         ;
;                                                                              ;
; INPUT:   ESI = Block #1 pointer (32-bit flat)                                ;
;          EDI = Block #2 pointer (32-bit flat)                                ;
;                For Read, Write, Modify use ESI as Source or Destination      ;
;                For Copy use ESI = Source , EDI = Destination                 ;
;          ECX = Block length, units = instructions                            ;
;          EBX:EBP = Number of measurement repeats, EBX=High32, EBP=Low32      ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;          All registers corrupted                                             ;
;                                                                              ;
;------------------------------------------------------------------------------;
Pattern_NtpRead_M_AVX512:
push ebx                     ; EBX = High 32 bits of measurement repeats count
Measurement_NtpRead_M_AVX512:
;---------- Prepare big cycle -------------------------------------------------;
mov eax,esi                  ; EAX = Reload source address
mov edx,ecx                  ; EDX = Reload length
shr edx,3                    ; EDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_NtpRead_M_AVX512    ; Go if Length < 8 instructions
mov ebx,512                  ; EBX = Addend
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtpRead_M_AVX512:
Block_NtpRead_M_AVX512:
;---------- Prefetch ----------------------------------------------------------;
; Note about 512 bytes per iteration, skip last 3 iterations means skip 1536 bytes
cmp edx,3                             ; Skip for 3 last iterations: EBX*2+512 = 512*3 = 1536
jbe @f                                ; Skip prefetch: prevent prefetch above block size
prefetchnta [eax-64*00+ebx*2 + 512]   ; Medium prefetch distance means + 512
prefetchnta [eax-64*01+ebx*2 + 512]
prefetchnta [eax-64*02+ebx*2 + 512]
prefetchnta [eax-64*03+ebx*2 + 512]
prefetchnta [eax+64*04+ebx*2 + 512]
prefetchnta [eax+64*05+ebx*2 + 512]
prefetchnta [eax+64*06+ebx*2 + 512]
prefetchnta [eax+64*07+ebx*2 + 512]
@@:
;---------- Read --------------------------------------------------------------;
vmovapd zmm0, [eax+64*00]
vmovapd zmm1, [eax+64*01]
vmovapd zmm2, [eax+64*02]
vmovapd zmm3, [eax+64*03]
vmovapd zmm4, [eax+64*04]
vmovapd zmm5, [eax+64*05]
vmovapd zmm6, [eax+64*06]
vmovapd zmm7, [eax+64*07]
add eax,ebx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
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
vmovapd ymm0, [eax+64*00]
add eax,ebx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtpRead_M_AVX512    ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtpRead_M_AVX512:
sub ebp,1                    ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_NtpRead_M_AVX512  ; Cycle for measurement, repeat same operation
pop ebx                           ; This POP for restore stack only
ret
