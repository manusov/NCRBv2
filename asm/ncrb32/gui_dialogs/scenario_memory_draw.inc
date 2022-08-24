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
pushad
cld
call InfoProcessor
mov al,MSG_RUNTIME_TIMINGS
jc .exit
mov al,1  ; Run context = drawings, with memory allocation multiplication
call GuiToUserParms   ; Update UPB
call SessionStart     ; Initializing, allocate events, threads, memory, IPB
jc .exit              ; Go if error returned, AL = Error code
;---------- Initializing pointers ---------------------------------------------;
mov ebp,BIND_LIST
mov ebx,MEM_IPB
;---------- Down comments strings, assembler method ---------------------------;
lea edi,[ebp + BINDLIST.mdAsm]
mov eax,[ebx + MEMIPB.updatedAsm]
mov cx,STR_AM_NT_00
cmp [ebx + MEMIPB.nonTemporalMode],0
jne .doneNT
mov cx,STR_AM_IA32_00
cmp al,5
jbe .done3264
mov cx,STR_AM_X64_00
.done3264:
cmp eax,LATENCY_MODE
jb .doneNT
cmp [ebx + MEMIPB.force32mode],0
je .doneNT
add eax,LATENCY_MODE_COUNT
.doneNT:
add ax,cx
call PoolStringWrite
mov al,0
stosb
;--- Down comments strings, prefetch distance ( default, medium, long, N/A ) --; 
lea edi,[ebp + BINDLIST.mdPrefetch]
mov eax,[ebx + MEMIPB.updatedDistance]
add ax,STR_MD_PD_NOT_USED
call PoolStringWrite
mov al,0
stosb
;--- Down comments strings, target object ( Cache, DRAM, Custom block ) -------;
lea edi,[ebp + BINDLIST.mdObject]
mov eax,[ebx + MEMIPB.updatedTarget]
add ax,STR_MEMORY_L1
cmp ax,STR_MEMORY_CUSTOM
jne @f
mov ax,STR_MEMORY_CUSTOM_DRAW
@@:
call PoolStringWrite
mov al,0
stosb
;---------- Down comments strings, number of threads --------------------------;
lea edi,[ebp + BINDLIST.mdThreads]
mov ax,STR_MD_THREADS
call PoolStringWrite
push ebx
mov eax,[ebx + MEMIPB.updatedThreads]
mov bl,0
call DecimalPrint32
pop ebx
mov eax,[ebx + MEMIPB.updatedHybrid]
add ax,STR_MD_NO_HYBRID
call PoolStringWrite
mov al,0
stosb
;---------- Down comments strings, number of measurement repeats --------------;
;---------- Special strings for adaptive repeats modes ------------------------;
; TODO. REPEATS WRITE.
; TODO. MEASUREMENT MODE WRITE.
lea edi,[ebp + BINDLIST.mdRepeats]
mov eax,[ebx + MEMIPB.updatedMeasure]
add ax,STR_MEASURE_BRIEF
call PoolStringWrite
mov al,0
stosb
;---------- Down comments strings, normal or large pages ----------------------;
lea edi,[ebp + BINDLIST.mdPages]
mov eax,[ebx + MEMIPB.updatedLP]
add ax,STR_MD_LP_NOT_USED
call PoolStringWrite
mov al,0
stosb
;---------- Down comments strings, NUMA optimization mode ---------------------;
lea edi,[ebp + BINDLIST.mdNuma]
mov eax,[ebx + MEMIPB.updatedNUMA]
add ax,STR_MD_NUMA_OFF
call PoolStringWrite
mov al,0
stosb
;--- Support option, actual for drawings only: adaptive measurement modes -----;
mov esi,ADAPTIVE_REPEATS
mov eax,[ebx + MEMIPB.updatedMeasure]
sub eax,MEASURE_B_ADAPTIVE
jc .skipAdaptive
cmp eax,1
ja .skipAdaptive
mov ecx,[esi + eax*8 + 0]
mov dword [ebx + MEMIPB.adaptiveSeconds + 0],ecx
mov ecx,[esi + eax*8 + 4]
mov dword [ebx + MEMIPB.adaptiveSeconds + 4],ecx
;---------- Select Bandwidth or Latency measurement mode ----------------------;
mov esi,THREAD_LIST
mov ecx,esi
cmp [ebx + MEMIPB.updatedAsm],LATENCY_MODE
jae .latencyAdaptive
;---------- Run with default recommended repeats count: Bandwidth mode --------;
mov edx,[ebx + MEMIPB.updatedThreads]
call ThreadsReStart
jc .errorTimings             ; If error, go to error handling branch
jmp .calcAdaptive
;---------- Run with default recommended repeats count: Latency mode ----------;
; Setup pattern for prepare walk list
.latencyAdaptive:
push ebx
push [ebx + MEMIPB.updatedThreads]
mov eax,[ebx + MEMIPB.patternRoutine]   ; for first call, prepare list for walk
mov ebx,ecx
pop ecx
call ThreadsUpdatePattern
pop ebx
;---------- Run service pattern, prepare walk list, delta TSC ignored here ----;
mov ecx,esi
mov edx,[ebx + MEMIPB.updatedThreads]
call ThreadsReStart
jc .errorTimings             ; If error, go to error handling branch
;---------- Setup pattern for walk --------------------------------------------;
push ebx
mov ecx,[ebx + MEMIPB.updatedThreads]
mov eax,[ebx + MEMIPB.walkRoutine]     ; for second call, walk at prepared list
mov ebx,esi
call ThreadsUpdatePattern
pop ebx
;---------- Run measurement pattern -------------------------------------------;
; Walk by prepared walk list, get delta TSC in the EAX
mov ecx,esi
mov edx,[ebx + MEMIPB.updatedThreads]
call ThreadsReStart
jc .errorTimings             ; If error, go to error handling branch
;---------- Done point for adaptive modes ( bandwidth and latency branches ) --;
; Pre-measurements executed, 
; next, test control = f ( results ) must be calculated
.calcAdaptive:
;---------- Calculate timings -------------------------------------------------;
; Calculate seconds and ratio after run with default recommended repeats count
; for measurements calibration 
push edx eax                        ; EDX:EAX = Delta TSC after ThreadsReStart
finit
mov ecx,dword [CPU_DATA.tscClockHz + 0]
mov edx,dword [CPU_DATA.tscClockHz + 4]
mov dword [MEM_OPB.tscFrequencyHz + 0],ecx
mov dword [MEM_OPB.tscFrequencyHz + 4],edx
fld1
fild [MEM_OPB.tscFrequencyHz]  
fdivp st1,st0
fstp [MEM_OPB.tscPeriodS]
push 1000000000
fild dword [esp]
fild [MEM_OPB.tscFrequencyHz]  
fdivp st1,st0
fstp [MEM_OPB.tscPeriodNs]
pop eax
push 1000000
fild [MEM_OPB.tscFrequencyHz]
fild dword [esp]
fdivp st1,st0
fstp [MEM_OPB.tscFrequencyMHz]
pop eax
fld [ebx + MEMIPB.adaptiveSeconds]  ; ST0 = Target seconds
fild qword [esp]                    ; ST0 = Measured delta TSC , ST1 = Target seconds
fmul [MEM_OPB.tscPeriodS]           ; ST0 = Measured seconds
fdivp st1,st0                       ; ST0 = Target / Measured 
fild [ebx + MEMIPB.measureRepeats]  ; ST0 = Product repeats
fmulp st1,st0                       ; ST0 = Corrected product
fild [ebx + MEMIPB.startBlockSize]
fmulp st1,st0                       ; ST0 = Corrected product for divide per bytes count 
fistp [ebx + MEMIPB.adaptiveProduct]
pop eax edx                         ; This for release stack only
;---------- Skip point for non-adaptive modes ---------------------------------;
; Note option: X-approximation modes, 
; interpreted and supported at draw session thread 
.skipAdaptive:
;---------- Load pointer to drawings parameters -------------------------------;
mov edi,DRAW_PARMS
;---------- Calculate X grid step values --------------------------------------;
; Must be integer power of 2 for next step correct
; Note this operation increase memory size requirements, because increase EAX
mov eax,[ebx + MEMIPB.startBlockSize]
bsr ecx,eax
mov eax,1
shl eax,cl   ; EAX = Target block size, note example here EAX=8K for L1=32K
;---------- Special support for DRAM and Custom modes -------------------------;
mov ecx,[ebx + MEMIPB.updatedTarget]
cmp ecx,TARGET_DRAM
je .slowObject            ; Go for size / 8 if target = DRAM
cmp ecx,TARGET_CUSTOM
jne .fastObject           ; Go skip without size / 8 if target = not a Custom
.slowObject:
shr eax,3                 ; size / 8 , for DRAM and Custom modes
.fastObject:
;---------- Set Xstart, Xdelta for drawings -----------------------------------;
push eax                            ; EAX = Block size per X-grid
xor edx,edx                         ; EDX = Dividend high 64 bits, =0
mov ecx,GRIDSTEPX                   ; ECX = Pixels per X-grid
div ecx                             ; (Block size per grid) / (pixels per grid)
mov [ebx + MEMIPB.startBlockSize],eax  ; X start value
mov [ebx + MEMIPB.deltaBlockSize],eax  ; X addend per draw iteration 
pop eax
;---------- Update and store X grid step values -------------------------------;
; Select and store X grid units: Bytes, Kilobytes, Megabytes
xor edx,edx                ; EDX = SelectUnits, 0=Bytes / 1=KB / 2=MB
cmp eax,1024
jb @f                      ; Go with Units=Bytes if grid step < 1 KB  
inc edx
shr eax,10
cmp eax,1024
jb @f                      ; Go with Units=KB if grid step < 1 MB
inc edx
shr eax,10
@@:                        ; Otherwise Units=MB
mov [edi + DRPM.valueGridX],eax
mov [edi + DRPM.selectUnits],edx
;---------- Set first approximation constant for Y-grid step values -----------;
xor ebp,ebp
mov eax,DEFAULT_Y_MBPS_PER_GRID  ; This parameters for MBPS mode, bandwidth
xor edx,edx
mov ecx,CONFIG_VALUES.optionMbpsL1
cmp [ebx + MEMIPB.updatedAsm],LATENCY_MODE
jb @f                            ; Go skip reload if MBPS mode
mov eax,DEFAULT_Y_NS_PER_GRID    ; This parameters for NS mode, latency
inc edx
mov ecx,CONFIG_VALUES.optionNsL1
@@:
mov esi,[ebx + MEMIPB.updatedTarget]
cmp esi,TARGET_CUSTOM
ja @f
mov ecx,[ecx + esi*4]    ; Read value, override by configuration INF file
jecxz @f                 ; Go skip if no overrides
xchg eax,ecx             ; EAX = Override value
inc ebp                  ; EBP = 1, means no auto scale Y
@@:
mov [edi + DRPM.valueGridY],eax
mov [edi + DRPM.selectMode],edx
mov [edi + DRPM.fixedYscale],ebp
;---------- Set Ymultiplier scale constant for conversion ---------------------; 
; measurement results to Y-coordinate
; Note same operation at win1.inc for re-scaling, 
finit
push eax -GRIDSTEPY
fild dword [esp]        ; ST0 = - grid step, pixels
fidiv dword [esp + 4]   ; ST0 = ( - grid step, pixels ) / ( Y units per grid )      
fstp [edi + DRPM.yMultiplier]
pop eax eax
;--- Initializing handles and run thread with DrawSessionThread  subroutine ---; 
mov esi,DRAW_THREAD_PARMS
mov edi,esi
cld
;---------- Create event for thread start, store event handle -----------------;
push 0               ; Parm#4 = Name, 0=None
push 0               ; Parm#3 = bInitialState
push 1               ; Parm#2 = bManualReset
push 0               ; Parm#1 = lpEventAttributes
call [CreateEvent]   ; Call WinAPI function
test eax,eax         ; Status=0 means error
jz .errorAlloc       ; Go if error creating event
stosd                ; Store handle in the thread parameters, EventStart 
;---------- Create event for thread done, store event handle ------------------;
push 0               ; Parm#4 = Name, 0=None
push 0               ; Parm#3 = bInitialState
push 1               ; Parm#2 = bManualReset
push 0               ; Parm#1 = lpEventAttributes
call [CreateEvent]   ; Call WinAPI function
test eax,eax         ; Status=0 means error
jz .errorAlloc       ; Go if error creating event
stosd                ; Store handle in the thread parameters, EventDone 
;---------- Create thread, store event handle ---------------------------------;
xor eax,eax          ; EAX = 0 for compact PUSH 0
push eax                    ; Parm#6 = Thread ID return , 0=None
push eax                    ; Parm#5 = Creating Flags = Not suspended, run immediately
push eax                    ; Parm#4 = Subroutine parameter, not used here
push DrawSessionThread      ; Parm#3 = Execute routine pointer
push eax                    ; Parm#2 = Stack size, 0=default
push eax                    ; Parm#1 = Security
call [CreateThread]         ; Call WinAPI function
test eax,eax                ; Status=0 means error
jz .errorAlloc              ; Go if error creating thread
stosd                       ; Store handle in the thread parameters, ThreadHandle 
;---------- Clear measurement counter and array of measurement results --------;
mov ecx,( DRAW_POINTS_COUNT + 15 ) * 2 + 3
xor eax,eax
rep stosd
;---------- Done, no errors ---------------------------------------------------; 
; Parallel thread with  DrawSessionThread  subroutine RUNNING
clc
.exit:                 ; Note AL = error status must be valid at this point
mov [esp + 28],al
popad
ret
;---------- Exit point for errors detected ------------------------------------;
.errorAlloc:
mov al,MSG_RUNTIME_ALLOC
stc
jmp .exit
;---------- CF=1 with AL=2  means timings measurement error -------------------; 
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
pushad
call SessionStop    ; De-initializing, release events, threads, memory
jc .exit            ; Go if error returned, AL = Error code
;---------- Initializing pointer ----------------------------------------------;
mov esi,DRAW_THREAD_PARMS
cld
;---------- Terminate threads -------------------------------------------------;
; Close handles, halt and remove thread with DrawSessionThread  subroutine
mov ecx,[esi + DTHP.threadHandle]   ; ECX = THREAD handle
jecxz @f                            ; Skip if this entry not allocated
push 0                              ; Parm#2 = Thread exit code
push ecx                            ; Parm#1 = THREAD handle 
call [TerminateThread]              ; Call WinAPI function
test eax,eax                        ; Status=0 means error
jz .error                           ; Go exit if error
@@:
;---------- Close Start Events ------------------------------------------------;
lodsd                               ; EAX = Load EVENT handle
test eax,eax
jz @f                               ; Skip if this entry not allocated
push eax                            ; Parm#1 = EVENT handle
call [CloseHandle]                  ; Call WinAPI function
test eax,eax                        ; Status=0 means error
jz .error                           ; Go exit if error 
@@:
;---------- Close Done Events -------------------------------------------------;
lodsd                               ; EAX = Load EVENT handle
test eax,eax
jz @f                               ; Skip if this entry not allocated
push eax                            ; Parm#1 = EVENT handle 
call [CloseHandle]                  ; Call WinAPI function
test eax,eax                        ; Status=0 means error
jz .error                           ; Go exit if error 
@@:
;---------- Close Threads -----------------------------------------------------;
lodsd                               ; EAX = Load THREAD handle 
test eax,eax
jz @f                               ; Skip if this entry not allocated
push eax                            ; Parm#1 = THREAD handle
call [CloseHandle]                  ; Call WinAPI function
test eax,eax                        ; Status=0 means error
jz .error                           ; Go exit if error
@@:
;---------- Done --------------------------------------------------------------; 
; Parallel thread with DrawSessionThread subroutine HALTED and REMOVED
clc
.exit:                 ; Note AL = error status must be valid at this point
mov [esp + 28],al
popad
ret
;---------- Exit point for errors detected ------------------------------------;
.error:
mov al,MSG_RUNTIME_RELEASE
stc
jmp .exit

