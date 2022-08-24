;--- Initializing and start measurement and drawings session, -----------------;
;    used for draw performance = f( block size ) function.                     ;
;    Include run  DrawSessionThread  subroutine as parallel thread             ;
;                                                                              ;
; INPUT:   None at registers                                                   ;
;          DialogueContent with user options settings data                     ;
;                                                                              ;
; OUTPUT:  CF = status flag, 0(NC)=No errors, 1(C)=Error                       ;
;          AL = error status, valid only if error detected (when CF=1)         ;
;          Status decode:                                                      ;
;          see file "ncrb64.asm", application runtime errors identifiers       ;
;          see file "threads_manager\sessionstart.inc"                         ;
;                                                                              ;
;          UPB = Update user parameters block with GUI widgets settings        ;
;          IPB = Update input parameters block with benchmark scenario data    ;   
;                                                                              ;
; UPB = f (GUI) , IPB = f (UPB, System Info)                                   ;
;                                                                              ;
; All CPU registers can be corrupted, this rule for complex modules:           ;
; directory threads_manager\:                                                  ;
;   sessionstart.inc, sessionstop.inc, sessionprogress.inc.                    ;
; directory gui\:                                                              ;
;   drawsessionstart.inc, drawsessionstop.inc                                  ;
;   but not drawsessionthread.inc because OS callback                          ;       
; Remember about PUSH-POP in the callers.                                      ;
;------------------------------------------------------------------------------;

; TODO. PRE-BLANK RESULTS, OTHERWISE CAN SHOW PREVIOUS RESULTS WHEN ERROR.
; TODO. RUNTIME ERROR REPORTING.
; TODO. CPU UTILIZATION AFTER DRAW CHART DONE.

