;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Read x86-64 (MOV) " , mode = temporal.              ;
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
Pattern_Read_MOV64:
Measurement_Read_MOV64:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi          ; RAX = Reload source address
mov rdx,rcx          ; RDX = Reload length
shr rdx,4            ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Read_MOV64  ; Go if Length < 16 instructions
mov rbx,128          ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Read_MOV64:
Block_Read_MOV64:
mov r8, [rax+08*00]
mov r9, [rax+08*01]
mov r10,[rax+08*02]
mov r11,[rax+08*03]
mov r12,[rax+08*04]
mov r13,[rax+08*05]
mov r8, [rax+08*06]
mov r9, [rax+08*07]
mov r10,[rax+08*08]
mov r11,[rax+08*09]
mov r12,[rax+08*10]
mov r13,[rax+08*11]
mov r8, [rax+08*12]
mov r9, [rax+08*13]
mov r10,[rax+08*14]
mov r11,[rax+08*15]
add rax,rbx           ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Read_MOV64  ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Read_MOV64:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Read_MOV64:
mov edx,ecx
and edx,00001111b     ; ECX = Extract tail length
jz Measure_Read_MOV64
mov ebx,8
;---------- Tail cycle --------------------------------------------------------;
Tail_Read_MOV64:
mov r8,[rax+08*00]
add rax,rbx           ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Read_MOV64   ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Read_MOV64:
dec rbp
jnz Measurement_Read_MOV64   ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
