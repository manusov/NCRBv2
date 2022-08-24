;---------- Get system information --------------------------------------------;
;                                                                              ;
; INPUT:   None                                                                ;
;                                                                              ;
; OUTPUT:  CF = Error flag, 0(NC) = OK, 1(C) = Error                           ;
;          AL = Error code, valid if CF = 1                                    ;
;                                                                              ;
; Can destroy registers, volatile by Microsoft x64 calling convention.         ;
;------------------------------------------------------------------------------;

; TODO. REDESIGN CYCLE WITH MORE EFFECTIVE CPU REGISTERS USAGE,
; WITHOUT STACK VARIABLES.
; TODO. USE BINARY-TEXT-BINARY-TEXT-... sequence instead ALL BINARY - ALL TEXT,
;       because can delete temporary data ? 
;       But some texters can use results of >1 binary getter ?
; TODO. Move this to separate include file with subroutine.
; TODO. Remove "magic numbers" use names for bits positions.
; TODO. All operations with BINDLIST locate here, subject-specific at
;       info_***.inc
; TODO. Work buffers must be pre-blanked (contain 0)
;       before this procedure call, or make this in the procedure.

SystemInfo:
push rbx rsi rdi rbp r12 r13 r14 r15
mov r13,rsp                  ; R13 used for RSP restore before return 
and rsp,0FFFFFFFFFFFFFFF0h   ; Stack alignment by Microsoft x64 calling convention
sub rsp,32                   ; Parameter shadow by Microsoft x64 calling convention 
cld
;---------- Processor information ---------------------------------------------;
call InfoProcessor
jc .error
;---------- Initializing dynamical import WinAPI list -------------------------; 
lea rsi,[NAME_KERNEL32]       ; RSI = Pointer to libraries names array
mov rbx,[APP_DATA.lockedImportList] ; RBX = Pointer to functions names
lea rdi,[DYNA_IMPORT]               ; RDI = Pointer to fnc. addresses               
.scanLibs:
cmp byte [rsi],0
je .doneLibs
mov rcx,rsi                 ; RCX = Parm#1 = Pointer to module name
call [GetModuleHandle]      ; RAX = Return module handle
xchg r15,rax                ; R15 = DLL handle, XCHG is compact
.scanFnc:
cmp byte [rbx],0
je .doneFnc
mov rcx,r15                 ; RCX = Parm#1 = Module handle
mov rdx,rbx                 ; RDX = Parm#2 = Pointer to function name
call [GetProcAddress]       ; RAX = Return function address
stosq                       ; Store function address to import list
.skipName:
inc rbx
cmp byte [rbx],0
jne .skipName
inc rbx
jmp .scanFnc                ; Internal cycle for functions list per one library
.doneFnc:
inc rbx
.skipLib:
lodsb
cmp al,0
jne .skipLib
jmp .scanLibs               ; External cycle for libraries list
.doneLibs:
;---------- Get system information --------------------------------------------;
push r13
call InfoOperatingSystem    ; System and memory information parameters
jc @f
call InfoNativeOs           ; Native operating system information
mov rbp,[APP_BUFFERS.pointerSysInfo]
call InfoTopology           ; Get platform topology by WinAPI
jc @f
call InfoExtendedTopology   ; Get platform extended topology by WinAPI
jc @f
call InfoNumaDomains        ; Get NUMA nodes list
call InfoProcessorGroups    ; Get processor groups list
call InfoSmbios             ; Get SMBIOS structures list
call InfoAcpi               ; Get ACPI tables list
call InfoAffinizedCpuid     ; Get affinized CPUID dump
;---------- Build texts by system information ---------------------------------;
call TextOperatingSystem
call TextNativeOs
call TextTopology
jc @f
call TextExtendedTopology
call TextNumaDomains
call TextProcessorGroups
call TextSmbios
call TextAcpi
call TextAffinizedCpuid
clc
@@:
pop r13
jc .error
;---------- Data fields for system information screen ( screen 0 ) ------------;
; RDI = Pointer for bind buffer addressing.
; R15 = Pointer for CPU data addressing in the application variables 
lea rdi,[BIND_LIST]
lea r15,[CPU_DATA.vendorString]
;---------- Update threads and cores count if OS topology not available -------;
; This update actual for some old versions of Windows XP
lea rcx,[SYS_PARMS.summaryTopology]
mov eax,[rcx + SUMMARYTOPOLOGY.cores]
mov edx,[rcx + SUMMARYTOPOLOGY.threads]
test eax,eax
jz .updateTopology
test edx,edx
jnz .skipTopology
.updateTopology:
mov eax,[OS_DATA.systemInfo.dwNumberOfProcessors]
mov edx,eax
cmp eax,2
jne .doneTopology
test [r15 + CPUDATA.cpuFlags],0001b
jz .doneTopology 
shr eax,1
.doneTopology:
mov [rcx + SUMMARYTOPOLOGY.cores],eax
mov [rcx + SUMMARYTOPOLOGY.threads],edx
.skipTopology:
;---------- CPU vendor string, 12 chars ---------------------------------------; 
mov rax,r15
stosq
;---------- CPU signature: type, family, model, stepping ----------------------;
mov ax,STR_TFMS
call PoolStringWrite
mov eax,[r15 + CPUDATA.cpuSignature]
call HexPrint32
mov ax,STR_H
call PoolStringWrite
mov al,0
stosb 
;---------- CPU model string, maximum 48 chars --------------------------------;
lea rax,[r15 + CPUDATA.modelString]
stosq
;---------- TSC frequency -----------------------------------------------------;
mov ecx,32
mov al,0
rep stosb
push rdi
sub rdi,32
mov ax,STR_TSC
call PoolStringWrite
finit
push 1000000
fild qword [r15 + CPUDATA.tscClockHz] 
fidiv dword [rsp]
fstp qword [rsp]
pop rax
mov bx,0100h
call DoublePrint
mov ax,STR_MHZ
call PoolStringWrite
pop rdi
;---------- CPU instructions and OS context management features bitmaps -------;
; Include SSE support flag for context management at thread callback routine.
mov rax,[r15 + CPUDATA.extractedFeaturesBitmap]
stosq
and eax,0010b  ; Bit EAX.1 = SSE support flag
shr eax,1
mov [SYS_PARMS.sseSupported],eax
mov rax,[r15 + CPUDATA.extractedAvx512Bitmap]
stosq
mov rax,[r15 + CPUDATA.extractedContextBitmap]
stosq
;---------- Set cache summary by CPUID info if topological WinAPI missing -----;
lea rsi,[CPU_CACHE_DATA]
lea rdi,[SYS_PARMS.summaryCache]
mov ecx,5
.updateCaches:
mov rax,[rsi + CPUCACHEDATA.cpuidTraceCache]
mov rdx,[rdi + SUMMARYCACHE.sizeTrace]
test rax,rax
jz .noThisCache 
test rdx,rdx
jnz .noThisCache
mov [rdi + SUMMARYCACHE.sizeTrace],rax
mov [rdi + SUMMARYCACHE.countTrace],1
.noThisCache:
add rsi,12
add rdi,12
loop .updateCaches
;---------- Virtualization technology name string, depend on CPU vendor -------;
; R15 = Pointer for CPU data addressing in the application variables 
lea rdx,[BIND_LIST.bindCpu]
mov ax,STR_SIGN_AMD
mov rsi,[APP_DATA.lockedStrings]
call IndexString                      ; RSI = Pointer to pattern for detect AMD
lea rdi,[r15 + CPUDATA.vendorString]  ; RDI = Pointer to CPU signature after CPUID
mov ecx,12                            ; RCX = CPU signature length
repe cmpsb
mov ax,STR_SVM                        ; Show AMD SVM if vendor = AMD
mov ebx,12                            ; extractedVeaturesBitmap.12 = AMD SVM 
je .vendorDetected
mov ax,STR_VMX                        ; Show Intel VMX otherwise
mov bl,11                             ; extractedVeaturesBitmap.11 = Intel VMX
.vendorDetected:
mov rcx,[r15 + CPUDATA.extractedFeaturesBitmap] 
bt rcx,rbx
setc bl
mov [rdx + BINDCPU.secondBitmap],rbx  ; Argument for SET_BOOL script command
mov rsi,[APP_DATA.lockedStrings]
call IndexString
mov [rdx + BINDCPU.secondVmm],rsi     ; Argument for SET_PTR script command 
;---------- Operating System data fields visualized at for screen 0 -----------;
lea rdx,[BIND_LIST.bindSys]
lea rdi,[rdx + BINDSYS.procTotal]
mov eax,[OS_DATA.activeProcessorCount]      ; Processors total
test eax,eax
jnz @f
mov eax,[OS_DATA.systemInfo.dwNumberOfProcessors]
@@:
mov bl,0
call DecimalPrint32
mov al,0
stosb
lea rdi,[rdx + BINDSYS.groups]
mov eax,[OS_DATA.activeProcessorGroupCount] ; Processors groups
call DecimalPrint32
mov al,0
stosb
lea rdi,[rdx + BINDSYS.procCur]
mov eax,[OS_DATA.systemInfo.dwNumberOfProcessors]
call DecimalPrint32
mov al,0
stosb
lea rdi,[rdx + BINDSYS.numaNodes]
mov eax,[OS_DATA.numaNodeCount]  ; NUMA domains
call DecimalPrint32
mov al,0
stosb
lea rdi,[rdx + BINDSYS.memPhys]
mov rax,[OS_DATA.memoryStatusEx.ullTotalPhys]
mov bl,2
call SizePrint64
mov al,0
stosb
lea rdi,[rdx + BINDSYS.memAvail]
mov rax,[OS_DATA.memoryStatusEx.ullAvailPhys]
call SizePrint64
mov al,0
stosb
lea rdi,[rdx + BINDSYS.largePage]
mov rax,[OS_DATA.largePageSize]
mov bl,0FFh
call SizePrint64
mov al,0
stosb
lea rdi,[rdx + BINDSYS.largeEnable]
cmp [OS_DATA.largePageEnable],0
mov ax,STR_OS_DISABLED
je @f
mov ax,STR_OS_ENABLED
@@:
call PoolStringWrite
mov al,0
stosb
;---------- Platform topology by OS data fields visualized at for screen 0 ----;
; Instruction cache , L1
lea rbp,[SYS_PARMS.summaryCache]
lea rsi,[rbp + SUMMARYCACHE.sizeL1C]
lea rdi,[BIND_LIST.bindCache.l1c]
mov cl,00000001b
call HelperSysinfoTopologyCache           ; L1 instruction cache
lea rsi,[rbp + SUMMARYCACHE.sizeTrace]
lea rdi,[BIND_LIST.bindCache.l1c]
mov cl,00000001b
test [BIND_LIST.bindCache.cacheBitmap],cl
jnz @f
call HelperSysinfoTopologyTrace           ;  cache
@@:
;---------- Data cache , L1 ---------------------------------------------------; 
lea rsi,[rbp + SUMMARYCACHE.sizeL1D]
lea rdi,[BIND_LIST.bindCache.l1d]
mov cl,00000010b
call HelperSysinfoTopologyCache
;---------- Unified cache , L2 ------------------------------------------------;
lea rsi,[rbp + SUMMARYCACHE.sizeL2U]
lea rdi,[BIND_LIST.bindCache.l2u]
mov cl,00000100b
call HelperSysinfoTopologyCache
;---------- Unified cache , L3 ------------------------------------------------;
lea rsi,[rbp + SUMMARYCACHE.sizeL3U]
lea rdi,[BIND_LIST.bindCache.l3u]
mov cl,00001000b
call HelperSysinfoTopologyCache
;---------- Unified cache , L4 ------------------------------------------------;
lea rsi,[rbp + SUMMARYCACHE.sizeL4U]
lea rdi,[BIND_LIST.bindCache.l4u]
mov cl,00010000b
call HelperSysinfoTopologyCache
;--- Support screen 0 threads, cores, sockets by extended topology WinAPI -----;
lea rdx,[BIND_LIST]
lea r8,[SYS_PARMS.summaryTopology]
mov bl,0
lea rdi,[rdx + BINDLIST.bindTopology.threads]
mov eax,[r8 + SUMMARYTOPOLOGY.threads]
call DecimalPrint32
mov al,0
stosb
lea rdi,[rdx + BINDLIST.bindTopology.cores]
mov eax,[r8 + SUMMARYTOPOLOGY.cores]
call DecimalPrint32
mov al,0
stosb
lea rdi,[rdx + BINDLIST.bindTopology.sockets]
mov eax,[r8 + SUMMARYTOPOLOGY.sockets]
test eax,eax
jnz @f
inc eax
@@:
call DecimalPrint32
mov al,0
stosb
;---------- Support screen 0 show P- and E-cores for hybrid CPU topology ------;
lea rdi,[rdx + BINDLIST.bindTopology.pCores]
mov eax,[r8 + OFFSET_P_CORES + SUMMARYTOPOLOGY.cores]
mov cl,00000001b
call HelperSysinfoHybridTopology
lea rdi,[rdx + BINDLIST.bindTopology.eCores]
mov eax,[r8 + OFFSET_E_CORES + SUMMARYTOPOLOGY.cores]
mov cl,00000010b
call HelperSysinfoHybridTopology
;---------- NUMA domains affinity masks at screen 0 ---------------------------;
mov rsi,[DYNA_PTR.listNuma.objectStart]
mov rcx,[DYNA_PTR.listNuma.objectStop]
lea rdi,[BIND_LIST.bindSys.masksList]
lodsd
test eax,eax
jnz .extendedNuma
;---------- NUMA without processor groups -------------------------------------;
xor edx,edx
.masksStd:
cmp rsi,rcx
jae .doneNuma
test edx,edx
jz .firstMaskStd
cmp edx,2
jb .nextMaskStd
ja .skipMaskStd
mov eax,', ..'
stosd
mov al,'.'
stosb
jmp .skipMaskStd
.nextMaskStd:
mov ax,', '
stosw
.firstMaskStd:
mov rax,[rsi + 04]
call HexPrint64
mov al,'h'
stosb
.skipMaskStd:
add rsi,20
inc edx
jmp .masksStd
;---------- NUMA with processor groups ----------------------------------------;
.extendedNuma:
xor edx,edx
.masksExt:
cmp rsi,rcx
jae .doneNuma
test edx,edx
jz .firstMaskExt
cmp edx,2
jb .nextMaskExt
ja .skipMaskExt 
mov eax,', ..'
stosd
mov al,'.'
stosb
jmp .skipMaskExt 
.nextMaskExt:
mov ax,', '
stosw
.firstMaskExt:
mov rax,[rsi + 04]
call HexPrint64
mov al,'h'
stosb
.skipMaskExt:
add rsi,28
inc edx
jmp .masksExt
;---------- Done for both branches --------------------------------------------;
.doneNuma:
mov al,0
stosb
;---------- Data fields for ACPI ----------------------------------------------;
; Copy from temporary buffer to BINDLIST, transit copy used for don't access
; BINDLIST at low level.
lea rsi,[ACPI_DATA]
lodsb
mov [BIND_LIST.acpiEnable],al
lea rdi,[BIND_LIST.bindMadt]
mov ecx,sizeof.BINDACPI * 2  ; * 2 because MADT and SRAT tables show at screen 0
rep movsb
;--- Data fields for memory and cache benchmarks setup screen ( screen 1 ) ----;
; Benchmark methods availability based on CPU instructions 
lea rdi,[BIND_LIST]
mov eax,dword [CPU_DATA.extractedFeaturesBitmap]
mov edx,dword [CPU_DATA.extractedContextBitmap]
mov rbx,0001000000000555h   ; Latency by LCM, REP MOVS, REP STOS, NOT, MOV (3)
test al,00000001b
jz @f
or rbx,00015000h      ; MMX (3) 
@@:
test al,00000010b
jz @f
or rbx,00540000h      ; SSE (3) 
@@:
test dl,00000010b
jz .noavx             ; If AVX context not supported, skip AVX, FMA, Gather
test al,10000000b
jz @f
or rbx,15000000h      ; AVX (3) 
@@:
test ah,01000000b
jz @f
bts rbx,36            ; FMA 256
@@:
test ah,00000001b
jz @f
bts rbx,40            ; VGATHERQPD 256
@@:
.noavx:
mov cl,dl
and cl,00011100b
cmp cl,00011100b
jne @f                ; If AVX512 context not supported, skip AVX512, FMA512, Gather-Scatter 512
test ah,00000010b
jz @f
mov rcx,144540000000h ; AVX512 (3), FMA 512, VGATHERQPD 512, VSCATTERQPD 512
or rbx,rcx 
@@:
test ah,10000000b
jz @f
bts rbx,46            ; CLZERO (AMD)
@@:
test ah,00000100b
jz @f
bts rbx,50            ; Latency by RDRAND
@@:
test al,00000100b
jz @f
bts rbx,60            ; SSE2 (for vector brief)
@@:
bt eax,16
jnc @f
bts rbx,62           ; x87 (for vector brief, redundant because run criteria)
@@:
mov ecx,0C0000000h   ; Read AVX512, option 15, enable and select bits = 30, 31 
test ebx,ecx
jnz @f
mov ecx,003000000h   ; Read AVX256, option 12, enable and select bits = 24, 25
test ebx,ecx
jnz @f
mov ecx,0000C0000h   ; Read SSE128, option 9, enable and select bits = 18, 19
test ebx,ecx
jnz @f
mov ecx,000003000h   ; Read MMX, option 6, enable and select bits = 12, 13
test ebx,ecx
jnz @f
mov ecx,000000003h   ; Read MOV, option 0, enable and select bits = 0, 1
@@:
or rbx,rcx           ; Select default ASM method by CPU features support check

