;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Write x86 (MOV) " , mode = temporal.                ;
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
Pattern_Write_MOV32:
xor eax,eax           ; EAX = Data pattern for write = 0
push ebx              ; EBX = High 32 bits of measurement repeats count
Measurement_Write_MOV32:
;---------- Prepare big cycle -------------------------------------------------;
mov ebx,esi           ; EBX = Reload source address
mov edx,ecx           ; EDX = Reload length
shr edx,4             ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Write_MOV32  ; Go if Length < 16 instructions
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Write_MOV32:
Block_Write_MOV32:
mov [ebx+04*00],eax
mov [ebx+04*01],eax
mov [ebx+04*02],eax
mov [ebx+04*03],eax
mov [ebx+04*04],eax
mov [ebx+04*05],eax
mov [ebx+04*06],eax
mov [ebx+04*07],eax
mov [ebx+04*08],eax
mov [ebx+04*09],eax
mov [ebx+04*10],eax
mov [ebx+04*11],eax
mov [ebx+04*12],eax
mov [ebx+04*13],eax
mov [ebx+04*14],eax
mov [ebx+04*15],eax
add ebx,64              ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_Write_MOV32   ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Write_MOV32:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Write_MOV32:
mov edx,ecx
and edx,00001111b       ; ECX = Extract tail length
jz Measure_Write_MOV32
;---------- Tail cycle --------------------------------------------------------;
Tail_Write_MOV32:
mov [ebx+04*00],eax
add ebx,4               ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Write_MOV32    ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Write_MOV32:
sub ebp,1
sbb dword [esp],0
mov edx,ebp                   ; EDX = Temporary disjunction result, must be re-loaded for each repeat
or edx,[esp]
jnz Measurement_Write_MOV32   ; Cycle for measurement, repeat same operation
pop ebx
ret
