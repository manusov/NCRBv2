;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Write x86 strings (REP STOSD) " , mode = temporal.  ;
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
Pattern_Write_STOSD:
mov edx,ecx                  ; EDX = Save block size , received ESI = Destination block base
xor eax,eax                  ; EAX = Write data = 0
Measurement_Write_STOSD:
mov edi,esi                  ; EDI = Restore destination block base , restore - because measurement cycle
mov ecx,edx                  ; ECX = Restore block size , units = hardware cycle iterations
DumpStart_Write_STOSD:
rep stosd                    ; This is target operation - hardware cycle
DumpStop_Write_STOSD:
;---------- Measurement cycle -------------------------------------------------;
sub ebp,1                    ; EBX:EBP = Measurement cycle counter
sbb ebx,0
mov ecx,ebp                  ; ECX = Temporary disjunction result, must be re-loaded for each repeat
or ecx,ebx
jnz Measurement_Write_STOSD  ; Measurement cycle
ret
