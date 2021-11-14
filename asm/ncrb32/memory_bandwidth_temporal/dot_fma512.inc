;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Dot product FMA-512 (VFMADD231PD) " ,               ;
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
Pattern_Dot_FMA512:
push ebx              ; EBX = High 32 bits of measurement repeats count
Measurement_Dot_FMA512:
;---------- Prepare big cycle -------------------------------------------------;
vzeroall
mov eax,esi          ; EAX = Reload source address
mov ebx,edi          ; EBX = Reload destination address
mov edx,ecx          ; EDX = Reload length
shr edx,3            ; EDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_Dot_FMA512  ; Go if Length < 8 instructions
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Dot_FMA512:
Block_Dot_FMA512:
vmovapd zmm0,[eax+64*00]
vmovapd zmm1,[eax+64*01]
vmovapd zmm2,[eax+64*02]
vmovapd zmm3,[eax+64*03]
vfmadd231pd zmm4,zmm0,[ebx+64*00]
vfmadd231pd zmm5,zmm1,[ebx+64*01]
vfmadd231pd zmm6,zmm2,[ebx+64*02]
vfmadd231pd zmm7,zmm3,[ebx+64*03]
vmovapd zmm0,[eax+64*04]
vmovapd zmm1,[eax+64*05]
vmovapd zmm2,[eax+64*06]
vmovapd zmm3,[eax+64*07]
vfmadd231pd zmm4,zmm0,[ebx+64*04]
vfmadd231pd zmm5,zmm1,[ebx+64*05]
vfmadd231pd zmm6,zmm2,[ebx+64*06]
vfmadd231pd zmm7,zmm3,[ebx+64*07]
add eax,512           ; Modify source address
add ebx,512           ; Modify destination address
dec edx
jnz Block_Dot_FMA512  ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Dot_FMA512:
;---------- Horizontal additions for main part, result is vector --------------;
; 512
vaddpd zmm4,zmm4,zmm5
vaddpd zmm6,zmm6,zmm7
vaddpd zmm4,zmm4,zmm6
vextractf64x4 ymm5,zmm4,1
; 256
vhaddpd ymm4,ymm4,ymm5
vextractf128 xmm5,ymm4,1
; 128
vaddpd xmm4,xmm4,xmm5
; 64
vhaddpd xmm4,xmm4,xmm4
vxorpd zmm0,zmm0,zmm0
vmovsd xmm0,xmm0,xmm4
;---------- Prepare tail cycle ------------------------------------------------;
Small_Dot_FMA512:
mov edx,ecx
and edx,00000111b     ; ECX = Extract tail length
jz Measure_Dot_FMA512
;---------- Tail cycle --------------------------------------------------------;
Tail_Dot_FMA512:
vmovapd zmm1,[eax+64*00]
vfmadd231pd zmm0,zmm1,[ebx+64*00]
add eax,64            ; Modify source address, addend=register (not constant) for optimization!
add ebx,64            ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Dot_FMA512   ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Horizontal additions for tail, result is scalar -------------------;
vextractf64x4 ymm1,zmm0,1
; 256
vhaddpd ymm0,ymm0,ymm1
vextractf128 xmm2,ymm0,1
; 128
vaddpd xmm0,xmm0,xmm2
; 64
vhaddpd xmm0,xmm0,xmm0
;---------- Measurement cycle -------------------------------------------------;
Measure_Dot_FMA512:
sub ebp,1                    ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_Dot_FMA512  ; Cycle for measurement, repeat same operation
pop ebx
ret
