;---------- NUMA-aware memory release -----------------------------------------;
; Note this single memory release subroutine used                              ;
; for NUMA / No NUMA , for Normal Pages / Large Pages.                         ; 
;                                                                              ;
; INPUT:   RCX = Pointer to nodes list                                         ;
;          EDX = Number of nodes, can be EDX=1 if NUMA not used / not sup.     ; 
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC) = no errors, 1(C) = error                    ;
;                                                                              ;
; Can destroy registers, volatile by Microsoft x64 calling convention.         ; 
;------------------------------------------------------------------------------;
MemReleaseNuma:
cld
push rbx rsi rdi rbp r12
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h    ; Stack alignment because WinAPI call
sub rsp,32                    ; Make parameters shadow because WinAPI call 
;---------- Reload input parameters to non-volatile registers -----------------;
mov rsi,rcx                   ; RSI = Pointer to NUMA nodes list
mov edi,edx                   ; EDI = Number of used NUMA nodes
;---------- Get current process handle ----------------------------------------;
call [GetCurrentProcess]      ; Return RAX = Handle
xchg rbx,rax                  ; Copy to non-volatile register, XCHG for compact
;---------- Set nodes list limit ----------------------------------------------;
mov r12d,MAX_NODES            ; R12D = Maximum number of NUMA nodes
;---------- Cycle for release memory, allocated at all nodes ------------------;
.numaRelease:
mov rcx,rbx                        ; Parm#1 = RCX = Process Handle
mov rdx,[rsi + NUMACTRL.trueBase]  ; Parm#2 = RDX = Address
xor r8d,r8d                        ; Parm#3 = R8 = Release size, here must be 0 = all block
mov r9d,MEM_RELEASE	               ; Parm#4 = R9 = Release type
test rdx,rdx
jz .skip                           ; Skip this entry if memory not allocated for it
call [VirtualFreeEx]               ; Call memory release WinAPI, return RAX = Status
test rax,rax
jz .error                          ; Go if error detected
.skip:
;---------- Cycle for NUMA nodes ----------------------------------------------;
dec edi
jz .done
add rsi,NUMACTRL_SIZE              ; Select next entry                    
dec r12
jnz .numaRelease                   ; Cycle for all (typical 64) entries 
;---------- Exit points -------------------------------------------------------;
.done:
clc                  ; CF = 0 means status = no errors 
.exit:
mov rsp,rbp          ; Restore RSP at subroutine entry, because stack alignment
pop r12 rbp rdi rsi rbx
ret
.error:
stc                  ; CF = 1 means status = error detected
jmp .exit
