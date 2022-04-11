;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Gather read AVX-256 (VGATHERQPD) " ,                ;
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

Pattern_Gather_AVX256:
;---------- Prepare constants, non volatile for measurement cycle -------------;
; Benchmark parameters note.
; One element width = QWORD = 8 bytes = 64 bits
; Elements per one 256-bit YMM register = 256 / 64 = 4 
; Number of fragments = 4, fragment size = block size / 4
; One gathered read = 4 elements,
; Reads per one YMM register = 4 / 4 = 1  
mov eax,8               ; RAX  = Vertical addend for address
vmovq xmm0,rax
vbroadcastsd ymm0,xmm0  ; YMM0 = 8, 8, 8, 8
mov rax,rcx             ; RAX = Block size in instructions, unit = 32 bytes
shl rax,3               ; RAX = Horizontal addend = size / 4 = N/32*8 = N/4
xor edx,edx
vpinsrq xmm1,xmm1,rdx,0
vpinsrq xmm1,xmm1,rax,1
lea rdx,[rax * 2]
vpinsrq xmm2,xmm2,rdx,0
add rdx,rax
vpinsrq xmm2,xmm2,rdx,1
vinsertf128 ymm1,ymm1,xmm2,1  ; YMM1 = indexes vector #1
vpaddq ymm2,ymm1,ymm0         ; YMM2 = indexes vector #2
vpaddq ymm3,ymm2,ymm0         ; YMM3 = indexes vector #3
vpaddq ymm4,ymm3,ymm0         ; YMM4 = indexes vector #4
vpcmpeqd ymm0,ymm0,ymm0       ; YMM0 = mask, all "1"
;---------- Measurement cycle, prepare big cycle ------------------------------;
Measurement_Gather_AVX256:
mov rax,rsi             ; RAX = Reload source address
mov rdx,rcx             ; RDX = Reload length
shr rdx,3               ; RDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_Gather_AVX256  ; Go if Length < 8 instructions
mov ebx,32              ; RBX = Vertical addend * 4
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Gather_AVX256:
Block_Gather_AVX256:
vmovapd ymm5,ymm0       ; Mask must be reloaded because gather instruction clear it
vmovapd ymm6,ymm0
vgatherqpd ymm8,[rax + ymm1],ymm5  
vgatherqpd ymm9,[rax + ymm2],ymm6
vmovapd ymm5,ymm0
vmovapd ymm6,ymm0
vgatherqpd ymm10,[rax + ymm3],ymm5
vgatherqpd ymm11,[rax + ymm4],ymm6
add rax,rbx             ; Modify address, addend=register (not constant) for optimization!
vmovapd ymm5,ymm0
vmovapd ymm6,ymm0
vgatherqpd ymm12,[rax + ymm1],ymm5  
vgatherqpd ymm13,[rax + ymm2],ymm6
vmovapd ymm5,ymm0
vmovapd ymm6,ymm0
vgatherqpd ymm14,[rax + ymm3],ymm5
vgatherqpd ymm15,[rax + ymm4],ymm6
add rax,rbx             ; Modify address, addend=register (not constant) for optimization!
dec rdx
jnz Block_Gather_AVX256 ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Gather_AVX256:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Gather_AVX256:
mov edx,ecx
and edx,00000111b           ; EDX = Extract tail length
jz Measure_Gather_AVX256
mov ebx,8                   ; RBX = Vertical addend
;---------- Tail cycle --------------------------------------------------------;
Tail_Gather_AVX256:
vmovapd ymm5,ymm0
vgatherqpd ymm8,[rax + ymm1],ymm5
add rax,rbx                 ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Gather_AVX256      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Gather_AVX256:
dec rbp
jnz Measurement_Gather_AVX256  ; Cycle for measurement, repeat same operation, DEC/JNZ is faster than LOOP!
ret