; UNDER CONSTRUCTION RESTRICTIONS
; TODO. VERIFY GATHER/SCATTER/CLZERO PATTERNS AND UNLOCK IT
; mov rax,0FFFF00FFFFFFFFFFh
; mov rax,0FFFF03FFFFFFFFFFh
  mov rax,0FFFF3FFFFFFFFFFFh  ; THIS LOCK CLZERO ONLY
  and rbx,rax
; UNDER CONSTRUCTION RESTRICTIONS

mov qword [rdi + BINDLIST.setMemMethod],rbx
;---------- Bits: AX.0 = nontemporal = f(sse), AX.2 = force x32 = off ---------;
xor eax,eax
bt ebx,18
setc al
mov word [rdi + BINDLIST.setMemOption],ax
;--- Benchmark objects: cache memory, DRAM or user-defined block size ---------;
; Plus custom block size
lea rsi,[SYS_PARMS.summaryCache]
mov ebx,010100000000b      ; Custom block, DRAM
xor eax,eax
mov rcx,[rsi + SUMMARYCACHE.sizeL1D]
jrcxz @f 
or bl,00000011b            ; L1 data cache, presence and select
xchg rax,rcx
@@:
mov rcx,[rsi + SUMMARYCACHE.sizeL2U]
jrcxz @f
or bl,00000100b            ; L2 unified cache presence
test rax,rax
jnz @f
xchg rax,rcx
or bl,00001000b            ; L2 unified cache select
@@:
mov rcx,[rsi + SUMMARYCACHE.sizeL3U]
jrcxz @f
or bl,00010000b            ; L3 unified cache presence
test rax,rax
jnz @f
xchg rax,rcx
or bl,00100000b            ; L3 unified cache select
@@:
mov rcx,[rsi + SUMMARYCACHE.sizeL4U]
jrcxz @f
or bl,01000000b            ; L4 unified cache presence
test rax,rax
jnz @f
xchg rax,rcx
or bl,10000000b            ; L4 unified cache select
@@:
test rax,rax
jnz @f
or bh,00001000b
mov eax,DEFAULT_CACHE_BLOCK
@@:
mov dword [rdi + BINDLIST.setMemObject],ebx
mov [rdi + BINDLIST.setBlkCustom],rax
;---------- Benchmark option: Hybrid cores ------------------------------------;
lea rsi,[SYS_PARMS]
mov bx,00000010b
cmp [rsi + SYSPARMS.hybridMode],0
je @f
mov bl,01110101b   ; Unaware, P+E, performance, effective
@@:
mov word [rdi + BINDLIST.setMemHybrid],bx
;---------- Hyper-Threading (SMT) detection with hybrid cores support ---------;
xor ebx,ebx   ; EBX = bitmap for GUI widgets
xor edx,edx   ; EDX = bitmap for detection procedure
mov eax,[rsi + SYSPARMS.summaryTopology.cores]
test eax,eax
jz @f
cmp eax,[rsi + SYSPARMS.summaryTopology.threads]
je @f
mov dl,1      ; Hyper-threading by legacy criteria = bit EDX.0
@@:
mov eax,[rsi + SYSPARMS.perfCoreTopology.cores]
test eax,eax
jz @f
cmp eax,[rsi + SYSPARMS.perfCoreTopology.threads]
je @f
mov dh,1      ; Hyper-threading by hybrid P-cores criteria = bit EDX.8
@@:
mov eax,[rsi + SYSPARMS.effCoreTopology.cores]
test eax,eax
jz @f
cmp eax,[rsi + SYSPARMS.effCoreTopology.threads]
je @f
bts edx,16    ; Hyper-threading by hybrid E-cores criteria = bit EDX.16
@@:
mov dword [rsi + SYSPARMS.htAll],edx 
test edx,00010101h
jz @f
or bl,00000100b   ; Hyper threading (SMT), for non-hybrid and hybrid,
@@:               ; for GUI widget, bit EBX.2
;---------- Benchmark options: SMP/SMT ----------------------------------------;
lea rsi,[SYS_PARMS.summaryTopology]
; xor ebx,ebx
mov eax,[rsi + SUMMARYTOPOLOGY.threads]
cmp eax,2
jb @f
inc ebx           ; Parallel threads (SMP)
@@:
; shr eax,1
; jz @f
; cmp eax,[rsi + SUMMARYTOPOLOGY.cores]
; jne @f
; or bl,00000100b   ; Hyper threading (SMT), legacy for non-hybrid
; @@:
;---------- Benchmark option: Processor groups --------------------------------; 
lea rsi,[OS_DATA]
cmp [rsi + OSDATA.activeProcessorGroupCount],2
jb @f
or bl,00010000b   ; Processor groups (PG)
@@:
mov word [rdi + BINDLIST.setMemSmp],bx
;---------- Benchmark option: NUMA --------------------------------------------;
mov bx,00000010b
cmp [rsi + OSDATA.numaNodeCount],2
jb @f
mov bl,01110101b   ; Unaware, Single domain, optimal, non-optimal
@@:
mov word [rdi + BINDLIST.setMemNuma],bx
;---------- Benchmark option: Large Pages -------------------------------------;
xor ebx,ebx
cmp [rsi + OSDATA.largePageEnable],ebx
je @f
inc ebx
@@:
mov word [rdi + BINDLIST.setMemLpages],bx
;---------- Benchmark options, set unconditionally when defaults set ----------;
mov word [rdi + BINDLIST.setMemAccess],00000010b  ; Access step
mov word [rdi + BINDLIST.setMemPref],000000010b   ; Prefetch modes
mov word [rdi + BINDLIST.setMemMeas],11010101b    ; Measure precision modes
mov word [rdi + BINDLIST.setMemPix],00010111b     ; Pixels approximation modes
mov word [rdi + BINDLIST.setMemSilent],00000001b  ; Silent draw mode
;---------- Text blocks for viewers -------------------------------------------;
; This 12 pointers MUST BE SEQUENTAL, because accessed in the cycle:
; 1.  Operating system information
; 2.  Native operating system information
; 3.  Topology information by operating system, text block 1, list
; 4.  Topology information by operating system, text block 2, summary
; 5.  Extended topology information by OS, text block 1, list
; 6.  Extended topology information by OS, text block 2, summary
; 7.  NUMA domains list by OS
; 8.  Processor groups list by OS
; 9.  SMBIOS structures   
; 10. ACPI tables, text block 1 = tables list
; 11. ACPI tables, text block 2 = tables detection by data base
; 12. Affinized CPUID dump
mov ecx,TEXT_SCREEN_COUNT
lea rsi,[DYNA_PTR.textOs.objectStart]
add rdi,BINDLIST.viewOs
@@:
movsq
lodsq    ; This instead ADD RSI,8
loop @b
;---------- Exit points -------------------------------------------------------;
clc      ; CF = 0, means no errors
.done:
mov rsp,r13
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret
.error:
stc      ; CF = 1, means error, AL contain error code
jmp .done

