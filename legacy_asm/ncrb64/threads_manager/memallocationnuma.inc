;---------- NUMA-aware memory allocation --------------------------------------;
;                                                                              ;
; INPUT:   RCX = Pointer to nodes list, source and destination for this subr.  ;
;          RDX = Required memory size per node                                 ;
;          R8D = Number of nodes                                               ;           
;          R9D = Large Pages option, used if value = 2                         ; 
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC) = no errors, 1(C) = error                    ;
;                                                                              ;
; Required size must be pre-aligned by used (normal or large) page size        ;
; This subroutine only allocate memory, required pre-builded NUMA nodes list.  ;
; Can destroy registers, volatile by Microsoft x64 calling convention.         ; 
;                                                                              ;
;------------------------------------------------------------------------------;
MemAllocationNuma:
cld
push rbx rsi rdi rbp r12 r13
mov rbp,rsp                  ; RBP = Save RSP at procedure entry
and rsp,0FFFFFFFFFFFFFFF0h   ; Stack alignment
sub rsp,32                   ; Make parameters shadow because WinAPI calls
;---------- Reload parameters to non-volatile registers -----------------------;
mov rbx,rcx                  ; RBX = Base address of NUMA nodes list
mov rsi,rdx                  ; RSI = Memory block size per each NUMA node
mov edi,r8d                  ; RDI = Number of nodes
;---------- Check for large pages ---------------------------------------------;
mov r12d,MEM_RESERVE + MEM_COMMIT  ; R12 = Memory allocation flags
cmp r9d,LP_USED              ; Check Large Pages mode, 2=supported and used
jne .skipLP                  ; Go if Large Pages not used
mov r12d,MEM_RESERVE + MEM_COMMIT + MEM_LARGE_PAGES  ; Memory allocation flags for Large Pages
.skipLP:
;---------- Get current process handle ----------------------------------------;
call [GetCurrentProcess]     ; Return RAX = Handle
test rax,rax
jz .error                    ; Go error if Handle=0
xchg r13,rax                 ; R13 = Process Handle, XCHG for compact
;---------- Select NUMA or Non-NUMA branches, EDI = nodes count ---------------;
cmp edi,1
jbe .noNumaAlloc 
;---------- Branch for platform/modes with NUMA -------------------------------;
; Cycle for nodes, allocate memory block per each NUMA node
.numaAlloc:
mov [rbx + NUMACTRL.alignedSize],rsi  ; Store memory block size
mov rcx,r13			                      ; Parm#1 = RCX = Handle
xor edx,edx			                      ; Parm#2 = RDX = Base address, 0 means selected by function
mov r8,rsi			                      ; Parm#3 = R8  = Block size
mov r9,r12                            ; Parm#4 = R9  = Allocation type (mode flags)
push qword [rbx + NUMACTRL.nodeID]    ; Parm#6 = NUMA node preferred
push PAGE_READWRITE                   ; Parm#5 = Pages protection attributes = Read/Write
sub rsp,32			                      ; Create parameters shadow
call [DYNA_IMPORT._VirtualAllocExNuma]  ; Call memory allocation WinAPI, return RAX = Pointer
add rsp,32+16			                    ; Remove parameters shadow + parameters 5,6
test rax,rax
jz .error                             ; Go error if returned 0
mov [rbx + NUMACTRL.trueBase],rax     ; Store true base address, it used for release
mov [rbx + NUMACTRL.alignedBase],rax  ; Store aligned base address, it used for memory access
add rbx,NUMACTRL_SIZE                 ; Select next entry in the nodes list 
dec edi                               ; Nodes count -1
jnz .numaAlloc                        ; Cycle for requested number of nodes 
;---------- Exit points -------------------------------------------------------;
.done:
clc                   ; CF = 0 means status = no errors 
.exit:
mov rsp,rbp           ; Restore RSP at subroutine entry, because stack alignment
pop r13 r12 rbp rdi rsi rbx
ret
.error:
stc                   ; CF = 1 means status = error detected
jmp .exit
;---------- Branch for platform/modes without NUMA ----------------------------;
.noNumaAlloc:
mov [rbx + NUMACTRL.alignedSize],rsi  ; Store memory block size
call [GetCurrentProcess]  ; RAX = Get Process Handle
test rax,rax              ; Check process handle
jz .status                ; Go error if process handle = 0
mov rcx,rax               ; Parm#1 = RCX = Process Handle
xor edx,edx               ; Parm#2 = RDX = Virtual address, 0=Auto select
mov r8,rsi                ; Parm#3 = R8 = Block size 
mov r9,r12                ; Parm#4 = R9 = Allocation type
push 0                    ; Push for stack alignment
push PAGE_READWRITE       ; Parm#5 = [RSP+40] = Page Attribute
sub rsp,32                ; Create stack frame
call [VirtualAllocEx]     ; Call memory allocation WinAPI, return RAX = Pointer
add rsp,32+16             ; Remove stack frame
;------- Status phase, RAX = allocation result --------------------------------;
.status:
test rax,rax
jz .error                             ; Go error if returned 0
mov [rbx + NUMACTRL.trueBase],rax     ; Store true base address, it used for release
mov [rbx + NUMACTRL.alignedBase],rax  ; Store aligned base address, it used for memory access
jmp .done
