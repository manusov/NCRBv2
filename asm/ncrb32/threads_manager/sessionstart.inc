;--- Start measurement session ------------------------------------------------;
; Analysing user settings and system configuration, build test scenario        ;
; Main actions is:                                                             ;
;  1) Build IPB = f ( UPB, SYSPARMS ).                                         ;
;  2) Build NUMA domains list or single domain entry if NUMA not supported.    ;
;  3) Swap NUMA domains list if non-optimal remote access mode selected.       ;
;  4) Allocate memory for NUMA nodes.                                          ;
;  5) Build execution threads list = f ( NUMA nodes list ).                    ;
;  6) TSC frequency measurement.                                               ;
;  7) Run execution threads.                                                   ;
; See complementar routines: sessionstop.inc , sessionprogress.inc.            ; 
;                                                                              ;
; INPUT:   None in the registers                                               ;
;          UPB = User Parms. Block = user options settings                     ;
;          SYSPARMS = SystemParameters = system configuration                  ;
;                                                                              ;
; OUTPUT:  CF = Error flag, 0(NC) = No errors, 1(C) = Error detected.          ;
;          AL = Error code, valid if CF=1(C)                                   ;
;               00 = Memory allocation error                                   ;
;               01 = Memory release error                                      ;
;               02 = Time measurement error                                    ;
;               04 = Address arithmetics error                                 ;
;               04-0FFh = Reserved                                             ;
;          IPB = Input Parms. Block = Benchmarks arguments data, as transit    ;
;          OPB = Output Parms. Block = Benchmark results data                  ;
;          Threads List, NUMA nodes list                                       ;
;                                                                              ;
;          IPB = InputParms = result of this routine, benchmark scenario       ;
;          Threads List, NUMA nodes list                                       ;
;          OPB = OutputParms = partially modified by this subroutine           ;
;          (TSC measurements), prepared for SessionProgress subroutine.        ;
;                                                                              ;
; All CPU registers can be corrupted, this rule for complex modules:           ;
; directory threads_manager\:                                                  ;
;   sessionstart.inc, sessionstop.inc, sessionprogress.inc.                    ;
; directory gui\:                                                              ;
;   drawsessionstart.inc, drawsessionstop.inc                                  ;
;   but not drawsessionthread.inc because OS callback                          ;       
; Remember about PUSH-POP in the callers.                                      ;
;------------------------------------------------------------------------------;

; TODO. ERROR REPORTING.
; TODO. CPU UTILIZATION AFTER DRAW CHART DONE.
; TODO. Actualize next comments.
; TODO. Data labels convention: dataLabel or DataLabel ? See also vectorBrief.
; TODO. Labels convention.
; TODO. Comments convention.
; TODO. Fix NUMA support.
; TODO. Fix Thread Count support.

