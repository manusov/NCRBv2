;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Gather read AVX-512 (VGATHERQPD) " ,                ;
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

Pattern_Gather_AVX512:
;---------- Prepare constants, non volatile for measurement cycle -------------;
; Benchmark parameters note.
; One element width = QWORD = 8 bytes = 64 bits
; Elements per one 512-bit ZMM register = 512 / 64 = 8 
; Number of fragments = 4, fragment size = block size / 4
; One gathered read = 4 elements,
; Reads per one ZMM register = 8 / 4 = 2  
mov eax,8                      ; RAX  = Addend for address, one vertical group
vmovq xmm2,rax                 ; XMM2 = Addend for address, one vertical group  
vpaddq xmm1,xmm2,xmm2          ; XMM1 = Addend for address, two vertical groups 
vbroadcastsd zmm1,xmm1         ; ZMM1 = 16, 16, 16, 16, 16, 16, 16, 16
vbroadcastsd ymm2,xmm2         ; YMM2 = 8, 8, 8, 8
vxorpd zmm0,zmm0,zmm0          ; ZMM0 = 0, 0, 0, 0, 0, 0, 0, 0 
vinsertf64x4 zmm0,zmm0,ymm2,1  ; ZMM0 = 8, 8, 8, 8, 0, 0, 0, 0
mov rax,rcx                    ; RAX = Block size in instructions, unit = 64 bytes
shl rax,4                      ; RAX = Horizontal addend = size / 4 = N/64*16 = N/4
xor edx,edx
vpinsrq xmm2,xmm2,rdx,0
vpinsrq xmm2,xmm2,rax,1
lea rdx,[rax * 2]
vpinsrq xmm3,xmm3,rdx,0
add rdx,rax
vpinsrq xmm3,xmm3,rdx,1
vinsertf128 ymm2,ymm2,xmm3,1   ; YMM2 = indexes vector  #1
vinsertf64x4 zmm2,zmm2,ymm2,1
vpaddq zmm2,zmm2,zmm0          ; ZMM2 = indexes vectors #1  , #2
vpaddq zmm3,zmm2,zmm1          ; ZMM3 = indexes vectors #3  , #4
vpaddq zmm4,zmm3,zmm1          ; ZMM4 = indexes vectors #5  , #6
vpaddq zmm5,zmm4,zmm1          ; ZMM5 = indexes vectors #7  , #8
vpaddq zmm6,zmm5,zmm1          ; ZMM6 = indexes vectors #9  , #10
vpaddq zmm7,zmm6,zmm1          ; ZMM7 = indexes vectors #11 , #12
vpaddq zmm8,zmm7,zmm1          ; ZMM8 = indexes vectors #13 , #14
vpaddq zmm9,zmm8,zmm1          ; ZMM9 = indexes vectors #15 , #16
mov eax,0FFFFh
kmovw k0,eax                   ; K0 = storage for predicate mask
;---------- Measurement cycle, prepare big cycle ------------------------------;
Measurement_Gather_AVX512:
mov rax,rsi             ; RAX = Reload source address
mov rdx,rcx             ; RDX = Reload length
shr rdx,3               ; RDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_Gather_AVX512  ; Go if Length < 8 instructions
mov ebx,128             ; RBX = Vertical addend * 16
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Gather_AVX512:
Block_Gather_AVX512:
kmovw k1,k0             ; Mask must be reloaded because gather instruction clear it
kmovw k2,k0
kmovw k3,k0
kmovw k4,k0
vgatherqpd zmm16{k1},[rax + zmm2]  
vgatherqpd zmm17{k2},[rax + zmm3]
vgatherqpd zmm18{k3},[rax + zmm4]  
vgatherqpd zmm19{k4},[rax + zmm5]
kmovw k1,k0
kmovw k2,k0
kmovw k3,k0
kmovw k4,k0
vgatherqpd zmm20{k1},[rax + zmm6]  
vgatherqpd zmm21{k2},[rax + zmm7]
vgatherqpd zmm22{k3},[rax + zmm8]  
vgatherqpd zmm23{k4},[rax + zmm9]
add rax,rbx             ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Gather_AVX512 ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Gather_AVX512:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Gather_AVX512:
mov edx,ecx
and edx,00000111b           ; EDX = Extract tail length
jz Measure_Gather_AVX512
mov ebx,16                  ; RBX = Vertical addend * 2 ( 2 quartets per ZMM)
;---------- Tail cycle --------------------------------------------------------;
Tail_Gather_AVX512:
kmovw k1,k0
vgatherqpd zmm16{k1},[rax + zmm2]
add rax,rbx                 ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Gather_AVX512      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Gather_AVX512:
dec rbp
jnz Measurement_Gather_AVX512  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
