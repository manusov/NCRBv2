;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Modify x86-64 (NOT) " , mode = temporal.            ;
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
Pattern_Modify_NOT64:
Measurement_Modify_NOT64:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi            ; RAX = Reload source address
mov rdx,rcx            ; RDX = Reload length
shr rdx,4              ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Modify_NOT64  ; Go if Length < 16 instructions
mov rbx,128            ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Modify_NOT64:
Block_Modify_NOT64:
not qword [rax+08*00]
not qword [rax+08*01]
not qword [rax+08*02]
not qword [rax+08*03]
not qword [rax+08*04]
not qword [rax+08*05]
not qword [rax+08*06]
not qword [rax+08*07]
not qword [rax+08*08]
not qword [rax+08*09]
not qword [rax+08*10]
not qword [rax+08*11]
not qword [rax+08*12]
not qword [rax+08*13]
not qword [rax+08*14]
not qword [rax+08*15]
add rax,rbx             ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Modify_NOT64  ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Modify_NOT64:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Modify_NOT64:
mov edx,ecx
and edx,00001111b        ; ECX = Extract tail length
jz Measure_Modify_NOT64
mov ebx,8
;---------- Tail cycle --------------------------------------------------------;
Tail_Modify_NOT64:
not qword [rax+08*00]
add rax,rbx              ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Modify_NOT64    ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Modify_NOT64:
dec rbp
jnz Measurement_Modify_NOT64  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
