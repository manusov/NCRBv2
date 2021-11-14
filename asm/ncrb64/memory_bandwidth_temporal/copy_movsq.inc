;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Copy x86-64 strings (REP MOVSQ) ", mode = temporal. ;
;                                                                              ;
; INPUT:   RSI = Block #1 pointer (64-bit flat)                                ;
;          RDI = Block #2 pointer (64-bit flat)                                ;
;          R8  = Block #3 pointer (64-bit flat) , reserved and not used yet    ;
;                For Read, Write, Modify use RSI as Source or Destination      ;
;                For Copy use RSI = Source , RDI = Destination                 ;
;          RCX = Block length, units = instructions                            ;
;          RBP = Number of measurement repeats                                 ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;          Registers corrupted, but must save R14, R15                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
Pattern_Copy_MOVSQ:
mov rbx,rsi                 ; RBX = Save source block base = Input RSI
mov rdx,rdi                 ; RDX = Save destination block base = Input RDI
mov r8,rcx                  ; R8 = Save block size = Input RCX
Measurement_Copy_MOVSQ:
mov rsi,rbx                 ; RSI = Restore source block base , restore - because measurement cycle
mov rdi,rdx                 ; RDI = Restore destination block base
mov rcx,r8                  ; RCX = Restore block size , units = hardware cycle iterations
DumpStart_Copy_MOVSQ:
rep movsq                   ; This is target operation - hardware cycle
DumpStop_Copy_MOVSQ:
dec rbp                     ; RBP = Measurement cycle counter
jnz Measurement_Copy_MOVSQ  ; Measurement cycle
ret
