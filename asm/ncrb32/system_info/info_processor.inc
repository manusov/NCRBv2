;---------- Get processor information -----------------------------------------;
; Use this call when start.                                                    ;
; Repeat this call before vector brief, memory run, memory draw.               ;
; Move local helpers to this file or eliminate helpers, optimize structure.    ;
;                                                                              ;
; INPUT:   None                                                                ;
;                                                                              ;
; OUTPUT:  CF = Error flag, 0(NC) = OK, 1(C) = Error                           ;
;          AL = Error code, valid if CF = 1                                    ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoProcessor:
;---------- Check CPUID instruction support, get 12-chars vendor string -------;
mov edi,CPU_DATA.vendorString
call HelperCheckCpuid
jc .errorCpuid        ; Go error if CPUID instruction not supported or locked
cmp eax,1
jb .errorFunction     ; Go error if CPUID function 1 not supported or locked
;---------- Get 48-chars CPU model string -------------------------------------;
call HelperGetCpuName
;---------- Get CPU signature: type, family, model, stepping ------------------;
mov eax,1
cpuid
stosd
test dl,00000001b
jz .errorX87          ; Go error if X87 FPU not supported or locked 
test dl,00010000b
jz .errorTsc          ; Go error if Time Stamp Counter not supported or locked
;---------- Set HT check flag by results of CPUID function 1 ------------------;
xor eax,eax
bt edx,28
jnc @f
shr ebx,16
cmp bl,2
sete al
@@:
stosd
;------- Build CPU common features bitmap, for system information screen ------; 
xor eax,eax
mov esi,[APP_DATA.lockedDataCpuCommon]
test esi,esi
jz @f
call HelperBuildBitmap
@@:
stosd
mov eax,80000000h
stosd
;------- Build CPU AVX512 features bitmap, for system information screen ------;
xor eax,eax
mov esi,[APP_DATA.lockedDataCpuAvx512]
test esi,esi
jz @f
call HelperBuildBitmap
@@:
stosd
xor eax,eax
stosd
;------- Build OS context features bitmap, for system information screen ------;
xor eax,eax
mov esi,[APP_DATA.lockedDataOsContext]
test esi,esi
jz @f
call HelperBuildBitmap
@@:
xor edx,edx
test eax,eax
jz @f
bts edx,31
@@:
stosd
xchg eax,edx
stosd
;---------- Measure TSC frequency ---------------------------------------------;
call HelperMeasureTsc
jc .errorTscFreq      ; Go error if TSC frequency measurement failed
;---------- Cache detection by CPUID ------------------------------------------;
mov edi,CPU_CACHE_DATA
call HelperCpuidCache
clc
ret
;---------- Errors handling ---------------------------------------------------;
.errorCpuid:
mov al,MSG_ERROR_CPUID
stc
ret
.errorFunction:
mov al,MSG_ERROR_CPUID_F1
stc
ret
.errorX87:
mov al,MSG_ERROR_X87
stc
ret
.errorTsc:
mov al,MSG_ERROR_TSC
stc
ret
.errorTscFreq:
mov al,MSG_ERROR_TSC_FREQ
stc
ret

