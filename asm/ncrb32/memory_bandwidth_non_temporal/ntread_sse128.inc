;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Read SSE-128 (MOVNTDQA) " ,                         ;
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
Pattern_NtRead_SSE128:
push ebx                ; EBX = High 32 bits of measurement repeats count
Measurement_NtRead_SSE128:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 256-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea eax,[esi+128]       ; EAX = Reload source address
mov edx,ecx             ; EDX = Reload length
shr edx,4               ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_NtRead_SSE128  ; Go if Length < 16 instructions
mov ebx,256             ; ebx = Addend
;---------- Big cycle ---------------------------------------------------------;
; 256 bytes per cycle with 16 128-bit SSE registers
DumpStart_NtRead_SSE128:
Block_NtRead_SSE128:
movntdqa xmm0,[eax-16*08]
movntdqa xmm1,[eax-16*07]
movntdqa xmm2,[eax-16*06]
movntdqa xmm3,[eax-16*05]
movntdqa xmm4,[eax-16*04]
movntdqa xmm5,[eax-16*03]
movntdqa xmm6,[eax-16*02]
movntdqa xmm7,[eax-16*01]
movntdqa xmm0,[eax+16*00]
movntdqa xmm1,[eax+16*01]
movntdqa xmm2,[eax+16*02]
movntdqa xmm3,[eax+16*03]
movntdqa xmm4,[eax+16*04]
movntdqa xmm5,[eax+16*05]
movntdqa xmm6,[eax+16*06]
movntdqa xmm7,[eax+16*07]
add eax,ebx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_NtRead_SSE128      ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtRead_SSE128:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtRead_SSE128:
mov edx,ecx
and edx,00001111b            ; ECX = Extract tail length
jz Measure_NtRead_SSE128
mov ebx,16                   ; 16 bytes per cycle with one 128-bit SSE register
;---------- Tail cycle --------------------------------------------------------;
Tail_NtRead_SSE128:
movntdqa xmm0,[eax-16*08]
add eax,ebx                  ; Modify address, addend=register (not constant) for optimization!
dec edx                    
jnz Tail_NtRead_SSE128       ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtRead_SSE128:
sub ebp,1                    ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_NtRead_SSE128  ; Cycle for measurement, repeat same operation
pop ebx
ret
