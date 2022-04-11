;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Cache optimized write (CLZERO) " ,                  ;
; mode = non temporal only, because CLZERO instruction properties.             ;
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

;
; TODO.
; THIS PROCEDURE UNDER VERIFICATION. YET LOCKED IN THE NCRB GUI MENU.
;

Pattern_NtWrite_CLZERO:
Measurement_NtWrite_CLZERO:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi              ; RAX = Reload source address
mov ebx,64               ; RBX = Addend, this operation also clear bits RBX[63-32]
mov rdx,rcx              ; RDX = Reload length
shr rdx,3                ; RDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_NtWrite_CLZERO  ; Go if Length < 8 instructions
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtWrite_CLZERO:
Block_NtWrite_CLZERO:
clzero rax
add rax,rbx
clzero rax
add rax,rbx
clzero rax
add rax,rbx
clzero rax
add rax,rbx
clzero rax
add rax,rbx
clzero rax
add rax,rbx
clzero rax
add rax,rbx
clzero rax
add rax,rbx                ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_NtWrite_CLZERO   ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtWrite_CLZERO:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtWrite_CLZERO:
mov edx,ecx
and edx,00000111b          ; ECX = Extract tail length
jz Measure_NtWrite_CLZERO
;---------- Tail cycle --------------------------------------------------------;
Tail_NtWrite_CLZERO:
clzero rax
add rax,rbx                  ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_NtWrite_CLZERO      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtWrite_CLZERO:
dec rbp
jnz Measurement_NtWrite_CLZERO  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
