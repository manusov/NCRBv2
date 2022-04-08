;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Dot product FMA-256 (VFMADD231PD) " ,               ;
; mode = temporal.                                                             ;
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
Pattern_Dot_FMA256:
push ebx              ; EBX = High 32 bits of measurement repeats count
Measurement_Dot_FMA256:
;---------- Prepare big cycle -------------------------------------------------;
vzeroall
;---------- Optimize offsets --------------------------------------------------;
; Set pointer to middle of 256-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea eax,[esi+128]    ; EAX = Reload source address
lea ebx,[edi+128]    ; EBX = Reload destination address
mov edx,ecx          ; EDX = Reload length
shr edx,3            ; EDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_Dot_FMA256  ; Go if Length < 8 instructions
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Dot_FMA256:
Block_Dot_FMA256:
vmovapd ymm0,[eax-32*04]
vmovapd ymm1,[eax-32*03]
vmovapd ymm2,[eax-32*02]
vmovapd ymm3,[eax-32*01]
vfmadd231pd ymm4,ymm0,[ebx-32*04]
vfmadd231pd ymm5,ymm1,[ebx-32*03]
vfmadd231pd ymm6,ymm2,[ebx-32*02]
vfmadd231pd ymm7,ymm3,[ebx-32*01]
vmovapd ymm0,[eax+32*00]
vmovapd ymm1,[eax+32*01]
vmovapd ymm2,[eax+32*02]
vmovapd ymm3,[eax+32*03]
vfmadd231pd ymm4,ymm0,[ebx+32*00]
vfmadd231pd ymm5,ymm1,[ebx+32*01]
vfmadd231pd ymm6,ymm2,[ebx+32*02]
vfmadd231pd ymm7,ymm3,[ebx+32*03]
add eax,256           ; Modify source address
add ebx,256           ; Modify destination address
dec edx
jnz Block_Dot_FMA256  ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Dot_FMA256:
;---------- Horizontal additions for main part, result is vector --------------;
vhaddpd ymm0,ymm4,ymm6
vhaddpd ymm1,ymm5,ymm7
vextractf128 xmm2,ymm0,1
vextractf128 xmm3,ymm1,1
vaddpd xmm0,xmm0,xmm1
vaddpd xmm2,xmm2,xmm3
vaddpd xmm0,xmm0,xmm2
;---------- Prepare tail cycle ------------------------------------------------;
Small_Dot_FMA256:
mov edx,ecx
and edx,00000111b       ; ECX = Extract tail length
jz Measure_Dot_FMA256
;---------- Tail cycle --------------------------------------------------------;
Tail_Dot_FMA256:
vmovapd ymm1,[eax-32*04]
vfmadd231pd ymm0,ymm1,[ebx-32*04]
add eax,32              ; Modify source address
add ebx,32              ; Modify destination address
dec edx
jnz Tail_Dot_FMA256     ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Horizontal additions for tail, result is scalar -------------------;
vextractf128 xmm1,ymm0,1
vaddpd xmm1,xmm1,xmm0
vhaddpd xmm0,xmm1,xmm1
;---------- Measurement cycle -------------------------------------------------;
Measure_Dot_FMA256:
sub ebp,1               ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_Dot_FMA256  ; Cycle for measurement, repeat same operation
pop ebx
ret
