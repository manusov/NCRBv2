;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Read AVX-512 (VMOVNTDQA) " ,                        ;
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
Pattern_NtRead_AVX512:
push ebx                     ; EBX = High 32 bits of measurement repeats count
Measurement_NtRead_AVX512:
;---------- Prepare big cycle -------------------------------------------------;
mov eax,esi             ; EAX = Reload source address
mov edx,ecx             ; EDX = Reload length
shr edx,3               ; EDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_NtRead_AVX512  ; Go if Length < 8 instructions
mov ebx,512             ; EBX = Addend, this operation also clear bits ebx[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtRead_AVX512:
Block_NtRead_AVX512:
vmovntdqa zmm0,[eax+64*00]
vmovntdqa zmm1,[eax+64*01]
vmovntdqa zmm2,[eax+64*02]
vmovntdqa zmm3,[eax+64*03]
vmovntdqa zmm4,[eax+64*04]
vmovntdqa zmm5,[eax+64*05]
vmovntdqa zmm6,[eax+64*06]
vmovntdqa zmm7,[eax+64*07]
add eax,ebx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_NtRead_AVX512      ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtRead_AVX512:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtRead_AVX512:
mov edx,ecx
and edx,00000111b            ; ECX = Extract tail length
jz Measure_NtRead_AVX512
mov ebx,64
;---------- Tail cycle --------------------------------------------------------;
Tail_NtRead_AVX512:
vmovntdqa zmm0,[eax+64*00]
add eax,ebx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtRead_AVX512       ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtRead_AVX512:
sub ebp,1                    ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_NtRead_AVX512  ; Cycle for measurement, repeat same operation
pop ebx
ret