;---------- Detect CPUID support and execute CPUID function #0 ----------------;
; Note CPUID can be supported by CPU but locked by Virtual Monitor.            ;
; Note check bit EFLAGS.21 toggleable, it is CPUID support indicator.          ;
; Note probably wrong result if debug trace this subroutine code.              ;
;                                                                              ;
; INPUT:   EDI = Destination pointer for save CPU Vendor String                ;
;                                                                              ;
; OUTPUT:  CF flag = Status: 0(NC)=Support OK, 1(C)=Not supported              ;
;          Output EAX, EDI, Destination memory valid only if CF=0(NC)          ;
;          EAX = Largest standard CPUID function supported                     ;
;          EDI = Input EDI + 13,                                               ; 
;                string size fixed = 12 chars + 0 = terminator byte            ;
;          Destination memory at [input EDI] =                                 ;
;           bytes [00-11] = CPU vendor string                                  ;
;           byte  [12-12] = 00h, string terminator                             ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperCheckCpuid:
;---------- Check for ID bit writeable for "1" --------------------------------;
mov ebx,21
pushf                     ; In the 32-bit mode, push EFLAGS
pop eax
bts eax,ebx               ; Set EAX.21=1
push eax
popf                      ; Load EFLAGS with EFLAGS.21=1
pushf                     ; Store EFLAGS
pop eax                   ; Load EFLAGS to EAX
btr eax,ebx               ; Check EAX.21=1, Set EAX.21=0
jnc .absent               ; Go error branch if cannot set EFLAGS.21=1
;---------- Check for ID bit writeable for "0" --------------------------------;
push eax
popf                      ; Load EFLAGS with EFLAGS.21=0
pushf                     ; Store EFLAGS
pop eax                   ; Load EFLAGS to EAX
btr eax,ebx               ; Check EAX.21=0
jc .absent                ; Go if cannot set EFLAGS.21=0
;---------- Execute CPUID function 0, store results ---------------------------;
xor eax,eax               ; EAX = Function number for CPUID instruction
cpuid                     ; Execute CPUID function 0
xchg eax,ebx              ; XCHG instead MOV, short code
stosd                     ; Store Vendor String [00-03]
xchg eax,edx	  
stosd                     ; Store Vendor String [04-07]
xchg eax,ecx
stosd                     ; Store Vendor String [08-11]
mov al,0
stosb                     ; Zero terminator byte
xchg eax,ebx              ; Restore EAX = Largest standard function supported
;---------- Exit points -------------------------------------------------------;
ret                       ; Return, at this point CF=0(NC) after XOR EAX,EAX
.absent:
stc                       ; CF=1(C) means error
ret 

;---------- Get CPU name string, execute CPUID functions 80000002h-80000004h --;
; Call this subroutine only if CPUID instruction supported.                    ;
;                                                                              ;
; INPUT:   EDI = Destination pointer for save CPU Vendor String                ;
;                                                                              ;
; OUTPUT:  EDI = Input EDI + 48 + 1 , string size fixed = 48 bytes             ;
;          Destination memory at [input EDI] =                                 ; 
;           bytes [00-47] = CPU Vendor String                                  ;
;           byte  [48-48] = 00h, terminator for copy by StringWrite            ;
;          String formatted by subroutine, left spaces deleted.                ;
;          If feature not supported, string contain "None",                    ;
;          but errors not reported, NCRB can work without this feature         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperGetCpuName:
cld                          ; Clear direction, because STOSB/STOSD used
push ebx esi
;---------- Blank and prepare for not support mode ----------------------------;
mov ecx,48                   ; ECX = String length
mov al,' '                   ; AL = Space
rep stosb                    ; Blank string
mov byte [edi - 48],'?'      ; Default string, if feature not supported
mov byte [edi],0             ; Zero terminator byte 
;---------- Check functions supported -----------------------------------------;
mov esi,80000000h            ; ESI = First extended function = 80000000h
mov eax,esi                  ; EAX = Function for verify support = 80000000h
cpuid                        ; Execute CPUID function 80000000h
lea ebx,[esi + 4]            ; EBX = Maximum function 80000004h
cmp eax,ebx
jb .exit                     ; Go exit if required functions not supported
sub edi,48                   ; Must be +48 if go exit when not supported
;---------- Get and store CPU name string -------------------------------------;
push edi
.storeCpuName:
lea eax,[esi + 2]          ; EAX = Function number, start from 80000002h
cpuid
stosd
xchg eax,ebx
stosd
xchg eax,ecx
stosd
xchg eax,edx
stosd
inc esi
cmp si,4-2                 ; This give function number EAX=80000004h
jbe .storeCpuName
pop edi
;---------- Formatting CPU name string, delete left spaces and byte 00h -------;
mov esi,edi
mov ecx,48                 ; ECX = String size limit
mov ebx,ecx                ; EBX = Used space count = 48 - Unused space count
.scanCpuName:              ; This cycle for delete left spaces
lodsb                      
dec ebx                   ; Count used space
cmp al,0
je .endCpuName            ; Go blank if end of string without name, here ECX=48
cmp al,' '
loope .scanCpuName        ; Cycle for skip left spaces
mov cl,48
je .endCpuName            ; Go blank if only spaces in the string, here ECX=48
inc ebx
dec esi
;---------- Copy used portion of name -----------------------------------------;
.copyCpuName:             ; This cycle for copy name before terminator = 00h
lodsb
cmp al,0
je .endCpuName
stosb
dec ecx
dec ebx
jnz .copyCpuName
;---------- Blank unused portion of name --------------------------------------;
.endCpuName:              ; Start blank tail with space = 20h
mov al,' '
rep stosb
;---------- Exit point --------------------------------------------------------;
.exit:
inc edi                   ; Skip terminator byte
pop esi ebx
ret

