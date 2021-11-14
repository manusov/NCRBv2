;---------- Get NUMA domains information --------------------------------------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoNumaDomains:
push 0
mov r13,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea rsi,[DYNA_IMPORT]
mov rax,[rsi + DYNAIMPORT._GetNumaHighestNodeNumber]
test rax,rax
jz .skipNumaList
mov rcx,r13           ; RCX = Parm#1 = Pointer to output variable
call rax
test rax,rax
jz .error
mov rdi,rbp           ; RDI = Pointer, RBP = Start
xor ebx,ebx           ; RBX = Node number and compare pattern
mov r12d,[r13]        ; R12 = Maximum node number
cmp [rsi + DYNAIMPORT._GetNumaNodeProcessorMaskEx],rbx
je .standardTry
cmp [rsi + DYNAIMPORT._GetNumaAvailableMemoryNodeEx],rbx
je .standardTry
mov eax,1         ; dword 00000001h = marker for extended topology scan results
stosd
.extendedNumaList:
mov [rdi],ebx
lea rdx,[rdi + 4]     ; RDX = Parm#2 = Pointer to output variable: group + mask
mov ecx,ebx           ; RCX = Parm#1 = Node 
call [rsi + DYNAIMPORT._GetNumaNodeProcessorMaskEx]
test rax,rax
jz @f
lea rdx,[rdi + 20]    ; RDX = Parm#2 = Pointer to output variable: memory size
mov ecx,ebx           ; RCX = Parm#1 = Node 
call [rsi + DYNAIMPORT._GetNumaAvailableMemoryNodeEx]
test rax,rax
jz @f
add rdi,28
@@:
inc ebx
cmp ebx,r12d
jbe .extendedNumaList 
jmp .doneNumaList
.standardTry:
cmp [rsi + DYNAIMPORT._GetNumaNodeProcessorMask],rbx
je .skipNumaList
cmp [rsi + DYNAIMPORT._GetNumaAvailableMemoryNode],rbx
je .skipNumaList
xor eax,eax       ; dword 00000000h = marker for standard topology scan results
stosd
.standardNumaList:
mov [rdi],ebx
lea rdx,[rdi + 4]     ; RDX = Parm#2 = Pointer to output variable: mask
mov ecx,ebx           ; RCX = Parm#1 = Node 
call [rsi + DYNAIMPORT._GetNumaNodeProcessorMask]
test rax,rax
jz @f
lea rdx,[rdi + 12]    ; RDX = Parm#2 = Pointer to output variable: memory size
mov ecx,ebx           ; RCX = Parm#1 = Node 
call [rsi + DYNAIMPORT._GetNumaAvailableMemoryNode]
test rax,rax
jz @f
add rdi,20
@@:
inc ebx
cmp ebx,r12d
jbe .standardNumaList 
.doneNumaList:
mov [DYNA_PTR.listNuma.objectStart],rbp
mov [DYNA_PTR.listNuma.objectStop],rdi
mov rbp,rdi
.skipNumaList:
mov rsp,r13
pop rax
ret
.error:
stc
jmp .skipNumaList

;--- Build NUMA domains information for text report and GUI widgets -----------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextNumaDomains:
mov rsi,[DYNA_PTR.listNuma.objectStart]
mov rcx,[DYNA_PTR.listNuma.objectStop]
test rcx,rcx
jz .noNuma
mov [DYNA_PTR.textNuma.objectStart],rbp
lodsd
test eax,eax
jnz .numaExt
.numaStd:
cmp rsi,rcx
jae .numaDoneStd
lodsd
call HelperNumberString
lea rdx,[rbp - 82 - 02]
call HelperAffinity
lea rdi,[rbp - 82 + 43]
mov bl,2
mov rax,[rsi + 08]
call SizePrint64
add rsi,16
inc ebx
jmp .numaStd
.numaDoneStd:
mov al,0
stosb
.numaExt:
cmp rsi,rcx
jae .numaDoneExt
lodsd
call HelperNumberString
lea rdx,[rbp - 82 - 02]
mov al,0
call HelperAffinityGroup
lea rdi,[rbp - 82 + 43]
mov bl,2
mov rax,[rsi + 16]
call SizePrint64
add rsi,24
jmp .numaExt
.numaDoneExt:
mov al,0
stosb
mov rbp,rdi
mov [DYNA_PTR.textNuma.objectStop],rbp
.noNuma:
ret

;--- Pre-blank string with CR, LF, write parameter name, set text pointers ----;
; INPUT:   RBP = Pointer to destination buffer                                 ;
;          EAX = Number write to string                                        ; 
; OUTPUT:  RBP = Updated pointer to destination buffer                         ;   
;------------------------------------------------------------------------------;
HelperNumberString:
push rcx rax
mov rdi,rbp
cld
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
pop rax
mov rbp,rdi
lea rdi,[rbp - 82 + 01]
mov bl,0
call DecimalPrint32
pop rcx
ret

