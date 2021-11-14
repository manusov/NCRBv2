;---------- Read options from GUI and setup benchmark scenario ----------------;
;                                                                              ;
; INPUT:   AL = mode: 0 = simple run, 1 = drawings                             ;
;          GUI data located at BINDLIST                                        ;
;                                                                              ;
; OUTPUT:  None at registers                                                   ;
;          Benchmark scenario build at IPB (Input Parameters Block)            ;
;                                                                              ;
;------------------------------------------------------------------------------;

; TODO. Can run as simple run or helper for draw, use input flag.
; TODO. PRE-BLANK RESULTS, OTHERWISE CAN SHOW PREVIOUS RESULTS WHEN ERROR.
; TODO. RUNTIME ERROR REPORTING.

GuiToUserParms:
;---------- Build User Parameters Block, UPB = F( GUI widgets state ) ---------;
lea rsi,[BIND_LIST]
lea rdi,[MEM_UPB]
movzx eax,al
mov [rdi + MEMUPB.runContext],eax  ; Setup run context for memory allocation multiplication mode control  
;---------- Option Asm --------------------------------------------------------;
mov eax,[rsi + BINDLIST.getMemMethod]
bsf eax,eax
mov [rdi + MEMUPB.optionAsm],eax 
;---------- Checkbox "Nontemporal ---------------------------------------------;
mov al,[rsi + BINDLIST.getMemOption]
and eax,0001b
mov [rdi + MEMUPB.optionNontemp],eax
;---------- Option prefetch ---------------------------------------------------;
movzx eax,[rsi + BINDLIST.getMemPref]
bsf eax,eax
mov [rdi + MEMUPB.optionDistance],eax 
;---------- Options parallel --------------------------------------------------;
mov cl,[rsi + BINDLIST.setMemSmp]
mov ch,[rsi + BINDLIST.getMemSmp]
xor eax,eax
test cl,00000001b
jz @f
inc eax
test ch,00000001b
jz @f
inc eax
@@:
mov [rdi + MEMUPB.optionParallel],eax
;---------- Option HT ---------------------------------------------------------;
xor eax,eax
test cl,00000100b
jz @f
inc eax
test ch,00000010b
jz @f 
inc eax
@@:
mov [rdi + MEMUPB.optionHT],eax
;---------- Option Processor Groups -------------------------------------------;
xor eax,eax
test cl,00010000b
jz @f
inc eax
test ch,00000100b
jz @f 
inc eax
@@:
mov [rdi + MEMUPB.optionPG],eax
;---------- Option NUMA -------------------------------------------------------;
xor eax,eax
test [rsi + BINDLIST.setMemNuma],00000001b
jz @f
movzx ecx,[rsi + BINDLIST.getMemNuma]
bsf ecx,ecx
add eax,ecx
@@:
mov [rdi + MEMUPB.optionNUMA],eax 
;---------- Option Object -----------------------------------------------------;
movzx eax,[rsi + BINDLIST.getMemObject]
bsf eax,eax
mov [rdi + MEMUPB.optionTarget],eax 
;---------- Option Large Pages ------------------------------------------------;
mov cl,[rsi + BINDLIST.setMemLpages]  ; Optimize this, don't use CL, direct test
mov ch,[rsi + BINDLIST.getMemLpages]
xor eax,eax
test cl,00000001b
jz @f
inc eax
test ch,00000001b
jz @f
inc eax
@@:
mov [rdi + MEMUPB.optionLP],eax
;---------- Option Measurement Repeats mode -----------------------------------;
movzx eax,[rsi + BINDLIST.getMemMeas]
bsf eax,eax
mov [rdi + MEMUPB.optionMeasure],eax 
;---------- Option Measurement Approximation mode -----------------------------;
movzx eax,[rsi + BINDLIST.getMemPix]
bsf eax,eax
mov [rdi + MEMUPB.optionApprox],eax 
;---------- Option user-defined block -----------------------------------------;
mov rax,[rsi + BINDLIST.getBlkCustom]
mov [rdi + MEMUPB.customBlockStart],rax
ret
;---- Execute simple run benchmark scenario and build text data for visual ----;
;                                                                              ;
; INPUT:   None at registers                                                   ;
;                                                                              ;
; OUTPUT:  None at registers                                                   ;
;          TODO. Error handling                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
ScenarioMemoryRun:
push rbx rsi rdi rbp r12 r13 r14 r15
cld
;---------- Refresh system information ----------------------------------------;
call InfoProcessor
;---------- Build User Parameters Block, UPB = F( GUI widgets state ) ---------;
mov al,0
call GuiToUserParms
;---------- Measurement start, execution, stop --------------------------------;
call SessionStart    ; Build IPB = F( UPB, System config. ), initializing
jc .error            ; Go if error at start
call SessionProgress ; Execute measurement
jc .error            ; Go if error at execution
call SessionStop     ; Stop measurement
jc .error            ; Go if error at termination

