;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Cache optimized write (CLZERO) " ,                  ;
; mode = non temporal only, because CLZERO instruction properties.             ;
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

;
; TODO.
; THIS PROCEDURE UNDER VERIFICATION. YET LOCKED IN THE NCRB GUI MENU.
;

Pattern_NtWrite_CLZERO:
push ebx                ; EBX = High 32 bits of measurement repeats count
Measurement_NtWrite_CLZERO:
;---------- Prepare big cycle -------------------------------------------------;
mov eax,esi              ; EAX = Reload source address
mov esi,64               ; ESI = Addend
mov edx,ecx              ; EDX = Reload length
shr edx,3                ; EDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_NtWrite_CLZERO  ; Go if Length < 8 instructions
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtWrite_CLZERO:
Block_NtWrite_CLZERO:
clzero eax
add eax,esi
clzero eax
add eax,esi
clzero eax
add eax,esi
clzero eax
add eax,esi
clzero eax
add eax,esi
clzero eax
add eax,esi
clzero eax
add eax,esi
clzero eax
add eax,esi                ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_NtWrite_CLZERO   ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtWrite_CLZERO:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtWrite_CLZERO:
mov edx,ecx
and edx,00000111b          ; EDX = Extract tail length
jz Measure_NtWrite_CLZERO
;---------- Tail cycle --------------------------------------------------------;
Tail_NtWrite_CLZERO:
clzero eax
add eax,esi                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtWrite_CLZERO      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtWrite_CLZERO:
sub ebp,1                       ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_NtWrite_CLZERO  ; Cycle for measurement, repeat same operation
pop ebx
ret
