;---------- Get system information --------------------------------------------;
;                                                                              ;
; INPUT:   None                                                                ;
;                                                                              ;
; OUTPUT:  CF = Error flag, 0(NC) = OK, 1(C) = Error                           ;
;          AL = Error code, valid if CF = 1                                    ;
;                                                                              ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.        ;
;------------------------------------------------------------------------------;

; TODO. Optimize for use single call per subject, instead 2 calls: Info_ and Text_
; TODO. Move this to separate include file with subroutine. (?)
; TODO. Remove "magic numbers" use names for bits positions.
; TODO. All operations with BINDLIST locate here, subject-specific at
;       info_***.inc
; TODO. Work buffers must be pre-blanked (contain 0)
;       before this procedure call, or make this in the procedure.

SystemInfo:
push ebx esi edi ebp
cld
;---------- Processor information ---------------------------------------------;
call InfoProcessor
jc .error
;---------- Initializing dynamical import WinAPI list -------------------------;
mov ebx,[APP_DATA.lockedImportList]  ; EBX = to functions names
lea edi,[DYNA_IMPORT]
lea esi,[NAME_KERNEL32]     ; ESI = Pointer to libraries names array
.scanLibs:
cmp byte [esi],0
je .doneLibs
push esi                    ; Parm#1 = Pointer to module name
call [GetModuleHandle]      ; EAX = Return module handle
xchg ebp,eax                ; EBP = DLL handle, XCHG is compact
.scanFnc:
cmp byte [ebx],0
je .doneFnc
push ebx                    ; Parm#2 = Pointer to function name
push ebp                    ; Parm#1 = Module handle
call [GetProcAddress]       ; EAX = Return function address
stosd                       ; Store function address to import list
.skipName:
inc ebx
cmp byte [ebx],0
jne .skipName
inc ebx
jmp .scanFnc                ; Internal cycle for functions list per one library
.doneFnc:
inc ebx
.skipLib:
lodsb
cmp al,0
jne .skipLib
jmp .scanLibs               ; External cycle for libraries list
.doneLibs:
;---------- Check for WoW64 process: ia32 application under Win64 -------------;
lea edi,[OS_DATA.isWow64]
mov ecx,[DYNA_IMPORT._IsWow64Process]
jecxz .noWow64
call [GetCurrentProcess]
test eax,eax
jz .noWow64
push edi                ; Parm#2 = Pointer to BOOL WoW64 Process
push eax                ; Parm#1 = hProcess handle 
call ecx                ; Call function: IsWow64Process 
test eax,eax
jnz .doneWow64          ; Go if WoW64 detected
.noWow64:
mov dword [edi],0       ; Clear WoW64 flag if WoW64 not detected  
.doneWow64: 
;---------- Get system information --------------------------------------------;
call InfoOperatingSystem    ; System and memory information parameters
jc .error
call InfoNativeOs           ; Native operating system information
mov ebp,[APP_MEMORY]
call InfoTopology           ; Get platform topology by WinAPI
jc .error
call InfoExtendedTopology   ; Get platform extended topology by WinAPI
jc .error
call InfoNumaDomains        ; Get NUMA nodes list
call InfoProcessorGroups    ; Get processor groups list
call InfoAcpi               ; Get ACPI tables list
call InfoAffinizedCpuid     ; Get affinized CPUID dump
;---------- Build texts by system information ---------------------------------;
call TextOperatingSystem
call TextNativeOs
call TextTopology
jc .error
call TextExtendedTopology
call TextNumaDomains
call TextProcessorGroups
call TextAcpi
call TextAffinizedCpuid
;---------- Data fields for system information screen ( screen 0 ) ------------;
; EDI = Pointer for bind buffer addressing.
; EBP = Pointer for CPU data addressing in the application variables
lea edi,[BIND_LIST]
lea ebp,[CPU_DATA.vendorString]
mov ebx,ebp
;---------- CPU vendor string, 12 chars ---------------------------------------;
xchg eax,ebp
stosd
xor eax,eax
stosd
;---------- CPU signature: type, family, model, stepping ----------------------;
mov ax,STR_TFMS
call PoolStringWrite
mov eax,[ebx + CPUDATA.cpuSignature]
call HexPrint32
mov ax,STR_H
call PoolStringWrite
mov al,0
stosb 
;---------- CPU model string, maximum 48 chars --------------------------------;
lea eax,[ebx + CPUDATA.modelString]
stosd
xor eax,eax
stosd
;---------- TSC frequency -----------------------------------------------------;
mov ecx,32
mov al,0
rep stosb
push edi ebx
sub edi,32
mov ax,STR_TSC
call PoolStringWrite
finit
push 0 1000000
fild qword [ebx + CPUDATA.tscClockHz] 
fidiv dword [esp]
fstp qword [esp]
pop eax edx
mov bx,0100h
call DoublePrint
mov ax,STR_MHZ
call PoolStringWrite
pop ebx edi
;---------- CPU instructions and OS context management features bitmaps -------; 
; Include SSE support flag for context management at thread callback routine.
mov eax,dword [ebx + CPUDATA.extractedFeaturesBitmap + 0]
stosd
and eax,0010b  ; Bit EAX.1 = SSE support flag
shr eax,1
mov [SYS_PARMS.sseSupported],eax
mov eax,dword [ebx + CPUDATA.extractedFeaturesBitmap + 4]
stosd
mov eax,dword [ebx + CPUDATA.extractedAvx512Bitmap + 0]
stosd
mov eax,dword [ebx + CPUDATA.extractedAvx512Bitmap + 4]
stosd
mov eax,dword [ebx + CPUDATA.extractedContextBitmap + 0]
stosd
mov eax,dword [ebx + CPUDATA.extractedContextBitmap + 4]
stosd
;---------- Set cache summary by CPUID info if topological WinAPI missing -----;
lea esi,[CPU_CACHE_DATA]
lea edi,[SYS_PARMS.summaryCache]
mov ecx,5
.updateCaches:
mov eax,dword [esi + CPUCACHEDATA.cpuidTraceCache + 0]
mov edx,dword [esi + CPUCACHEDATA.cpuidTraceCache + 4]
mov ebp,eax
or ebp,eax
jz .noThisCache 
mov ebp,dword [edi + SUMMARYCACHE.sizeTrace + 0]
or ebp,dword [edi + SUMMARYCACHE.sizeTrace + 4]
jnz .noThisCache
mov dword [edi + SUMMARYCACHE.sizeTrace + 0],eax
mov dword [edi + SUMMARYCACHE.sizeTrace + 4],edx
mov [edi + SUMMARYCACHE.countTrace],1
.noThisCache:
add esi,12
add edi,12
loop .updateCaches
;---------- Virtualization technology name string, depend on CPU vendor -------;
; EBX = Pointer for CPU data addressing in the application variables 
lea edx,[BIND_LIST.bindCpu]
mov ax,STR_SIGN_AMD
mov esi,[APP_DATA.lockedStrings]
call IndexString                      ; ESI = Pointer to pattern for detect AMD
lea edi,[ebx + CPUDATA.vendorString]  ; EDI = Pointer to CPU signature after CPUID
mov ecx,12                            ; ECX = CPU signature length
repe cmpsb
mov ecx,dword [ebx + CPUDATA.extractedFeaturesBitmap]
mov ax,STR_SVM                        ; Show AMD SVM if vendor = AMD
mov ebx,12                            ; extractedVeaturesBitmap.12 = AMD SVM 
je .vendorDetected
mov ax,STR_VMX                        ; Show Intel VMX otherwise
mov bl,11                             ; extractedVeaturesBitmap.11 = Intel VMX
.vendorDetected:
bt ecx,ebx
setc bl
mov dword [edx + BINDCPU.secondBitmap + 0],ebx  ; Argument for SET_BOOL script command
mov dword [edx + BINDCPU.secondBitmap + 4],0
mov esi,[APP_DATA.lockedStrings]
call IndexString
mov dword [edx + BINDCPU.secondVmm + 0],esi     ; Argument for SET_PTR script command 
mov dword [edx + BINDCPU.secondVmm + 4],0
;---------- Operating System data fields visualized at for screen 0 -----------;
lea ecx,[BIND_LIST.bindSys]
lea edi,[ecx + BINDSYS.procTotal]
mov eax,[OS_DATA.activeProcessorCount]      ; Processors total
test eax,eax
jnz @f
mov eax,[OS_DATA.systemInfo.dwNumberOfProcessors]
@@:
mov bl,0
call DecimalPrint32
mov al,0
stosb
lea edi,[ecx + BINDSYS.groups]
mov eax,[OS_DATA.activeProcessorGroupCount] ; Processors groups
call DecimalPrint32
mov al,0
stosb
lea edi,[ecx + BINDSYS.procCur]
mov eax,[OS_DATA.systemInfo.dwNumberOfProcessors]
call DecimalPrint32
mov al,0
stosb
lea edi,[ecx + BINDSYS.numaNodes]
mov eax,[OS_DATA.numaNodeCount]  ; NUMA domains
call DecimalPrint32
mov al,0
stosb
lea edi,[ecx + BINDSYS.memPhys]
mov eax,dword [OS_DATA.memoryStatusEx.ullTotalPhys + 0]
mov edx,dword [OS_DATA.memoryStatusEx.ullTotalPhys + 4]
mov bl,2
call SizePrint64
mov al,0
stosb
lea edi,[ecx + BINDSYS.memAvail]
mov eax,dword [OS_DATA.memoryStatusEx.ullAvailPhys + 0]
mov edx,dword [OS_DATA.memoryStatusEx.ullAvailPhys + 4]
call SizePrint64
mov al,0
stosb
lea edi,[ecx + BINDSYS.largePage]
mov eax,[OS_DATA.largePageSize]
xor edx,edx
mov bl,0FFh
call SizePrint64
mov al,0
stosb
lea edi,[ecx + BINDSYS.largeEnable]
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
lea ebp,[SYS_PARMS.summaryCache]
lea esi,[ebp + SUMMARYCACHE.sizeL1C]
lea edi,[BIND_LIST.bindCache.l1c]
mov cl,00000001b
call HelperSysinfoTopologyCache           ; L1 instruction cache
lea esi,[ebp + SUMMARYCACHE.sizeTrace]
lea edi,[BIND_LIST.bindCache.l1c]
mov cl,00000001b
test [BIND_LIST.bindCache.cacheBitmap],cl
jnz @f
call HelperSysinfoTopologyTrace           ; Trace cache
@@:
;---------- Data cache , L1 ---------------------------------------------------; 
lea esi,[ebp + SUMMARYCACHE.sizeL1D]
lea edi,[BIND_LIST.bindCache.l1d]
mov cl,00000010b
call HelperSysinfoTopologyCache
;---------- Unified cache , L2 ------------------------------------------------;
lea esi,[ebp + SUMMARYCACHE.sizeL2U]
lea edi,[BIND_LIST.bindCache.l2u]
mov cl,00000100b
call HelperSysinfoTopologyCache
;---------- Unified cache , L3 ------------------------------------------------;
lea esi,[ebp + SUMMARYCACHE.sizeL3U]
lea edi,[BIND_LIST.bindCache.l3u]
mov cl,00001000b
call HelperSysinfoTopologyCache
;---------- Unified cache , L4 ------------------------------------------------;
lea esi,[ebp + SUMMARYCACHE.sizeL4U]
lea edi,[BIND_LIST.bindCache.l4u]
mov cl,00010000b
call HelperSysinfoTopologyCache
;--- Support screen 0 threads, cores, sockets by extended topology WinAPI -----;
lea edx,[BIND_LIST]
lea ecx,[SYS_PARMS.summaryTopology]
mov bl,0
lea edi,[edx + BINDLIST.bindTopology.threads]
mov eax,[ecx + SUMMARYTOPOLOGY.threads]
call DecimalPrint32
mov al,0
stosb
lea edi,[edx + BINDLIST.bindTopology.cores]
mov eax,[ecx + SUMMARYTOPOLOGY.cores]
call DecimalPrint32
mov al,0
stosb
lea edi,[edx + BINDLIST.bindTopology.sockets]
mov eax,[ecx + SUMMARYTOPOLOGY.sockets]
test eax,eax
jnz @f
inc eax
@@:
call DecimalPrint32
mov al,0
stosb
;---------- NUMA domains affinity masks at screen 0 ---------------------------;
mov esi,[DYNA_PTR.listNuma.objectStart]
mov ecx,[DYNA_PTR.listNuma.objectStop]
lea edi,[BIND_LIST.bindSys.masksList]
lodsd
test eax,eax
jnz .extendedNuma
;---------- NUMA without processor groups -------------------------------------;
xor edx,edx
.masksStd:
cmp esi,ecx
jae .doneNuma
test edx,edx
jz .firstMaskStd
cmp edx,4
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
mov eax,[esi + 04]
call HexPrint32
mov al,'h'
stosb
.skipMaskStd:
add esi,20
inc edx
jmp .masksStd
;---------- NUMA with processor groups ----------------------------------------;
.extendedNuma:
xor edx,edx
.masksExt:
cmp esi,ecx
jae .numaDone
test edx,edx
jz .firstMaskExt
cmp edx,4
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
mov eax,[esi + 04]
call HexPrint32
mov al,'h'
stosb
.skipMaskExt:
add esi,28
inc edx
jmp .masksExt
.numaDone:
mov al,0
stosb
;---------- Done for both branches --------------------------------------------;
.doneNuma:
mov al,0
stosb
;---------- Data fields for ACPI ----------------------------------------------;
; Copy from temporary buffer to BINDLIST, transit copy used for don't access
; BINDLIST at low level.
lea esi,[ACPI_DATA]
lodsb
mov [BIND_LIST.acpiEnable],al
lea edi,[BIND_LIST.bindMadt]
mov ecx,sizeof.BINDACPI * 2  ; * 2 because MADT and SRAT tables show at screen 0
rep movsb
;--- Data fields for memory and cache benchmarks setup screen ( screen 1 ) ----;
; Benchmark methods availability based on CPU instructions 
lea edi,[BIND_LIST]
mov eax,dword [CPU_DATA.extractedFeaturesBitmap]
mov edx,dword [CPU_DATA.extractedContextBitmap]
mov ebx,00000555h       ; Latency by LCM, REP MOVS, REP STOS, NOT, MOV (3)
mov ebp,00010000h
test al,00000001b
jz @f
or ebx,00015000h      ; MMX (3) 
@@:
test al,00000010b
jz @f
or ebx,00540000h      ; SSE (3) 
@@:
test al,10000000b
jz @f
test dl,00000010b
jz @f
or ebx,15000000h      ; AVX (3) 
@@:
test ah,00000010b
jz @f
mov cl,dl
and cl,00011100b
cmp cl,00011100b
jne @f
or ebx,40000000h      ; AVX512 (3), FMA 512, VGATHERQPD 512, VSCATTERQPD 512
or ebp,00001445h 
@@:
test ah,01000000b
jz @f
bts ebp,4             ; FMA 256
@@:
test ah,00000001b
jz @f
bts ebp,8             ; VGATHERQPD 256
@@:
test ah,10000000b
jz @f
bts ebp,14            ; CLZERO (AMD)
@@:
test ah,00000100b
jz @f
bts ebp,18            ; Latency by RDRAND
@@:
test al,00000100b
jz @f
bts ebp,28            ; SSE2 (for vector brief)
@@:
bt eax,16
jnc @f
bts ebp,30           ; x87 (for vector brief, redundant because run criteria)
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
or ebx,ecx            ; Select default ASM method by CPU features support check
; UNDER CONSTRUCTION RESTRICTIONS
and ebp,0FFFF00FFh
; UNDER CONSTRUCTION RESTRICTIONS
mov [edi + BINDLIST.setMemMethod + 0],ebx
mov [edi + BINDLIST.setMemMethod + 4],ebp
;--- Benchmark objects: cache memory, DRAM or user-defined block size ---------;
; Plus custom block size
lea ecx,[SYS_PARMS.summaryCache]
mov ebx,010100000000b      ; Custom block, DRAM
xor eax,eax
cdq
mov ebp,dword [ecx + SUMMARYCACHE.sizeL1D + 0]
or ebp,dword [ecx + SUMMARYCACHE.sizeL1D + 4]
jz @f 
or bl,00000011b            ; L1 data cache
mov eax,dword [ecx + SUMMARYCACHE.sizeL1D + 0]
mov edx,dword [ecx + SUMMARYCACHE.sizeL1D + 4]
@@:
mov ebp,dword [ecx + SUMMARYCACHE.sizeL2U + 0]
or ebp,dword [ecx + SUMMARYCACHE.sizeL2U + 4]
jz @f
or bl,00000100b            ; L2 unified cache
mov ebp,eax
or ebp,edx
jnz @f
mov eax,dword [ecx + SUMMARYCACHE.sizeL2U + 0]
mov edx,dword [ecx + SUMMARYCACHE.sizeL2U + 4]
or bl,00001000b
@@:
mov ebp,dword [ecx + SUMMARYCACHE.sizeL3U + 0]
or ebp,dword [ecx + SUMMARYCACHE.sizeL3U + 4]
jz @f
or bl,00010000b            ; L3 unified cache
mov ebp,eax
or ebp,edx
jnz @f
mov eax,dword [ecx + SUMMARYCACHE.sizeL3U + 0]
mov edx,dword [ecx + SUMMARYCACHE.sizeL3U + 4]
or bl,00100000b
@@:
mov ebp,dword [ecx + SUMMARYCACHE.sizeL4U + 0]
or ebp,dword [ecx + SUMMARYCACHE.sizeL4U + 4]
jz @f
or bl,00010000b            ; L4 unified cache
mov ebp,eax
or ebp,edx
jnz @f
mov eax,dword [ecx + SUMMARYCACHE.sizeL4U + 0]
mov edx,dword [ecx + SUMMARYCACHE.sizeL4U + 4]
or bl,10000000b
@@:
mov ebp,eax
or ebp,edx
jnz @f
or bh,00001000b
mov eax,32768
@@:
mov dword [edi + BINDLIST.setMemObject],ebx
mov dword [edi + BINDLIST.setBlkCustom + 0],eax
mov dword [edi + BINDLIST.setBlkCustom + 4],edx
;---------- Benchmark options: SMP/SMT ----------------------------------------;
lea ecx,[SYS_PARMS.summaryTopology]
xor ebx,ebx
mov eax,[ecx + SUMMARYTOPOLOGY.threads]
cmp eax,2
jb @f
inc ebx           ; Parallel threads (SMP)
@@:
shr eax,1
jz @f
cmp eax,[ecx + SUMMARYTOPOLOGY.cores]
jne @f
or bl,00000100b   ; Hyper threading (SMT)
@@:
lea ecx,[OS_DATA]
cmp [ecx + OSDATA.activeProcessorGroupCount],2
jb @f
or bl,00010000b   ; Processor groups (PG)
@@:
mov word [edi + BINDLIST.setMemSmp],bx
;---------- Benchmark option: NUMA --------------------------------------------;
mov bx,00000010b
cmp [ecx + OSDATA.numaNodeCount],2
jb @f
mov bl,01110101b   ; Unaware, Single domain, optimal, non-optimal
@@:
mov word [edi + BINDLIST.setMemNuma],bx
;---------- Benchmark option: Large Pages -------------------------------------;
xor ebx,ebx
cmp [ecx + OSDATA.largePageEnable],ebx
je @f
inc ebx
@@:
mov word [edi + BINDLIST.setMemLpages],bx
;---------- Benchmark options, set unconditionally when defaults set ----------;
mov word [edi + BINDLIST.setMemOption],00000001b  ; Nontemporal = on, Force x32 = off
mov word [edi + BINDLIST.setMemAccess],00000010b  ; Access step
mov word [edi + BINDLIST.setMemPref],000000010b   ; Prefetch modes
mov word [edi + BINDLIST.setMemMeas],11010101b    ; Measure precision modes
mov word [edi + BINDLIST.setMemPix],00010111b     ; Pixels approximation modes
;---------- Text blocks for viewers -------------------------------------------;
; This 11 pointers MUST BE SEQUENTAL, because accessed in the cycle:
; 1.  Operating system information
; 2.  Native operating system information
; 3.  Topology information by operating system, text block 1, list
; 4.  Topology information by operating system, text block 2, summary
; 5.  Extended topology information by OS, text block 1, list
; 6.  Extended topology information by OS, text block 2, summary
; 7.  NUMA domains list by OS
; 8.  Processor groups list by OS
; 9.  ACPI tables, text block 1 = tables list
; 10. ACPI tables, text block 2 = tables detection by data base
; 11. Affinized CPUID dump
mov ecx,TEXT_SCREEN_COUNT
lea esi,[DYNA_PTR.textOs.objectStart]
add edi,BINDLIST.viewOs
@@:
movsd
xor eax,eax
stosd
lodsd    ; This instead ADD ESI,4
loop @b
;---------- Exit points -------------------------------------------------------;
clc      ; CF = 0, means no errors
.done:
pop ebp edi esi ebx
ret
.error:
stc      ; CF = 1, means error, AL contain error code
jmp .done

;---------- Helper for write cache summary by platform topology WinAPI --------;
;           For code, data and unified caches.                                 ;
;                                                                              ; 
; INPUT:   ESI = Source pointer for cache size and count variables             ;
;          EDI = Destination pointer for write text string                     ;
;          CL  = Bitmap for set GUI objects (cache items) active               ;      
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperSysinfoTopologyCache:
mov eax,[esi + 0]
mov edx,[esi + 4]
mov ebx,eax
or ebx,edx
jz .nodata
mov ebx,[esi + 8]
test ebx,ebx
jz .nodata
or [BIND_LIST.bindCache.cacheBitmap],cl
push ebx
mov bl,0FFh
call SizePrint64
mov al,' '
stosb
mov ax,STR_TP_X
call PoolStringWrite
pop eax
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
mov eax,[esi + 0]
shr eax,10
jz .nodata
cmp dword [esi + 8],0
jz .nodata
or [BIND_LIST.bindCache.cacheBitmap],cl
push eax
mov ax,STR_TRACE
call PoolStringWrite
mov al,' '
stosb
pop eax
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
