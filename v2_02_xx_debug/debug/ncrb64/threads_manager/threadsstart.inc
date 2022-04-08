;---------- Start threads, this subroutine for first start --------------------;
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
ThreadsStart:                  ; This entry point for measure
;---------- Clear stop flag ---------------------------------------------------;
mov [MEM_IPB.threadStop],0
;---------- Entry procedure ---------------------------------------------------;
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
xor eax,eax                  ; CPUID function 0
cpuid                        ; This CPUID for serializing only, results ignored
rdtsc                        ; Get TSC to EDX=High, EAX=Low
shl rdx,32
lea r12,[rax + rdx]          ; R12 = TSC value, 64-bit
;---------- Scan list of threads ----------------------------------------------;
lea r13,[rsi + THREAD_LIST_SIZE + EVENT_LIST_SIZE]  ; R13 = DONE events list
mov ebx,edi                  ; EBX = Store number of threads
;---------- Cycle for run ( resume ) threads ----------------------------------;
.runThreads:
mov rcx,[rsi + THCTRL.threadHandle]  ; RCX = Parm#1 = Load THREAD handle
jrcxz ErrorThreads                   ; Error if this entry not allocated
call [ResumeThread]                  ; Call WinAPI function
cmp rax,-1                           ; Status=-1 means error
mov eax,0                            ; RAX=0, can't XOR EAX,EAX, must save flags
je ErrorThreads                      ; Go exit if error
;---------- Cycle -------------------------------------------------------------;
add rsi,THCTRL_SIZE            ; Select next thread entry
dec edi                        ; Count threads
jnz .runThreads                ; Cycle for all entries
;---------- This point also referenced by Re-Start subroutine -----------------;
WaitThreads:
;---------- Wait for N threads executed, count at EBX -------------------------;
mov ecx,ebx                    ; RCX = Parm#1 = Number of objects ( threads )
mov rdx,r13                    ; RDX = Parm#2 = Pointer to events list
mov r8d,1                      ; R8  = Parm#3 = Wait All flag
mov r9d,60000                  ; R9  = Parm#4 = Timeout, milliseconds                
call [WaitForMultipleObjects]  ; Call WinAPI function
test rax,rax                   ; Status=0 means wait OK
mov rax,-1
jnz ErrorThreads
;---------- Serializing and get TSC, measurement interval end -----------------; 
xor eax,eax                  ; CPUID function 0
cpuid                        ; This CPUID for serializing only, results ignored
rdtsc                        ; Get TSC to EDX=High, EAX=Low
shl rdx,32
add rax,rdx                  ; RAX = TSC value, 64-bit
sub rax,r12                  ; RAX = Delta TSC = Current TSC - Start TSC
;---------- Exit points -------------------------------------------------------;
; This points also referenced by Re-Start subroutine
clc                   ; CF=0 means status = no errors
ExitThreads:
mov rsp,rbp           ; Restore RSP at subroutine entry, because stack alignment
pop r13 r12 rbp rdi rsi rbx
ret
ErrorThreads:
stc                   ; CF=1 means status = error detected
jmp ExitThreads
