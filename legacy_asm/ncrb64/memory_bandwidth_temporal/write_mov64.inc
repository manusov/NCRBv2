;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Write x86-64 (MOV) " , mode = temporal.             ;
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
Pattern_Write_MOV64:
xor r8,r8             ; Clear R8-R13 for write data
xor r9,r9
xor r10,r10
xor r11,r11
xor r12,r12
xor r13,r13 
Measurement_Write_MOV64:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi           ; RAX = Reload source address
mov rdx,rcx           ; RDX = Reload length
shr rdx,4             ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Write_MOV64  ; Go if Length < 16 instructions
mov rbx,128           ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Write_MOV64:
Block_Write_MOV64:
mov [rax+08*00],r8
mov [rax+08*01],r9
mov [rax+08*02],r10
mov [rax+08*03],r11
mov [rax+08*04],r12
mov [rax+08*05],r13
mov [rax+08*06],r8
mov [rax+08*07],r9
mov [rax+08*08],r10
mov [rax+08*09],r11
mov [rax+08*10],r12
mov [rax+08*11],r13
mov [rax+08*12],r8
mov [rax+08*13],r9
mov [rax+08*14],r10
mov [rax+08*15],r11
add rax,rbx            ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Write_MOV64  ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Write_MOV64:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Write_MOV64:
mov edx,ecx
and edx,00001111b      ; ECX = Extract tail length
jz Measure_Write_MOV64
mov ebx,8
;---------- Tail cycle --------------------------------------------------------;
Tail_Write_MOV64:
mov [rax+08*00],r8
add rax,rbx            ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Write_MOV64   ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Write_MOV64:
dec rbp
jnz Measurement_Write_MOV64   ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