DrawSessionStart:
push rbx rsi rdi rbp r12 r13 r14 r15
cld
call InfoProcessor
mov al,MSG_RUNTIME_TIMINGS
jc .exit
mov al,1  ; Run context = drawings, with memory allocation multiplication
call GuiToUserParms   ; Update UPB
call SessionStart     ; Initializing, allocate events, threads, memory, IPB
jc .exit              ; Go if error returned, AL = Error code
;---------- Initializing pointers ---------------------------------------------;
lea rbx,[MEM_IPB]
lea r14,[BIND_LIST]
;---------- Down comments strings, assembler method ---------------------------;
lea rdi,[r14 + BINDLIST.mdAsm]
mov eax,[rbx + MEMIPB.updatedAsm]
mov cx,STR_AM_X64_00
cmp [rbx + MEMIPB.nonTemporalMode],0
je @f
mov cx,STR_AM_NT_00
@@:
add ax,cx
call PoolStringWrite
mov al,0
stosb
;--- Down comments strings, prefetch distance ( default, medium, long, N/A ) --;
lea rdi,[r14 + BINDLIST.mdPrefetch]
mov eax,[rbx + MEMIPB.updatedDistance]
add ax,STR_MD_PD_NOT_USED
call PoolStringWrite
mov al,0
stosb
;---------- Down comments strings, target object (Cache, DRAM, Custom block) --;
lea rdi,[r14 + BINDLIST.mdObject]
mov eax,[rbx + MEMIPB.updatedTarget]
add ax,STR_MEMORY_L1
cmp ax,STR_MEMORY_CUSTOM
jne @f
mov ax,STR_MEMORY_CUSTOM_DRAW
@@:
call PoolStringWrite
mov al,0
stosb
;---------- Down comments strings, number of threads --------------------------;
lea rdi,[r14 + BINDLIST.mdThreads]
mov ax,STR_MD_THREADS
call PoolStringWrite
push rbx
mov eax,[rbx + MEMIPB.updatedThreads]
mov bl,0
call DecimalPrint32
pop rbx
mov eax,[rbx + MEMIPB.updatedHybrid]
add ax,STR_MD_NO_HYBRID
call PoolStringWrite
mov al,0
stosb
;---------- Down comments strings, number of measurement repeats --------------;
;---------- Special strings for adaptive repeats modes ------------------------;
; TODO. REPEATS WRITE.
; TODO. MEASUREMENT MODE WRITE.
lea rdi,[r14 + BINDLIST.mdRepeats]
mov eax,[rbx + MEMIPB.updatedMeasure]
add ax,STR_MEASURE_BRIEF
call PoolStringWrite
mov al,0
stosb
;---------- Down comments strings, normal or large pages ----------------------;
lea rdi,[r14 + BINDLIST.mdPages]
mov eax,[rbx + MEMIPB.updatedLP]
add ax,STR_MD_LP_NOT_USED
call PoolStringWrite
mov al,0
stosb
;---------- Down comments strings, NUMA optimization mode ---------------------;
lea rdi,[r14 + BINDLIST.mdNuma]
mov eax,[rbx + MEMIPB.updatedNUMA]
add ax,STR_MD_NUMA_OFF
call PoolStringWrite
mov al,0
stosb
;--- Support option, actual for drawings only: adaptive measurement modes -----;
lea rsi,[ADAPTIVE_REPEATS]
mov eax,[rbx + MEMIPB.updatedMeasure]
sub eax,MEASURE_B_ADAPTIVE
jc .skipAdaptive
cmp eax,1
ja .skipAdaptive
mov rax,[rsi + rax*8]
mov [rbx + MEMIPB.adaptiveSeconds],rax
;---------- Select Bandwidth or Latency measurement mode ----------------------;
lea rsi,[THREAD_LIST]  ; RSI = storage for RCX reloads
mov rcx,rsi
cmp [rbx + MEMIPB.updatedAsm],LATENCY_MODE
jae .latencyAdaptive
;---------- Run with default recommended repeats count: Bandwidth mode --------;
mov edx,[rbx + MEMIPB.updatedThreads]
call ThreadsReStart
jc .errorTimings             ; If error, go to error handling branch
jmp .calcAdaptive
;---------- Run with default recommended repeats count: Latency mode ----------;
; Setup pattern for prepare walk list
.latencyAdaptive:
mov r8d,[rbx + MEMIPB.updatedThreads]
mov r9,[rbx + MEMIPB.patternRoutine]    ; for first call, prepare list for walk
call ThreadsUpdatePattern
;---------- Run service pattern, prepare walk list, delta TSC ignored here ----;
mov rcx,rsi
mov edx,[rbx + MEMIPB.updatedThreads]
call ThreadsReStart
jc .errorTimings             ; If error, go to error handling branch
;---------- Setup pattern for walk --------------------------------------------;
mov rcx,rsi
mov r8d,[rbx + MEMIPB.updatedThreads]
mov r9,[rbx + MEMIPB.walkRoutine]       ; for second call, walk at prepared list
call ThreadsUpdatePattern
;--------- Run measurement pattern --------------------------------------------;
; walk by prepared walk list, get delta TSC in the RAX
mov rcx,rsi
mov edx,[rbx + MEMIPB.updatedThreads]
call ThreadsReStart
jc .errorTimings                    ; If error, go to error handling branch
;--------- Done point for adaptive modes ( bandwidth and latency branches ) ---;
; Pre-measurements executed, 
; next, test control = f ( results ) must be calculated
.calcAdaptive:
;---------- Calculate timings -------------------------------------------------;
; Calculate seconds and ratio after run with default recommended repeats count
; for measurements calibration 
push rax                            ; RAX = Delta TSC after ThreadsReStart
finit
mov rcx,[CPU_DATA.tscClockHz]
mov [MEM_OPB.tscFrequencyHz],rcx
fld1
fild [MEM_OPB.tscFrequencyHz]  
fdivp st1,st0
fstp [MEM_OPB.tscPeriodS]
push 1000000000
fild qword [rsp]
fild [MEM_OPB.tscFrequencyHz]  
fdivp st1,st0
fstp [MEM_OPB.tscPeriodNs]
pop rax
push 1000000
fild [MEM_OPB.tscFrequencyHz]
fild qword [rsp]
fdivp st1,st0
fstp [MEM_OPB.tscFrequencyMHz]
pop rax
fld [rbx + MEMIPB.adaptiveSeconds]    ; ST0 = Target seconds
fild qword [rsp]                      ; ST0 = Measured delta TSC , ST1 = Target seconds
fmul [MEM_OPB.tscPeriodS]             ; ST0 = Measured seconds
fdivp st1,st0                         ; ST0 = Target / Measured 
fild [rbx + MEMIPB.measureRepeats]    ; ST0 = Product repeats
fmulp st1,st0                         ; ST0 = Corrected product
fild [rbx + MEMIPB.startBlockSize]
fmulp st1,st0                         ; ST0 = Corrected product for divide per bytes count 
fistp [rbx + MEMIPB.adaptiveProduct]
pop rax                               ; This for release stack only
;---------- Skip point for non-adaptive modes ---------------------------------;
.skipAdaptive:
;---------- Note option: X-approximation modes --------------------------------; 
; interpreted and supported at drawsessionthread.inc module. 
; Load pointer to drawings parameters
lea rdi,[DRAW_PARMS]
;---------- Calculate X grid step values --------------------------------------; 
; must be integer power of 2 for next step correct
; Note this operation increase memory size requirements, because increase RAX
mov rax,[rbx + MEMIPB.startBlockSize]
bsr rcx,rax
mov eax,1
shl eax,cl   ; RAX = Target block size, note example here RAX=8K for L1=32K
;---------- Special support for DRAM and Custom modes -------------------------;
mov ecx,[rbx + MEMIPB.updatedTarget]
cmp ecx,TARGET_DRAM
je .slowObject          ; Go for size / 8 if target = DRAM
cmp ecx,TARGET_CUSTOM
jne .fastObject         ; Go skip without size / 8 if target = not a Custom
.slowObject:
shr eax,3               ; size / 8 , for DRAM and Custom modes
.fastObject:
;---------- Set Xstart, Xdelta for drawings -----------------------------------;
push rax                ; RAX = Block size per X-grid
xor edx,edx             ; RDX = Dividend high 64 bits, =0
mov ecx,GRIDSTEPX       ; RCX = Pixels per X-grid
div rcx                 ; (Block size per grid) / (pixels per grid)
mov [rbx + MEMIPB.startBlockSize],rax  ; X start value
mov [rbx + MEMIPB.deltaBlockSize],rax  ; X addend per draw iteration 
pop rax
;---------- Update and store X grid step values -------------------------------;
; Select and store X grid units: Bytes, Kilobytes, Megabytes
xor edx,edx             ; EDX = SelectUnits, 0=Bytes / 1=KB / 2=MB
cmp eax,1024
jb @f                   ; Go with Units=Bytes if grid step < 1 KB  
inc edx
shr eax,10
cmp eax,1024
jb @f                   ; Go with Units=KB if grid step < 1 MB
inc edx
shr eax,10
@@:                     ; Otherwise Units=MB
mov [rdi + DRPM.valueGridX],eax
mov [rdi + DRPM.selectUnits],edx
;---------- Set first approximation constant for Y-grid step values -----------;
xor ebp,ebp
mov eax,DEFAULT_Y_MBPS_PER_GRID  ; This parameters for MBPS mode, bandwidth
xor edx,edx
lea rcx,[CONFIG_VALUES.optionMbpsL1]
cmp [rbx + MEMIPB.updatedAsm],LATENCY_MODE
jb @f                            ; Go skip reload if MBPS mode
mov eax,DEFAULT_Y_NS_PER_GRID    ; This parameters for NS mode, latency
inc edx
lea rcx,[CONFIG_VALUES.optionNsL1]
@@:
mov esi,[rbx + MEMIPB.updatedTarget]
cmp esi,TARGET_CUSTOM
ja @f
mov ecx,[rcx + rsi*4]    ; Read value, override by configuration INF file
jecxz @f                 ; Go skip if no overrides
xchg eax,ecx             ; EAX = Override value
inc ebp                  ; EBP = 1, means no auto scale Y
@@:
mov [rdi + DRPM.valueGridY],eax
mov [rdi + DRPM.selectMode],edx
mov [rdi + DRPM.fixedYscale],ebp
;---------- Set Ymultiplier scale constant for conversion ---------------------; 
; measurement results to Y-coordinate
; Note same operation at win1.inc for re-scaling, 
finit
push rax -GRIDSTEPY
fild dword [rsp]        ; ST0 = - grid step, pixels
fidiv dword [rsp + 8]   ; ST0 = ( - grid step, pixels ) / ( Y units per grid )      
fstp [rdi + DRPM.yMultiplier]
pop rax rax
;--- Align stack and create parameters shadow, required for use WinAPI --------;
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
;--- Initializing handles and run thread with DrawSessionThread  subroutine ---; 
lea rsi,[DRAW_THREAD_PARMS]
mov rdi,rsi
cld
;---------- Create event for thread start, store event handle -----------------;
xor ecx,ecx          ; RCX = Parm#1 = lpEventAttributes  
mov edx,1            ; RDX = Parm#2 = bManualReset
xor r8d,r8d          ; R8  = Parm#3 = bInitialState
xor r9d,r9d          ; R9  = Parm#4 = Name, 0=None
call [CreateEvent]   ; Call WinAPI function
test rax,rax         ; Status=0 means error
jz .errorAlloc       ; Go if error creating event
stosq                ; Store handle in the thread parameters, EventStart 
;---------- Create event for thread done, store event handle ------------------;
xor ecx,ecx          ; RCX = Parm#1 = lpEventAttributes  
mov edx,1            ; RDX = Parm#2 = bManualReset
xor r8d,r8d          ; R8  = Parm#3 = bInitialState
xor r9d,r9d          ; R9  = Parm#4 = Name, 0=None
call [CreateEvent]   ; Call WinAPI function
test rax,rax         ; Status=0 means error
jz .errorAlloc       ; Go if error creating event
stosq                ; Store handle in the thread parameters, EventDone 
;---------- Create thread, store event handle ---------------------------------;
xor ecx,ecx                 ; RCX = Parm#1 = Security
xor edx,edx                 ; RDX = Parm#2 = Stack size, 0=default
lea r8,[DrawSessionThread]  ; R8  = Parm#3 = Execute routine pointer
xor r9d,r9d                 ; R9  = Parm#4 = Subroutine parameter, not used here
push rcx                    ; Parm#6 = Thread ID return , 0=None
push rcx                    ; Parm#5 = Creating Flags = Not suspended, run immediately
sub rsp,32                  ; Make parameters shadow
call [CreateThread]         ; Call WinAPI function
add rsp,32+16               ; Remove parameters shadow + 2 parameters
test rax,rax                ; Status=0 means error
jz .errorAlloc              ; Go if error creating thread
stosq                       ; Store handle in the thread parameters, ThreadHandle 
;---------- Clear measurement counter and array of measurement results --------;
mov ecx,DRAW_POINTS_COUNT + 20 - 3   ; OLD: + 4 dwords: 2 counters + break flag + align
xor eax,eax                 ; -3 because 3 qwords already stored
rep stosq
;---------- Done, no errors ---------------------------------------------------; 
; parallel thread with  DrawSessionThread  subroutine RUNNING
clc
.exitStack:
mov rsp,rbp
.exit:                 ; Note AL = error status must be valid at this point
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret
;---------- Exit point for errors detected ------------------------------------;
.errorAlloc:
mov al,MSG_RUNTIME_ALLOC
stc
jmp .exitStack
;---------- CF=1 with AL=2  means timings measurement error -------------------; 
; note this branch without valid RBP
.errorTimings:
mov al,MSG_RUNTIME_TIMINGS
stc
jmp .exit

