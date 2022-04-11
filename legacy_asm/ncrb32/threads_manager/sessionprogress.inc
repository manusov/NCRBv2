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
push 0 0
;---------- Setup pointers ----------------------------------------------------;
lea esi,[MEM_IPB]        ; ESI = Pointer to IPB, benchmarks scenario
lea edi,[MEM_OPB]        ; EDI = Pointer to OPB, benchmarks results
;---------- Pre-heat phase for result stabilization ---------------------------;
; Note this pre-heat for simple only, not use this for drawings
lea ecx,[THREAD_LIST]    ; ECX = Pointer to threads list
mov edx,[esi + MEMIPB.updatedThreads]
call ThreadsReStart
;---------- Replace fill pattern to walk pattern for latency mode -------------;
lea ebx,[THREAD_LIST]
cmp [esi + MEMIPB.updatedAsm],LATENCY_MODE
jb .skip                ; Go skip this update if not a LATENCY MEASUREMENT mode
mov ecx,[esi + MEMIPB.updatedThreads]   ; ECX = Threads count
mov eax,[esi + MEMIPB.walkRoutine]      ; EAX  = Pointer to pattern routine
call ThreadsUpdatePattern  ; replace list build pattern to list walk pattern
.skip:
;---------- Get start time point, by OS timer API (alternative of TSC) --------;
mov eax,esp        ; EAX = Pointer to updated variable, OS timer, 64-bit
push eax           ; Parm#1 = Pointer to updated variable, OS timer, 64-bit 
call [GetSystemTimeAsFileTime]
mov ebx,[esp]
.sync:
mov eax,esp
push eax
call [GetSystemTimeAsFileTime]
cmp ebx,[esp]
je .sync          ; Wait for next change for detect start of interval
;---------- Measurement pattern -----------------------------------------------;
lea ecx,[THREAD_LIST]                   ; ECX = Pointer to threads list
mov edx,[esi + MEMIPB.updatedThreads]   ; EDX = number of threads
call ThreadsReStart
mov al,MSG_RUNTIME_TIMINGS              ; AL = error status, valid if CF=1 only
jc .exit
mov dword [edi + MEMOPB.tscTimerDelta + 0],eax   ; Store delta-TSC, units = TSC clocks
mov dword [edi + MEMOPB.tscTimerDelta + 4],edx
;---------- Get stop time point, by OS timer API ( alternative of TSC ) -------;
lea eax,[edi + MEMOPB.osTimerDelta]  ; EAX = Pointer to updated variable, OS timer, 64-bit
push eax                             ; Parm#1 = Pointer to updated variable, OS timer, 64-bit 
call [GetSystemTimeAsFileTime]
mov eax,[esp + 0]  ; EDX:EAX = Time at this point
mov edx,[esp + 4]
sub dword [edi + MEMOPB.osTimerDelta + 0],eax    ; Calculate delta-OS timer, units = 100 ns 
sbb dword [edi + MEMOPB.osTimerDelta + 4],edx
;---------- Done --------------------------------------------------------------;
clc            ; CF=0 means no errors
.exit:         ; Can go to this point if CF=1 when errors detected
pop ecx ecx    ; Note POP for remove stack variable 
ret