;--- This subroutine runs as parallel background thread, make measurements ----; 
;    and store results. Used for draw performance = f( block size ) function.  ;
;                                                                              ;
; INPUT:   None at CPU registers                                               ;
;          But reserve space for one DWORD parameter in the stack              ;
;                                                                              ;
; OUTPUT:  EAX = Return code = 0                                               ;
;          Non-volatile regs. unchanged for Microsoft ia32  calling convention ;
;                                                                              ;
; CPU registers volatile / non volatile by Microsoft ia32 calling convention,  ;
; note about this subroutine is object for OS callback.                        ;
;------------------------------------------------------------------------------;
DrawSessionThread:
;---------- Entry point, must be compliant Microsoft ia32 calling convention --;
pushad
;---------- Signaling thread START --------------------------------------------;
push [DRAW_THREAD_PARMS.eventStart]    ; Parm#1 = Handle for thread start event
call [SetEvent]                        ; Call WinAPI function
;---------- Measurement initialization, pre-heat ------------------------------;
mov esi,MEM_IPB                        ; ESI = Pointer to input parameters block
mov ecx,THREAD_LIST                    ; ECX = Pointer to Threads List
mov edx,[esi + MEMIPB.updatedThreads]  ; EDX = Threads count
call ThreadsReStart
jc .backgroundSkip  ; Go skip operations if error, note this is separate thread
;---------- Measurement cycle -------------------------------------------------;
.backgroundProcess:
cmp [DRAW_THREAD_PARMS.measureCounter],DRAW_POINTS_COUNT
jae .backgroundSkip    ; TODO. Can replace this to wait with minimum utilization ?
mov esi,MEM_IPB        ; ESI = Pointer to Input Parameters Block (IPB)
mov edi,MEM_OPB        ; EDI = Pointer to Output Parameters Block (OPB)
;---------- Update block size at threads control block ------------------------;
mov edx,esi                             ; EDX = Pointer to IPB
mov ecx,[esi + MEMIPB.updatedThreads]   ; ECX = Number of threads
mov eax,[esi + MEMIPB.startBlockSize]   ; EAX = Block size per thread, bytes
mov ebx,THREAD_LIST                     ; EBX = Pointer to threads list
call ThreadsUpdateBlockSize
cmp [esi + MEMIPB.updatedMeasure],MEASURE_B_ADAPTIVE
jb .noAdaptive                          ; Go skip if adaptive mode not used
mov ebx,THREAD_LIST                     ; EBX = Pointer to threads list 
mov edx,esi                             ; EDX = Pointer to IPB 
mov ecx,[esi + MEMIPB.updatedThreads]   ; ECX = Number of threads 
call ThreadsUpdateAdaptive
.noAdaptive:
;---------- Select Bandwidth or Latency measurement mode ----------------------;
cmp [esi + MEMIPB.updatedAsm],LATENCY_MODE
jae .latencyMode   ; Go if LATENCY mode, otherwise BANDWIDTH mode 
.bandwidthMode:
;---------- Measurement pattern -----------------------------------------------;
mov ecx,THREAD_LIST
mov edx,[esi + MEMIPB.updatedThreads]
call ThreadsReStart
jc .backgroundSkip  ; Go skip operations if error, note this is separate thread
jmp .doneBandwidthOrLatency
.latencyMode:
;---------- Setup pattern for prepare walk list -------------------------------;
mov ebx,THREAD_LIST                        ; EBX = Pointer to threads list
mov ecx,[esi + MEMIPB.updatedThreads]      ; ECX = Number of threads
mov eax,[esi + MEMIPB.patternRoutine]      ; EAX = Pointer to pattern routine
call ThreadsUpdatePattern
;---------- Run service pattern, prepare walk list, delta TSC ignored here ----;
mov ecx,THREAD_LIST                        ; ECX = Pointer to threads list
mov edx,[esi + MEMIPB.updatedThreads]      ; EDX = Number of threads
call ThreadsReStart
jc .backgroundSkip  ; Go skip operations if error, note this is separate thread
;---------- Setup pattern for walk with list, prepared on previous step -------;
mov ebx,THREAD_LIST
mov ecx,[esi + MEMIPB.updatedThreads]
mov eax,[esi + MEMIPB.walkRoutine]
call ThreadsUpdatePattern
;---------- Run measurement pattern -------------------------------------------; 
; Walk by prepared walk list, get delta TSC in the EAX
mov ecx,THREAD_LIST
mov edx,[esi + MEMIPB.updatedThreads]
call ThreadsReStart
jc .backgroundSkip  ; Go skip operations if error, note this is separate thread
.doneBandwidthOrLatency:
;---------- Calculte and store CPI = Clocks per Instruction -------------------;
; CPI = f ( Delta TSC, Instructions count )
mov ecx,[DRAW_THREAD_PARMS.measureCounter]
shl ecx,3      ; 2^3 = 8 bytes per store (QWORD)
lea edi,[DRAW_THREAD_PARMS.measureArray + ecx]
push edx eax   ; EDX:EAX = Delta TSC per target operation, integer 64-bit
finit
fild qword [esp]
fild [esi + MEMIPB.measureRepeats]   ; Multiply by total bytes per measurement cycle
fimul [esi + MEMIPB.startBlockSize]  ; Block size, units = bytes 
mov eax,[esi + MEMIPB.operandWidth]  ; EAX = bits per instruction
shr eax,3                            ; EAX = bytes per instruction
mov [esp],eax
fidiv dword [esp]                    ; Divide by BPI, ST0 = Instructions count, ST1 = Delta TSC
fdivp st1,st0     ; ST0 = CPI
fstp qword [esp]
pop eax       ; EDX:EAX = Clocks per Instruction (CPI), floating point, double
stosd         ; CPI, Low DWORD, store this for communication with parent thread
pop eax
stosd         ; CPI, High DWORD
;---------- Update block size for performance = f(size) -----------------------;
mov eax,[esi + MEMIPB.deltaBlockSize]
add [esi + MEMIPB.startBlockSize],eax
;---------- Cycle for measurement draw, increment counter by 1 drawing pixel --;
inc [DRAW_THREAD_PARMS.measureCounter]
;---------- Support approximation mode ----------------------------------------;
; x16 and x32. One pixel already saved, required it copy
;  16-1=15  or  32-1=31  times.
mov eax,[esi + MEMIPB.updatedApprox]
test eax,eax
jz .noApproxMode       ; go if no approximation mode
mov ecx,15             ; 1 already + 15 duplicated = 16 total stores
cmp eax,APPROX_X16
je .yesApproxMode
mov cl,31              ; 1 already + 31 duplicated = 32 total stores
.yesApproxMode:        ; approximation duplication cycle
mov eax,[edi-8]
stosd                  ; one duplication per loop
mov eax,[edi-8]
stosd                  ; one duplication per loop
mov ebx,[esi + MEMIPB.deltaBlockSize]
add [esi + MEMIPB.startBlockSize],ebx
inc [DRAW_THREAD_PARMS.measureCounter]
loop .yesApproxMode    ; approximation duplication cycle
.noApproxMode:
;---------- Cycle with support termination by break ---------------------------;
.backgroundSkip:
cmp [DRAW_THREAD_PARMS.measureBreak],0
je .backgroundProcess
;---------- Signaling thread STOP ---------------------------------------------;
push [DRAW_THREAD_PARMS.eventDone]    ; Parm#1 = Handle for thread done event
call [SetEvent]                       ; Call WinAPI function
;---------- Exit point --------------------------------------------------------;
popad
xor eax,eax                           ; EAX = 0, return code = 0
ret 4
