;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Read MMX-64 (MOVQ) " , mode = temporal.             ;
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
Pattern_Read_MMX64:
Measurement_Read_MMX64:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi          ; RAX = Reload source address
mov rdx,rcx          ; RDX = Reload length
shr rdx,4            ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Read_MMX64  ; Go if Length < 16 instructions
mov rbx,128          ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Read_MMX64:
Block_Read_MMX64:
movq mm0,[rax+08*00]
movq mm1,[rax+08*01]
movq mm2,[rax+08*02]
movq mm3,[rax+08*03]
movq mm4,[rax+08*04]
movq mm5,[rax+08*05]
movq mm6,[rax+08*06]
movq mm7,[rax+08*07]
movq mm0,[rax+08*08]
movq mm1,[rax+08*09]
movq mm2,[rax+08*10]
movq mm3,[rax+08*11]
movq mm4,[rax+08*12]
movq mm5,[rax+08*13]
movq mm6,[rax+08*14]
movq mm7,[rax+08*15]
add rax,rbx            ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Read_MMX64   ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Read_MMX64:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Read_MMX64:
mov edx,ecx
and edx,00001111b      ; ECX = Extract tail length
jz Measure_Read_MMX64
mov ebx,8
;---------- Tail cycle --------------------------------------------------------;
Tail_Read_MMX64:
movq mm0,[rax+08*00]
add rax,rbx            ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Read_MMX64    ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Read_MMX64:
dec rbp
jnz Measurement_Read_MMX64  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
emms                        ; Required empty MMX state before return
ret
