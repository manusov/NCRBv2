;---------- Get Affinized CPUID dump information ------------------------------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Configuration detection restrictions ------------------------------;
LOGICAL_PROCESSORS_LIMIT = 32  ; Yet limited for maximum 32 logical processors

InfoAffinizedCpuid:
mov edi,ebp
cmp [DYNA_IMPORT._SetThreadAffinityMask],0
je .skipAffCpuid
call [GetCurrentThread]
test eax,eax
jz .skipAffCpuid 
xchg ebx,eax             ; EBX = Current thread handle
mov ecx,[OS_DATA.systemInfo.dwNumberOfProcessors]
jecxz .skipAffCpuid
cmp ecx,LOGICAL_PROCESSORS_LIMIT
jbe @f
mov ecx,LOGICAL_PROCESSORS_LIMIT      
@@:                      ; ECX = Logical processors count
xor edx,edx              ; EDX = Storage for original affinity mask
mov eax,1                ; EAX = Variable affinity mask for dump
.affinizedDump:
push eax ecx edx
push eax                 ; Parm#2 = Affinity mask
push ebx                 ; Parm#1 = Current thread handle 
call [DYNA_IMPORT._SetThreadAffinityMask]
pop edx
test edx,edx
jnz @f
xchg edx,eax
@@:
push edx
call GetCPUID
mov eax,[edi]
shl eax,5
lea edi,[edi + eax + 32]
pop edx ecx eax
rol eax,1
loop .affinizedDump 
test edx,edx
jz @f
push edx                 ; Parm#2 = Affinity mask
push ebx                 ; Parm#1 = Current thread handle 
call [DYNA_IMPORT._SetThreadAffinityMask]
@@:
mov [DYNA_PTR.listAffCpuid.objectStart],ebp
mov [DYNA_PTR.listAffCpuid.objectStop],edi
mov ebp,edi
.skipAffCpuid:
ret

;--- Build Affinized CPUID dump information for text report and GUI widgets ---;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextAffinizedCpuid:
mov edx,[DYNA_PTR.listAffCpuid.objectStart]
mov ecx,[DYNA_PTR.listAffCpuid.objectStop]
test ecx,ecx
jz .noAffCpuidData
mov [DYNA_PTR.textAffCpuid.objectStart],ebp
push esi
mov esi,edx
xor edx,edx
.scanBlocks:
push ecx
mov ecx,[esi]
bts ecx,31
add esi,32
.scanLines:
mov edi,ebp
push ecx
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
pop ecx
btr ecx,31
jnc @f
lea edi,[ebp + 2]
mov bl,0
mov eax,edx
call DecimalPrint32
@@:
mov ebx,esi
push ecx edx
mov ax,0C04h
call HelperHexStringNh
mov ax,1910h
call HelperHexStringNh
mov ax,2414h
call HelperHexStringNh
mov ax,2F18h
call HelperHexStringNh
mov ax,3A1Ch
call HelperHexStringNh
pop edx ecx
add esi,32
add ebp,82
dec ecx
jnz .scanLines
mov edi,ebp
mov ax,0A0Dh
stosw
mov ebp,edi
inc edx
pop ecx
cmp esi,ecx
jb .scanBlocks
.affCpuidDataDone:
pop esi
mov al,0
stosb
mov ebp,edi
mov [DYNA_PTR.textAffCpuid.objectStop],ebp
.noAffCpuidData:
ret