;---------- Measure CPU TSC (Time Stamp Counter) clock frequency --------------;
; Store results F = Frequency=[Hz].                                            ;
; Call this subroutine only if CPUID and RDTSC both supported.                 ;
;                                                                              ;
; INPUT:   EDI = Destination pointer for save TSC frequency                    ;
;                                                                              ;
; OUTPUT:  CF flag = Status: 0(NC)=Measured OK, 1(C)=Measurement error	       ;
;          Output EDI and destination memory valid only if CF=0(NC)            ;
;          EDI = Input EDI + 8 , buffer size fixed = 8 bytes                   ;
;          Destination memory at [input EDI] = Results                         ;
;           Qword [00-07] = TSC frequency, Hz = delta TSC per 1 second         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperMeasureTsc:
cld                        ; Clear direction, because STOSQ used
push ebx esi ebp eax eax   ; Last 2 pushes = reserve space for local variables
mov ebp,esp                ; EBP = pointer to local variables
;---------- Start measure frequency, get current change -----------------------;
push ebp                          ; Parm#1 = pointer to output 64-bit variable
call [GetSystemTimeAsFileTime]    ; Get current count
mov esi,[ebp]
@@:                               ; Wait for start 1 second interval
push ebp
call [GetSystemTimeAsFileTime]    ; Get next count for wait 100 ns
cmp esi,[ebp]
je @b
mov esi,[ebp + 0]                 ; Set time interval = 1 second
mov ebx,[ebp + 4]
add esi,10000000                  ; 10^7 * 100ns = 1 second
adc ebx,0
rdtsc                             ; Get start TSC
push edx eax
@@:                               ; Wait for end 1 second interval
push ebp
call [GetSystemTimeAsFileTime]    ; Get count for wait 1 second
cmp [ebp + 4],ebx
jb @b
ja @f
cmp [ebp + 0],esi
jb @b
@@:
rdtsc                             ; Get end TSC, calculate delta-TSC
pop esi ebx
sub eax,esi
sbb edx,ebx     ; EDX:EAX = Delta TSC per 1 second = frequency, Hz
jb .error       ; Go error if frequency < 0               
mov ecx,eax
or ecx,edx
jz .error       ; Go error if frequency = 0
;-------- Store Frequency, as 64-bit integer value, Hz, delta-TSC per second --;
stosd                   ; Store EAX = Frequency, low dword
xchg eax,edx
stosd                   ; Store EAX = Frequency, high dword
;---------- Restore ESP, pop extra registers, exit ----------------------------;
clc             ; CF=0 (NC) means CPU clock measured OK
.exit:
pop eax eax ebp esi ebx
ret
.error:
stc             ; CF=1 (CY) means CPU clock measured ERROR
jmp .exit

