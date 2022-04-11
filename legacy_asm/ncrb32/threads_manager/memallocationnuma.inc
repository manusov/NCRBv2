;---------- NUMA-aware memory allocation --------------------------------------;
;                                                                              ;
; INPUT:   EBX = Pointer to nodes list, source and destination for this subr.  ;
;          ECX = Required memory size per node                                 ;
;          EDX = Number of nodes                                               ;           
;          EAX = Large Pages option, used if value=2                           ; 
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC) = no errors, 1(C) = error                    ;
;                                                                              ;
; Required size must be pre-aligned by used (normal or large) page size        ;
; This subroutine only allocate memory, required pre-builded NUMA nodes list.  ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.        ; 
;                                                                              ;
;------------------------------------------------------------------------------;
MemAllocationNuma:
cld
push ebx esi edi ebp 0  ; last push 0 for local variable
;---------- Reload parameters to non-volatile registers -----------------------;
              ; RBX = Base address of NUMA nodes list
mov esi,ecx   ; ESI = Memory block size per each NUMA node
mov edi,edx   ; EDI = Number of nodes
;---------- Check for large pages ---------------------------------------------;
mov ebp,MEM_RESERVE + MEM_COMMIT  ; EBP = Memory allocation flags
cmp eax,LP_USED                   ; Check Large Pages mode, 2=supported and used
jne .SkipLP                       ; Go if Large Pages not used
mov ebp,MEM_RESERVE + MEM_COMMIT + MEM_LARGE_PAGES  ; Memory allocation flags for Large Pages
.SkipLP:
;---------- Get current process handle ----------------------------------------;
call [GetCurrentProcess]     ; Return EAX = Handle
test eax,eax
jz .error                    ; Go error if Handle=0
mov [esp],eax                ; Store handle at local variable
;---------- Select NUMA or Non-NUMA branches, EDI = nodes count ---------------;
cmp edi,1
jbe .noNumaAlloc 
;---------- Branch for platform/modes with NUMA -------------------------------;
; Cycle for nodes, allocate memory block per each NUMA node
.numaAlloc:
mov [ebx + NUMACTRL.alignedSize],esi  ; Store memory block size
push [ebx + NUMACTRL.nodeID]          ; Parm#6 = NUMA node preferred
push PAGE_READWRITE                   ; Parm#5 = Pages protection attributes = Read/Write
push ebp                              ; Parm#4 = Allocation type (mode flags)
push esi  			                      ; Parm#3 = Block size
push 0                                ; Parm#2 = Base address, 0 means selected by function
push dword [esp + 20]	                ; Parm#1 = Handle
call [DYNA_IMPORT._VirtualAllocExNuma]  ; Call memory allocation WinAPI, return EAX = Pointer  
test eax,eax
jz .error                             ; Go error if returned 0
mov [ebx + NUMACTRL.trueBase],eax     ; Store true base address, it used for release
mov [ebx + NUMACTRL.alignedBase],eax  ; Store aligned base address, it used for memory access
add ebx,NUMACTRL_SIZE                 ; Select next entry in the nodes list 
dec edi                               ; Nodes count -1
jnz .numaAlloc                        ; Cycle for requested number of nodes 
;---------- Exit points -------------------------------------------------------;
.done:
clc                       ; CF = 0 means status = no errors 
.exit:
pop ecx ebp edi esi ebx   ; first pop ecx for local variable
ret
.error:
stc                       ; CF = 1 means status = error detected
jmp .exit
;---------- Branch for platform/modes without NUMA ----------------------------;
.noNumaAlloc:
mov [ebx + NUMACTRL.alignedSize],esi  ; Store memory block size
call [GetCurrentProcess]  ; EAX = Get Process Handle
test eax,eax              ; Check process handle
jz .status                ; Go error if process handle = 0
push PAGE_READWRITE       ; Parm#5 = Page Attribute
push ebp                  ; Parm#4 = Allocation type
push esi                  ; Parm#3 = Block size
push 0                    ; Parm#2 = Virtual address, 0=Auto select
push eax                  ; Parm#1 = Process Handle
call [VirtualAllocEx]     ; Call memory allocation WinAPI, return RAX = Pointer
;------- Status phase, EAX = allocation result --------------------------------;
.status:
test eax,eax
jz .error                             ; Go error if returned 0
mov [ebx + NUMACTRL.trueBase],eax     ; Store true base address, it used for release
mov [ebx + NUMACTRL.alignedBase],eax  ; Store aligned base address, it used for memory access
jmp .done