;--- Stop measurement and drawings session, release resources -----------------;
;    used for draw performance = f( block size ) function.                     ;
;    Include terminate  DrawSessionThread  subroutine as parallel thread       ;
;                                                                              ;
; INPUT:   None at CPU registers                                               ;
;                                                                              ;
; OUTPUT:  CF = status flag, 0(NC)=No errors, 1(C)=Error                       ;
;          AL = error status, valid only if error detected (when CF=1)         ;
;          Status decode:                                                      ; 
;          see file "ncrb64.asm", application runtime errors identifiers       ;
;          see file "threads_manager\sessionstart.inc"                         ;
;                                                                              ;
; All CPU registers can be corrupted, this rule for complex modules:           ;
; directory threads_manager\:                                                  ;
;   sessionstart.inc, sessionstop.inc, sessionprogress.inc.                    ;
; directory gui\:                                                              ;
;   drawsessionstart.inc, drawsessionstop.inc                                  ;
;   but not drawsessionthread.inc because OS callback                          ;       
; Remember about PUSH-POP in the callers.                                      ;
;------------------------------------------------------------------------------;
DrawSessionStop:
push rbx rsi rdi rbp r12 r13 r14 r15
call SessionStop    ; De-initializing, release events, threads, memory
jc .exit            ; Go if error returned, AL = Error code
;--- Align stack and create parameters shadow, required for use WinAPI --------;
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
;---------- Initializing pointer ----------------------------------------------;
lea rsi,[DRAW_THREAD_PARMS]
cld
;---------- Close handles -----------------------------------------------------; 
; halt and remove thread with DrawSessionThread subroutine, terminate threads
mov rcx,[rsi + DTHP.threadHandle]   ; RCX = Parm#1 = THREAD handle
jrcxz @f                            ; Skip if this entry not allocated
xor edx,edx                         ; RDX = Parm#2 = Thread exit code
call [TerminateThread]              ; Call WinAPI function
test rax,rax                        ; Status=0 means error
jz .error                           ; Go exit if error
@@:
;---------- Close Start Events ------------------------------------------------;
lodsq
xchg rcx,rax                        ; RCX = Parm#1 = Load EVENT handle
jrcxz @f                            ; Skip if this entry not allocated
call [CloseHandle]                  ; Call WinAPI function
test rax,rax                        ; Status=0 means error
jz .error                           ; Go exit if error 
@@:
;---------- Close Done Events -------------------------------------------------;
lodsq
xchg rcx,rax                        ; RCX = Parm#1 = Load EVENT handle
jrcxz @f                            ; Skip if this entry not allocated
call [CloseHandle]                  ; Call WinAPI function
test rax,rax                        ; Status=0 means error
jz .error                           ; Go exit if error 
@@:
;---------- Close Threads -----------------------------------------------------;
lodsq
xchg rcx,rax                        ; RCX = Parm#1 = Load THREAD handle
jrcxz @f                            ; Skip if this entry not allocated
call [CloseHandle]                  ; Call WinAPI function
test rax,rax                        ; Status=0 means error
jz .error                           ; Go exit if error
@@:
;---------- Done --------------------------------------------------------------; 
; parallel thread with DrawSessionThread subroutine HALTED and REMOVED
clc
.exitStack:
mov rsp,rbp
.exit:                 ; Note AL = error status must be valid at this point
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret
;---------- Exit point for errors detected ------------------------------------;
.error:
mov al,MSG_RUNTIME_RELEASE
stc
jmp .exitStack

