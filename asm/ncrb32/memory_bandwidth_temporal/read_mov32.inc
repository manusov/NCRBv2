;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Read x86-64 (MOV) " , mode = temporal.              ;
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
Pattern_Read_MOV32:
push ebx             ; EBX = High 32 bits of measurement repeats count
Measurement_Read_MOV32:
;---------- Prepare big cycle -------------------------------------------------;
mov ebx,esi          ; EBX = Reload source address
mov edx,ecx          ; EDX = Reload length
shr edx,4            ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Read_MOV32  ; Go if Length < 16 instructions
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Read_MOV32:
Block_Read_MOV32:
mov eax,[ebx+04*00]
mov eax,[ebx+04*01]
mov eax,[ebx+04*02]
mov eax,[ebx+04*03]
mov eax,[ebx+04*04]
mov eax,[ebx+04*05]
mov eax,[ebx+04*06]
mov eax,[ebx+04*07]
mov eax,[ebx+04*08]
mov eax,[ebx+04*09]
mov eax,[ebx+04*10]
mov eax,[ebx+04*11]
mov eax,[ebx+04*12]
mov eax,[ebx+04*13]
mov eax,[ebx+04*14]
mov eax,[ebx+04*15]
add ebx,64            ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_Read_MOV32  ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Read_MOV32:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Read_MOV32:
mov edx,ecx
and edx,00001111b      ; ECX = Extract tail length
jz Measure_Read_MOV32
;---------- Tail cycle --------------------------------------------------------;
Tail_Read_MOV32:
mov eax,[ebx+04*00]
add ebx,4              ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Read_MOV32    ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Read_MOV32:
sub ebp,1
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_Read_MOV32   ; Cycle for measurement, repeat same operation
pop ebx
ret
