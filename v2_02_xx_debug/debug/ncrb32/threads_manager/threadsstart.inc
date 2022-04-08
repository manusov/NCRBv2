;---------- Start threads, this subroutine for first start --------------------;
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
ThreadsStart:            ; This entry point for measure
mov [MEM_IPB.threadStop],0
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
mov ebx,edi              ; EBX = Store number of threads
;---------- Cycle for run (resume) threads ------------------------------------;
.runThreads:
mov ecx,[esi + THCTRL.threadHandle]  ; ECX = Load THREAD handle
jecxz ErrorThreads                   ; Error if this entry not allocated
push ecx                             ; Parm#1 = Load THREAD handle
call [ResumeThread]                  ; Call WinAPI function
cmp eax,-1                           ; Status=-1 means error
mov eax,0                            ; EAX=0, can't XOR EAX,EAX, must save flags
je ErrorThreads                      ; Go exit if error
;---------- Cycle -------------------------------------------------------------;
add esi,THCTRL_SIZE      ; Select next thread entry
dec edi                  ; Count threads
jnz .runThreads          ; Cycle for all entries
;---------- This point also referenced by Re-Start subroutine -----------------;
WaitThreads:
;---------- Wait for N threads executed, count at EBX -------------------------;
push 60000               ; Parm#4 = Timeout, milliseconds                
push 1                   ; Parm#3 = Wait All flag
push ebp                 ; Parm#2 = Pointer to events list
push ebx                 ; Parm#1 = Number of objects (threads)
call [WaitForMultipleObjects]  ; Call WinAPI function
test eax,eax                   ; Status=0 means wait OK
mov eax,-1
jnz ErrorThreads
;---------- Serializing and get TSC, measurement interval end -----------------; 
xor eax,eax              ; CPUID function 0
cpuid                    ; This CPUID for serializing only, results ignored
rdtsc                    ; Get TSC to EDX=High, EAX=Low
sub eax,[esp + 00]
sbb edx,[esp + 04]
mov [esp + 00],eax
mov [esp + 04],edx
;---------- Exit points -------------------------------------------------------;
; This points also referenced by Re-Start subroutine
clc                   ; CF=0 means status = no errors
ExitThreads:
pop  eax edx ebp edi esi ebx
ret
ErrorThreads:
stc                   ; CF=1 means status = error detected
jmp ExitThreads
