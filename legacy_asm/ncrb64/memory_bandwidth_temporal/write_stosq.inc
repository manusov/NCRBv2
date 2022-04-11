;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Write x86-64 strings (REP STOSQ) " ,                ;
; mode = temporal.                                                             ;
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
Pattern_Write_STOSQ:
xor eax,eax                  ; Clear RAX for write data (RAX used by REP STOSQ)
mov rdx,rcx                  ; RDX = Save block size , received RSI = Destination block base
Measurement_Write_STOSQ:
mov rdi,rsi                  ; RDI = Restore destination block base , restore - because measurement cycle
mov rcx,rdx                  ; RCX = Restore block size , units = hardware cycle iterations
DumpStart_Write_STOSQ:
rep stosq                    ; This is target operation - hardware cycle
DumpStop_Write_STOSQ:
dec rbp                      ; RBP = Measurement cycle counter
jnz Measurement_Write_STOSQ  ; Measurement cycle
ret

