;---------- Memory performance patterns ---------------------------------------;
; Memory access method = " Read AVX-256 (PREFETCHNTA + VMOVAPD) " ,            ;
; mode = non temporal, prefetch distance = long ( + 2048 )                     ;
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
Pattern_NtpRead_L_AVX256:
push ebx                     ; EBX = High 32 bits of measurement repeats count
Measurement_NtpRead_L_AVX256:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 512-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea eax,[esi+256]            ; EAX = Reload source address
mov edx,ecx                  ; EDX = Reload length
shr edx,4                    ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_NtpRead_L_AVX256    ; Go if Length < 16 instructions
mov ebx,512                  ; EBX = Addend
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtpRead_L_AVX256:
Block_NtpRead_L_AVX256:
;---------- Prefetch ----------------------------------------------------------;
; Note about 512 bytes per iteration, skip last 6 iterations means skip 3072 bytes
cmp edx,6                                ; Skip for 6 last iterations: EBX+2048 = 512*6 = 3072
jbe @f                                   ; Skip prefetch: prevent prefetch above block size
prefetchnta [eax-32*08+ebx*2 + 2048]     ; Long prefetch distance means + 2048
prefetchnta [eax-32*06+ebx*2 + 2048]
prefetchnta [eax-32*04+ebx*2 + 2048]
prefetchnta [eax-32*02+ebx*2 + 2048]
prefetchnta [eax+32*00+ebx*2 + 2048]
prefetchnta [eax+32*02+ebx*2 + 2048]
prefetchnta [eax+32*04+ebx*2 + 2048]
prefetchnta [eax+32*06+ebx*2 + 2048]
@@:
;---------- Read --------------------------------------------------------------;
vmovapd ymm0,[eax-32*08]
vmovapd ymm1,[eax-32*07]
vmovapd ymm2,[eax-32*06]
vmovapd ymm3,[eax-32*05]
vmovapd ymm4,[eax-32*04]
vmovapd ymm5,[eax-32*03]
vmovapd ymm6,[eax-32*02]
vmovapd ymm7,[eax-32*01]
vmovapd ymm0,[eax+32*00]
vmovapd ymm1,[eax+32*01]
vmovapd ymm2,[eax+32*02]
vmovapd ymm3,[eax+32*03]
vmovapd ymm4,[eax+32*04]
vmovapd ymm5,[eax+32*05]
vmovapd ymm6,[eax+32*06]
vmovapd ymm7,[eax+32*07]
add eax,ebx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_NtpRead_L_AVX256   ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtpRead_L_AVX256:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtpRead_L_AVX256:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_NtpRead_L_AVX256
mov ebx,32
;---------- Tail cycle --------------------------------------------------------;
Tail_NtpRead_L_AVX256:
vmovapd ymm0,[eax-32*08]
add eax,ebx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtpRead_L_AVX256    ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtpRead_L_AVX256:
sub ebp,1                    ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_NtpRead_L_AVX256  ; Cycle for measurement, repeat same operation
pop ebx
ret
