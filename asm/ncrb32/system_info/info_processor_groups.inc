;---------- Get Processor Groups information ----------------------------------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoProcessorGroups:
push eax
xor ebx,ebx           ; EBX = Node number and compare pattern
cmp [DYNA_IMPORT._GetActiveProcessorGroupCount],ebx  
je .skipGroupList 
cmp [DYNA_IMPORT._GetActiveProcessorCount],ebx       
je .skipGroupList
call [DYNA_IMPORT._GetActiveProcessorGroupCount]
test ax,ax
jz .skipGroupList
mov [esp],ax 
mov edi,ebp           ; EDI = Pointer, EBP = Start
.groupList:
mov eax,ebx
stosd
push ebx              ; Parm#1 = Group number
call [DYNA_IMPORT._GetActiveProcessorCount]
; no check for EAX = 0, empty group is acceptable ?
stosd
inc ebx
cmp bx,[esp]
jb .groupList
.doneGroupList:
mov [DYNA_PTR.listGroup.objectStart],ebp
mov [DYNA_PTR.listGroup.objectStop],edi
mov ebp,edi
.skipGroupList:
pop eax
ret

;--- Build Processor groups information for text report and GUI widgets -------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextProcessorGroups:
mov edx,[DYNA_PTR.listGroup.objectStart]
mov ecx,[DYNA_PTR.listGroup.objectStop]
test ecx,ecx
jz .noGroups
mov [DYNA_PTR.textGroup.objectStart],ebp
push esi
mov esi,edx
.groupsScan:
cmp esi,ecx
jae .groupsDone
lodsd
call HelperNumberString
lea edi,[ebp - 82 + 10]
lodsd
mov bl,0
call DecimalPrint32
jmp .groupsScan 
.groupsDone:
pop esi
mov al,0
stosb
mov ebp,edi
mov [DYNA_PTR.textGroup.objectStop],ebp
.noGroups:
ret

