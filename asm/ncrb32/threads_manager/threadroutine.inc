;---------- Thread routine, called by OS callback mechanism -------------------;
; Variant without support Processor Groups, systems with <= 64 logical CPUs    ;
;                                                                              ;
; INPUT:   [esp + 04] = Parm#1 = Thread ID, 0-63                               ;
; OUTPUT:  EAX = Return code = 0                                               ;
;          Non-volatile regs. unchanged for Microsoft ia32 calling convention  ;
;                                                                              ;
; This subroutine must be compliant Microsoft ia32 calling convention.         ;
;------------------------------------------------------------------------------;
ThreadRoutineSingleGroup:
push ebx esi edi ebp          ; Note about this 16 bytes when read parameter
;---------- Calculate thread control entry offset = f( thread id ) ------------;
lea edx,[THREAD_LIST]
mov ecx,[esp + 4 + 16]        ; Read first parameter by IA32 convention
cmp ecx,MAX_THREADS_PER_GROUP
jb @f
xor ecx,ecx                   ; Force thread 0 if bad thread number
@@:
imul ecx,ecx,THCTRL_SIZE
lea ebx,[edx + ecx]           ; EBX = Address for block for this thread
;---------- This point for restart thread -------------------------------------;
.threadRepeat:
;---------- Set affinity mask for this thread ---------------------------------;
mov eax,[ebx + THCTRL.threadAffinity]   ; EAX = Thread Affinity mask
test eax,eax    ; Check for affinity mask = 0
jz @f           ; Skip set affinity mask if memory allocated without NUMA
push eax                                ; Parm#2 = Thread Affinity mask
push [ebx + THCTRL.threadHandle]        ; Parm#1 = Thread handle
call [DYNA_IMPORT._SetThreadAffinityMask]
@@:
;---------- Target memory performance operation -------------------------------;
push ebx
call RunTarget
pop ebx
;---------- Pre-clear thread run event ----------------------------------------;
push [ebx + THCTRL.eventStart]     ; Parm#1 = Handle for this thread
call [ResetEvent]                  ; Call WinAPI function
;---------- Signaling thread terminated ---------------------------------------;
push [ebx + THCTRL.eventDone]      ; Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function
;---------- Wait for thread re-run --------------------------------------------;
push -1                            ; Parm#2 = Wait infinite, no timeout
push [ebx + THCTRL.eventStart]     ; Parm#1 = Handle for this thread
call [WaitForSingleObject]
;---------- Repeat measurement iteration in this thread -----------------------;
cmp [MEM_IPB.threadStop],0
je .threadRepeat
;---------- Signaling thread terminated ---------------------------------------;
push [ebx + THCTRL.eventDone]      ; Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function
;---------- Return from thread to OS caller -----------------------------------;
pop ebp edi esi ebx
xor eax,eax                        ; EAX = 0, return code = 0
ret 4 
;---------- Thread routine, called by OS callback mechanism -------------------;
; Variant with support Processor Groups, systems with >64 logical CPUs         ;
;                                                                              ;
; INPUT:   [esp + 04] = Parm#1 = Thread ID, 0-255                              ;
; OUTPUT:  EAX = Return code = 0                                               ;
;          Non-volatile regs. unchanged for Microsoft ia32 calling convention  ;
;                                                                              ;
; This subroutine must be compliant Microsoft ia32 calling convention.         ;
;------------------------------------------------------------------------------;
ThreadRoutineMultiGroup:
push ebx esi edi ebp          ; Note about this 16 bytes when read parameter
;---------- Calculate thread control entry offset = f( thread id ) ------------;
lea edx,[THREAD_LIST]
mov ecx,[esp + 4 + 16]        ; Read first parameter by IA32 convention
cmp ecx,MAX_THREADS_PER_GROUP
jb @f
xor ecx,ecx                   ; Force thread 0 if bad thread number
@@:
imul ecx,ecx,THCTRL_SIZE
lea ebx,[edx + ecx]           ; EBX = Address for block for this thread
;---------- This point for restart thread -------------------------------------;
.threadRepeat:
;---------- Set affinity mask for this thread ---------------------------------;
lea eax,[ebx + THCTRL.threadAffinity]    ; EAX = Pointer to thread affinity
cmp dword [eax],0                        ; Check for affinity mask = 0
je @f                 ; Skip set affinity mask if memory allocated without NUMA
lea ecx,[ebx + THCTRL.origAffinity]
push ecx                             ; Parm#3 = Pointer to get original 
push eax                             ; Parm#2 = Pointer to mask + group
push [ebx + THCTRL.threadHandle]     ; Parm#1 = Thread handle
call [DYNA_IMPORT._SetThreadGroupAffinity]   ; This changed for Processor Groups
@@:
;---------- Target memory performance operation -------------------------------;
push ebx
call RunTarget
pop ebx
;---------- Pre-clear thread run event ----------------------------------------;
push [ebx + THCTRL.eventStart]     ; Parm#1 = Handle for this thread
call [ResetEvent]                  ; Call WinAPI function
;---------- Signaling thread terminated ---------------------------------------;
push [ebx + THCTRL.eventDone]      ; Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function
;---------- Wait for thread re-run --------------------------------------------;
push -1                            ; Parm#2 = Wait infinite, no timeout
push [ebx + THCTRL.eventStart]     ; Parm#1 = Handle for this thread
call [WaitForSingleObject]
;---------- Repeat measurement iteration in this thread -----------------------;
cmp [MEM_IPB.threadStop],0
je .threadRepeat
;---------- Signaling thread terminated ---------------------------------------;
push [ebx + THCTRL.eventDone]      ; Parm#1 = Handle for this thread
call [SetEvent]                    ; Call WinAPI function
;---------- Return from thread to OS caller -----------------------------------;
pop ebp edi esi ebx
xor eax,eax                        ; EAX = 0, return code = 0
ret 4 
