;---------- Close handles at threads list: events and threads -----------------;
;           Before close handles, release threads                              ;
;                                                                              ;
; INPUT:   ECX = Pointer to Threads Control List                               ;
;          EDX = Number of threads                                             ;
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC) = no errors, 1(C) = error                    ;
;                                                                              ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.        ;
;------------------------------------------------------------------------------;
ReleaseThreadsList:
cld
push ebx esi edi ebp 
;---------- Reload input parameters to non-volatile registers -----------------;
mov esi,ecx                    ; ESI = Pointer to Threads list
mov edi,edx                    ; EDI = Number of used Threads
;---------- Set nodes list limit ----------------------------------------------;
mov ebx,MAX_THREADS            ; EBX = Maximum number of Threads
;---------- Terminate threads routines ----------------------------------------;
call ThreadsReStartBreak
;---------- Cycle for release threads -----------------------------------------;
.releaseThreads:
;--- Wait threads termination by threads handle, in the cycle for all threads -;
mov ecx,[esi + THCTRL.threadHandle]  ; ECX = = THREAD handle
jecxz @f                             ; Skip if this entry not allocated
push THREAD_TIME_LIMIT               ; Parm#2 = Time Limit
push ecx                             ; Parm#1 = THREAD handle
call [WaitForSingleObject]
test eax,eax                         ; Status=0 means wait OK, special encoding
jnz .error                           ; Go exit if error 
@@:
;---------- Terminate threads -------------------------------------------------;
mov ecx,[esi + THCTRL.threadHandle]  ; ECX = THREAD handle
jecxz @f                             ; Skip if this entry not allocated
push 0                               ; Parm#2 = Thread exit code
push ecx                             ; Parm#1 = THREAD handle 
call [TerminateThread]               ; Call WinAPI function
test eax,eax                         ; Status=0 means error
jz .error                            ; Go exit if error
@@:
;---------- Close Start Events ------------------------------------------------;
mov ecx,[esi + THCTRL.eventStart]    ; ECX = Load EVENT handle
jecxz @f                             ; Skip if this entry not allocated
push ecx                             ; Parm#1 = EVENT handle 
call [CloseHandle]                   ; Call WinAPI function
test eax,eax                         ; Status=0 means error
jz .error                            ; Go exit if error 
@@:
;---------- Close Done Events -------------------------------------------------;
mov ecx,[esi + THCTRL.eventDone]     ; ECX = Load EVENT handle
jecxz @f                             ; Skip if this entry not allocated
push ecx                             ; Parm#1 = EVENT handle
call [CloseHandle]                   ; Call WinAPI function
test eax,eax                         ; Status=0 means error
jz .error                            ; Go exit if error 
@@:
;---------- Close Threads -----------------------------------------------------;
mov ecx,[esi + THCTRL.threadHandle]  ; ECX = Load THREAD handle
jecxz @f                             ; Skip if this entry not allocated
push ecx                             ; Parm#1 = EVENT handle
call [CloseHandle]                   ; Call WinAPI function
test eax,eax                         ; Status = 0 means error
jz .error                            ; Go exit if error
@@:
;---------- Cycle for release threads -----------------------------------------;
dec edi                             ; Count threads entries, for used entries
jz .done                            ; Go exit if used threads done before limit
;---------- Limit cycle for maximum number of threads -------------------------;
add esi,THCTRL_SIZE                 ; Select next thread entry
dec ebx                             ; Count threads entries, for limit
jnz .releaseThreads                 ; Cycle for all entries
;---------- Exit points -------------------------------------------------------;
.done:
clc                  ; CF = 0 means status = no errors 
.exit:
pop ebp edi esi ebx
ret
.error:
stc                  ; CF = 1 means status = error detected
jmp .exit