;--- This subroutine runs as parallel background thread, make measurements ----; 
;    and store results. Used for draw performance = f( block size ) function.  ;
;                                                                              ;
; INPUT:   None at CPU registers                                               ;
;                                                                              ;
; OUTPUT:  RAX = Return code = 0                                               ;
;          Non-volatile regs. unchanged for Microsoft x64  calling convention  ;
;                                                                              ;
; CPU registers volatile / non volatile by Microsoft x64 calling convention,   ;
; note about this subroutine is object for OS callback.                        ;
;------------------------------------------------------------------------------;
DrawSessionThread:
;---------- Entry point, must be compliant Microsoft x64 calling convention ---;
push rbx rsi rdi rbp r12 r13 r14 r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
push rbp
sub rsp,32+8
;---------- Load pointer to thread control structure --------------------------;
lea r15,[DRAW_THREAD_PARMS]
;---------- Signaling thread START --------------------------------------------;
mov rcx,[r15 + DTHP.eventStart]  ; RCX = Parm#1 = Handle for thread start event
call [SetEvent]                  ; Call WinAPI function
;---------- Measurement initialization, pre-heat ------------------------------;
lea rsi,[MEM_IPB]
lea rcx,[THREAD_LIST]
mov edx,[rsi + MEMIPB.updatedThreads]
call ThreadsReStart
jc .backgroundSkip  ; Go skip operations if error, note this is separate thread
;---------- Measurement cycle -------------------------------------------------;
.backgroundProcess:
cmp [r15 + DTHP.measureCounter],DRAW_POINTS_COUNT
jae .backgroundSkip     ; TODO. Can replace this to wait with minimum utilization ?
lea rsi,[MEM_IPB]
lea rdi,[MEM_OPB]
;---------- Update block size at threads control block ------------------------;
lea rcx,[THREAD_LIST]
mov rdx,rsi
mov r8d,[rsi + MEMIPB.updatedThreads]
mov r9,[rsi + MEMIPB.startBlockSize]
call ThreadsUpdateBlockSize
cmp [rsi + MEMIPB.updatedMeasure],MEASURE_B_ADAPTIVE
jb .noAdaptive
lea rcx,[THREAD_LIST]
mov rdx,rsi
mov r8d,[rsi + MEMIPB.updatedThreads]
call ThreadsUpdateAdaptive
.noAdaptive:
;---------- Select Bandwidth or Latency measurement mode ----------------------;
cmp [rsi + MEMIPB.updatedAsm],LATENCY_MODE
jae .latencyMode 
.bandwidthMode:
;---------- Measurement pattern -----------------------------------------------;
lea rcx,[THREAD_LIST]
mov edx,[rsi + MEMIPB.updatedThreads]
call ThreadsReStart
jc .backgroundSkip  ; Go skip operations if error, note this is separate thread
jmp .doneBandwidthOrLatency
.latencyMode:
;---------- Setup pattern for prepare walk list -------------------------------;
lea rcx,[THREAD_LIST]
mov r8d,[rsi + MEMIPB.updatedThreads]
mov r9,[rsi + MEMIPB.patternRoutine]
call ThreadsUpdatePattern
;---------- Run service pattern, prepare walk list, delta TSC ignored here ----;
lea rcx,[THREAD_LIST]
mov edx,[rsi + MEMIPB.updatedThreads]
call ThreadsReStart
jc .backgroundSkip  ; Go skip operations if error, note this is separate thread
;---------- Setup pattern for walk --------------------------------------------;
lea rcx,[THREAD_LIST]
mov r8d,[rsi + MEMIPB.updatedThreads]
mov r9,[rsi + MEMIPB.walkRoutine]
call ThreadsUpdatePattern
;---------- Run measurement pattern -------------------------------------------;
; walk by prepared walk list, get delta TSC in the RAX
lea rcx,[THREAD_LIST]
mov edx,[rsi + MEMIPB.updatedThreads]
call ThreadsReStart
jc .backgroundSkip  ; Go skip operations if error, note this is separate thread
.doneBandwidthOrLatency:
;---------- Calculte and store CPI = Clocks per Instruction -------------------;
; CPI = f ( Delta TSC, Instructions count )
mov edx,[r15 + DTHP.measureCounter]
shl edx,3      ; 2^3 = 8 bytes per store (QWORD)
lea rdi,[r15 + DTHP.measureArray + rdx]
push rax       ; RAX = Delta TSC per target operation, integer 64-bit
finit
fild qword [rsp]
mov rax,[rsi + MEMIPB.startBlockSize]  ; Block size, units = bytes
mul [rsi + MEMIPB.measureRepeats]      ; Multiply by total bytes per measurement cycle
mov ecx,[rsi + MEMIPB.operandWidth]    ; RCX = bits per instruction
shr ecx,3                           ; RCX = bytes per instruction
xor edx,edx
div rcx                             ; Note can use x87 to prevent exception 0
mov [rsp],rax
fild qword [rsp]  ; ST0 = Instruction cound, ST1 = Delta TSC
fdivp st1,st0     ; ST0 = CPI
fstp qword [rsp]
pop rax        ; RAX = Clocks per Instruction (CPI), floating point, double
stosq          ; Store this for communication with parent thread
;---------- Update block size for performance = f(size) -----------------------;
mov rax,[rsi + MEMIPB.deltaBlockSize]
add [rsi + MEMIPB.startBlockSize],rax
;---------- Cycle for measurement draw, increment counter by 1 drawing pixel --;
inc [r15 + DTHP.measureCounter]
;---------- Support approximation mode ----------------------------------------; 
; x16 and x32. One pixel already saved, required
; it copy  16-1=15  or  32-1=31  times.
mov eax,[rsi + MEMIPB.updatedApprox]
test eax,eax
jz .noApproxMode       ; go if no approximation mode
mov ecx,15             ; 1 already + 15 duplicated = 16 total stores
cmp eax,APPROX_X16
je .yesApproxMode
mov cl,31              ; 1 already + 31 duplicated = 32 total stores
.yesApproxMode:        ; approximation duplication cycle
mov rax,[rdi-8]
stosq                  ; one duplication per loop
mov r8,[rsi + MEMIPB.deltaBlockSize]
add [rsi + MEMIPB.startBlockSize],r8
inc [r15 + DTHP.measureCounter]
loop .yesApproxMode    ; approximation duplication cycle
.noApproxMode:
;---------- Cycle with support termination by break ---------------------------;
.backgroundSkip:
cmp [r15 + DTHP.measureBreak],0
je .backgroundProcess
;---------- Signaling thread STOP ---------------------------------------------;
mov rcx,[r15 + DTHP.eventDone]  ; RCX = Parm#1 = Handle for thread done event
call [SetEvent]                 ; Call WinAPI function
;---------- Exit point --------------------------------------------------------;
add rsp,32+8
pop rbp
mov rsp,rbp
pop r15 r14 r13 r12 rbp rdi rsi rbx
xor eax,eax                       ; RAX = 0, return code = 0
ret