;---------- Helper for write cache summary by platform topology WinAPI --------;
;           For code, data and unified caches.                                 ;
;                                                                              ; 
; INPUT:   RSI = Source pointer for cache size and count variables             ;
;          RDI = Destination pointer for write text string                     ;
;          CL  = Bitmap for set GUI objects (cache items) active               ;      
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperSysinfoTopologyCache:
xor eax,eax
cmp [rsi + OFFSET_P_CORES + 0],rax
jne .hybrid
cmp [rsi + OFFSET_E_CORES + 0],rax
jne .hybrid
.noHybrid:
mov rax,[rsi + 0]
mov edx,[rsi + 8]
test rax,rax
jz .nodata
test edx,edx
jz .nodata
or [BIND_LIST.bindCache.cacheBitmap],cl
call HelperTopologyCacheParms
.done:
mov al,0
stosb
ret
.nodata:
mov ax,STR_N_A
call PoolStringWrite
jmp .done
;---------- Support asymmetric cache for hybrid CPU ---------------------------;
.hybrid:
or [BIND_LIST.bindCache.cacheBitmap],cl
mov rax,[rsi + OFFSET_P_CORES + 0]
mov edx,[rsi + OFFSET_P_CORES + 8]
call HelperTopologyCacheParms
mov al,' '
stosb
push rsi
mov ax,STR_TP_PLUS
call PoolStringWrite
pop rsi
mov rax,[rsi + OFFSET_E_CORES + 0]
mov edx,[rsi + OFFSET_E_CORES + 8]
call HelperTopologyCacheParms
jmp .done