; TODO.
; Resources (memory) must be released even if error detected

;---------- Calculating results by Output Parameters Block (OPB) --------------;
lea r14,[BIND_LIST]
lea r12,[MEM_IPB]
;---------- Build application type string -------------------------------------;
lea rdi,[r14 + BINDLIST.mrApplication]
mov ax,STR_MR_APP_TYPE_64
call PoolStringWrite          ; TODO. Make this 3 instructions as subroutine or complex parametrize with 2 offsets
mov al,0
stosb
;---------- Build Assembler method string -------------------------------------;
lea rdi,[r14 + BINDLIST.mrMethod]
mov eax,[r12 + MEMIPB.updatedAsm]
mov cx,STR_AM_X64_00
cmp [r12 + MEMIPB.nonTemporalMode],0
je @f
mov cx,STR_AM_NT_00
@@:
add ax,cx
call PoolStringWrite
mov al,0
stosb
;---------- Build Operand width string ----------------------------------------;
lea rdi,[r14 + BINDLIST.mrWidth]
mov eax,[r12 + MEMIPB.operandWidth]
mov bl,0
call DecimalPrint32
mov al,0
stosb
;---------- Build Threads count string ----------------------------------------;
lea rdi,[r14 + BINDLIST.mrThreads]
mov eax,[r12 + MEMIPB.updatedThreads]
call DecimalPrint32
mov al,0
stosb
;---------- Build Hyper threading string --------------------------------------;
lea rdi,[r14 + BINDLIST.mrHyperT]
mov eax,[r12 + MEMIPB.updatedHT]
add ax,STR_MR_HT_00
call PoolStringWrite
mov al,0
stosb
;---------- Build Large pages string ------------------------------------------;
lea rdi,[r14 + BINDLIST.mrLargeP]
mov eax,[r12 + MEMIPB.updatedLP]
add ax,STR_MR_LP_00
call PoolStringWrite
mov al,0
stosb
;---------- Build NUMA string -------------------------------------------------;
lea rdi,[r14 + BINDLIST.mrNuma]
mov eax,[r12 + MEMIPB.updatedNUMA]
add ax,STR_MR_NUMA_00
call PoolStringWrite
mov al,0
stosb
;---------- Build Processor groups string -------------------------------------;
lea rdi,[r14 + BINDLIST.mrGroups]
mov eax,[r12 + MEMIPB.updatedPG]
add ax,STR_MR_PG_00
call PoolStringWrite
mov al,0
stosb
;---------- Build Target object string ----------------------------------------;
lea rdi,[r14 + BINDLIST.mrTarget]
mov eax,[r12 + MEMIPB.updatedTarget]
add ax,STR_MEMORY_L1
call PoolStringWrite
mov al,0
stosb
;---------- Build Prefetch distance string ------------------------------------;
lea rdi,[r14 + BINDLIST.mrDistance]
mov eax,[r12 + MEMIPB.updatedDistance]
add ax,STR_MR_PFDST_00
call PoolStringWrite
mov al,0
stosb
;---------- Build Total memory string -----------------------------------------;
lea rdi,[r14 + BINDLIST.mrSizeTotal]
mov rax,[r12 + MEMIPB.startBlockSize]
mov bl,0FFh
call SizePrint64
mov al,0
stosb
;---------- Build Memory per thread string ------------------------------------;
lea rdi,[r14 + BINDLIST.mrSizeThread]
mov rax,[r12 + MEMIPB.startBlockSize]
call SizePrint64
mov al,0
stosb
;---------- Build Measurement profile string ----------------------------------;
lea rdi,[r14 + BINDLIST.mrMeasProf]
mov eax,[r12 + MEMIPB.updatedMeasure]
add ax,STR_MR_MEASURE_00
call PoolStringWrite
mov al,0
stosb

; TODO. Overflow possible, 
; repeats can be above 32-bit value, actual for adaptive modes.

