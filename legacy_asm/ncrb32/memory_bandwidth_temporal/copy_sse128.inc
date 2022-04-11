;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Copy SSE-128 (MOVAPS) " , mode = temporal.          ;
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
Pattern_Copy_SSE128:
push ebx                ; EBX = High 32 bits of measurement repeats count
Measurement_Copy_SSE128:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 256-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea eax,[esi+128]     ; EAX = Reload source address
lea ebx,[edi+128]     ; EBX = Reload destination address
mov edx,ecx           ; EDX = Reload length
shr edx,4             ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Copy_SSE128  ; Go if Length < 16 instructions
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Copy_SSE128:
Block_Copy_SSE128:
movaps xmm0,[eax-16*08]
movaps [ebx-16*08],xmm0
movaps xmm1,[eax-16*07]
movaps [ebx-16*07],xmm1
movaps xmm2,[eax-16*06]
movaps [ebx-16*06],xmm2
movaps xmm3,[eax-16*05]
movaps [ebx-16*05],xmm3
movaps xmm4,[eax-16*04]
movaps [ebx-16*04],xmm4
movaps xmm5,[eax-16*03]
movaps [ebx-16*03],xmm5
movaps xmm6,[eax-16*02]
movaps [ebx-16*02],xmm6
movaps xmm7,[eax-16*01]
movaps [ebx-16*01],xmm7
movaps xmm0,[eax+16*00]
movaps [ebx+16*00],xmm0
movaps xmm1,[eax+16*01]
movaps [ebx+16*01],xmm1
movaps xmm2,[eax+16*02]
movaps [ebx+16*02],xmm2
movaps xmm3,[eax+16*03]
movaps [ebx+16*03],xmm3
movaps xmm4,[eax+16*04]
movaps [ebx+16*04],xmm4
movaps xmm5,[eax+16*05]
movaps [ebx+16*05],xmm5
movaps xmm6,[eax+16*06]
movaps [ebx+16*06],xmm6
movaps xmm7,[eax+16*07]
movaps [ebx+16*07],xmm7
add eax,256               ; Modify source address
add ebx,256               ; Modify destination address
dec edx
jnz Block_Copy_SSE128     ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Copy_SSE128:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Copy_SSE128:
mov edx,ecx
and edx,00001111b         ; ECX = Extract tail length
jz Measure_Copy_SSE128
;---------- Tail cycle --------------------------------------------------------;
Tail_Copy_SSE128:
movaps xmm0,[eax-16*08]
movaps [ebx-16*08],xmm0
add eax,16                ; Modify source address, addend=register (not constant) for optimization!
add ebx,16                ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Copy_SSE128      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Copy_SSE128:
sub ebp,1                 ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_Copy_SSE128  ; Cycle for measurement, repeat same operation
pop ebx
ret
