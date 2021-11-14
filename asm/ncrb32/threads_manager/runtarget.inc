;---------- Call benchmark target subroutine ----------------------------------;
;                                                                              ;
; INPUT:   EBX = Pointer to Control Entry                                      ;
;          Control entry data , see global\connect_equ.inc , THCTRL layout     ;
;                                                                              ;
; OUTPUT:  EDX:EAX = Delta TSC , EDX = High 32 bits , EAX = Low 32 bits        ;  
;          Registers corrupted                                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
RunTarget:
;---------- Load input parameters ---------------------------------------------;
mov eax,[ebx + THCTRL.entryPoint]   ; EAX = Target subroutine address
mov ecx,[ebx + THCTRL.sizeInst]     ; ECX = Block length, units = instructions
mov esi,[ebx + THCTRL.base1]        ; ESI = Block #1 pointer (32-bit flat)
mov edi,[ebx + THCTRL.base2]        ; EDI = Block #2 pointer (32-bit flat)
mov ebp,dword [ebx + THCTRL.repeats + 00]  ; EBP = Measurement repeats, low 32
mov ebx,dword [ebx + THCTRL.repeats + 04]  ; EBX = Measurement repeats, high 32  
;---------- Serialization, note EAX,EBX,ECX,EDX changed by CPUID --------------;
push eax ebx ecx
xor eax,eax        ; EAX = 0 means function 0 for CPUID
cpuid              ; This CPUID for serialization only, results ignored
pop ecx ebx
;---------- Get start time ----------------------------------------------------;
rdtsc              ; EDX:EAX = TSC value at start: EDX=High, EAX=Low
push edx eax
;---------- Call target routine ( required not destroy R14, R15 ) -------------;
call dword [esp + 08]    ; Run target operation
;---------- Get end time ------------------------------------------------------;
rdtsc                    ; EDX:EAX = TSC value at end: EDX=High, EAX=Low
sub eax,[esp + 00]
sbb edx,[esp + 04]
mov [esp + 00],eax
mov [esp + 04],edx
;---------- Serialization, note EAX,EBX,ECX,EDX changed by CPUID --------------;
xor eax,eax        ; EAX = 0 means function 0 for CPUID
cpuid              ; This CPUID for serialization only, results ignored
;---------- Return, EDX:EAX = delta TSC , pop ecx for remove variable ---------;
pop eax edx ecx 
ret
