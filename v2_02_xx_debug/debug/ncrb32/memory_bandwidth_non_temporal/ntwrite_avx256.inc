;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Write AVX-256 (VMOVNTPD) " ,                        ;
; mode = non temporal, one of variants, depends on options.                    ;
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
Pattern_NtWrite_AVX256:
vzeroall                 ; Clear ZMM0-ZMM7 (8 registers only because 32-bit mode) for write data
push ebx                 ; EBX = High 32 bits of measurement repeats count
Measurement_NtWrite_AVX256:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 512-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea eax,[esi+256]        ; EAX = Reload source address
mov edx,ecx              ; EDX = Reload length
shr edx,4                ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_NtWrite_AVX256  ; Go if Length < 16 instructions
mov ebx,512              ; EBX = Addend
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtWrite_AVX256:
Block_NtWrite_AVX256:
vmovntpd [eax-32*08],ymm0
vmovntpd [eax-32*07],ymm1
vmovntpd [eax-32*06],ymm2
vmovntpd [eax-32*05],ymm3
vmovntpd [eax-32*04],ymm4
vmovntpd [eax-32*03],ymm5
vmovntpd [eax-32*02],ymm6
vmovntpd [eax-32*01],ymm7
vmovntpd [eax+32*00],ymm0
vmovntpd [eax+32*01],ymm1
vmovntpd [eax+32*02],ymm2
vmovntpd [eax+32*03],ymm3
vmovntpd [eax+32*04],ymm4
vmovntpd [eax+32*05],ymm5
vmovntpd [eax+32*06],ymm6
vmovntpd [eax+32*07],ymm7
add eax,ebx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_NtWrite_AVX256     ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtWrite_AVX256:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtWrite_AVX256:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_NtWrite_AVX256
mov ebx,32
;---------- Tail cycle --------------------------------------------------------;
Tail_NtWrite_AVX256:
vmovntpd [eax-32*08],ymm0
add eax,ebx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtWrite_AVX256      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtWrite_AVX256:
sub ebp,1                    ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_NtWrite_AVX256  ; Cycle for measurement, repeat same operation
pop ebx
ret
