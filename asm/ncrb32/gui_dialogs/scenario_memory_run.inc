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
mov edi,MEM_UPB
mov esi,BIND_LIST
movzx eax,al
mov [edi + MEMUPB.runContext],eax  ; Setup run context for memory allocation multiplication mode control  
;---------- Option Asm --------------------------------------------------------;
mov eax,[esi + BINDLIST.getMemMethod]  ; TODO. Optimize offsets size by nested structures.
bsf eax,eax
mov [edi + MEMUPB.optionAsm],eax 
;---------- Checkbox "Nontemporal ---------------------------------------------;
mov al,[esi + BINDLIST.getMemOption]
and eax,0011b
mov ecx,eax
and eax,0001b
mov [edi + MEMUPB.optionNontemp],eax
;---------- Checkbox "Force 32x2 (yet for ia32 version only) ------------------;
shr ecx,1
mov [edi + MEMUPB.optionForce32],ecx
;---------- Option prefetch ---------------------------------------------------;
movzx eax,[esi + BINDLIST.getMemPref]
bsf eax,eax
mov [edi + MEMUPB.optionDistance],eax 
;---------- Options parallel --------------------------------------------------;
mov cl,[esi + BINDLIST.setMemSmp]
mov ch,[esi + BINDLIST.getMemSmp]
xor eax,eax
test cl,00000001b
jz @f
inc eax
test ch,00000001b
jz @f
inc eax
@@:
mov [edi + MEMUPB.optionParallel],eax
;---------- Option HT ---------------------------------------------------------;
xor eax,eax
test cl,00000100b
jz @f
inc eax
test ch,00000010b
jz @f 
inc eax
@@:
mov [edi + MEMUPB.optionHT],eax
;---------- Option Processor Groups -------------------------------------------;
xor eax,eax
test cl,00010000b
jz @f
inc eax
test ch,00000100b
jz @f 
inc eax
@@:
mov [edi + MEMUPB.optionPG],eax
;---------- Option NUMA -------------------------------------------------------;
xor eax,eax
test [esi + BINDLIST.setMemNuma],00000001b
jz @f
movzx eax,[esi + BINDLIST.getMemNuma]
bsf eax,eax
inc eax
@@:
mov [edi + MEMUPB.optionNUMA],eax 
;---------- Option Object -----------------------------------------------------;
movzx eax,[esi + BINDLIST.getMemObject]
bsf eax,eax
mov [edi + MEMUPB.optionTarget],eax 
;---------- Option Hybrid -----------------------------------------------------;
xor eax,eax
test [esi + BINDLIST.setMemHybrid],00000001b
jz @f
movzx eax,[esi + BINDLIST.getMemHybrid]
bsf eax,eax
inc eax
@@:
mov [edi + MEMUPB.optionHybrid],eax 
;---------- Option Large Pages ------------------------------------------------;
mov cl,[esi + BINDLIST.setMemLpages]  ; Optimize this, don't use CL, direct test
mov ch,[esi + BINDLIST.getMemLpages]
xor eax,eax
test cl,00000001b
jz @f
inc eax
test ch,00000001b
jz @f
inc eax
@@:
mov [edi + MEMUPB.optionLP],eax
;---------- Option Measurement Repeats mode -----------------------------------;
movzx eax,[esi + BINDLIST.getMemMeas]
bsf eax,eax
mov [edi + MEMUPB.optionMeasure],eax 
;---------- Option Measurement Approximation mode -----------------------------;
movzx eax,[esi + BINDLIST.getMemPix]
bsf eax,eax
mov [edi + MEMUPB.optionApprox],eax 
;---------- Option user-defined block -----------------------------------------;
mov eax,dword [esi + BINDLIST.getBlkCustom]
mov [edi + MEMUPB.customBlockStart],eax
ret

;---- Execute simple run benchmark scenario and build text data for visual ----;
;                                                                              ;
; INPUT:   None at registers                                                   ;
;                                                                              ;
; OUTPUT:  CF = status flag, 0(NC)=No errors, 1(C)=Error                       ;
;          AL = error status, valid only if error detected (when CF=1)         ;
;          Status decode:                                                      ;
;          see file "ncrb64.asm", application runtime errors identifiers       ;
;          see file "threads_manager\sessionstart.inc"                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
ScenarioMemoryRun:
pushad
cld
;---------- Refresh system information ----------------------------------------;
call InfoProcessor
mov al,MSG_RUNTIME_TIMINGS
jc .error
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
; Resources (memory) must be relesed even if error detected
; Timeout error at second subroutine if trace by OllyDbg.

