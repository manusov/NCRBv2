;---------- Memory performance pattern ----------------------------------------;
; Memory access method = " Copy AVX-256 (VMOVNTDQA + VMOVNTPD) " ,             ;
; mode = non temporal, one of variants, depends on options.                    ;
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
Pattern_NtRCopy_AVX256:
push ebx                 ; EBX = High 32 bits of measurement repeats count
Measurement_NtRCopy_AVX256:
;---------- Prepare big cycle -------------------------------------------------;
; Set pointer to middle of 512-byte interval, +/- offsets is code size optimal,
; because offsets [-128...+127] encoded as one byte
lea eax,[esi+256]        ; EAX = Reload source address
lea ebx,[edi+256]        ; EBX = Reload destination address
mov edx,ecx              ; EDX = Reload length
shr edx,4                ; EDX = Block length, convert from INSTRUCTIONS to 16xINSTRUCTION units
jz Small_NtRCopy_AVX256  ; Go if Length < 16 instructions
;---------- Big cycle ---------------------------------------------------------;
DumpStart_NtRCopy_AVX256:
Block_NtRCopy_AVX256:
vmovntdqa ymm0,[eax-32*08]
vmovntpd [ebx-32*08],ymm0
vmovntdqa ymm1,[eax-32*07]
vmovntpd [ebx-32*07],ymm1
vmovntdqa ymm2,[eax-32*06]
vmovntpd [ebx-32*06],ymm2
vmovntdqa ymm3,[eax-32*05]
vmovntpd [ebx-32*05],ymm3
vmovntdqa ymm4,[eax-32*04]
vmovntpd [ebx-32*04],ymm4
vmovntdqa ymm5,[eax-32*03]
vmovntpd [ebx-32*03],ymm5
vmovntdqa ymm6,[eax-32*02]
vmovntpd [ebx-32*02],ymm6
vmovntdqa ymm7,[eax-32*01]
vmovntpd [ebx-32*01],ymm7
vmovntdqa ymm0,[eax+32*00]
vmovntpd [ebx+32*00],ymm0
vmovntdqa ymm1,[eax+32*01]
vmovntpd [ebx+32*01],ymm1
vmovntdqa ymm2,[eax+32*02]
vmovntpd [ebx+32*02],ymm2
vmovntdqa ymm3,[eax+32*03]
vmovntpd [ebx+32*03],ymm3
vmovntdqa ymm4,[eax+32*04]
vmovntpd [ebx+32*04],ymm4
vmovntdqa ymm5,[eax+32*05]
vmovntpd [ebx+32*05],ymm5
vmovntdqa ymm6,[eax+32*06]
vmovntpd [ebx+32*06],ymm6
vmovntdqa ymm7,[eax+32*07]
vmovntpd [ebx+32*07],ymm7
add eax,512                  ; Modify source address
add ebx,512                  ; Modify destination address
dec edx
jnz Block_NtRCopy_AVX256     ; Cycle for block data transfer, DEC/JNZ is faster than LOOP!
DumpStop_NtRCopy_AVX256:
;---------- Prepare tail cycle ------------------------------------------------;
Small_NtRCopy_AVX256:
mov edx,ecx                    
and edx,00001111b            ; ECX = Extract tail length
jz Measure_NtRCopy_AVX256
;---------- Tail cycle --------------------------------------------------------;
Tail_NtRCopy_AVX256:
vmovntdqa ymm0,[eax-32*08]
vmovntpd [ebx-32*08],ymm0
add eax,32                   ; Modify source address
add ebx,32                   ; Modify destination address
dec edx
jnz Tail_NtRCopy_AVX256      ; Cycle for tail data transfer, DEC/JNZ is faster than LOOP!
;---------- Measurement cycle -------------------------------------------------;
Measure_NtRCopy_AVX256:
sub ebp,1                    ; EBX:EBP = Measurement cycle counter
sbb dword [esp],0
mov eax,ebp
or eax,[esp]
jnz Measurement_NtRCopy_AVX256  ; Cycle for measurement, repeat same operation
pop ebx
ret
