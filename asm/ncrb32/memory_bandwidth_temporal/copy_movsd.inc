;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Copy x86 strings (REP MOVSD) " , mode = temporal.   ;
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
Pattern_Copy_MOVSD:
push ebx                ; EBX = High 32 bits of measurement repeats count
mov ebx,esi             ; EBX = Save source block base = Input ESI
mov edx,edi             ; EDX = Save destination block base = Input EDI
mov eax,ecx             ; EAX = Save block size = Input ECX
Measurement_Copy_MOVSD:
mov esi,ebx             ; ESI = Restore source block base , restore - because measurement cycle
mov edi,edx             ; EDI = Restore destination block base
mov ecx,eax             ; ECX = Restore block size , units = hardware cycle iterations
DumpStart_Copy_MOVSD:
rep movsd               ; This is target operation - hardware cycle
DumpStop_Copy_MOVSD:
;---------- Measurement cycle -------------------------------------------------;
sub ebp,1                    ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov esi,ebp
or esi,[esp]
jnz Measurement_Copy_MOVSD   ; Measurement cycle
pop ebx
ret
