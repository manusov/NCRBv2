;---------- Call benchmark target subroutine ----------------------------------;
;                                                                              ;
; INPUT:   RBX = Pointer to Control Entry                                      ;
;          Control entry data , see global\connect_equ.inc , THCTRL layout     ;
;                                                                              ;
; OUTPUT:  RAX = Delta TSC                                                     ;  
;          Registers corrupted, except R15 = saved                             ;
;                                                                              ;
;------------------------------------------------------------------------------;
RunTarget:
;---------- Load input parameters, part 1 -------------------------------------;
mov rsi,[rbx + THCTRL.base1]       ; RSI = Block #1 pointer (64-bit flat)
mov rdi,[rbx + THCTRL.base2]       ; RDI = Block #2 pointer (64-bit flat)
mov rbp,[rbx + THCTRL.repeats]     ; RBP = Number of measurement repeats
mov r14,rbx
;---------- Serialization, note EAX,EBX,ECX,EDX changed -----------------------;
xor eax,eax        ; EAX = 0 means function 0 for CPUID
cpuid              ; This CPUID for serialization only, results ignored
;---------- Load input parameters, part 2 -------------------------------------;
mov rcx,[r14 + THCTRL.sizeInst]    ; RCX = Block length, units = instructions 
mov rbx,[r14 + THCTRL.entryPoint]  ; RBX = Target subroutine address
;---------- Get start time ----------------------------------------------------;
rdtsc              ; EDX:EAX = TSC value at start: EDX=High, EAX=Low
shl rdx,32         ; Positioning TSC high 32 bits to RDX.[63-32]
lea r14,[rax+rdx]  ; R14 = Interval start TSC value, integral 64 bits
;---------- Call target routine ( required not destroy R14, R15 ) -------------;
call rbx           ; Run target operation
;---------- Get end time ------------------------------------------------------;
rdtsc              ; EDX:EAX = TSC value at end: EDX=High, EAX=Low
shl rdx,32         ; Positioning TSC high 32 bits to RDX.[63-32]
or rax,rdx         ; RAX = Interval end TSC value, integral 64 bits
sub rax,r14        ; RAX = Delta TSC value
push rax
xor eax,eax        ; EAX = 0 means function 0 for CPUID
cpuid              ; This CPUID for serialization only, results ignored
pop rax
;---------- Return, RAX = delta TSC -------------------------------------------;
ret
