;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Scatter write AVX-512 (VSCATTERQPD) " ,             ;
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

;
; TODO.
; THIS PROCEDURE UNDER VERIFICATION. YET LOCKED IN THE NCRB GUI MENU.
;

Pattern_Scatter_AVX512:
;---------- Prepare constants, non volatile for measurement cycle -------------;
; Benchmark parameters note.
; One element width = QWORD = 8 bytes = 64 bits
; Elements per one 512-bit ZMM register = 512 / 64 = 8 
; Number of fragments = 4, fragment size = block size / 4
; One scattered write = 4 elements,
; Writes per one ZMM register = 8 / 4 = 2  
vzeroall                         ; Clear ZMM0-ZMM15, not changes ZMM16-ZMM31
mov eax,8                        ; RAX  = Addend for address, one vertical group
vmovq xmm18,rax                  ; XMM18 = Addend for address, one vertical group  
vpaddq xmm17,xmm18,xmm18         ; XMM17 = Addend for address, two vertical groups 
vbroadcastsd zmm17,xmm17         ; ZMM17 = 16, 16, 16, 16, 16, 16, 16, 16
vbroadcastsd ymm18,xmm18         ; YMM18 = 8, 8, 8, 8
vxorpd zmm16,zmm16,zmm16         ; ZMM16 = 0, 0, 0, 0, 0, 0, 0, 0 
vinsertf64x4 zmm16,zmm16,ymm18,1 ; ZMM16 = 8, 8, 8, 8, 0, 0, 0, 0
mov rax,rcx                      ; RAX = Block size in instructions, unit = 64 bytes
shl rax,4                        ; RAX = Horizontal addend = size / 4 = N/64*16 = N/4
xor edx,edx
vpinsrq xmm14,xmm14,rdx,0
vpinsrq xmm14,xmm14,rax,1
lea rdx,[rax * 2]
vpinsrq xmm15,xmm15,rdx,0
add rdx,rax
vpinsrq xmm15,xmm15,rdx,1
vinsertf128 ymm14,ymm14,xmm15,1  ; YMM14 = indexes vector  #1
vinsertf64x4 zmm18,zmm14,ymm14,1
vpaddq zmm18,zmm18,zmm16         ; ZMM18 = indexes vectors #1  , #2
vpaddq zmm19,zmm18,zmm17         ; ZMM19 = indexes vectors #3  , #4
vpaddq zmm20,zmm19,zmm17         ; ZMM20 = indexes vectors #5  , #6
vpaddq zmm21,zmm20,zmm17         ; ZMM21 = indexes vectors #7  , #8
vpaddq zmm22,zmm21,zmm17         ; ZMM22 = indexes vectors #9  , #10
vpaddq zmm23,zmm22,zmm17         ; ZMM23 = indexes vectors #11 , #12
vpaddq zmm24,zmm23,zmm17         ; ZMM24 = indexes vectors #13 , #14
vpaddq zmm25,zmm24,zmm17         ; ZMM25 = indexes vectors #15 , #16
mov eax,0FFFFh
kmovw k0,eax                     ; K0 = storage for predicate mask
;---------- Measurement cycle, prepare big cycle ------------------------------;
Measurement_Scatter_AVX512:
mov rax,rsi             ; RAX = Reload source address
mov rdx,rcx             ; RDX = Reload length
shr rdx,3               ; RDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_Scatter_AVX512 ; Go if Length < 8 instructions
mov ebx,128             ; RBX = Vertical addend * 16
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Scatter_AVX512:
Block_Scatter_AVX512:
kmovw k1,k0             ; Mask must be reloaded because gather instruction clear it
kmovw k2,k0
kmovw k3,k0
kmovw k4,k0
vscatterqpd [rax + zmm18]{k1},zmm0  
vscatterqpd [rax + zmm19]{k2},zmm1
vscatterqpd [rax + zmm20]{k3},zmm2  
vscatterqpd [rax + zmm21]{k4},zmm3
kmovw k1,k0
kmovw k2,k0
kmovw k3,k0
kmovw k4,k0
vscatterqpd [rax + zmm22]{k1},zmm4  
vscatterqpd [rax + zmm23]{k2},zmm5
vscatterqpd [rax + zmm24]{k3},zmm6  
vscatterqpd [rax + zmm25]{k4},zmm7
add rax,rbx              ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Scatter_AVX512 ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Scatter_AVX512:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Scatter_AVX512:
mov edx,ecx
and edx,00000111b           ; EDX = Extract tail length
jz Measure_Scatter_AVX512
mov ebx,16                  ; RBX = Vertical addend * 2 ( 2 quartets per ZMM)
;---------- Tail cycle --------------------------------------------------------;
Tail_Scatter_AVX512:
kmovw k1,k0
vscatterqpd [rax + zmm18]{k1},zmm0
add rax,rbx                 ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Scatter_AVX512     ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Scatter_AVX512:
dec rbp
jnz Measurement_Scatter_AVX512  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