;---------- Calculating results by Output Parameters Block (OPB) --------------;
mov ebp,BIND_LIST
mov ebx,MEM_IPB
;---------- Build application type string -------------------------------------;
lea edi,[ebp + BINDLIST.mrApplication]
mov ax,STR_MR_APP_TYPE_32
cmp [OS_DATA.isWow64],0       ; Differentiate ia32 and ia32 under Win64 (WoW)
je @f
mov ax,STR_MR_APP_TYPE_WOW
@@:
call PoolStringWrite          ; TODO. Make this 3 instructions as subroutine or complex parametrize with 2 offsets
mov al,0
stosb
;---------- Build Assembler method string -------------------------------------;
lea edi,[ebp + BINDLIST.mrMethod]
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
;---------- Build Operand width string ----------------------------------------;
lea edi,[ebp + BINDLIST.mrWidth]
mov eax,[ebx + MEMIPB.operandWidth]
push ebx
mov bl,0
call DecimalPrint32
pop ebx
mov al,0
stosb
;---------- Build Threads count string ----------------------------------------;
lea edi,[ebp + BINDLIST.mrThreads]
mov eax,[ebx + MEMIPB.updatedThreads]
push ebx
mov bl,0
call DecimalPrint32
pop ebx
mov al,0
stosb
;---------- Build Hyper threading string --------------------------------------;
lea edi,[ebp + BINDLIST.mrHyperT]
mov eax,[ebx + MEMIPB.updatedHT]
add ax,STR_MR_HT_00
call PoolStringWrite
mov al,0
stosb
;---------- Build Large pages string ------------------------------------------;
lea edi,[ebp + BINDLIST.mrLargeP]
mov eax,[ebx + MEMIPB.updatedLP]
add ax,STR_MR_LP_00
call PoolStringWrite
mov al,0
stosb
;---------- Build NUMA string -------------------------------------------------;
lea edi,[ebp + BINDLIST.mrNuma]
mov eax,[ebx + MEMIPB.updatedNUMA]
add ax,STR_MR_NUMA_00
call PoolStringWrite
mov al,0
stosb
;---------- Build Processor groups string -------------------------------------;
lea edi,[ebp + BINDLIST.mrGroups]
mov eax,[ebx + MEMIPB.updatedPG]
add ax,STR_MR_PG_00
call PoolStringWrite
mov al,0
stosb
;---------- Build Hybrid topology string --------------------------------------;
lea edi,[ebp + BINDLIST.mrHybrid]
mov eax,[ebx + MEMIPB.updatedHybrid]
add ax,STR_MR_HYBRID_00
call PoolStringWrite
mov al,0
stosb
;---------- Build Target object string ----------------------------------------;
lea edi,[ebp + BINDLIST.mrTarget]
mov eax,[ebx + MEMIPB.updatedTarget]
add ax,STR_MEMORY_L1
call PoolStringWrite
mov al,0
stosb
;---------- Build Prefetch distance string ------------------------------------;
lea edi,[ebp + BINDLIST.mrDistance]
mov eax,[ebx + MEMIPB.updatedDistance]
add ax,STR_MR_PFDST_00
call PoolStringWrite
mov al,0
stosb
;---------- Build Total memory string -----------------------------------------;
lea edi,[ebp + BINDLIST.mrSizeTotal]
mov eax,[ebx + MEMIPB.startBlockSize]
xor edx,edx
push ebx
mov bl,0FFh
call SizePrint64
pop ebx
mov al,0
stosb
;---------- Build Memory per thread string ------------------------------------;
lea edi,[ebp + BINDLIST.mrSizeThread]
mov eax,[ebx + MEMIPB.startBlockSize]
xor edx,edx
push ebx
mov bl,0FFh
call SizePrint64
pop ebx
mov al,0
stosb
;---------- Build Measurement profile string ----------------------------------;
lea edi,[ebp + BINDLIST.mrMeasProf]
mov eax,[ebx + MEMIPB.updatedMeasure]
add ax,STR_MR_MEASURE_00
call PoolStringWrite
mov al,0
stosb

; TODO. Overflow possible, 
; repeats can be above 32-bit value, actual for adaptive modes.

