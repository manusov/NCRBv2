;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Write MMX-64 (MOVQ) " , mode = temporal.            ;
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
Pattern_Write_MMX64:
pxor mm0,mm0         ; Clear MM0-MM7 (MMX registers) for write data
pxor mm1,mm1
pxor mm2,mm2
pxor mm3,mm3
pxor mm4,mm4
pxor mm5,mm5
pxor mm6,mm6
pxor mm7,mm7
Measurement_Write_MMX64:
;---------- Prepare big cycle -------------------------------------------------;
mov rax,rsi           ; RAX = Reload source address
mov rdx,rcx           ; RDX = Reload length
shr rdx,4             ; RDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_Write_MMX64  ; Go if Length < 16 instructions
mov rbx,128           ; RBX = Addend, this operation also clear bits RBX[63-32]
;---------- Big cycle ---------------------------------------------------------'
DumpStart_Write_MMX64:
Block_Write_MMX64:
movq [rax+08*00],mm0
movq [rax+08*01],mm1
movq [rax+08*02],mm2
movq [rax+08*03],mm3
movq [rax+08*04],mm4
movq [rax+08*05],mm5
movq [rax+08*06],mm6
movq [rax+08*07],mm7
movq [rax+08*08],mm0
movq [rax+08*09],mm1
movq [rax+08*10],mm2
movq [rax+08*11],mm3
movq [rax+08*12],mm4
movq [rax+08*13],mm5
movq [rax+08*14],mm6
movq [rax+08*15],mm7
add rax,rbx                ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Write_MMX64      ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Write_MMX64:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Write_MMX64:
mov edx,ecx
and edx,00001111b          ; ECX = Extract tail length
jz Measure_Write_MMX64
mov ebx,8
;---------- Tail cycle --------------------------------------------------------;
Tail_Write_MMX64:
movq [rax+08*00],mm0
add rax,rbx                ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Write_MMX64       ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Write_MMX64:
dec rbp
jnz Measurement_Write_MMX64  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
emms                         ; Required empty MMX state before return
ret
