;---------- Restart threads, this subroutine for non-first runs ---------------;
;                                                                              ;
; INPUT:   ECX = Pointer to Threads Control List                               ;
;          EDX = Number of threads                                             ;
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC) = no errors, 1(C) = error                    ;
;          EDX:EAX = Delta TSC, valid if no errors, when CF=0                  ;  
;                                                                              ;
; Note.                                                                        ;
; Limit number of updated entries by input EDX (not all 256 threads) can       ;
; improve quality of measurements, because skip empty part of list.            ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.        ;
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
push ebx esi edi ebp
;---------- Reload parameters to non-volatile registers -----------------------;
mov esi,ecx              ; ESI = Control list pointer
mov edi,edx              ; EDI = Number of used threads
;---------- Serializing and get TSC, measurement interval start ---------------;
xor eax,eax              ; CPUID function 0
cpuid                    ; This CPUID for serializing only, results ignored
rdtsc                    ; Get TSC to EDX=High, EAX=Low
push edx eax             ; EDX:EAX as local variable
;---------- Scan list of threads ----------------------------------------------;
lea ebp,[esi + THREAD_LIST_SIZE + EVENT_LIST_SIZE]  ; EBP = DONE events list
mov ebx,edi                    ; EBX = Store number of threads
;---------- Restart threads by clear done events, set start events ------------;
.restartThreads:
mov ecx,[esi + THCTRL.eventDone]   ; ECX = Event handle
jecxz ErrorThreads                 ; Error if this entry not allocated
push ecx                           ; Parm#1 = Event handle 
call [ResetEvent]                  ; Reset event cause done signal clear
mov ecx,[esi + THCTRL.eventStart]  ; ECX = Event handle
jecxz ErrorThreads                 ; Error if this entry not allocated
push ecx                           ; Parm#1 = Event handle
call [SetEvent]                    ; Set event cause repeat benchmark call
;---------- Cycle -------------------------------------------------------------;
add esi,THCTRL_SIZE                ; Select next thread entry
dec edi                            ; Count threads
jnz .restartThreads                ; Cycle for all entries
;---------- Go to wait for execution, actual EBX = number of threads ----------;
jmp WaitThreads
