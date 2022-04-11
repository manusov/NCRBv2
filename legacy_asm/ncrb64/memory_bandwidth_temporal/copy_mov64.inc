;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Copy x86-64 (MOV) " , mode = temporal.              ;
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
Pattern_Copy_MOV64:
Measurement_Copy_MOV64:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi          ; RAX = Reload source address
mov rbx,rdi          ; RBX = Reload destination address
mov rdx,rcx          ; RDX = Reload length
shr rdx,4            ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Copy_MOV64  ; Go if Length < 16 instructions
mov r8d,128          ; R8 = Addend, this operation also clear bits R8[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Copy_MOV64:
Block_Copy_MOV64:
mov r9,[rax+08*00]
mov [rbx+08*00],r9
mov r10,[rax+08*01]
mov [rbx+08*01],r10
mov r11,[rax+08*02]
mov [rbx+08*02],r11
mov r12,[rax+08*03]
mov [rbx+08*03],r12
mov r13,[rax+08*04]
mov [rbx+08*04],r13
mov r9,[rax+08*05]
mov [rbx+08*05],r9
mov r10,[rax+08*06]
mov [rbx+08*06],r10
mov r11,[rax+08*07]
mov [rbx+08*07],r11
mov r12,[rax+08*08]
mov [rbx+08*08],r12
mov r13,[rax+08*09]
mov [rbx+08*09],r13
mov r9,[rax+08*10]
mov [rbx+08*10],r9
mov r10,[rax+08*11]
mov [rbx+08*11],r10
mov r11,[rax+08*12]
mov [rbx+08*12],r11
mov r12,[rax+08*13]
mov [rbx+08*13],r12
mov r13,[rax+08*14]
mov [rbx+08*14],r13
mov r9,[rax+08*15]
mov [rbx+08*15],r9
add rax,r8           ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8           ; Modify destination address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Copy_MOV64   ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Copy_MOV64:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Copy_MOV64:
mov edx,ecx
and edx,00001111b      ; ECX = Extract tail length
jz Measure_Copy_MOV64
mov r8d,8
;---------- Tail cycle --------------------------------------------------------;
Tail_Copy_MOV64:
mov r9,[rax+08*00] 
mov [rbx+08*00],r9  
add rax,r8             ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8             ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Copy_MOV64    ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Copy_MOV64:
dec rbp
jnz Measurement_Copy_MOV64   ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
