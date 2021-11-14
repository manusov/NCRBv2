;---------- Execute "Vector brief" test and build results text data -----------;
;                                                                              ;
; INPUT  : None                                                                ;
;                                                                              ;
; OUTPUT : Structure VECBR_OPB, output parameters block, binary data           ; 
;          Structure BIND_LIST, output text strings for visual                 ;
;                                                                              ;
;------------------------------------------------------------------------------; 
ScenarioVectorBrief:
pushad
cld
;---------- Refresh system information-- --------------------------------------;
call InfoProcessor
mov al,MSG_RUNTIME_TIMINGS
jc .error
;---------- Measurements ------------------------------------------------------;
lea esi,[VECTOR_BRIEF]
lea edi,[VECBR_OPB]
mov ecx,VB_REPORT_COUNT
push edi ecx               ; Pre-blank OPB
shl ecx,1
xor eax,eax
rep stosd
pop ecx edi
.results:
movzx eax,byte [esi + 3]  ; cpuValid
mov edx,dword [CPU_DATA.extractedFeaturesBitmap]
bt edx,eax
jnc .skip
mov al,[esi + 4]          ; osValid
cmp al,0FFh
je .unconditional
mov ah,al
and al,byte [CPU_DATA.extractedContextBitmap] 
cmp al,ah
jne .skip
.unconditional:
push ebx ecx esi edi
mov al,[esi + 0]          ; id
test al,al
js .math
and al,07Fh
call GetBandwidthTemporalPattern
jmp .havePattern
.math:
and al,07Fh
call GetMathPattern
.havePattern:
lea edi,[THREAD_LIST]
mov [edi + THCTRL.entryPoint],ebx    ; EBX = Subroutine address
mov cl,[esi + 1]                     ; CL = Count, power of 2 
mov eax,1
shl eax,cl
mov [edi + THCTRL.sizeInst],eax      ; EAX = Instructions count per 4K

movzx eax,byte [esi + 2]             ; EAX = Repeats
cdq
mov ecx,VECTOR_BRIEF_REPEATS_SCALE
mul ecx                              ; EDX:EAX = Repeats count
mov dword [edi + THCTRL.repeats + 0],eax
mov dword [edi + THCTRL.repeats + 4],edx
lea eax,[TEMP_BUFFER]
mov [edi + THCTRL.base1],eax
add eax,VECTOR_BRIEF_DATA_SIZE / 2
mov [edi + THCTRL.base2],eax
;---------- Set work buffer address -------------------------------------------;
; If this run for math. pattern, base1 used for load numeric constants vector,
; not for block 1 base.
test byte [esi + 0],80h
jz @f
mov [edi + THCTRL.base1],DATA_MATH
@@:
mov ebx,edi            ; Note can remove this if Get...Pattern return in RAX 
call RunTarget         ; Return EDX:EAX = dTSC per pattern
pop edi esi ecx ebx    ; Restore after performance patterns run
jmp .next
.skip:
xor eax,eax
cdq
.next:
stosd
xchg eax,edx
stosd
add esi,5
dec ecx        ; DEC/JNZ timings better than LOOP 
jnz .results
;---------- Results calculation and text strings write ------------------------; 
finit
push 0 1000000000
mov ebp,esp
fild qword [ebp]
fild qword [CPU_DATA.tscClockHz]
fdivp st1,st0
fstp qword [ebp]
lea esi,[VECTOR_BRIEF]
lea edx,[VECBR_OPB]
lea edi,[BIND_LIST]
add edi,BINDLIST.vbSse128read
mov ecx,VB_REPORT_COUNT
;---------- Cycle for build text strings --------------------------------------;
.strings:
push ebx ecx edx esi edi
movzx eax,byte [esi + 3]  ; cpuValid
mov ecx,dword [CPU_DATA.extractedFeaturesBitmap]
bt ecx,eax
mov ax,STR_NOT_SUPPORTED_CPU
jnc .nocalc
mov al,[esi + 4]          ; osValid
mov ecx,dword [CPU_DATA.extractedContextBitmap]
cmp al,0FFh
je .calc
mov ah,al
and al,cl
cmp al,ah
mov ax,STR_NOT_SUPPORTED_OS
jne .nocalc
;---------- This point if result valid, pattern supported and executed --------;
.calc:
mov cl,[esi + 1]               ; Instructions count, 32-bit
mov eax,1
shl eax,cl
mov ecx,edx
movzx edx,byte [esi + 2]       ; Measurement count, 32-bit
imul edx,edx,VECTOR_BRIEF_REPEATS_SCALE
mul edx                        ; EDX:EAX = 64-bit product
push edx eax edx eax
fild qword [esp]        ; st0 = instructions count
fild qword [ecx]        ; st0 = clocks count , st1 = instructions count
fdiv st0,st1            ; st0 = clocks per instruction (CPI)
fst qword [esp]         ; store CPI
fmul qword [ebp]        ; st0 = nanoseconds per instruction (nsPI)
fstp qword [esp + 8]    ; Store nsPI
ffree st0
fincstp
pop eax edx             ; Load CPI
mov bx,0300h
call DoublePrint        ; Print CPI
mov ax,' ('
stosw
pop eax edx             ; Load nsPI
call DoublePrint        ; Print nsPI, " ( x ns ) "
mov al,' '
stosb
mov ax,STR_NS
call PoolStringWrite
mov al,')'
stosb
jmp .doneString
.nocalc:
call PoolStringWrite
.doneString:
mov al,0
stosb
pop edi esi edx ecx ebx 
add esi,5
add edx,8
add edi,VB_REPORT_STEP
dec ecx
jnz .strings
pop eax eax ; remove local variable
.error:     ; CF = 1 and AL must be valid if exit by error branch
mov [esp + 28],al
popad
ret