SessionStart:
;---------- Build IPB ( Input Parameters Block ) for benchmarking -------------; 
; IPB = f ( UPB, SYSPARMS ),  IPB content depends on:
; 1) UPB ( User Parameters Block ), user settings by options widgets.
; 2) SYSPARMS ( System Parameters ), platform configuration, detected when
;    NCRB application starts.
mov esi,MEM_UPB      ; ESI = Pointer to user parameters block (UPB)
mov edi,MEM_IPB      ; EDI = Pointer to input parameters block (IPB)  
mov ebx,SYS_PARMS    ; EBX = Pointer for sys. info detected at NCRB start 
;---------- Setup benchmark method --------------------------------------------; 
; assembler routine = f ( user settings method, test object, system config. )
; test object = f ( user settings object )
mov eax,[esi + MEMUPB.optionAsm]
mov edx,[esi + MEMUPB.optionTarget]
cmp eax,ASM_ARGUMENT_LIMIT
ja .limit
cmp edx,TARGET_LIMIT
ja .limit
;---------- Select non-temporal optimization mode -----------------------------;
mov ch,0               ; CH = 0 , means TEMPORAL mode
mov ebp,[esi + MEMUPB.optionNontemp]
mov [edi + MEMIPB.nonTemporalMode],ebp
cmp ebp,NONTEMPORAL_ON
je .ntMode
mov [edi + MEMIPB.updatedDistance],PD_NOT_USED  ; Remove prefetch distance option if not a DRAM
jmp .doneMode
;---------- Select prefetch distance for non-temporal optimization ------------;
.ntMode:
mov ecx,[esi + MEMUPB.optionDistance]
mov [edi + MEMIPB.updatedDistance],ecx
cmp ecx,PD_LONG
ja @f
sub ecx,PD_DEFAULT
jb @f
imul ecx,ecx,AM_BYTE_COUNT
add eax,ecx
@@:
;---------- Continue after executed or skipped distance selection -------------; 
mov cl,[NON_TEMPORAL_TRANSLATOR + eax]
cmp cl,0FFh
je .doneMode
mov ch,1               ; CH = 1 , means NON TEMPORAL mode
mov al,cl
jmp .doneMode
;---------- End of nontemporal and prefetch distance branches -----------------;
.limit:
xor eax,eax
.doneMode:
mov [edi + MEMIPB.updatedAsm],eax       ; Select assembler method ID
;---------- Setup tested object and mode --------------------------------------;
mov [edi + MEMIPB.updatedTarget],edx    ; Select target object (Cache/DRAM/Custom)
movzx ecx,ch
mov [edi + MEMIPB.nonTemporalMode],ecx
mov edx,[esi + MEMUPB.optionForce32]
mov [edi + MEMIPB.force32mode],edx
;---------- Setup asm routine address -----------------------------------------;
; set address and memory dump region, 
; note here EAX valid after previous fragment
push ebx
cmp eax,LATENCY_MODE
jae .latencyDump          ; Go if measurement mode = LATENCY
;---------- Bandwidth branch --------------------------------------------------;
push eax                  ; this branch for bandwidth patterns
jecxz .temporalPattern 
call GetBandwidthNonTemporalPattern  
pop eax
call GetBandwidthNonTemporalDump
jmp .doneDump
.temporalPattern:
call GetBandwidthTemporalPattern  
pop eax
call GetBandwidthTemporalDump
jmp .doneDump
;---------- Latency branch ----------------------------------------------------;
.latencyDump:             ; this branch for latency patterns
sub eax,LATENCY_MODE      ; first latency method ID must be 0
test edx,edx
jz .skip32x2 
add eax,LATENCY_MODE_COUNT
.skip32x2:
push eax
call GetLatencyPattern
mov [edi + MEMIPB.walkRoutine],eax  
pop eax
call GetLatencyDump
;---------- Branches done -----------------------------------------------------;
.doneDump:
mov [edi + MEMIPB.patternRoutine],ebx     ; Pointer to performance pattern 
mov [edi + MEMIPB.dumpMethodStart],eax    ; Pointer to hex-dumped part of pattern
mov [edi + MEMIPB.dumpMethodLength],edx   ; Hex-dumped part length
shl ecx,3                                 ; convert bytes to bits, means * 8
mov [edi + MEMIPB.operandWidth],ecx       ; Select instruction operand width
pop ebx
;---------- Setup number of threads -------------------------------------------;
; Use platform total number of threads if auto.
mov eax,1
cmp [esi + MEMUPB.optionParallel],PARALLEL_USED
jne .checkHT
;---------- Support threads count for hybrid topology -------------------------;
mov eax,[esi + MEMUPB.optionHybrid]
cmp eax,HYBRID_P_PLUS_E
je .hybridPplusE
cmp eax,HYBRID_P_ONLY
je .hybridP
cmp eax,HYBRID_E_ONLY
jne .threadsNonHybrid
;--- Hybrid, E-Cores only --- 
mov eax,[ebx + SYSPARMS.effCoreTopology.threads]
cmp [esi + MEMUPB.optionHT],HT_NOT_USED
jne .hybridSetup
cmp [ebx + SYSPARMS.htEcores],1
jne .hybridSetup
cmp eax,1
je .hybridSetup 
shr eax,1
jmp .hybridSetup 
;--- Hybrid, P-Cores only ---
.hybridP:
mov eax,[ebx + SYSPARMS.perfCoreTopology.threads]
cmp [esi + MEMUPB.optionHT],HT_NOT_USED
jne .hybridSetup
cmp [ebx + SYSPARMS.htPcores],1
jne .hybridSetup
cmp eax,1
je .hybridSetup 
shr eax,1
jmp .hybridSetup 
;--- Hybrid, P+E Cores ---
.hybridPplusE:
mov eax,[ebx + SYSPARMS.perfCoreTopology.threads]
cmp [esi + MEMUPB.optionHT],HT_NOT_USED
jne @f
cmp [ebx + SYSPARMS.htPcores],1
jne @f 
cmp eax,1
je @f 
shr eax,1
@@:
mov edx,[ebx + SYSPARMS.effCoreTopology.threads]
cmp [esi + MEMUPB.optionHT],HT_NOT_USED
jne @f
cmp [ebx + SYSPARMS.htEcores],1
jne @f 
cmp edx,1
je @f 
shr edx,1
@@:
add eax,edx
.hybridSetup:
test eax,eax
jnz .setThreads
;---------- Support threads count for non-hybrid topology ---------------------;
.threadsNonHybrid:
mov ecx,[DYNA_PTR.listNuma]
mov ecx,[ecx + 8]
mov edx,1
xor eax,eax
.countCpu:
test ecx,edx
jz .skipCpu
inc eax     ; EAX = processors at first NUMA domain
.skipCpu:
shl edx,1
jnz .countCpu
cmp [esi + MEMUPB.optionNUMA],NUMA_CURRENT_ONLY
je .checkHT      ; Go if threads count limited by first NUMA domain
mov edx,[OS_DATA.activeProcessorCount]             ; EDX = processors at all groups
cmp [esi + MEMUPB.optionPG],PG_OPTIMAL
je .checkHT      ; Go if use all processors from all processor groups
mov eax,[OS_DATA.systemInfo.dwNumberOfProcessors]  ; EAX = processors at current group
.checkHT:
cmp [esi + MEMUPB.optionHT],HT_NOT_USED
jne .setThreads  ; Skip divide if not a variant "HT supported but not used" 
cmp eax,1
je .setThreads   ; Skip divide if threads count = 1
shr eax,1        ; Threads count / 2  if Hyper-Threading supported but not used
.setThreads:
mov [edi + MEMIPB.updatedThreads],eax  ; Setup number of threads
;---------- Setup Hyper-Threading mode ----------------------------------------; 
; plus, see above dependences on HT for threads count
mov eax,[esi + MEMUPB.optionHT]
mov [edi + MEMIPB.updatedHT],eax       ; Copy Hyper-Threading option (UPB to IPB)
;---------- Option Hybrid -----------------------------------------------------;
mov eax,[esi + MEMUPB.optionHybrid]
mov [edi + MEMIPB.updatedHybrid],eax       ; Copy Hybrid option (UPB to IPB)
;---------- Setup Processor Group mode ----------------------------------------; 
; plus, see above dependences on PG for threads count
mov eax,[esi + MEMUPB.optionPG]
mov [edi + MEMIPB.updatedPG],eax       ; Copy Processor Groups option 
;---------- Setup NUMA mode ---------------------------------------------------; 
; plus, see above dependences on NUMA for threads count
mov eax,[esi + MEMUPB.optionNUMA]
mov [edi + MEMIPB.updatedNUMA],eax     ; Copy NUMA option
;---------- Setup Large Page mode ---------------------------------------------;
mov eax,[esi + MEMUPB.optionLP]
mov [edi + MEMIPB.updatedLP],eax       ; Copy Large Pages option
;---------- Setup measurement precision ( calibration repeats ) mode ----------;
mov eax,[esi + MEMUPB.optionMeasure]
mov [edi + MEMIPB.updatedMeasure],eax
mov ecx,BENCHMARKS_REPEATS    ; select measurements constants
; Note for "Run simple" mode, no differentiation adaptive/non-adaptive
; see also gui\drawsessionstart.inc, gui\drawsessionthread.inc
; for adaptive modes support
cmp al,MEASURE_BRIEF
je .repeatsSelected          ; Go with constants for fast (non precision) mode 
cmp al,MEASURE_B_ADAPTIVE
je .repeatsSelected          ; Go with constants for fast (non precision) mode
add ecx,REPEAT_SERIES_BYTES  ; use constants for slow (precision) mode
.repeatsSelected:
mov eax,[edi + MEMIPB.updatedTarget]
mov eax,[ecx + eax * REPEAT_ENTRY_BYTES]
cmp [edi + MEMIPB.updatedAsm],LATENCY_MODE
jb .modeSelected
xor edx,edx
mov ecx,LATENCY_DIVISOR  ; Important DIVISOR CONSTANT for latency mode
div ecx         ; If Latency mode, make measurement repeats / LATENCY_DIVISOR,
.modeSelected:
mov dword [edi + MEMIPB.measureRepeats + 0],eax  ; Setup number of measurement repeats
mov dword [edi + MEMIPB.measureRepeats + 4],0
; Note this measurement repeats value can be updated in the adaptive modes,
; for drawings, at subroutine gui\drawsessionstart.inc,
; at this subroutine sessionstart.inc set default approximation only 
;---------- Setup measurement scenario as block sizes sequence ----------------;
; for "Run simple" mode required one block size only 
;---------- Support block size detection for L1, L2 cache at hybrid topology --;
mov eax,[edi + MEMIPB.updatedTarget]
mov ecx,SUMMARYCACHE.sizeL1D
cmp al,TARGET_L1
je .cacheHybrid 
mov ecx,SUMMARYCACHE.sizeL2U
cmp al,TARGET_L2
jne .cacheNonHybrid 
.cacheHybrid:
mov eax,[esi + MEMUPB.optionHybrid]
cmp eax,HYBRID_P_PLUS_E
je .hybridCachePplusE
cmp eax,HYBRID_P_ONLY
je .hybridCacheP
cmp eax,HYBRID_E_ONLY
jne .cacheNonHybrid
;--- Hybrid, E-Cores only ---
mov eax,[ebx + SYSPARMS.effCoreCache + ecx]
jmp .hybridCacheSetup
;--- Hybrid, P-Cores only ---
.hybridCacheP:
mov eax,[ebx + SYSPARMS.perfCoreCache + ecx]
jmp .hybridCacheSetup
;--- Hybrid, P+E Cores ---
.hybridCachePplusE:
mov eax,[ebx + SYSPARMS.perfCoreCache + ecx]
mov edx,[ebx + SYSPARMS.effCoreCache + ecx]
cmp eax,edx
jae .hybridCacheSetup
xchg eax,edx 
.hybridCacheSetup:
xchg ecx,eax
test ecx,ecx
jnz .sizeCalc 
;---------- Support block size detection for non-hybrid topology --------------; 
.cacheNonHybrid:
;---------- Detect L1 cache ---------------------------------------------------;
mov eax,[edi + MEMIPB.updatedTarget]
xor edx,edx    ; Previous level for L1 is 0 , not exist
mov ecx,dword [ebx + SYSPARMS.summaryCache.sizeL1D]
cmp al,TARGET_L1
je .sizeCalc 
;---------- Detect L2 cache ---------------------------------------------------;
mov edx,ecx    ; Previous level for L2 is L1
mov ecx,dword [ebx + SYSPARMS.summaryCache.sizeL2U]
cmp al,TARGET_L2
je .sizeCalc 
;---------- Detect L3 cache ---------------------------------------------------;
mov edx,ecx    ; Previous level for L3 is L2
mov ecx,dword [ebx + SYSPARMS.summaryCache.sizeL3U]
cmp al,TARGET_L3
je .sizeCalc 
;---------- Detect L4 cache ---------------------------------------------------;
mov edx,ecx    ; Previous level for L4 is L3
mov ecx,DEFAULT_DRAM_BLOCK
cmp al,TARGET_L4               ; Note required add detect L4 cache here
je .sizeCalc
;---------- Detect DRAM -------------------------------------------------------;
cmp al,TARGET_DRAM
je .doneCalc
;---------- Support Custom block ----------------------------------------------;
mov eax,[esi + MEMUPB.customBlockStart]  ; Note requir. check for wrong block size
;---------- Restrictions for custom block size, set result at ECX -------------;
call CustomBlockSizeRestrictions
xchg ecx,eax
jmp .doneCalc
;---------- Block size arithmetic for cache -----------------------------------;
.sizeCalc:
shr ecx,1          ; ( Size/2 - 512 ) for "Run simple", but
cmp ecx,edx        ; other logic required for "Run drawings"
ja @f              ; Go skip correction if ( ECX = L(i)/2 ) > ( EDX = L(i-1) )
add ecx,edx        ; Support for exclusive cache L(i) / L(i-1) sizes ratio
@@:
sub ecx,512 
;---------- Done arithmetic, store block size ---------------------------------;
.doneCalc:
mov [edi + MEMIPB.startBlockSize],ecx  ; Setup Block Size
;---------- Setup approximation option, yet value copy only -------------------;
mov eax,[esi + MEMUPB.optionApprox]
mov [edi + MEMIPB.updatedApprox],eax   ; Copy X-approximation option (UPB to IPB)
;---------- Setup Processor Groups parameters ---------------------------------;
mov eax,[OS_DATA.activeProcessorGroupCount]
mov [edi + MEMIPB.groupsCount],eax     ; Setup number of Processor Groups
;---------- Setup NUMA domains parameters -------------------------------------;
mov eax,[OS_DATA.numaNodeCount]
mov [edi + MEMIPB.domainsCount],eax    ; Setup number of NUMA domains
;---------- Setup page size ---------------------------------------------------;
; select default ( typical 4KB ) or large ( typical 2M for x64 )
mov eax,[OS_DATA.systemInfo.dwPageSize]    ; EAX = Standard page size
cmp [edi + MEMIPB.updatedLP],LP_USED
jb @f
mov eax,[OS_DATA.largePageSize]            ; EAX = Large page size 
@@:
mov [edi + MEMIPB.pageSize],eax    ; Setup used Standard or Large Page size, bytes
;---------- Setup memory allocation parameters --------------------------------;
; align allocated size by page size
mov eax,[edi + MEMIPB.startBlockSize]
shl eax,1             ; Memory size * 2 because source+destination for copy
mov ecx,[edi + MEMIPB.pageSize]
dec ecx               ; for example, make addend 0FFFh from 1000h
add eax,ecx           ; add this addend
not ecx               ; for example, make mask FFFFF000h from 0FFFh
and eax,ecx           ; AND-masking with this mask
;---------- Check drawings mode -----------------------------------------------;
; big block required for service all sizes when draw Performance = F(Size)
cmp [esi + MEMUPB.runContext],0
je .skipForSimple      ; Go skip multiplication if simple measurement mode
;---------- This required for memory reservation per drawings session ---------;
bsr ecx,eax
mov edx,1
shl edx,cl
cmp eax,edx
jbe .skipAlignedExpand
lea eax,[edx * 2]      ; Make integer power of 2
.skipAlignedExpand:
;---------- X*5 for memory reservation per X axis values ----------------------;
imul eax,eax,5         ; Xmax = Xtarget * 5
.skipForSimple:
;---------- Store memory size per one thread and total ------------------------;
mov [edi + MEMIPB.memoryPerThread],eax   ; Setup memory size per one thread
mov ecx,[edi + MEMIPB.updatedThreads]
mul ecx                                  ; multiply by number of threads
mov [edi + MEMIPB.memoryTotal],eax       ; Setup memory size per all threads
;---------- Check memory size overflow, specific for 32-bit application -------;
test eax,eax
js .allocError           ; Go error if low 32 bits above or equal 2 GB 
test edx,edx             ; EDX = High 32 bits of product after MUL instruction
jnz .allocError          ; Go error if high 32 bits non zero, means above 4 GB
;--- IPB initialization as  IPB = f ( UPB, SYSPARMS ) done at this point ------;
; next steps is resources allocation. 
;---------- Build NUMA nodes list ---------------------------------------------; 
; this is FIRST step before memory allocation
; NUMA unaware mode by option and by system limitations,
; supported by transfer EAX = option value to subroutine BuildNumaNodesList 
mov ecx,NUMA_LIST                   ; ECX = Pointers to NUMA nodes list for build
mov edx,[edi + MEMIPB.updatedPG]    ; EDX = Processor Groups option
mov eax,[edi + MEMIPB.updatedNUMA]  ; EAX = NUMA option
call BuildNumaNodesList             ; CF returned, build list of NUMA nodes
jnc .numaValid                      ; Go if no errors, means EAX = NUMA nodes count
.allocError:
mov al,MSG_RUNTIME_ALLOC            ; otherwise AL = error status
stc
jmp .exit                           ; Go for error handling
.numaValid:
;--- Update NUMA nodes count after rejection nodes with affinity mask = 0 -----;
mov [edi + MEMIPB.domainsCount],eax
; todo. Note verify required).
; Support NUMA options by update IPB and NUMA nodes list
; Option = NUMA_NOT_SUPPORTED, NUMA_NO_CONTROL
;   NUMA unaware = not change prepared list 
; Option = NUMA_CURRENT_ONLY
;   NUMA single domain = reduce node count to 1, reduce CPU count = per node 0
; Option = NUMA_OPTIMAL 
;   NUMA optimal = not change prepared list
; Option = NUMA_NON_OPTIMAL 
;   NUMA not optimal = shift affinity masks by ring, example 1-2-3-4 to 4-1-2-3
;---------- Reduce domains count for NUMA single domain scenario --------------;
mov ecx,NUMA_LIST
cmp [edi + MEMIPB.updatedNUMA],NUMA_CURRENT_ONLY
jne .skipSingleDomain            ; Go if not a single NUMA domain mode
mov [edi + MEMIPB.domainsCount],1
cmp [edi + MEMIPB.updatedThreads],1
jbe .skipSingleDomain            ; Go if not a multi-thread mode
;--- This code executed only for NUMA Option = single domain 
xor eax,eax                 ; Pre-blank logical CPU count = 0
mov edx,dword [ecx + NUMACTRL.nodeAffinity]  ; Node 0 affinity mask
.onesCount:
shr edx,1
jnc .zeroBit
inc eax                     ; This counts "1" bits at node 0 affinity mask 
.zeroBit:
test edx,edx
jnz .onesCount              ; Cycle exit when all "1" shifted
cmp [esi + MEMUPB.optionHT],HT_NOT_USED
jne .skipHtRejection        ; Skip "Threads count / 2" if no HT rejection case
cmp eax,1
je .skipHtRejection         ; Skip "Threads count / 2" if Threads count = 1
shr eax,1                   ; Threads count / 2  if required HT rejection
.skipHtRejection:
mov [edi + MEMIPB.updatedThreads],eax  ; Update number of threads = f ( NUMA mode )
;--- End of conditionally executed code only for NUMA Option = single domain
;--- Swap affinity masks for NUMA Non Optimal scenario, force remote access ---;
; Note possible bug if assymetric domains (processors per domain).
; Note ESI destroyed by this step, but not used at next steps.
.skipSingleDomain:
cmp [edi + MEMIPB.updatedNUMA],NUMA_NON_OPTIMAL
jne .skipSwap     ; Go if NUMA mode not a "force non-optimal"
lea edx,[ecx + NUMACTRL.nodeAffinity]
mov esi,edx
mov ecx,[edi + MEMIPB.domainsCount]
jecxz .skipSwap   ; skip swap if domains count = 0
dec ecx           ; domains count - 1
jecxz .skipSwap   ; skip swap if domains count = 1
mov eax,[edx]     ; read mask from first NUMA domain entry
.swapCycle:
add edx,NUMACTRL_SIZE
xchg eax,[edx]   ; rotate masks at domains entries, example 1-2-3-4 to 4-1-2-3
loop .swapCycle  ; cycle for swapped domains
mov [esi],eax    ; write mask to last NUMA domain entry 
.skipSwap:
;---------- Both NUMA-aware / unaware memory allocation -----------------------; 
; inside subroutine MemAllocationNuma, 
; this si SECOND step with memory allocation,
; after NUMA nodes list build and optionally swapped
; Note EBX destroyed by this step, but not used at next steps.
mov eax,[edi + MEMIPB.memoryTotal]   ; EAX = Total memory allocated 
mov ecx,[edi + MEMIPB.domainsCount]  ; ECX = Number of NUMA nodes
;---------- Skip division buffer size by nodes count if threads count = 1 -----; 
cmp [edi + MEMIPB.updatedThreads],1
je .skipForSingle  ; Skip if threads = 1 and all memory allocated at node 0
xor edx,edx
div ecx            ; Divide  allocated memory / node count  for multi-thread 
.skipForSingle:
mov ebx,NUMA_LIST        ; EBX = Base address of NUMA nodes list
mov edx,ecx              ; EDX = Number of NUMA nodes 
xchg ecx,eax             ; ECX = Memory allocation size per one node
mov eax,[edi + MEMIPB.updatedLP]     ; EAX = Large Pages option
call MemAllocationNuma               ; CF returned
mov al,MSG_RUNTIME_ALLOC             ; AL = error status
jc .exit                             ; Go if error
;---------- Build threads list ------------------------------------------------;
; this is THIRD step after NUMA domains list build and its memory allocated
mov ecx,THREAD_LIST
mov edx,MEM_IPB
call BuildThreadsList     ; CF returned 
mov al,MSG_RUNTIME_ALLOC  ; AL = error status, valid if CF=1
jc .exit                  ; Go if error
;---------- Update visualized block#1, block#2 bases at IPB -------------------;
; Update from Threads List first entry
mov ecx,THREAD_LIST
mov eax,[ecx + THCTRL.base1]
mov [edi + MEMIPB.allocatedBlock1],eax
mov eax,[ecx + THCTRL.base2]
mov [edi + MEMIPB.allocatedBlock2],eax
;---------- Now IPB initialized -----------------------------------------------; 
; NUMA domains listed, its memory allocated, execution threads listed
; Next steps is TSC frequency measurements and benchmarks calibration
; Note repeat TSC frequency measurement after NCRB application start,
; for better adaptation to run conditions.
;---------- Start execution threads before per-iterations calls ---------------; 
mov ecx,THREAD_LIST
mov edx,[edi + MEMIPB.updatedThreads]
call ThreadsStart             ; CF returned
mov al,MSG_RUNTIME_TIMINGS    ; AL = error status, valid if CF=1 only
;---------- Initialization done -----------------------------------------------;
; next phase is per-iterations calls, CF actual here
.exit:
ret