;---------- Interpreting list of entries ENTRY_CPUID, ENTRY_XCR0 --------------;
;                                                                              ;
; INPUT:   ESI = Pointer to builder script                                     ;
;                                                                              ;
; OUTPUT:  EAX = Bitmap, result of builder script                              ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperBuildBitmap:
push esi edi ebp ebx
cld
xor ebp,ebp          ; EBP = Current selected bit number
xor ebx,ebx          ; EBX = Bitmap accumulation
mov edi,0FFFFFFFFh   ; EDI = Additional prefix-controlled conditions bitmap
;---------- Cycle for build bitmap --------------------------------------------;
.build:
lodsb
mov dl,al            ; DL[7-6]  = register id
and eax,00111111b    ; EAX[4-0] = script command id
dec eax
jz .entryCpuid
dec eax
jz .entryCpuidS
dec eax
jz .prefixCpuid
dec eax
jz .prefixCpuidS
dec eax
jz .entryXcr0
dec eax
jz .prefixXcr0
;---------- Return with EAX = bitmap, bits [31-00] used -----------------------;
.done:
xchg eax,ebx
pop ebx ebp edi esi
ret
;---------- Detected script command: CPUID without subfunction ----------------;
.entryCpuid:
lodsd
xchg ecx,eax
lodsb
mov dh,al            ; DH  = Bit number, read from script
xchg ecx,eax         ; EAX = CPUID function, read from script
xor ecx,ecx          ; ECX = CPUID subfunction = 0
.helperCpuid:
call HelperEntryCpuid 
.next:
inc ebp              ; EBP = Bit counter, go to next bit
and ebx,edi          ; EBX = Accumulate bitmap, mask by additional bitmap = EDI
mov edi,0FFFFFFFFh   ; EDI = Additional bitmap, restore
jmp .build
;---------- Detected script command: CPUID with subfunction -------------------;
.entryCpuidS:
lodsd                ; EAX = CPUID function, read from script
xchg ecx,eax
lodsd
xchg ecx,eax         ; ECX = CPUID subfunction, read from script
mov dh,[esi]         ; DH  = Bit number, read from script
inc esi
jmp .helperCpuid
;---------- Detected script command: CPUID w/o sf as additional condition -----;
.prefixCpuid:
lodsd
xchg ecx,eax
lodsb
mov dh,al            ; DH  = Bit number, read from script 
xchg ecx,eax         ; EAX = CPUID function, read from script 
xor ecx,ecx          ; ECX = CPUID subfunction = 0
.helperPrefixCpuid:
push ebx
xor ebx,ebx
call HelperEntryCpuid 
btr edi,ebp
or edi,ebx
pop ebx
jmp .build
;---------- Detected script command: CPUID + sf as additional condition -------;
.prefixCpuidS:
lodsd                ; EAX = CPUID function, read from script
xchg ecx,eax    
lodsd
xchg ecx,eax         ; ECX = CPUID subfunction, read from script 
mov dh,[esi]         ; DH  = Bit number, read from script
inc esi
jmp .helperPrefixCpuid
;---------- Detected script command: XCR0 -------------------------------------;
.entryXcr0:
xor eax,eax
lodsb           ; EAX = Tested bit number at XCR0, read from script
call HelperEntryXcr0
jmp .next
;---------- Detected script command: XCR0 as additional condition -------------;
.prefixXcr0:    ; This branch yet not used
jmp .build

