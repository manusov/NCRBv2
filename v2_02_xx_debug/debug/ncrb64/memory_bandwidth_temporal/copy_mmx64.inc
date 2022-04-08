;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Copy MMX-64 (MOVQ) " , mode = temporal.             ;
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
Pattern_Copy_MMX64:
Measurement_Copy_MMX64:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi          ; RAX = Reload source address
mov rbx,rdi          ; RBX = Reload destination address
mov rdx,rcx          ; RDX = Reload length
shr rdx,4            ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Copy_MMX64  ; Go if Length < 16 instructions
mov r8d,128          ; R8 = Addend, this operation also clear bits R8[63-32]
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Copy_MMX64:
Block_Copy_MMX64:
movq mm0,[rax+08*00]
movq [rbx+08*00],mm0
movq mm1,[rax+08*01]
movq [rbx+08*01],mm1
movq mm2,[rax+08*02]
movq [rbx+08*02],mm2
movq mm3,[rax+08*03]
movq [rbx+08*03],mm3
movq mm4,[rax+08*04]
movq [rbx+08*04],mm4
movq mm5,[rax+08*05]
movq [rbx+08*05],mm5
movq mm6,[rax+08*06]
movq [rbx+08*06],mm6
movq mm7,[rax+08*07]
movq [rbx+08*07],mm7
movq mm0,[rax+08*08]
movq [rbx+08*08],mm0
movq mm1,[rax+08*09]
movq [rbx+08*09],mm1
movq mm2,[rax+08*10]
movq [rbx+08*10],mm2
movq mm3,[rax+08*11]
movq [rbx+08*11],mm3
movq mm4,[rax+08*12]
movq [rbx+08*12],mm4
movq mm5,[rax+08*13]
movq [rbx+08*13],mm5
movq mm6,[rax+08*14]
movq [rbx+08*14],mm6
movq mm7,[rax+08*15]
movq [rbx+08*15],mm7
add rax,r8              ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8              ; Modify destination address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Copy_MMX64    ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Copy_MMX64:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Copy_MMX64:
mov edx,ecx
and edx,00001111b       ; ECX = Extract tail length
jz Measure_Copy_MMX64
mov r8d,8
;---------- Tail cycle --------------------------------------------------------;
Tail_Copy_MMX64:
movq mm0,[rax+08*00] 
movq [rbx+08*00],mm0  
add rax,r8              ; Modify source address, addend=register (not constant) for optimization!
add rbx,r8              ; Modify destination address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Copy_MMX64     ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Copy_MMX64:
dec rbp
jnz Measurement_Copy_MMX64   ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
emms                         ; Required empty MMX state before return
ret
