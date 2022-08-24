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
; TODO. CPU UTILIZATION AFTER DRAW DONE.
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
lea rsi,[MEM_UPB]     ; RSI = Pointer to user parameters block (UPB)
lea rdi,[MEM_IPB]     ; RDI = Pointer to input parameters block (IPB)  
lea rbx,[SYS_PARMS]   ; RBX = Pointer for sys. info detected at NCRB start 
;---------- Setup benchmark method --------------------------------------------; 
; assembler routine = f ( user settings method, test object, system config. )
; test object = f ( user settings object )
mov eax,[rsi + MEMUPB.optionAsm]
mov edx,[rsi + MEMUPB.optionTarget]
cmp eax,ASM_ARGUMENT_LIMIT
ja .limit
cmp edx,TARGET_LIMIT
ja .limit
;---------- Select non-temporal optimization mode -----------------------------;
mov ch,0               ; CH = 0 , means TEMPORAL mode
mov ebp,[rsi + MEMUPB.optionNontemp]
mov [rdi + MEMIPB.nonTemporalMode],ebp
cmp ebp,NONTEMPORAL_ON
je .ntMode
mov [rdi + MEMIPB.updatedDistance],PD_NOT_USED  ; Remove prefetch distance option if not a DRAM
jmp .doneMode
;---------- Select prefetch distance for non-temporal optimization ------------;
.ntMode:
mov ecx,[rsi + MEMUPB.optionDistance]
mov [rdi + MEMIPB.updatedDistance],ecx
cmp ecx,PD_LONG
ja @f
sub ecx,PD_DEFAULT
jb @f
imul ecx,ecx,AM_BYTE_COUNT
add rax,rcx
@@:
;---------- Continue after executed or skipped distance selection -------------; 
mov cl,[NON_TEMPORAL_TRANSLATOR + rax]
cmp cl,0FFh
je .doneMode
mov ch,1               ; CH = 1 , means NON TEMPORAL mode
mov al,cl
jmp .doneMode
;---------- End of nontemporal and prefetch distance branches -----------------;
.limit:
xor eax,eax
.doneMode:
mov [rdi + MEMIPB.updatedAsm],eax       ; Select assembler method ID
;---------- Setup tested object and mode --------------------------------------;
mov [rdi + MEMIPB.updatedTarget],edx    ; Select target object (Cache/DRAM/Custom)
movzx ecx,ch
mov [rdi + MEMIPB.nonTemporalMode],ecx
;---------- Setup asm. routine address, set address and memory dump region ----; 
; note here EAX valid after previous fragment
push rbx
cmp eax,LATENCY_MODE
jae .latencyDump          ; Go if measurement mode = LATENCY
;---------- Bandwidth branch --------------------------------------------------;
push rax                  ; this branch for bandwidth patterns
jrcxz .temporalPattern 
call GetBandwidthNonTemporalPattern  
pop rax
call GetBandwidthNonTemporalDump
jmp .doneDump
.temporalPattern:
call GetBandwidthTemporalPattern  
pop rax
call GetBandwidthTemporalDump
jmp .doneDump
;---------- Latency branch ----------------------------------------------------;
.latencyDump:             ; this branch for latency patterns
sub eax,LATENCY_MODE      ; first latency method ID must be 0
push rax
call GetLatencyPattern
mov [rdi + MEMIPB.walkRoutine],rax  
pop rax
call GetLatencyDump
;---------- Branches done -----------------------------------------------------;
.doneDump:
mov [rdi + MEMIPB.patternRoutine],rbx     ; Pointer to performance pattern 
mov [rdi + MEMIPB.dumpMethodStart],rax    ; Pointer to hex-dumped part of pattern
mov [rdi + MEMIPB.dumpMethodLength],edx   ; Hex-dumped part length
shl ecx,3                                 ; convert bytes to bits, means * 8
mov [rdi + MEMIPB.operandWidth],ecx       ; Select instruction operand width
pop rbx
;--- Setup number of threads, use platform total number of threads if auto ----;
; Use platform total number of threads if auto.
mov eax,1
cmp [rsi + MEMUPB.optionParallel],PARALLEL_USED
jne .checkHT
;---------- Support threads count for hybrid topology -------------------------;
mov eax,[rsi + MEMUPB.optionHybrid]
cmp eax,HYBRID_P_PLUS_E
je .hybridPplusE
cmp eax,HYBRID_P_ONLY
je .hybridP
cmp eax,HYBRID_E_ONLY
jne .threadsNonHybrid
;--- Hybrid, E-Cores only --- 
mov eax,[rbx + SYSPARMS.effCoreTopology.threads]
cmp [rsi + MEMUPB.optionHT],HT_NOT_USED
jne .hybridSetup
cmp [rbx + SYSPARMS.htEcores],1
jne .hybridSetup
cmp eax,1
je .hybridSetup 
shr eax,1
jmp .hybridSetup 
;--- Hybrid, P-Cores only ---
.hybridP:
mov eax,[rbx + SYSPARMS.perfCoreTopology.threads]
cmp [rsi + MEMUPB.optionHT],HT_NOT_USED
jne .hybridSetup
cmp [rbx + SYSPARMS.htPcores],1
jne .hybridSetup
cmp eax,1
je .hybridSetup 
shr eax,1
jmp .hybridSetup 
;--- Hybrid, P+E Cores ---
.hybridPplusE:
mov eax,[rbx + SYSPARMS.perfCoreTopology.threads]
cmp [rsi + MEMUPB.optionHT],HT_NOT_USED
jne @f
cmp [rbx + SYSPARMS.htPcores],1
jne @f 
cmp eax,1
je @f 
shr eax,1
@@:
mov edx,[rbx + SYSPARMS.effCoreTopology.threads]
cmp [rsi + MEMUPB.optionHT],HT_NOT_USED
jne @f
cmp [rbx + SYSPARMS.htEcores],1
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
mov rcx,[DYNA_PTR.listNuma]
mov rcx,[rcx + 8]
mov edx,1
xor eax,eax
.countCpu:
test rcx,rdx
jz .skipCpu
inc eax     ; EAX = processors at first NUMA domain
.skipCpu:
shl rdx,1
jnz .countCpu
cmp [rsi + MEMUPB.optionNUMA],NUMA_CURRENT_ONLY
je .checkHT      ; Go if threads count limited by first NUMA domain
mov edx,[OS_DATA.activeProcessorCount]             ; EDX = processors at all groups
cmp [rsi + MEMUPB.optionPG],PG_OPTIMAL
je .checkHT        ; Go if use all processors from all processor groups
mov eax,[OS_DATA.systemInfo.dwNumberOfProcessors]  ; EAX = processors at current group
.checkHT:
cmp [rsi + MEMUPB.optionHT],HT_NOT_USED
jne .setThreads    ; Skip divide if not a variant "HT supported but not used" 
cmp eax,1
je .setThreads     ; Skip divide if threads count = 1
shr eax,1          ; Threads count / 2  if Hyper-Threading supported but not used
.setThreads:
mov [rdi + MEMIPB.updatedThreads],eax  ; Setup number of threads
;---------- Setup Hyper-Threading mode ----------------------------------------; 
; plus, see above dependences on HT for threads count
mov eax,[rsi + MEMUPB.optionHT]
mov [rdi + MEMIPB.updatedHT],eax       ; Copy Hyper-Threading option (UPB to IPB)
;---------- Option Hybrid -----------------------------------------------------;
mov eax,[rsi + MEMUPB.optionHybrid]
mov [rdi + MEMIPB.updatedHybrid],eax   ; Copy Hybrid option (UPB to IPB)
;---------- Setup Processor Group mode ----------------------------------------; 
; plus, see above dependences on PG for threads count
mov eax,[rsi + MEMUPB.optionPG]
mov [rdi + MEMIPB.updatedPG],eax       ; Copy Processor Groups option 
;---------- Setup NUMA mode ---------------------------------------------------; 
; plus, see above dependences on NUMA for threads count
mov eax,[rsi + MEMUPB.optionNUMA]
mov [rdi + MEMIPB.updatedNUMA],eax     ; Copy NUMA option
;---------- Setup Large Page mode ---------------------------------------------;
mov eax,[rsi + MEMUPB.optionLP]
mov [rdi + MEMIPB.updatedLP],eax       ; Copy Large Pages option
;---------- Setup measurement precision ( calibration repeats ) mode ----------;
mov eax,[rsi + MEMUPB.optionMeasure]
mov [rdi + MEMIPB.updatedMeasure],eax
lea rcx,[BENCHMARKS_REPEATS]  ; select measurements constants
; Note for "Run simple" mode, no differentiation adaptive/non-adaptive
; see also gui\drawsessionstart.inc, gui\drawsessionthread.inc
; for adaptive modes support
cmp al,MEASURE_BRIEF
je .repeatsSelected          ; Go with constants for fast (non precision) mode 
cmp al,MEASURE_B_ADAPTIVE
je .repeatsSelected          ; Go with constants for fast (non precision) mode
add rcx,REPEAT_SERIES_BYTES  ; use constants for slow (precision) mode
.repeatsSelected:
mov eax,[rdi + MEMIPB.updatedTarget]
mov eax,[rcx + rax * REPEAT_ENTRY_BYTES]  ; note after this, bits RAX.63-32 = 0
cmp [rdi + MEMIPB.updatedAsm],LATENCY_MODE
jb .modeSelected
xor edx,edx
mov ecx,LATENCY_DIVISOR  ; Important DIVISOR CONSTANT for latency mode
div ecx         ; If Latency mode, make measurement repeats / LATENCY_DIVISOR,
.modeSelected:  ; note after this, bits RAX.63-32 = 0
mov [rdi + MEMIPB.measureRepeats],rax  ; Setup number of measurement repeats
; Note this measurement repeats value can be updated in the adaptive modes,
; for drawings, at subroutine gui\drawsessionstart.inc,
; at this subroutine sessionstart.inc set default approximation only 
;---------- Setup measurement scenario as block sizes sequence ----------------;
; for "Run simple" mode required one block size only 
;---------- Support block size detection for L1, L2 cache at hybrid topology --;
mov eax,[rdi + MEMIPB.updatedTarget]
mov ecx,SUMMARYCACHE.sizeL1D
cmp al,TARGET_L1
je .cacheHybrid 
mov ecx,SUMMARYCACHE.sizeL2U
cmp al,TARGET_L2
jne .cacheNonHybrid 
.cacheHybrid:
mov eax,[rsi + MEMUPB.optionHybrid]
cmp eax,HYBRID_P_PLUS_E
je .hybridCachePplusE
cmp eax,HYBRID_P_ONLY
je .hybridCacheP
cmp eax,HYBRID_E_ONLY
jne .cacheNonHybrid
;--- Hybrid, E-Cores only ---
mov eax,[rbx + SYSPARMS.effCoreCache + rcx]
jmp .hybridCacheSetup
;--- Hybrid, P-Cores only ---
.hybridCacheP:
mov eax,[rbx + SYSPARMS.perfCoreCache + rcx]
jmp .hybridCacheSetup
;--- Hybrid, P+E Cores ---
.hybridCachePplusE:
mov eax,[rbx + SYSPARMS.perfCoreCache + rcx]
mov edx,[rbx + SYSPARMS.effCoreCache + rcx]
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
mov eax,[rdi + MEMIPB.updatedTarget]
xor edx,edx    ; Previous level for L1 is 0 , not exist
mov rcx,[rbx + SYSPARMS.summaryCache.sizeL1D]
cmp al,TARGET_L1
je .sizeCalc 
;---------- Detect L2 cache ---------------------------------------------------;
mov rdx,rcx    ; Previous level for L2 is L1
mov rcx,[rbx + SYSPARMS.summaryCache.sizeL2U]
cmp al,TARGET_L2
je .sizeCalc 
;---------- Detect L3 cache ---------------------------------------------------;
mov rdx,rcx    ; Previous level for L3 is L2
mov rcx,[rbx + SYSPARMS.summaryCache.sizeL3U]
cmp al,TARGET_L3
je .sizeCalc 
;---------- Detect L4 cache ---------------------------------------------------;
mov rdx,rcx    ; Previous level for L4 is L3
mov rcx,DEFAULT_DRAM_BLOCK
cmp al,TARGET_L4               ; Note required add detect L4 cache here
je .sizeCalc
;---------- Detect DRAM -------------------------------------------------------;
cmp al,TARGET_DRAM
je .doneCalc
;---------- Support Custom block ----------------------------------------------;
mov rax,[rsi + MEMUPB.customBlockStart]  ; Note required check for wrong block size
;---------- Restrictions for custom block size, set result at RCX -------------;
call CustomBlockSizeRestrictions
xchg rcx,rax
jmp .doneCalc
;---------- Block size arithmetic for cache -----------------------------------;
.sizeCalc:
shr rcx,1            ; ( Size/2 - 512 ) for "Run simple", but
cmp rcx,rdx          ; other logic required for "Run drawings"
ja @f                ; Go skip correction if ( RCX = L(i)/2 ) > ( RDX = L(i-1) )
add rcx,rdx          ; Support for exclusive cache L(i) / L(i-1) sizes ratio
@@:
sub rcx,512 
;---------- Done arithmetic, store block size ---------------------------------;
.doneCalc:
mov [rdi + MEMIPB.startBlockSize],rcx  ; Setup Block Size
;---------- Setup approximation option, yet value copy only -------------------;
mov eax,[rsi + MEMUPB.optionApprox]
mov [rdi + MEMIPB.updatedApprox],eax   ; Copy X-approximation option (UPB to IPB)
;---------- Setup Processor Groups parameters ---------------------------------;
mov eax,[OS_DATA.activeProcessorGroupCount]
mov [rdi + MEMIPB.groupsCount],eax     ; Setup number of Processor Groups
;---------- Setup NUMA domains parameters -------------------------------------;
mov eax,[OS_DATA.numaNodeCount]
mov [rdi + MEMIPB.domainsCount],eax    ; Setup number of NUMA domains
;---------- Setup page size ---------------------------------------------------; 
; Select default ( typical 4KB ) or large ( typical 2M for x64 )
mov eax,[OS_DATA.systemInfo.dwPageSize]  ; RAX = Std. page
cmp [rdi + MEMIPB.updatedLP],LP_USED
jb @f
mov rax,[OS_DATA.largePageSize]          ; RAX = Large page size 
@@:
mov [rdi + MEMIPB.pageSize],rax          ; Setup used Standard or Large Page size, bytes
;---------- Setup memory allocation parameters --------------------------------;
; align allocated size by page size
mov rax,[rdi + MEMIPB.startBlockSize]
shl rax,1             ; Memory size * 2 because source+destination for copy
mov rcx,[rdi + MEMIPB.pageSize]
dec rcx               ; for example, make addend 0FFFh from 1000h
add rax,rcx           ; add this addend
not rcx               ; for example, make mask FFFFFFFFFFFFF000h from 0FFFh
and rax,rcx           ; AND-masking with this mask
;---------- Check drawings mode -----------------------------------------------;
; big block required for service all sizes when draw Performance = F(Size)
cmp [rsi + MEMUPB.runContext],0
je .skipForSimple      ; Go skip multiplication if simple measurement mode
;---------- This required for memory reservation per drawings session ---------;
bsr rcx,rax
mov edx,1
shl rdx,cl
cmp rax,rdx
jbe .skipAlignedExpand
lea rax,[rdx * 2]      ; Make integer power of 2
.skipAlignedExpand:
;---------- X*5 for memory reservation per X axis values ----------------------;
imul rax,rax,5         ; Xmax = Xtarget * 5
.skipForSimple:
;---------- Store memory size per one thread and total ------------------------;
mov [rdi + MEMIPB.memoryPerThread],rax   ; Setup memory size per one thread
mov ecx,[rdi + MEMIPB.updatedThreads]
mul rcx                                  ; multiply by number of threads
mov [rdi + MEMIPB.memoryTotal],rax       ; Setup memory size per all threads
;--- IPB initialization as  IPB = f ( UPB, SYSPARMS ) done at this point ------;
; next steps is resources allocation. 
;---------- Build NUMA nodes list ---------------------------------------------; 
; this is FIRST step before memory allocation
; NUMA unaware mode by option and by system limitations,
; supported by transfer R8D = option value to subroutine BuildNumaNodesList 
; TODO. Fix registry 32/64 field alignment.
; TODO. Add runtime errors messages boxes.
lea rcx,[NUMA_LIST]            ; RCX = Pointers to NUMA nodes list for build
mov edx,[rdi + MEMIPB.updatedPG]    ; EDX = Processor Groups option
mov r8d,[rdi + MEMIPB.updatedNUMA]  ; R8D = NUMA option
call BuildNumaNodesList             ; CF returned, build list of NUMA nodes
jnc .numaValid                      ; Go if no errors, means EAX = NUMA nodes count
mov al,MSG_RUNTIME_ALLOC            ; otherwise AL = error status
jmp .exit                           ; Go for error handling
.numaValid:
;--- Update NUMA nodes count after rejection nodes with affinity mask = 0 -----;
mov [rdi + MEMIPB.domainsCount],eax
; TODO. Note verify required.
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
lea rcx,[NUMA_LIST]
cmp [rdi + MEMIPB.updatedNUMA],NUMA_CURRENT_ONLY
jne .skipSingleDomain            ; Go if not a single NUMA domain mode
mov [rdi + MEMIPB.domainsCount],1
cmp [rdi + MEMIPB.updatedThreads],1
jbe .skipSingleDomain            ; Go if not a multi-thread mode
;---------- This code executed only for NUMA Option = single domain -----------; 
xor eax,eax                 ; Pre-blank logical CPU count = 0
mov rdx,qword [rcx + NUMACTRL.nodeAffinity]  ; Node 0 affinity mask
.onesCount:
shr rdx,1
jnc .zeroBit
inc eax                     ; This counts "1" bits at node 0 affinity mask 
.zeroBit:
test rdx,rdx
jnz .onesCount              ; Cycle exit when all "1" shifted
cmp [rsi + MEMUPB.optionHT],HT_NOT_USED
jne .skipHtRejection        ; Skip "Threads count / 2" if no HT rejection case
cmp eax,1
je .skipHtRejection         ; Skip "Threads count / 2" if Threads count = 1
shr eax,1                   ; Threads count / 2  if required HT rejection
.skipHtRejection:
mov [rdi + MEMIPB.updatedThreads],eax  ; Update number of threads = f ( NUMA mode )
;--- End of conditionally executed code only for NUMA Option = single domain --;
;--- Swap affinity masks for NUMA Non Optimal scenario, force remote access ---;
; TODO. Note possible bug if assymetric domains ( processors per domain ).
.skipSingleDomain:
cmp [rdi + MEMIPB.updatedNUMA],NUMA_NON_OPTIMAL
jne .skipSwap     ; Go if NUMA mode not a "force non-optimal"
lea rdx,[rcx + NUMACTRL.nodeAffinity]
mov r8,rdx
mov ecx,[rdi + MEMIPB.domainsCount]
jrcxz .skipSwap   ; skip swap if domains count = 0
dec ecx           ; domains count - 1
jrcxz .skipSwap   ; skip swap if domains count = 1
mov rax,[rdx]     ; read mask from first NUMA domain entry
.swapCycle:
add rdx,NUMACTRL_SIZE
xchg rax,[rdx]   ; rotate masks at domains entries, example 1-2-3-4 to 4-1-2-3
loop .swapCycle  ; cycle for swapped domains
mov [r8],rax     ; write mask to last NUMA domain entry 
.skipSwap:
;---------- Both NUMA-aware / unaware memory allocation -----------------------; 
; inside subroutine MemAllocationNuma, 
; this si SECOND step with memory allocation,
; after NUMA nodes list build and optionally swapped
mov rax,[rdi + MEMIPB.memoryTotal]
mov r8d,[rdi + MEMIPB.domainsCount]  ; R8 = Number of nodes
;--- Skip division buffer size by nodes count if threads count = 1 ------------; 
cmp [rdi + MEMIPB.updatedThreads],1
je .skipForSingle  ; Skip if threads = 1 and all memory allocated at node 0
xor edx,edx
div r8             ; Divide  allocated memory / node count  for multi-thread 
.skipForSingle:
lea rcx,[NUMA_LIST]               ; RCX = Base address of NUMA nodes list
mov rdx,rax                       ; RDX = Memory allocation size per one node
mov r9d,[rdi + MEMIPB.updatedLP]  ; R9D = Large Pages option
call MemAllocationNuma            ; CF returned
mov al,MSG_RUNTIME_ALLOC          ; AL = error status
jc .exit                          ; Go if error
;---------- Build threads list ------------------------------------------------;
; This is THIRD step after NUMA domains list build and its memory allocated
lea rcx,[THREAD_LIST]
lea rdx,[MEM_IPB]
call BuildThreadsList    ; CF returned 
mov al,MSG_RUNTIME_ALLOC ; AL = error status, valid if CF=1
jc .exit                 ; Go if error
;---------- Update visualized block#1, block#2 bases --------------------------;
; Update IPB from Threads List first entry
lea rcx,[THREAD_LIST]
mov rax,[rcx + THCTRL.base1]
mov [rdi + MEMIPB.allocatedBlock1],rax
mov rax,[rcx + THCTRL.base2]
mov [rdi + MEMIPB.allocatedBlock2],rax
;---------- Now IPB initialized -----------------------------------------------; 
; NUMA domains listed, its memory allocated, execution threads listed
; Next steps is TSC frequency measurements and benchmarks calibration
; Note repeat TSC frequency measurement after NCRB application start,
; for better adaptation to run conditions.
; Re-measure TSC clock before start benchmarks
;---------- Start execution threads before per-iterations calls ---------------; 
lea rcx,[THREAD_LIST]
mov edx,[rdi + MEMIPB.updatedThreads]
call ThreadsStart  ; CF returned
mov al,MSG_RUNTIME_TIMINGS           ; AL = error status, valid if CF=1 only
;---------- Initialization done -----------------------------------------------;
; next phase is per-iterations calls, CF actual here
.exit:
ret