;---------- Interpreting one ENTRY_CPUID object at build bitmap list ----------;
; Check CPUID instruction support before this subroutine call.                 ;
;                                                                              ;
; INPUT:   EAX = CPUID function code                                           ;
;          ECX = CPUID sub-function code                                       ;    
;          EDX = Bit number at DH, register number at DL bits [7-6]            ;
;                DL bits [5-0] undefined, can be non zero                      ;
;          EBX = Bitmap for accumulate features flags by build scenario        ;
;          EBP = Selector for bit of bitmap, used for select one of EBX bits   ;
;                                                                              ;
; OUTPUT:  EBX = Updated 32-bit bitmap, caller can twice execute build cycle   ;
;                for low and high 32-bit dwords of 64-bit bitmap,              ;
;                note about 64-bit code portability to 32-bit code.            ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperEntryCpuid:
push esi edi ebp ebx
mov esi,eax     ; ESI = function
mov edi,ecx     ; EDI = subfunction 
mov ebp,edx     ; EBP = bit number : register id
mov eax,esi
and eax,80000000h
cpuid
cmp eax,esi
jb .bitNo
;---------- Subfunction number validation -------------------------------------;
; TODO. Subfunction number validity check yet for function 00000007h only,
;       required unified CPUID driver.
cmp esi,7
jne .simpleCpuid
test edi,edi
jz .simpleCpuid
mov eax,esi
xor ecx,ecx
cpuid
cmp eax,edi
mov eax,0
mov ebx,eax
mov ecx,eax
mov edx,eax
jb .skipCpuid
.simpleCpuid:
mov eax,esi
mov ecx,edi
cpuid
.skipCpuid:
;---------- End of subfunction number validation ------------------------------;
mov edi,ebp
shr edi,8
and edi,00FFh
and ebp,00FFh
shr ebp,6
jz .regEax
dec ebp
jz .regEbx
dec ebp
jz .regEcx
.regEdx:
bt edx,edi
jmp .bitDone
.regEcx:
bt ecx,edi
jmp .bitDone
.regEbx:
bt ebx,edi
jmp .bitDone
.regEax:
bt eax,edi
jmp .bitDone
.bitNo:
clc
.bitDone:
pop ebx ebp edi esi
jnc .zero
bts ebx,ebp
.zero:
ret

;---------- Interpreting one ENTRY_XCR0 object at build bitmap list -----------;
; Check CPUID instruction support before this subroutine call,                 ;
; XGETBV instruction support verified in this subroutine.                      ;
;                                                                              ;
; INPUT:   EAX = Tested bit number at XCR0                                     ;
;          EBX = Bitmap for accumulate features flags by build scenario        ;
;          EBP = Selector for bit of bitmap, used for select one of EBX bits   ;
;                                                                              ;
; OUTPUT:  EBX = Updated 32-bit bitmap, caller can twice execute build cycle   ;
;                for low and high 32-bit dwords of 64-bit bitmap,              ;
;                note about 64-bit code portability to 32-bit code.            ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperEntryXcr0:
push ebp ebx
xchg ebp,eax
xor eax,eax
cpuid
cmp eax,1
jb .bitNo
mov eax,1
cpuid
bt ecx,27
jnc .bitNo
mov ebx,ebp
and ebp,00011111b
xor ecx,ecx
xgetbv
test ebx,00100000b
jnz .high32
bt eax,ebp
jmp .bitDone
.high32:
bt edx,ebp
jmp .bitDone
.bitNo:
clc
.bitDone:
pop ebx ebp
jnc .zero
bts ebx,ebp
.zero:
ret

