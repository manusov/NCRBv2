;---------- Get Processor Groups information ----------------------------------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoProcessorGroups:
mov r13,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea rsi,[DYNA_IMPORT]
xor ebx,ebx           ; RBX = Node number and compare pattern
cmp [rsi + DYNAIMPORT._GetActiveProcessorGroupCount],rbx  
je .skipGroupList 
cmp [rsi + DYNAIMPORT._GetActiveProcessorCount],rbx       
je .skipGroupList
call [rsi + DYNAIMPORT._GetActiveProcessorGroupCount]
test ax,ax
jz .skipGroupList
movzx r12d,ax 
mov rdi,rbp           ; RDI = Pointer, RBP = Start
.groupList:
mov eax,ebx
stosd
mov rcx,rbx           ; RCX = Parm#1 = Group number
call [rsi + DYNAIMPORT._GetActiveProcessorCount]
; no check for EAX = 0, empty group is acceptable ?
stosd
inc ebx
cmp ebx,r12d
jb .groupList
.doneGroupList:
mov [DYNA_PTR.listGroup.objectStart],rbp
mov [DYNA_PTR.listGroup.objectStop],rdi
mov rbp,rdi
.skipGroupList:
mov rsp,r13
ret

;--- Build Processor groups information for text report and GUI widgets -------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextProcessorGroups:
mov rsi,[DYNA_PTR.listGroup.objectStart]
mov rcx,[DYNA_PTR.listGroup.objectStop]
test rcx,rcx
jz .noGroups
mov [DYNA_PTR.textGroup.objectStart],rbp
.groupsScan:
cmp rsi,rcx
jae .groupsDone
lodsd
call HelperNumberString
lea rdi,[rbp - 82 + 10]
lodsd
mov bl,0
call DecimalPrint32
jmp .groupsScan 
.groupsDone:
mov al,0
stosb
mov rbp,rdi
mov [DYNA_PTR.textGroup.objectStop],rbp
.noGroups:
ret
