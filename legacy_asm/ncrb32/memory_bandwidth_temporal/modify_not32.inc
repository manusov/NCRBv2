;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Modify x86 (NOT) " , mode = temporal.               ;
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
Pattern_Modify_NOT32:
push ebx              ; EBX = High 32 bits of measurement repeats count
Measurement_Modify_NOT32:
;---------- Prepare big cycle -------------------------------------------------;
mov ebx,esi            ; EBX = Reload source address
mov edx,ecx            ; EDX = Reload length
shr edx,4              ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Modify_NOT32  ; Go if Length < 16 instructions
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Modify_NOT32:
Block_Modify_NOT32:
not dword [ebx+04*00]
not dword [ebx+04*01]
not dword [ebx+04*02]
not dword [ebx+04*03]
not dword [ebx+04*04]
not dword [ebx+04*05]
not dword [ebx+04*06]
not dword [ebx+04*07]
not dword [ebx+04*08]
not dword [ebx+04*09]
not dword [ebx+04*10]
not dword [ebx+04*11]
not dword [ebx+04*12]
not dword [ebx+04*13]
not dword [ebx+04*14]
not dword [ebx+04*15]
add ebx,64              ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_Modify_NOT32  ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Modify_NOT32:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Modify_NOT32:
mov edx,ecx
and edx,00001111b       ; ECX = Extract tail length
jz Measure_Modify_NOT32
;---------- Tail cycle --------------------------------------------------------;
Tail_Modify_NOT32:
not dword [ebx+04*00]
add ebx,4               ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Modify_NOT32   ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Modify_NOT32:
sub ebp,1
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_Modify_NOT32   ; Cycle for measurement, repeat same operation
pop ebx
ret
