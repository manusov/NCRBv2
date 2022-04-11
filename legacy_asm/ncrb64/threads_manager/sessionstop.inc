;--- Stop measurement session -------------------------------------------------;
; Release allocated memory and resources handles when session stops            ;
; Main actions is:                                                             ;
;  1) Release threads list: handles for events and threads.                    ;
;  2) Release memory, allocated at NUMA nodes.                                 ;
; See complementar routines: sessionstart.inc , sessionprogress.inc.           ; 
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
SessionStop:
lea rdi,[MEM_IPB]      ; RDI = Pointer to IPB (benchmark scenario)
;---------- Threads stop, release handles for Threads and Events --------------;
lea rcx,[THREAD_LIST]                   ; RCX = Pointer to threads list
mov edx,[rdi + MEMIPB.updatedThreads]   ; EDX = Threads count
call ReleaseThreadsList
mov al,MSG_RUNTIME_RELEASE              ; AL = error status, valid if CF=1 only
jc .exit                                ; Go if error
;---------- NUMA-aware or unaware memory release ------------------------------;
lea rcx,[NUMA_LIST]                     ; RCX = Pointer to NUMA nodes list
mov edx,[rdi + MEMIPB.domainsCount]     ; EDX = NUMA domains count
call MemReleaseNuma
mov al,MSG_RUNTIME_RELEASE              ; AL = error status, valid if CF=1 only
;---------- Done, return CF=Error flag, AL=Error code ( valid if CF=1 ) -------;
.exit:
ret