;---------- Build Measurement repeats string ----------------------------------;
lea edi,[ebp + BINDLIST.mrMeasRep]
mov eax,dword [ebx + MEMIPB.measureRepeats]
push ebx
mov bl,0
call DecimalPrint32
pop ebx
mov al,0
stosb
;---------- Build Memory block #1 string --------------------------------------;
lea edi,[ebp + BINDLIST.mrBlock1]
mov eax,[ebx + MEMIPB.allocatedBlock1]
call HexPrint32
mov ax,0000h + 'h'
stosw
;---------- Build Memory block #2 string --------------------------------------;
lea edi,[ebp + BINDLIST.mrBlock2]
mov eax,[ebx + MEMIPB.allocatedBlock2]
call HexPrint32
mov ax,0000h + 'h'
stosw
;---------- Build Total memory allocated string -------------------------------;
lea edi,[ebp + BINDLIST.mrAllocAll]
mov eax,[ebx + MEMIPB.memoryTotal]
push ebx
mov bl,0FFh
call SizePrint64
pop ebx
mov al,0
stosb
;---------- Build Memory per thread allocated string --------------------------;
lea edi,[ebp + BINDLIST.mrAllocThr]
mov eax,[ebx + MEMIPB.memoryPerThread]
push ebx
mov bl,0FFh
call SizePrint64
pop ebx
mov al,0
stosb
;---------- Calculate values and build Milliseconds string --------------------;
finit
mov ebx,MEM_OPB
lea edi,[ebp + BINDLIST.mrDtMs]
fild [ebx + MEMOPB.osTimerDelta]  ; st0 = delta time in 100 ns units 
push 0 10000                      ; 10000 units * 100 ns = 1000000 nS = 1 ms
fidiv dword [esp]                 ; st0 = delta time in 1 ms units 
fstp qword [esp]
pop eax edx              ; EDX:EAX = bitfield as double value
push ebx
mov bx,0100h            ; Float part is 1 digit, Integer part without template
call DoublePrint        ; print milliseconds
pop ebx
mov ax,STR_MR_PLUS_TIME
call PoolStringWrite
mov al,0
stosb
;---------- Calculate values and build TSC MHz string -------------------------;
lea edi,[ebp + BINDLIST.mrDtscSec]
push dword [CPU_DATA.tscClockHz + 4]  ; optimize by x87 access
push dword [CPU_DATA.tscClockHz + 0]  ; optimize by x87 access
push 1000000
fild qword [esp + 4]
fidiv dword [esp + 0]
fstp qword [esp + 4]
pop eax eax edx
push ebx
mov bx,0100h            ; Float part is 1 digit, Integer part without template
call DoublePrint        ; print MHz
pop ebx
mov al,0
stosb
;--- Calculate values and build Clocks per Instruction (clocks, CPI) string ---;
mov esi,MEM_IPB
lea edi,[ebp + BINDLIST.mrDtscIns]
mov eax,[esi + MEMIPB.startBlockSize]  ; EAX = Block size, bytes 
xor edx,edx
mov ecx,[esi + MEMIPB.operandWidth]    ; Must be not zero, otherwise divide overflow
shr ecx,3                        ; ECX = Bytes per instr., convert bits to bytes
div ecx                          ; ECX = Instructions per block
mov ecx,dword [esi + MEMIPB.measureRepeats]
mul ecx                          ; EDX:EAX = Total instructions
push edx eax
fild qword [esp]                   ; st0 = total instructions
fild qword [ebx + MEMOPB.tscTimerDelta]  ; st0 = total delta TSC, st1 = total instructions  
fdiv st0,st1
fstp qword [esp]
pop eax edx             ; EDX:EAX = bitfield as double value
push ebx
mov bx,0300h            ; Float part is 3 digits, Integer part without template
call DoublePrint
pop ebx
push eax
mov al,0
stosb
pop eax     ; EDX:EAX used for Latency branch
;---------- Final result select: Bandwidth or Latency measurement mode --------;
lea edi,[ebp + BINDLIST.mrLastValue]
cmp [esi + MEMIPB.updatedAsm],LATENCY_MODE
jae .latencyMode
;---------- Branch for Bandwidth, Speed (MBPS) --------------------------------;
push edx eax
fild [esi + MEMIPB.measureRepeats]   ; st0 = Repeats, iterations, note 64-bit
fimul [esi + MEMIPB.startBlockSize]  ; st0 * Operational block size, bytes 
fimul [esi + MEMIPB.updatedThreads]   ; Required all threads same size
;---------- At this point, st0 = BlockSize * Repeats * Threads ----------------;
mov dword [esp],1000000     ; alternative is 1048576 (binary/decimal MB select)
fidiv dword [esp]           ; st0 = total megabytes executed
fild [ebx + MEMOPB.osTimerDelta]
mov dword [esp],10000000    ; 10000000 units * 100 ns = 1 second
fidiv dword [esp]
fdivr st0,st1
fstp qword [esp]
pop eax edx             ; EDX:EAX = bitfield as double value
mov bx,0200h            ; Float part is 2 digits, Integer part without template
jmp .entryMode
;---------- Branch for Latency, Latency (ns) ----------------------------------;
.latencyMode:
push edx eax
mov eax,dword [CPU_DATA.tscClockHz + 0]
mov edx,dword [CPU_DATA.tscClockHz + 4]
mov dword [ebx + MEMOPB.tscFrequencyHz + 0],eax
mov dword [ebx + MEMOPB.tscFrequencyHz + 4],edx
push 1000000000
fild dword [esp]
fild [ebx + MEMOPB.tscFrequencyHz]  
fdivp st1,st0
fstp [ebx + MEMOPB.tscPeriodNs]
fld qword [esp + 4]
fmul [ebx + MEMOPB.tscPeriodNs]
fstp qword [esp + 4]
pop eax eax edx
mov bx,0300h        ; Float part is 3 digits, Integer part without template
.entryMode:
;---------- Write value and terminate string ----------------------------------;
call DoublePrint
mov al,0
stosb
clc
.error:  ; CF = 1 error flag and AL must be valid if exit by error
mov [esp + 28],al
popad
ret

