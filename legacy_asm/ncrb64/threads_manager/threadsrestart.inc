;---------- Restart threads, this subroutine for non-first runs ---------------;
;                                                                              ;
; INPUT:   RCX = Pointer to Threads Control List                               ;
;          EDX = Number of threads                                             ;
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC)=no errors, 1(C)=error                        ;
;          RAX = Delta TSC, valid if no errors, when CF=0                      ;  
;                                                                              ;
; Note.                                                                        ;
; Limit number of updated entries by input EDX (not all 256 threads) can       ;
; improve quality of measurements, because skip empty part of list.            ;
; Can destroy registers, volatile by Microsoft x64 calling convention.         ;
;------------------------------------------------------------------------------;
;---------- This entry point for return from thread subroutine ----------------;
ThreadsReStartBreak:
mov [MEM_IPB.threadStop],1
jmp EntryThreadsReStart 
;---------- This entry point for measure --------------------------------------;
ThreadsReStart:
mov [MEM_IPB.threadStop],0
;---------- Entry procedure ---------------------------------------------------;
EntryThreadsReStart:
cld
push rbx rsi rdi rbp r12 r13
;--- Stack alignment, required because OS API used, save number of threads ----;
mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
sub rsp,32                     ; Build parameters shadow, 4 qwords
;---------- Reload parameters to non-volatile registers -----------------------;
mov rsi,rcx                    ; RSI = Control list pointer
mov edi,edx                    ; EDI = Number of used threads
;---------- Serializing and get TSC, measurement interval start ---------------;
xor eax,eax
cpuid                          ; This for serializing only
rdtsc                          ; Get TSC to EDX=High, EAX=Low
shl rdx,32
lea r12,[rax + rdx]            ; R12 = TSC value, 64-bit
;---------- Scan list of threads ----------------------------------------------;
lea r13,[rsi + THREAD_LIST_SIZE + EVENT_LIST_SIZE]  ; R13 = DONE events list
mov ebx,edi                    ; EBX = Store number of threads
;---------- Restart threads by clear done events, set start events ------------;
.restartThreads:
mov rcx,[rsi + THCTRL.eventDone]   ; RCX = Parm#1 = Event handle
jrcxz ErrorThreads                 ; Error if this entry not allocated
call [ResetEvent]                  ; Reset event cause done signal clear
mov rcx,[rsi + THCTRL.eventStart]  ; RCX = Parm#1 = Event handle
jrcxz ErrorThreads                 ; Error if this entry not allocated
call [SetEvent]                    ; Set event cause repeat benchmark call
;---------- Cycle -------------------------------------------------------------;
add rsi,THCTRL_SIZE                ; Select next thread entry
dec edi                            ; Count threads
jnz .restartThreads                ; Cycle for all entries
;---------- Go to wait for execution, actual EBX = number of threads ----------;
jmp WaitThreads
