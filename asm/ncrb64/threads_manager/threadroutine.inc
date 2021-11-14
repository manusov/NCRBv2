;---------- Thread routine, called by OS callback mechanism -------------------;
; Variant without support Processor Groups, systems with <= 64 logical CPUs    ;
;                                                                              ;
; INPUT:   RCX = Parm#1 = Thread ID, 0-63                                      ;
;                                                                              ;
; OUTPUT:  RAX = Return code = 0                                               ;
;          Non-volatile regs. unchanged for Microsoft x64  calling convention  ;
;                                                                              ;
; This subroutine must be compliant Microsoft x64 calling convention,          ;
; include non-volatile requirements for XMM6-XMM15 (128 low bits only if YMM). ;
;------------------------------------------------------------------------------;
ThreadRoutineSingleGroup:  ; Entry thread with save CPU context and stack alignment
push rbx rsi rdi rbp r12 r13 r14 r15
mov rax,rsp
and rsp,0FFFFFFFFFFFFFFF0h
cmp [SYS_PARMS.sseSupported],0
je @f                            ; Skip this operation if SSE not supported
sub rsp,10*16                    ; 10 128-bit SSE registers XMM6-XMM15 must be
movaps [rsp + 16*0],xmm6         ; non-volatile by convention, note ALIGN16
movaps [rsp + 16*1],xmm7
movaps [rsp + 16*2],xmm8
movaps [rsp + 16*3],xmm9
movaps [rsp + 16*4],xmm10
movaps [rsp + 16*5],xmm11
movaps [rsp + 16*6],xmm12
movaps [rsp + 16*7],xmm13
movaps [rsp + 16*8],xmm14
movaps [rsp + 16*9],xmm15
@@:
push rax
sub rsp,32 + 8                ; Build parameters shadow, 4 qwords + alignment
;---------- Calculate thread control entry offset = f( thread id ) ------------;
lea rdx,[THREAD_LIST]
cmp ecx,MAX_THREADS_PER_GROUP
jb @f
xor ecx,ecx                   ; Force thread 0 if bad thread number
@@:
imul ecx,ecx,THCTRL_SIZE
lea r15,[rdx + rcx]           ; R15 = Address for block for this thread
;---------- This point for restart thread -------------------------------------;
.threadRepeat:
mov rbx,r15
;---------- Set affinity mask for this thread ---------------------------------;
mov rcx,[rbx + THCTRL.threadHandle]     ; Parm#1 = RCX = Thread handle
mov rdx,[rbx + THCTRL.threadAffinity]   ; Parm#2 = RDX = Thread Affinity mask
test rdx,rdx    ; Check for affinity mask = 0
jz @f           ; Skip set affinity mask if memory allocated without NUMA
call [DYNA_IMPORT._SetThreadAffinityMask]
@@:
;---------- Target memory performance operation -------------------------------;
call RunTarget
;---------- Pre-clear thread run event ----------------------------------------;
mov rcx,[r15 + THCTRL.eventStart]  ; RCX = Parm#1 = Handle for this thread
call [ResetEvent]                  ; Call WinAPI function
;---------- Signaling thread terminated ---------------------------------------;
mov rcx,[r15 + THCTRL.eventDone]   ; RCX = Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function
;---------- Wait for thread re-run --------------------------------------------;
mov rcx,[r15 + THCTRL.eventStart]  ; RCX = Parm#1 = Handle for this thread
mov edx,-1                         ; RDX = Parm#2 = Wait infinite, no timeout
call [WaitForSingleObject]
;---------- Repeat measurement iteration in this thread -----------------------;
cmp [MEM_IPB.threadStop],0
je .threadRepeat
;---------- Signaling thread terminated ---------------------------------------;
mov rcx,[r15 + THCTRL.eventDone]   ; RCX = Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function
;---------- Return from thread to OS caller -----------------------------------;
add rsp,32 + 8
pop rax
cmp [SYS_PARMS.sseSupported],0
jz @f                         ; Skip this operation if SSE not supported
movaps xmm6,[rsp + 16*0]
movaps xmm7,[rsp + 16*1]
movaps xmm8,[rsp + 16*2]
movaps xmm9,[rsp + 16*3]
movaps xmm10,[rsp + 16*4]
movaps xmm11,[rsp + 16*5]
movaps xmm12,[rsp + 16*6]
movaps xmm13,[rsp + 16*7]
movaps xmm14,[rsp + 16*8]
movaps xmm15,[rsp + 16*9]     ; 10 128-bit SSE registers XMM6-XMM15 must be
@@:                           ; non-volatile by convention, note ALIGN16
mov rsp,rax
pop r15 r14 r13 r12 rbp rdi rsi rbx
xor eax,eax                   ; RAX = 0, return code = 0 
ret
;---------- Thread routine, called by OS callback mechanism -------------------;
; Variant with support Processor Groups, systems with >64 logical CPUs         ;
;                                                                              ;
; INPUT:   RCX = Parm#1 = Thread ID, 0-255                                     ;
;                                                                              ;
; OUTPUT:  RAX = Return code = 0                                               ;
;          Non-volatile regs. unchanged for Microsoft x64  calling convention  ;
;                                                                              ;
; This subroutine must be compliant Microsoft x64 calling convention,          ;
; include non-volatile requirements for XMM6-XMM15 (128 low bits only if YMM). ;
;------------------------------------------------------------------------------;