;---------- Helper for write cache parameters ---------------------------------;
;           Optimized for call twice if hybrid CPU asymmetric cache topology.  ;
;                                                                              ;
; INPUT:   RAX = Cache size, bytes, 64-bit value                               ;
;          EDX = Cache units count                                             ; 
;          RDI = Destination pointer for write text string                     ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperTopologyCacheParms:
push rsi
mov bl,0FFh
call SizePrint64
cmp edx,1
je .doneCount
mov al,' '
stosb
mov ax,STR_TP_X
call PoolStringWrite
xchg eax,edx
mov bl,0
call DecimalPrint32
.doneCount:
pop rsi
ret

;---------- Helper for write cache summary by platform topology WinAPI --------;
;           For trace cache.                                                   ;
;                                                                              ; 
; INPUT:   RSI = Source pointer for cache size and count variables             ;
;          RDI = Destination pointer for write text string                     ;
;          CL  = Bitmap for set GUI objects (cache items) active               ;      
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperSysinfoTopologyTrace:
mov rax,[rsi + 0]
shr eax,10
jz .nodata
cmp dword [rsi + 8],0
jz .nodata
or [BIND_LIST.bindCache.cacheBitmap],cl
push rax
mov ax,STR_TRACE
call PoolStringWrite
mov al,' '
stosb
pop rax
mov bl,0
call DecimalPrint32
mov al,' '
stosb
mov ax,STR_KUOPS
call PoolStringWrite
.done:
mov al,0
stosb
ret
.nodata:
mov ax,STR_N_A
call PoolStringWrite
jmp .done

;---------- Helper for write P and E cores for hybrid CPU ---------------------;
;           detected by platform topology WinAPI                               ;
;                                                                              ; 
; INPUT:   EAX = Objects count (P-cores or E-cores),                           ;
;                subroutine write string "n/a" if EAX = 0                      ;
;          RDI = Destination pointer for write text string                     ;
;          CL  = Bitmap for set GUI objects (P and E cores) active             ;      
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperSysinfoHybridTopology:
test eax,eax
jz .nodata
or [BIND_LIST.bindTopology.hybridBitmap],cl
mov bl,0
call DecimalPrint32
.done:
mov al,0
stosb
ret
.nodata:
mov ax,STR_N_A
call PoolStringWrite
jmp .done
