;---------- Get Affinized CPUID dump information ------------------------------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Configuration detection restrictions ------------------------------;
LOGICAL_PROCESSORS_LIMIT = 64  ; Yet limited for maximum 64 logical processors

InfoAffinizedCpuid:
mov r13,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
mov rdi,rbp
cmp [rsi + DYNAIMPORT._SetThreadAffinityMask],0
je .skipAffCpuid
call [GetCurrentThread]
test rax,rax
jz .skipAffCpuid 
mov r15,rax              ; R15 = Current thread handle
mov r14d,[OS_DATA.systemInfo.dwNumberOfProcessors] 
test r14d,r14d
jz .skipAffCpuid
cmp r14d,LOGICAL_PROCESSORS_LIMIT
jbe @f
mov r14d,LOGICAL_PROCESSORS_LIMIT      
@@:                      ; R14D = Logical processors count
xor r12d,r12d            ; R12 = Storage for original affinity mask
mov ebx,1                ; RBX = Variable affinity mask for dump
.affinizedDump:
mov rdx,rbx              ; RDX = Parm#2 = Affinity mask
mov rcx,r15              ; RCX = Parm#1 = Current thread handle 
call [rsi + DYNAIMPORT._SetThreadAffinityMask]
test r12,r12
jnz @f
xchg r12,rax
@@:
call GetCPUID
mov eax,[rdi]
shl eax,5
lea rdi,[rdi + rax + 32]
rol rbx,1
dec r14d
jnz .affinizedDump 
test r12,r12
jz @f
mov rdx,r12              ; RDX = Parm#2 = Affinity mask
mov rcx,r15              ; RCX = Parm#1 = Current thread handle 
call [rsi + DYNAIMPORT._SetThreadAffinityMask]
@@:
mov [DYNA_PTR.listAffCpuid.objectStart],rbp
mov [DYNA_PTR.listAffCpuid.objectStop],rdi
mov rbp,rdi
.skipAffCpuid:
mov rsp,r13
ret

;--- Build Affinized CPUID dump information for text report and GUI widgets ---;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextAffinizedCpuid:
mov rsi,[DYNA_PTR.listAffCpuid.objectStart]
mov rcx,[DYNA_PTR.listAffCpuid.objectStop]
test rcx,rcx
jz .noAffCpuidData
mov [DYNA_PTR.textAffCpuid.objectStart],rbp
xor r14,r14
.scanBlocks:
push rcx
mov r12d,[rsi]
bts r12,63
add rsi,32
.scanLines:
mov rdi,rbp
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
btr r12,63
jnc @f
lea rdi,[rbp + 2]
mov bl,0
mov eax,r14d
call DecimalPrint32
@@:
mov rbx,rsi
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
add rsi,32
add rbp,82
dec r12d
jnz .scanLines
mov rdi,rbp
mov ax,0A0Dh
stosw
mov rbp,rdi
inc r14d
pop rcx
cmp rsi,rcx
jb .scanBlocks
.affCpuidDataDone:
mov al,0
stosb
mov rbp,rdi
mov [DYNA_PTR.textAffCpuid.objectStop],rbp
.noAffCpuidData:
ret
