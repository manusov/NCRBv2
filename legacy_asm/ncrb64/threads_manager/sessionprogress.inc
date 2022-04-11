;--- Execute measurement session ----------------------------------------------;
; Execute measurements (run target performance pattern),                       ; 
; use allocated memory and threads resources handles,                          ; 
; allocated when session starts (see sessionstart.inc).                        ;
; See complementar routines: sessionstart.inc , sessionstop.inc.               ;
;                                                                              ;
; INPUT:   None in the registers                                               ;
;          IPB = Memory and Threads allocation data, used for release          ;
;          Threads List, NUMA nodes list                                       ;
;                                                                              ;
; OUTPUT:  CF = Error flag, 0(NC) = No errors, 1(C) = Error detected.          ;
;          AL = Error code, valid if CF=1(C)                                   ;
;               00 = Memory allocation error                                   ;
;               01 = Memory release error                                      ;
;               02 = Time measurement error                                    ;
;               04 = Address arithmetics error                                 ;
;               04-0FFh = Reserved                                             ;
;          OPB = Benchmark results data                                        ;
;          Threads List, NUMA nodes list                                       ;
;                                                                              ;
; All CPU registers can be corrupted, this rule for complex modules:           ;
; directory threads_manager\:                                                  ;
;   sessionstart.inc, sessionstop.inc, sessionprogress.inc.                    ;
; directory gui\:                                                              ;
;   drawsessionstart.inc, drawsessionstop.inc                                  ;
;   but not drawsessionthread.inc because OS callback                          ;       
; Remember about PUSH-POP in the callers.                                      ;
;------------------------------------------------------------------------------;
SessionProgress:
;--- Stack alignment, required because OS API used, save number of threads ----;
mov rbp,rsp                    ; RBP = Save input RSP
and rsp,0FFFFFFFFFFFFFFF0h     ; Align stack because WinAPI used
sub rsp,32+16                  ; Build parameters shadow, 4 qwords
;---------- Setup pointers ----------------------------------------------------;
lea rsi,[MEM_IPB]   ; RSI = Pointer to IPB, benchmarks scenario
lea rdi,[MEM_OPB]   ; RDI = Pointer to OPB, benchmarks results
;---------- Pre-heat phase for result stabilization ---------------------------;
; Note this pre-heat for simple only, not use this for drawings
lea rcx,[THREAD_LIST]
mov edx,[rsi + MEMIPB.updatedThreads]
call ThreadsReStart
;---------- Replace fill pattern to walk pattern for latency mode -------------;
lea rcx,[THREAD_LIST]
cmp [rsi + MEMIPB.updatedAsm],LATENCY_MODE
jb .bwmode             ; Go skip this update if not a LATENCY MEASUREMENT mode
mov r8d,[rsi + MEMIPB.updatedThreads]   ; R8D = Threads count
mov r9,[rsi + MEMIPB.walkRoutine]       ; R9  = Pointer to pattern routine
call ThreadsUpdatePattern  ; replace list build pattern to list walk pattern
.bwmode:
;---------- Get start time point, by OS timer API ( alternative of TSC ) ------;
lea rcx,[rsp + 32]       ; RCX = Pointer to updated variable, OS timer, 64-bit
call [GetSystemTimeAsFileTime]
mov rbx,[rsp + 32]
.sync:
lea rcx,[rsp + 32]  ; RCX = Pointer to updated variable, OS timer, 64-bit
call [GetSystemTimeAsFileTime]
cmp rbx,[rsp + 32]
je .sync            ; Wait for next change for detect start of interval
;---------- Measurement pattern -----------------------------------------------;
lea rcx,[THREAD_LIST]                  ; RCX = Pointer to threads list
mov edx,[rsi + MEMIPB.updatedThreads]  ; EDX = number of threads
call ThreadsReStart
mov al,MSG_RUNTIME_TIMINGS             ; AL = error status, valid if CF=1 only
jc .exit
mov [rdi + MEMOPB.tscTimerDelta],rax   ; Store delta-TSC, units = TSC clocks
;---------- Get stop time point, by OS timer API ( alternative of TSC ) -------;
lea rcx,[rsp + 40]  ; RCX = Pointer to updated variable, OS timer, 64-bit
call [GetSystemTimeAsFileTime]
mov rax,[rsp + 40]  ; RAX = Time at this point
sub rax,[rsp + 32]  ; Subtract time before target fragment execution
mov [rdi + MEMOPB.osTimerDelta],rax    ; Store delta-OS timer, units = 100 ns 
;---------- Done --------------------------------------------------------------;
clc            ; CF=0 means no errors
.exit:         ; Can go to this point if CF=1 when errors detected
mov rsp,rbp
ret
