;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Gather read AVX-256 (VGATHERQPD) " ,                ;
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

Pattern_Gather_AVX256:
push ebx                ; EBX = High 32 bits of measurement repeats count
;---------- Prepare constants, non volatile for measurement cycle -------------;
; Benchmark parameters note.
; One element width = QWORD = 8 bytes = 64 bits
; Elements per one 256-bit YMM register = 256 / 64 = 4 
; Number of fragments = 4, fragment size = block size / 4
; One gathered read = 4 elements,
; Reads per one YMM register = 4 / 4 = 1  
vzeroall                       ; Clear ZMM0-ZMM7 (ia32 mode, 8 registers)
mov eax,8                      ; EAX  = Vertical addend for address
vmovd xmm0,eax
vbroadcastsd ymm0,xmm0    ; YMM0 = 8, 8, 8, 8
mov eax,ecx               ; EAX = Block size in instructions, unit = 32 bytes
shl eax,3                 ; EAX = Horizontal addend = size / 4 = N/32*8 = N/4
xor edx,edx
vpinsrd xmm1,xmm1,edx,0   ; Address bits [63-32] not changed by this,
vpinsrd xmm1,xmm1,eax,2   ; this bits can be not valid, but clear after VZEROALL
lea edx,[eax * 2]
vpinsrd xmm2,xmm2,edx,0
add edx,eax
vpinsrd xmm2,xmm2,edx,2
vinsertf128 ymm1,ymm1,xmm2,1  ; YMM1 = indexes vector #1
vpaddq ymm2,ymm1,ymm0         ; YMM2 = indexes vector #2
vpaddq ymm3,ymm2,ymm0         ; YMM3 = indexes vector #3
vpaddq ymm4,ymm3,ymm0         ; YMM4 = indexes vector #4
vpcmpeqd ymm0,ymm0,ymm0       ; YMM0 = mask, all "1"
;---------- Measurement cycle, prepare big cycle ------------------------------;
Measurement_Gather_AVX256:
mov eax,esi             ; EAX = Reload source address
mov edx,ecx             ; EDX = Reload length
shr edx,3               ; EDX = Block length, convert from INSTRUCTIONS to 8xINSTRUCTION units
jz Small_Gather_AVX256  ; Go if Length < 8 instructions
mov ebx,32              ; EBX = Vertical addend * 4
;---------- Big cycle ---------------------------------------------------------;
DumpStart_Gather_AVX256:
Block_Gather_AVX256:
vmovapd ymm5,ymm0       ; Mask must be reloaded because gather instruction clear it
vmovapd ymm6,ymm0
vgatherqpd ymm7,[eax + ymm1],ymm5  
vgatherqpd ymm7,[eax + ymm2],ymm6
vmovapd ymm5,ymm0
vmovapd ymm6,ymm0
vgatherqpd ymm7,[eax + ymm3],ymm5
vgatherqpd ymm7,[eax + ymm4],ymm6
add eax,ebx             ; Modify address, addend=register (not constant) for optimization!
vmovapd ymm5,ymm0
vmovapd ymm6,ymm0
vgatherqpd ymm7,[eax + ymm1],ymm5  
vgatherqpd ymm7,[eax + ymm2],ymm6
vmovapd ymm5,ymm0
vmovapd ymm6,ymm0
vgatherqpd ymm7,[eax + ymm3],ymm5
vgatherqpd ymm7,[eax + ymm4],ymm6
add eax,ebx             ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Block_Gather_AVX256 ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_Gather_AVX256:
;---------- Prepare tail cycle ------------------------------------------------;
Small_Gather_AVX256:
mov edx,ecx
and edx,00000111b           ; EDX = Extract tail length
jz Measure_Gather_AVX256
mov ebx,8                   ; EBX = Vertical addend
;---------- Tail cycle --------------------------------------------------------;
Tail_Gather_AVX256:
vmovapd ymm5,ymm0
vgatherqpd ymm7,[eax + ymm1],ymm5
add eax,ebx                 ; Modify address, addend=register (not constant) for optimization!
dec edx
jnz Tail_Gather_AVX256      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_Gather_AVX256:
sub ebp,1                      ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_Gather_AVX256  ; Cycle for measurement, repeat same operation
pop ebx
ret
