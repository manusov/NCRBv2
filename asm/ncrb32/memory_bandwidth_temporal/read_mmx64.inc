;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Read MMX-64 (MOVQ) " , mode = temporal.             ;
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
Pattern_Read_MMX64:
push ebx             ; EBX = High 32 bits of measurement repeats count
mov eax,128
Measurement_Read_MMX64:
;---------- Prepare big cycle -------------------------------------------------;
mov ebx,esi          ; EBX = Reload source address
mov edx,ecx          ; EDX = Reload length
shr edx,4            ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Read_MMX64  ; Go if Length < 16 instructions
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Read_MMX64:
Block_Read_MMX64:
movq mm0,[ebx+08*00]
movq mm1,[ebx+08*01]
movq mm2,[ebx+08*02]
movq mm3,[ebx+08*03]
movq mm4,[ebx+08*04]
movq mm5,[ebx+08*05]
movq mm6,[ebx+08*06]
movq mm7,[ebx+08*07]
movq mm0,[ebx+08*08]
movq mm1,[ebx+08*09]
movq mm2,[ebx+08*10]
movq mm3,[ebx+08*11]
movq mm4,[ebx+08*12]
movq mm5,[ebx+08*13]
movq mm6,[ebx+08*14]
movq mm7,[ebx+08*15]
add ebx,eax           ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_Read_MMX64  ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Read_MMX64:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Read_MMX64:
mov edx,ecx
and edx,00001111b     ; ECX = Extract tail length
jz Measure_Read_MMX64
;---------- Tail cycle --------------------------------------------------------;
Tail_Read_MMX64:
movq mm0,[ebx+08*00]
add ebx,8             ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Read_MMX64   ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Read_MMX64:
sub ebp,1             ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov edx,ebp
or edx,[esp]
jnz Measurement_Read_MMX64   ; Cycle for measurement, repeat same operation
emms                         ; Required empty MMX state before return
pop ebx
ret