ThreadRoutineMultiGroup:  ; Entry thread with save CPU context and stack alignment
push rbx rsi rdi rbp r12 r13 r14 r15
mov rax,rsp
and rsp,0FFFFFFFFFFFFFFF0h
cmp [SYS_PARMS.sseSupported],0
je @f                         ; Skip this operation if SSE not supported
sub rsp,10*16                 ; 10 128-bit SSE registers XMM6-XMM15 must be
movaps [rsp + 16*0],xmm6      ; non-volatile by convention, note ALIGN16
movaps [rsp + 16*1],xmm7
movaps [rsp + 16*2],xmm8
movaps [rsp + 16*3],xmm9
movaps [rsp + 16*4],xmm10
movaps [rsp + 16*5],xmm11
movaps [rsp + 16*6],xmm12
movaps [rsp + 16*7],xmm13
movaps [rsp + 16*8],xmm14
movaps [rsp + 16*9],xmm15
@@:
push rax
sub rsp,32 + 8                ; Build parameters shadow, 4 qwords + alignment
;---------- Calculate thread control entry offset = f( thread id ) ------------; 
lea rdx,[THREAD_LIST]
cmp ecx,MAX_THREADS
jb @f
xor ecx,ecx                   ; Force thread 0 if bad thread number
@@:
imul ecx,ecx,THCTRL_SIZE
lea r15,[rdx + rcx]           ; R15 = Address for block for this thread
;---------- This point for restart thread -------------------------------------;
.threadRepeat:
mov rbx,r15
;---------- Set affinity mask for this thread ---------------------------------;
mov rcx,[rbx + THCTRL.threadHandle]     ; Parm#1 = RCX = Thread handle
lea rdx,[rbx + THCTRL.threadAffinity]   ; Parm#2 = RDX = Pointer to mask + group
lea r8,[rbx + THCTRL.origAffinity]      ; Parm#3 = R8 = Pointer to get original
cmp qword [rdx],0    ; Check for affinity mask = 0
je @f                ; Skip set affinity mask if memory allocated without NUMA
call [DYNA_IMPORT._SetThreadGroupAffinity]  ; This changed for Processor Groups
@@:
;---------- Target memory performance operation -------------------------------;
call RunTarget
;---------- Pre-clear thread run event ----------------------------------------;
mov rcx,[r15 + THCTRL.eventStart]  ; RCX = Parm#1 = Handle for this thread
call [ResetEvent]                  ; Call WinAPI function
;---------- Signaling thread terminated ---------------------------------------;
mov rcx,[r15 + THCTRL.eventDone]   ; RCX = Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function
;---------- Wait for thread re-run --------------------------------------------;
mov rcx,[r15 + THCTRL.eventStart]  ; RCX = Parm#1 = Handle for this thread
mov edx,-1                         ; RDX = Parm#2 = Wait infinite, no timeout
call [WaitForSingleObject]
;---------- Repeat measurement iteration in this thread -----------------------;
cmp [MEM_IPB.threadStop],0
je .threadRepeat
;---------- Signaling thread terminated ---------------------------------------;
mov rcx,[r15 + THCTRL.eventDone]   ; RCX = Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function
;---------- Return from thread to OS caller -----------------------------------;
add rsp,32 + 8
pop rax
cmp [SYS_PARMS.sseSupported],0
je @f                         ; Skip this operation if SSE not supported
movaps xmm6,[rsp + 16*0]
movaps xmm7,[rsp + 16*1]
movaps xmm8,[rsp + 16*2]
movaps xmm9,[rsp + 16*3]
movaps xmm10,[rsp + 16*4]
movaps xmm11,[rsp + 16*5]
movaps xmm12,[rsp + 16*6]
movaps xmm13,[rsp + 16*7]
movaps xmm14,[rsp + 16*8]
movaps xmm15,[rsp + 16*9]     ; 10 128-bit SSE registers XMM6-XMM15 must be
@@:                           ; non-volatile by convention, note ALIGN16
mov rsp,rax
pop r15 r14 r13 r12 rbp rdi rsi rbx
xor eax,eax                   ; RAX = 0, return code = 0
ret
