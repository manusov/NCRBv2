;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Scatter write AVX-512 (VSCATTERQPD) " ,             ;
; mode = temporal.                                                             ;
;                                                                              ;
; INPUT:   ESI = Block #1 pointer (32-bit flat)                                ;
;          EDI = Block #2 pointer (32-bit flat)                                ;
;                For Read, Write, Modify use ESI as Source or Destination      ;
;                For Copy use ESI = Source , EDI = Destination                 ;
;          ECX = Block length, units = instructions                            ;
;          EBX:EBP = Number of measurement repeats, EBX=High32, EBP=Low32      ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;          All registers corrupted                                             ;
;                                                                              ;
;------------------------------------------------------------------------------;

;
; TODO.
; THIS PROCEDURE UNDER VERIFICATION. YET LOCKED IN THE NCRB GUI MENU.
;

Pattern_Scatter_AVX512:
push ebx                ; EBX = High 32 bits of measurement repeats count
;---------- Prepare constants, non volatile for measurement cycle -------------;
; Benchmark parameters note.
; One element width = QWORD = 8 bytes = 64 bits
; Elements per one 512-bit ZMM register = 512 / 64 = 8 
; Number of fragments = 4, fragment size = block size / 4
; One scattered write = 4 elements,
; Writes per one ZMM register = 8 / 4 = 2  
vzeroall                       ; Clear ZMM0-ZMM7 (ia32 mode, 8 registers)
mov eax,8                      ; EAX  = Addend for address, one vertical group
vmovd xmm2,eax                 ; XMM2 = Addend for address, one vertical group  
vpaddq xmm1,xmm2,xmm2          ; XMM1 = Addend for address, two vertical groups 
vbroadcastsd zmm1,xmm1         ; ZMM1 = 16, 16, 16, 16, 16, 16, 16, 16
vbroadcastsd ymm2,xmm2         ; YMM2 = 8, 8, 8, 8
vxorpd zmm0,zmm0,zmm0          ; ZMM0 = 0, 0, 0, 0, 0, 0, 0, 0 
vinsertf64x4 zmm0,zmm0,ymm2,1  ; ZMM0 = 8, 8, 8, 8, 0, 0, 0, 0
mov eax,ecx                    ; EAX = Block size in instructions, unit = 64 bytes
shl eax,4                      ; EAX = Horizontal addend = size / 4 = N/64*16 = N/4
xor edx,edx
vpinsrd xmm2,xmm2,edx,0        ; Address bits [63-32] not changed by this,
vpinsrd xmm2,xmm2,eax,2        ; this bits can be not valid, but clear after VZEROALL
lea edx,[eax * 2]
vpinsrd xmm3,xmm3,edx,0
add edx,eax
vpinsrd xmm3,xmm3,edx,2
vinsertf128 ymm2,ymm2,xmm3,1   ; YMM2 = indexes vector  #1
vinsertf64x4 zmm2,zmm2,ymm2,1
vpaddq zmm2,zmm2,zmm0          ; ZMM2 = indexes vectors #1  , #2
vpaddq zmm3,zmm2,zmm1          ; ZMM3 = indexes vectors #3  , #4
vpaddq zmm4,zmm3,zmm1          ; ZMM4 = indexes vectors #5  , #6
vpaddq zmm5,zmm4,zmm1          ; ZMM4 = indexes vectors #7  , #8
mov eax,0FFFFh
kmovw k0,eax                   ; K0 = storage for predicate mask
;---------- Measurement cycle, prepare big cycle ------------------------------;
Measurement_Scatter_AVX512:
mov eax,esi             ; EAX = Reload source address
mov edx,ecx             ; EDX = Reload length
shr edx,3               ; EDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_Scatter_AVX512 ; Go if Length < 8 instructions
mov ebx,64              ; EBX = Vertical addend * 8
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Scatter_AVX512:
Block_Scatter_AVX512:
kmovw k1,k0             ; Mask must be reloaded because gather instruction clear it
kmovw k2,k0
kmovw k3,k0
kmovw k4,k0
vscatterqpd [eax + zmm2]{k1},zmm6  
vscatterqpd [eax + zmm3]{k2},zmm7
vscatterqpd [eax + zmm4]{k3},zmm6  
vscatterqpd [eax + zmm5]{k4},zmm7
add eax,ebx             ; Modify address, addend=register (not constant) for optimization!
kmovw k1,k0             ; Mask must be reloaded because gather instruction clear it
kmovw k2,k0
kmovw k3,k0
kmovw k4,k0
vscatterqpd [eax + zmm2]{k1},zmm6  
vscatterqpd [eax + zmm3]{k2},zmm7
vscatterqpd [eax + zmm4]{k3},zmm6  
vscatterqpd [eax + zmm5]{k4},zmm7
add eax,ebx             ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_Scatter_AVX512 ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Scatter_AVX512:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Scatter_AVX512:
mov edx,ecx
and edx,00000111b           ; EDX = Extract tail length
jz Measure_Scatter_AVX512
mov ebx,16                  ; EBX = Vertical addend * 2
;---------- Tail cycle --------------------------------------------------------;
Tail_Scatter_AVX512:
kmovw k1,k0
vgatherqpd zmm6{k1},[eax + zmm2]
add eax,ebx                 ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Scatter_AVX512     ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Scatter_AVX512:
sub ebp,1                      ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_Scatter_AVX512  ; Cycle for measurement, repeat same operation
pop ebx
ret