;---------- Build Measurement repeats string ----------------------------------;
lea rdi,[r14 + BINDLIST.mrMeasRep]
mov rax,[r12 + MEMIPB.measureRepeats]
mov bl,0
call DecimalPrint32
mov al,0
stosb
;---------- Build Memory block #1 string --------------------------------------;
lea rdi,[r14 + BINDLIST.mrBlock1]
mov rax,[r12 + MEMIPB.allocatedBlock1]
call HexPrint64
mov ax,0000h + 'h'
stosw
;---------- Build Memory block #2 string --------------------------------------;
lea rdi,[r14 + BINDLIST.mrBlock2]
mov rax,[r12 + MEMIPB.allocatedBlock2]
call HexPrint64
mov ax,0000h + 'h'
stosw
;---------- Build Total memory allocated string -------------------------------;
lea rdi,[r14 + BINDLIST.mrAllocAll]
mov rax,[r12 + MEMIPB.memoryTotal]
mov bl,0FFh
call SizePrint64
mov al,0
stosb
;---------- Build Memory per thread allocated string --------------------------;
lea rdi,[r14 + BINDLIST.mrAllocThr]
mov rax,[r12 + MEMIPB.memoryPerThread]
call SizePrint64
mov al,0
stosb
;---------- Calculate values and build Milliseconds string --------------------;
finit
lea r12,[MEM_OPB]
lea rdi,[r14 + BINDLIST.mrDtMs]
fild [r12 + MEMOPB.osTimerDelta]  ; st0 = delta time in 100 ns units 
push 10000                        ; 10000 units * 100 ns = 1000000 nS = 1 ms
fidiv dword [rsp]                 ; st0 = delta time in 1 ms units 
fstp qword [rsp]
pop rax                 ; RAX = bitfield as double value
mov bx,0100h            ; Float part is 1 digit, Integer part without template
call DoublePrint        ; print milliseconds
mov ax,STR_MR_PLUS_TIME
call PoolStringWrite
mov al,0
stosb
;---------- Calculate values and build TSC MHz string -------------------------;
lea rdi,[r14 + BINDLIST.mrDtscSec]
mov rax,[CPU_DATA.tscClockHz]  ; optimize by x87 access
push rax 1000000
fild qword [rsp + 8]
fidiv dword [rsp + 0]
fstp qword [rsp + 8]
pop rax rax
mov bx,0100h            ; Float part is 1 digit, Integer part without template
call DoublePrint        ; print MHz
mov al,0
stosb
;--- Calculate values and build Clocks per Instruction (clocks, CPI) string ---;
lea r11,[MEM_IPB]
lea rdi,[r14 + BINDLIST.mrDtscIns]
mov rax,[r11 + MEMIPB.startBlockSize] ; RAX = Block size, bytes 
xor edx,edx
mov ecx,[r11 + MEMIPB.operandWidth]   ; Must be not zero, otherwise divide overflow
shr ecx,3                             ; RCX = Bytes per instr., convert bits to bytes
div rcx                               ; RCX = Instructions per block
mov rcx,[r11 + MEMIPB.measureRepeats]
mul rcx                               ; RAX = Total instructions
push rax
fild qword [rsp]                      ; st0 = total instructions
fild qword [r12 + MEMOPB.tscTimerDelta]  ; st0 = total delta TSC, st1 = total instructions  
fdiv st0,st1
fstp qword [rsp]
pop rax                 ; RAX = bitfield as double value, clocks per instruction
mov r10,rax             ; R10 = this copy used for Latency Mode branch  
mov bx,0300h            ; Float part is 3 digits, Integer part without template
call DoublePrint
mov al,0
stosb
;---------- Final result select: Bandwidth or Latency measurement mode --------;
lea rdi,[r14 + BINDLIST.mrLastValue]
cmp [r11 + MEMIPB.updatedAsm],LATENCY_MODE
jae .latencyMode
;---------- Branch for Bandwidth, Speed (MBPS) --------------------------------;
mov rax,[r11 + MEMIPB.startBlockSize]    ; RAX = Operational block size, bytes 
mov rcx,[r11 + MEMIPB.measureRepeats]    ; RCX = Repeats, iterations, note 64-bit     
mul rcx                                  ; RAX = BlockSize * Repeats
mov ecx,[r11 + MEMIPB.updatedThreads]    ; Required all threads same size
mul rcx                                  ; RAX = BlockSize * Repeats * Threads
push rax
fild qword [rsp]            ; st0 = total bytes executed 
;---------- At this point, st0 = BlockSize * Repeats * Threads ----------------;
mov dword [rsp],1000000     ; alternative is 1048576 (binary/decimal MB select)
fidiv dword [rsp]           ; st0 = total megabytes executed
fild [r12 + MEMOPB.osTimerDelta]
mov dword [rsp],10000000    ; 10000000 units * 100 ns = 1 second
fidiv dword [rsp]
fdivr st0,st1
fstp qword [rsp]
pop rax             ; RAX = bitfield as double value
mov bx,0200h        ; Float part is 2 digits, Integer part without template
jmp .entryMode
;---------- Branch for Latency, Latency (ns) ----------------------------------;
.latencyMode:
mov rax,[CPU_DATA.tscClockHz]
mov [r12 + MEMOPB.tscFrequencyHz],rax
push 1000000000
fild qword [rsp]
fild [r12 + MEMOPB.tscFrequencyHz]  
fdivp st1,st0
fstp [r12 + MEMOPB.tscPeriodNs]
mov [rsp],r10
fld qword [rsp]
fmul [r12 + MEMOPB.tscPeriodNs]
fstp qword [rsp]
pop rax
mov bx,0300h        ; Float part is 3 digits, Integer part without template
.entryMode:
;---------- Write value and terminate string ----------------------------------;
call DoublePrint
mov al,0
stosb
clc
.error:  ; CF = 1 error flag and AL must be valid if exit by error 
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret
