;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Write SSE-128 (MOVAPS) " , mode = temporal.         ;
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
Pattern_Write_SSE128:
xorps xmm0,xmm0        ; Clear XMM0-XMM7 for write data
xorps xmm1,xmm1
xorps xmm2,xmm2
xorps xmm3,xmm3
xorps xmm4,xmm4
xorps xmm5,xmm5
xorps xmm6,xmm6
xorps xmm7,xmm7
push ebx               ; EBX = High 32 bits of measurement repeats count
Measurement_Write_SSE128:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 256-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea eax,[esi+128]      ; EAX = Reload source address
mov edx,ecx            ; EDX = Reload length
shr edx,4              ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Write_SSE128  ; Go if Length < 16 instructions
mov ebx,256            ; EBX = Addend
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Write_SSE128:
Block_Write_SSE128:
movaps [eax-16*08],xmm0
movaps [eax-16*07],xmm1
movaps [eax-16*06],xmm2
movaps [eax-16*05],xmm3
movaps [eax-16*04],xmm4
movaps [eax-16*03],xmm5
movaps [eax-16*02],xmm6
movaps [eax-16*01],xmm7
movaps [eax+16*00],xmm0
movaps [eax+16*01],xmm1
movaps [eax+16*02],xmm2
movaps [eax+16*03],xmm3
movaps [eax+16*04],xmm4
movaps [eax+16*05],xmm5
movaps [eax+16*06],xmm6
movaps [eax+16*07],xmm7
add eax,ebx               ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_Write_SSE128    ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Write_SSE128:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Write_SSE128:
mov edx,ecx
and edx,00001111b         ; ECX = Extract tail length
jz Measure_Write_SSE128
mov ebx,16
;---------- Tail cycle --------------------------------------------------------;
Tail_Write_SSE128:
movaps [eax-16*08],xmm0
add eax,ebx               ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Write_SSE128     ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Write_SSE128:
sub ebp,1                    ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_Write_SSE128  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
pop ebx
ret
