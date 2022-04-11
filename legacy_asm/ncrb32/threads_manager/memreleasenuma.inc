;---------- NUMA-aware memory release -----------------------------------------;
; Note this single memory release subroutine used                              ;
; for NUMA / No NUMA , for Normal Pages / Large Pages.                         ; 
;                                                                              ;
; INPUT:   EBX = Pointer to nodes list                                         ;
;          EDX = Number of nodes, can be EDX=1 if NUMA not used / not sup.     ; 
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC) = no errors, 1(C) = error                    ;
;                                                                              ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.        ; 
;------------------------------------------------------------------------------;
MemReleaseNuma:
cld
push ebx esi edi ebp
;---------- Reload input parameters to non-volatile registers -----------------;
                ; EBX = Pointer to NUMA nodes list
mov ebp,edx     ; EBP = Number of used NUMA nodes
;---------- Get current process handle ----------------------------------------;
call [GetCurrentProcess]    ; Return EAX = Handle
xchg esi,eax                ; Copy to non-volatile register, XCHG for compact
;---------- Set nodes list limit ----------------------------------------------;
mov edi,MAX_NODES           ; EDI = Maximum number of NUMA nodes
;---------- Cycle for release memory, allocated at all nodes ------------------;
.numaRelease:
mov ecx,[ebx + NUMACTRL.trueBase]  ; ECX = Address
jecxz .skip                        ; Skip this entry if memory not allocated for it
push MEM_RELEASE        ; Parm#4 = Release type
push 0                  ; Parm#3 = Release size, here must be 0 = all block
push ecx                ; Parm#2 = Address
push esi                ; Parm#1 = Process Handle
call [VirtualFreeEx]    ; Call memory release WinAPI, return RAX = Status
test eax,eax
jz .error               ; Go if error detected
.skip:
;---------- Cycle for NUMA nodes ----------------------------------------------;
dec ebp
jz .done
add ebx,NUMACTRL_SIZE              ; Select next entry                    
dec edi
jnz .numaRelease                   ; Cycle for all (typical 64) entries 
;---------- Exit points -------------------------------------------------------;
.done:
clc                  ; CF = 0 means status = no errors 
.exit:
pop ebp edi esi ebx
ret
.error:
stc                  ; CF = 1 means status = error detected
jmp .exit
