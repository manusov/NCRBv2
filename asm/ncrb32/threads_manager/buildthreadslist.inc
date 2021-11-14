;---------- Build threads list: events, threads, memory ranges ----------------;
;                                                                              ;
; INPUT:   ECX = Pointer to Threads Control List +                             ;
;                Signaled Start-Done Events List + NUMA nodes list             ;
;          EDX = Pointer to Input Parameters Block (IPB)                       ;
;                                                                              ;             
; OUTPUT:  CF = Error flag: 0(NC) = no errors, 1(C) = error                    ;
;                                                                              ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.        ;
;------------------------------------------------------------------------------;
;---------- Stack variables list ----------------------------------------------;
; dword [esp + 00] = Pointer to IPB (Input Parameters Block)
; dword [esp + 04] = Number of threads from IPB
; dword [esp + 08] = Threads / Node ratio, means frequency of
;                    next NODE selection when build THREADS
; dword [esp + 12] = Copy of Threads / Node ratio
; dword [esp + 16] = Memory allocation offset for threads at single node 
BuildThreadsList:
cld
push ebx esi edi ebp 0 0 0 0 edx  ; Note push after EBP for local variables
;---------- Reload parameters to non-volatile registers -----------------------;
mov esi,ecx                        ; ESI  = Pointer to Threads List
lea edi,[ecx + THREAD_LIST_SIZE]   ; EDI  = Pointer to 2 x Events Lists 
lea ebx,[ecx + THREAD_LIST_SIZE + EVENT_LIST_SIZE * 2]  ; RBX = NUMA nodes list  
xor ebp,ebp                         ; EBP = Threads counter
mov eax,[edx + MEMIPB.updatedThreads]  
mov [esp + 04],eax                  ; variable = EAX = Number of threads
;---------- Initialize THREADS per NODE counter, use EAX = Number of threads --; 
mov ecx,[edx + MEMIPB.domainsCount]
xor edx,edx
test ecx,ecx                       ; Cannot JECXZ because offset out of range
jz .error                          ; Go error if number of domains = 0 
div ecx                            ; EAX = Threads / Node ratio
mov [esp + 08],eax
mov [esp + 12],eax
;---------- Threads build cycle -----------------------------------------------;
.threadsBuild:
;---------- Create event for thread start, store event handle -----------------;
push 0                             ; Parm#4 = Name, 0=None
push 0                             ; Parm#3 = bInitialState
push 1                             ; Parm#2 = bManualReset
push 0                             ; Parm#1 = lpEventAttributes
call [CreateEvent]                 ; Call WinAPI function
test eax,eax                       ; Status=0 means error
jz .error                          ; Go if error creating event
mov [esi + THCTRL.eventStart],eax  ; Store handle in the threads block
stosd                              ; Copy to sequental list, used by OS API
;---------- Create event for thread done, store event handle ------------------;
push 0                             ; Parm#4 = Name, 0=None
push 0                             ; Parm#3 = bInitialState
push 1                             ; Parm#2 = bManualReset
push 0                             ; Parm#1 = lpEventAttributes
call [CreateEvent]                 ; Call WinAPI function
test eax,eax                       ; Status=0 means error
jz .error                          ; Go if error creating event
mov [esi + THCTRL.eventDone],eax     ; Store handle in the threads block
mov [edi + EVENT_LIST_SIZE - 4],eax  ; Copy to sequental list, used by OS API 
;---------- Create thread, store event handle ---------------------------------;
push 0                                 ; Parm#6 = Thread ID return , 0=None
push CREATE_SUSPENDED                  ; Parm#5 = Creating Flags = Suspended
push ebp                               ; Parm#4 = Subroutine parameter, 0-63 or 0-255(if Groups)
mov eax,[esp + 12 + 00]                ; EAX = Pointer to IPB
cmp [eax + MEMIPB.updatedPG],PG_OPTIMAL   ; Processor Group option, 2=used
lea edx,[ThreadRoutineSingleGroup]     ; EDX = Execute routine pointer
mov [esi + THCTRL.affinityMode],1      ; This added for single-thread affinitization
jne .afterSetRoutine                   ; Go if simple system, single group 
lea edx,[ThreadRoutineMultiGroup]      ; This routine with affinitization with groups
mov [esi + THCTRL.affinityMode],2      ; This added for single-thread affinitization
.afterSetRoutine:
push edx                               ; Parm#3 = Execute routine pointer
push 0                                 ; Parm#2 = Stack size, 0=default
push 0                                 ; Parm#1 = Security, 0=None
call [CreateThread]                    ; Call WinAPI function
test eax,eax                           ; Status=0 means error
jz .exit                               ; Go if error creating thread
mov [esi + THCTRL.threadHandle],eax    ; Store thread handle in the per-thread list
;--- Setup affinity mask (include processor group) from NUMA to Thread Entry --;
; include special affinization for HT supported but not used
mov eax,[ebx + NUMACTRL.nodeAffinity]
mov ecx,eax
mov edx,[esp + 00]                        ; EDX = Pointer to IPB
cmp [edx + MEMIPB.updatedHT],HT_NOT_USED  ; Detect means HT supported but not used
jne .afterMasking                         ; Go skip masking for other variants
mov ecx,055555555h
and ecx,eax
jnz .afterMasking
mov ecx,eax                            ; Restore unmasked if masked to zero 
.afterMasking:
mov [esi + THCTRL.threadAffinity],ecx 
mov eax,[ebx + NUMACTRL.nodeGroup]
mov [esi + THCTRL.threadGroup],eax 
;--- Setup asm method subroutine entry point address from IPB to Thread Entry -;
; note used EDX from previous step 
mov eax,[edx + MEMIPB.patternRoutine]
mov [esi + THCTRL.entryPoint],eax
;---------- Setup base address for block1, block2 -----------------------------;
; note 2 blocks because block copy supported
; note used EDX from previous step
mov eax,[ebx + NUMACTRL.alignedBase]   ; EAX = base address allocated at node
add eax,[esp + 16]                     ; add offset
mov [esi + THCTRL.origBase],eax        ; store base address
mov ecx,[edx + MEMIPB.memoryPerThread] ; RCX = ALLOCATED size
add [esp + 16],ecx                     ; modify offset by this thread memory
shr ecx,1
mov [esi + THCTRL.base1],eax           ; Operational base 1 = base
add eax,ecx
mov [esi + THCTRL.base2],eax           ; Operational base 2 = base + size / 2
;--- Setup block size, bytes, this field must be updated per draw iteration ---;
mov [esi + THCTRL.sizeBytes],ecx       ; Operational block size, units = BYTES
;---------- Setup block size, instructions ------------------------------------; 
; this field must be updated per draw iteration
; note used EDX from previous step
mov ecx,[edx + MEMIPB.operandWidth]    ; ECX = bits per instruction
shr ecx,3                              ; ECX = bytes per instruction, BPI 
jz .error
mov eax,[edx + MEMIPB.startBlockSize]  ; here OPERATIONAL size
push edx
xor edx,edx
div ecx                          ; Instruction count = Work length / BPI
pop edx 
mov [esi + THCTRL.sizeInst],eax  ; Operational block size, units = INSTRUCTIONS
;---------- Setup Large Page option, 16-bit value -----------------------------;
; note used EDX from previous step
mov eax,[edx + MEMIPB.updatedLP]
mov [esi + THCTRL.largePages],ax
;--- Setup number of repeat measurement iterations, note copy 64-bit value ----;
; note used EDX from previous step
mov eax,dword [edx + MEMIPB.measureRepeats + 00]  ; Low 32 bit
mov dword [esi + THCTRL.repeats + 00],eax 
mov eax,dword [edx + MEMIPB.measureRepeats + 04]  ; High 32 bit
mov dword [esi + THCTRL.repeats + 04],eax 
;---------- Select next NUMA node if THREADS per NODES counter done -----------;
dec dword [esp + 08]     ; [esp + 08] = Threads per NUMA node ratio
jnz .afterCountNode      ; Go skip reload if continue threads per this node
mov eax,[esp + 12]       ; Reload threads per this node
mov [esp + 08],eax
mov dword [esp + 16],0   ; Blank block offset because next NUMA node
add ebx,NUMACTRL_SIZE    ; EBX = NUMA nodes entries pointer
.afterCountNode:
;---------- Cycle for Threads -------------------------------------------------;
add esi,THCTRL_SIZE     ; ESI  = Threads entries pointer
inc ebp                 ; EBP = Threads counter for Thread ID
dec dword [esp + 04]    ; [esp + 04] = Threads counter for cycle
jnz .threadsBuild
;---------- Exit points -------------------------------------------------------;
clc                     ; CF = 0 means status = no errors 
.exit:
pop eax eax eax eax eax ebp edi esi ebx  ; note pop eax for remove local variables
ret
.error:
stc                     ; CF = 1 means status = error detected
jmp .exit