;---------- Detection CPU Caches by CPUID instruction -------------------------;
;                                                                              ;
; INPUT:   EDI = Destination pointer for save cache info                       ;
;                                                                              ;
; OUTPUT:  EDI = Input EDI + 60 , buffer size fixed = 60 bytes                 ;
;          Destination memory at [input EDI] = Results, 0=Not detected         ;
;          Qword [00-07] = Trace instruction cache size, micro ops             ;
;          Word  [08-09] = Maximum threads at this level                       ;
;          Word  [10-11] = Maximum APIC IDs per package                        ;
;          Qword [12-19] = L1 instruction cache size, bytes                    ;
;          Word  [20-21] = Maximum threads at this level                       ;
;          Word  [22-23] = Maximum APIC IDs per package                        ;
;          Qword [24-31] = L1 data cache size, bytes                           ;
;          Word  [32-33] = Maximum threads at this level                       ;
;          Word  [34-35] = Maximum APIC IDs per package                        ;
;          Qword [36-43] = L2 unified cache size, bytes                        ;
;          Word  [44-45] = Maximum threads at this level                       ;
;          Word  [46-47] = Maximum APIC IDs per package                        ;
;          Qword [48-55] = L3 unified cache size, bytes                        ;
;          Word  [56-57] = Maximum threads at this level                       ;
;          Word  [58-59] = Maximum APIC IDs per package                        ; 
;                                                                              ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.        ; 
;------------------------------------------------------------------------------;
HelperCpuidCache:
cld                            ; Clear direction, because STOSB used
push ebx esi ebp
;---------- Blank results -----------------------------------------------------;
mov ecx,60
xor eax,eax                    ; This entire EAX used for CPUID later
rep stosb
;---------- Check support CPUID#4 = Deterministic cache info, execute CPUID#0 -;
; Here EAX=0
cpuid
cmp eax,4
jb .noMoreDeterministicCaches
;---------- Execute CPUID#4, subfunctions sequence ----------------------------; 
xor esi,esi                    ; ESI = Subfunctions count
.cycleCache:
mov eax,4                      ; EAX = Function
mov ecx,esi                    ; ECX = Subfunction
cpuid
;---------- Select handler = F( Type,Level ) ----------------------------------;
test al,01Fh                   ; AL.[7-5]=Level, AL.[4-0]=Type
jz .noMoreDeterministicCaches  ; Go exit if no more caches
mov dl,12-60                   ; DL = Offset for addressing output data
cmp al,022h                    ; AL = Selector
je .storeCache                 ; Go analyse if L1 instruction cache
mov dl,24-60
cmp al,021h
je .storeCache                 ; Go analyse if L1 data cache
mov dl,36-60
cmp al,043h
je .storeCache                 ; Go analyse if L2 Unified cache
;---------- This patch for Oracle VMBox ---------------------------------------;
cmp al,41h                     ; Oracle VMBox can map physical L3 Unified cache as virtual L2 Data Cache
je .storeCache                 ; Go analyse if L2 Data cache (really L3 Unified cache)
;---------- End of patch for Oracle VMBox -------------------------------------;
mov dl,48-60
cmp al,063h                
je .storeCache                 ; Go analyse if L3 Unified cache
.nextCache:
inc esi
cmp esi,256                    ; This for hang prevent if failure	
jb .cycleCache
.noMoreDeterministicCaches:
;---------- Special support for Intel Trace Cache -----------------------------;
xor eax,eax
cpuid
cmp eax,2
jb .doneIntelCaches
mov eax,2       ; EAX = Function = 2 = Get Cache Descriptors
xor ecx,ecx     ; ECX = Subfunction = 0
cpuid
cmp al,1        ; Leaf count must be 1, otherwise skip
jne .doneIntelCaches
test eax,eax    ; If bit EAX.31 = 1, means EAX contain invalid data   
jns @f          ; Go skip clear, if EAX data valid
xor eax,eax
@@:
test ebx,ebx    ; If bit EBX.31 = 1, means EBX contain invalid data
jns @f          ; Go skip clear, if EBX data valid
xor ebx,ebx
@@:
test ecx,ecx    ; If bit ECX.31 = 1, means ECX contain invalid data
jns @f          ; Go skip clear, if ECX data valid
xor ecx,ecx
@@:
test edx,edx    ; If bit EDX.31 = 1, means EDX contain invalid data
jns @f          ; Go skip clear, if EDX data valid
xor edx,edx
@@:
mov ebp,15      ; 4 registers * 4 bytes - 1 unused (AL)
;---------- Cycle for interpreting cache descriptors --------------------------;
.cacheDescriptors:
shrd eax,ebx,8
shrd ebx,ecx,8
shrd ecx,edx,8
shr edx,8
mov esi,[APP_DATA.lockedDataIntelCache]
push ecx
.cacheValues:
movzx ecx,byte [esi]  ; ECX = Cache type tag by NCRB List, or 0 = Terminator
jecxz .listDone 
cmp al,[esi + 1]      ; AL = Descriptor byte, compare with ID by INTEL
jne .descMismatch
push eax ecx
movzx eax,word [esi + 2]       ; EAX = This Cache size, Trace=KuOps, Other=KB
shl eax,10                     ; EAX = Trace Cache size, uOps or Bytes
dec ecx
imul ecx,ecx,12 
.cacheStore:
mov dword [edi - 60 + ecx + 0],eax   ; Store Cache size, uOps or Bytes
mov dword [edi - 60 + ecx + 4],0 
pop ecx eax
.descMismatch:
add esi,4
jmp .cacheValues        ; Cycle for possible Cache types IDs  
.listDone:
pop ecx
dec ebp
jnz .cacheDescriptors   ; Cycle for Cache Descriptors array 
;---------- Detect caches by AMD method ---------------------------------------;
.doneIntelCaches:
mov esi,80000005h
mov eax,80000000h
cpuid
cmp eax,esi
jb .doneAmdCaches
mov eax,esi
cpuid                       ; Execute CPUID function 80000005h: AMD L1
and edx,0FF000000h
shr edx,24 - 10
cmp dword [edi - 60 + 12],0
jne @f
mov [edi - 60 + 12],edx     ; L1 instruction cache, CPUID 80000005h, EDX[31-24]
@@:                         ; units = KB
and ecx,0FF000000h
shr ecx,24 - 10
cmp dword [edi - 60 + 24],0
jne @f
mov [edi - 60 + 24],ecx     ; L1 data cache, CPUID 80000005h, ECX[31-24]
@@:                         ; units = KB
inc esi
mov eax,80000000h
cpuid
cmp eax,esi
jb .doneAmdCaches
mov eax,esi
cpuid                       ; Execute CPUID function 80000006h: AMD L2, L3
and ecx,0FFFF0000h
shr ecx,16 - 10
cmp dword [edi - 60 + 36],0
jne @f
mov [edi - 60 + 36],ecx     ; L2 unified cache, CPUID 80000006h, ECX[31-16]
@@:                         ; units = KB
and edx,0FFFC0000h
shl edx,1
cmp dword [edi - 60 + 48],0
jne @f
mov [edi - 60 + 48],edx     ; L3 unified cache, CPUID 80000006h, EDX[31-18]
@@:                         ; units = 512 KB
;---------- Exit --------------------------------------------------------------;
.doneAmdCaches:
pop ebp esi ebx
ret
;---------- Start handling detected cache type/level --------------------------;
.storeCache:
movsx edx,dl                   ; Save address offset
push edx
;---------- Calculate cache size ----------------------------------------------;
; Cache size (bytes) = (ways+1)*(partitions+1)*(linesize+1)*(sets+1) =
; = (EBX[31-22]+1) * (EBX[21-12]+1) * (EBX[11-00]+1) * (ECX+1)
mov edx,ebx
shr edx,22                     ; Shift EBX[31-22] = Ways
and edx,03FFh 
inc edx
inc ecx
imul ecx,edx                   ; Multiply to ECX = Sets
mov edx,ebx                    
shr edx,12                     ; Shift EBX[21-12] = Partitions
and edx,03FFh		     
inc edx
imul ecx,edx                   ; Mask EBX[11-00] = Line Size
and ebx,0FFFh
inc ebx
imul ecx,ebx                   ; ECX = Cache size, bytes
;---------- Store cache size --------------------------------------------------;
pop edx
mov dword [edi + edx + 0],ecx
mov dword [edi + edx + 4],0
;---------- Extract threads share cache ---------------------------------------;
mov ebx,eax
shr eax,14                     ; Logical CPUs: 12 bit [26-14] shift to [11-00]
shr ebx,26                     ; CPU Cores: 6 bit [31-26] shift to [05-00]
and ax,0FFFh                   ; Extract Logical CPUs = Threads
and bx,003Fh                   ; Extrach Cores		  
inc eax                        ; This because "-1" encoding
inc ebx                        ; This because "-1" encoding
;---------- Store threads share cache -----------------------------------------;
mov [edi + edx + 08],ax
mov [edi + edx + 10],bx
;---------- Return to cycle ---------------------------------------------------;
jmp .nextCache



