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
lea rdi,[CPU_DATA.vendorString]  ; RDI = Destination pointer
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
;------- Build CPU common features bitmap, for system information screen ------; 
xor eax,eax
mov rsi,[APP_DATA.lockedDataCpuCommon]
test rsi,rsi
jz @f
call HelperBuildBitmap
@@:
bts rax,63
stosq
;------- Build CPU AVX512 features bitmap, for system information screen ------;
xor eax,eax
mov rsi,[APP_DATA.lockedDataCpuAvx512]
test rsi,rsi
jz @f
call HelperBuildBitmap
@@:
stosq
;------- Build OS context features bitmap, for system information screen ------;
xor eax,eax
mov rsi,[APP_DATA.lockedDataOsContext]
test rsi,rsi
jz @f
call HelperBuildBitmap
@@:
test rax,rax
jz @f
bts rax,63
@@:
stosq
;---------- Measure TSC frequency ---------------------------------------------;
call HelperMeasureTsc
jc .errorTscFreq      ; Go error if TSC frequency measurement failed
;---------- Cache detection by CPUID ------------------------------------------;
lea rdi,[CPU_CACHE_DATA]
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

;---------- Detect CPUID support and execute CPUID function #0. ---------------;
; Note CPUID can be supported by CPU but locked by Virtual Monitor.            ;
; Note check bit EFLAGS.21 toggleable, it is CPUID support indicator.          ;
; Note probably wrong result if debug trace this subroutine code.              ;
;                                                                              ;
; INPUT:   RDI = Destination pointer for save CPU Vendor String                ;
;                                                                              ;
; OUTPUT:  CF flag = Status: 0(NC)=Support OK, 1(C)=Not supported              ;
;          Output EAX, RDI, Destination memory valid only if CF=0(NC)          ;
;          EAX = Largest standard CPUID function supported                     ;
;          RDI = Input RDI + 13,                                               ; 
;                string size fixed = 12 chars + 0 = terminator byte            ;
;          Destination memory at [input RDI] =                                 ;
;           bytes [00-11] = CPU vendor string                                  ;
;           byte  [12-12] = 00h, string terminator                             ;
;------------------------------------------------------------------------------;
HelperCheckCpuid:
;---------- Check for ID bit writeable for "1" --------------------------------;
mov ebx,21
pushf                     ; In the 64-bit mode, push RFLAGS
pop rax
bts eax,ebx               ; Set EAX.21=1
push rax
popf                      ; Load RFLAGS with RFLAGS.21=1
pushf                     ; Store RFLAGS
pop rax                   ; Load RFLAGS to RAX
btr eax,ebx               ; Check EAX.21=1, Set EAX.21=0
jnc .absent               ; Go error branch if cannot set EFLAGS.21=1
;---------- Check for ID bit writeable for "0" --------------------------------;
push rax
popf                      ; Load RFLAGS with RFLAGS.21=0
pushf                     ; Store RFLAGS
pop rax                   ; Load RFLAGS to RAX
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
; INPUT:   RDI = Destination pointer for save CPU Vendor String                ;
;                                                                              ;
; OUTPUT:  RDI = Input RDI + 48 + 1 , string size fixed = 48 bytes             ;
;          Destination memory at [input RDI] =                                 ; 
;           bytes [00-47] = CPU Vendor String                                  ;
;           byte  [48-48] = 00h, terminator for copy by StringWrite            ;
;          String formatted by subroutine, left spaces deleted.                ;
;          If feature not supported, string contain "None",                    ;
;          but errors not reported, NCRB can work without this feature         ;
;------------------------------------------------------------------------------;
HelperGetCpuName:
cld                          ; Clear direction, because STOSB/STOSD used
push rbx rsi
;---------- Blank and prepare for not support mode ----------------------------;
mov ecx,48                   ; RCX = String length, also clear bits RCX[63-32]
mov al,' '                   ; AL = Space
rep stosb                    ; Blank string
mov dword [rdi - 48],'None'  ; Default string, if feature not supported
mov byte [rdi],0             ; Zero terminator byte 
;---------- Check functions supported -----------------------------------------;
mov esi,80000000h            ; ESI = First extended function = 80000000h
mov eax,esi                  ; EAX = Function for verify support = 80000000h
cpuid                        ; Execute CPUID function 80000000h
lea ebx,[esi + 4]            ; EBX = Maximum function 80000004h
cmp eax,ebx
jb .exit                     ; Go exit if required functions not supported
sub rdi,48                   ; Must be +48 if go exit when not supported
;---------- Get and store CPU name string -------------------------------------;
push rdi
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
pop rdi
;---------- Formatting CPU name string, delete left spaces and byte 00h -------;
mov rsi,rdi
mov ecx,48                 ; RCX = String size limit
mov ebx,ecx                ; EBX = Used space count = 48 - Unused space count
.scanCpuName:              ; This cycle for delete left spaces
lodsb                      
dec ebx                   ; Count used space
cmp al,0
je .endCpuName            ; Go blank if end of string without name, here RCX=48
cmp al,' '
loope .scanCpuName        ; Cycle for skip left spaces
mov cl,48
je .endCpuName            ; Go blank if only spaces in the string, here RCX=48
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
inc rdi                   ; Skip terminator byte
pop rsi rbx
ret

;---------- Measure CPU TSC (Time Stamp Counter) clock frequency --------------;
; Store results F = Frequency=[Hz].                                            ;
; Call this subroutine only if CPUID and RDTSC both supported.                 ;
;                                                                              ;
; INPUT:   RDI = Destination pointer for save TSC frequency                    ;
;                                                                              ;
; OUTPUT:  CF flag = Status: 0(NC)=Measured OK, 1(C)=Measurement error	       ;
;          Output RDI and destination memory valid only if CF=0(NC)            ;
;          RDI = Input RDI + 8 , buffer size fixed = 8 bytes                   ;
;          Destination memory at [input RDI] = Results                         ;
;           Qword [00-07] = TSC frequency, Hz = delta TSC per 1 second         ;
;------------------------------------------------------------------------------;
HelperMeasureTsc:
cld                        ; Clear direction, because STOSQ used
push rbx rbp rbp           ; Last push for reserve local variable space
mov rbp,rsp                ; RBP used for restore RSP and addressing variables
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32                 ; Make parameters shadow
;---------- Start measure frequency -------------------------------------------;
mov rcx,rbp
call [GetSystemTimeAsFileTime]    ; Get current count
mov rsi,[rbp]
@@:
mov rcx,rbp
call [GetSystemTimeAsFileTime]    ; Get next count for wait 100 ns
cmp rsi,[rbp]
je @b
mov rsi,[rbp]
add rsi,10000000                  ; 10^7 * 100ns = 1 second
rdtsc
shl rdx,32
lea rbx,[rax + rdx]               ; RBX = 64-bit TSC at operation start
@@:
mov rcx,rbp
call [GetSystemTimeAsFileTime]    ; Get count for wait 1 second
cmp rsi,[rbp]
ja @b
rdtsc
shl rdx,32
or rax,rdx                        ; RAX = 64-bit TSC at operation end
sub rax,rbx                       ; RAX = Delta TSC
jbe .error
;---------- Store result ------------------------------------------------------; 
stosq     ; Store Frequency, as 64-bit integer value, Hz, delta-TSC per second
;---------- Restore RSP, pop extra registers, exit ----------------------------;
clc             ; CF=0 (NC) means CPU clock measured OK
.exit:
mov rsp,rbp
pop rbp rbp rbx
ret
.error:
stc             ; CF=1 (CY) means CPU clock measured ERROR
jmp .exit

;---------- Interpreting list of entries ENTRY_CPUID, ENTRY_XCR0 --------------;
;                                                                              ;
; INPUT:   RSI = Pointer to builder script                                     ;
;                                                                              ;
; OUTPUT:  RAX = Bitmap, result of builder script                              ;  
;                                                                              ;
;------------------------------------------------------------------------------;
HelperBuildBitmap:
push rsi rdi rbp rbx
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
;---------- Return with RAX = bitmap, bits [31-00] only used ------------------;
.done:
xchg rax,rbx
pop rbx rbp rdi rsi
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
mov dh,[rsi]         ; DH  = Bit number, read from script
inc rsi
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
push rbx
xor ebx,ebx
call HelperEntryCpuid 
btr edi,ebp
or edi,ebx
pop rbx
jmp .build
;---------- Detected script command: CPUID + sf as additional condition -------;
.prefixCpuidS:
lodsd                ; EAX = CPUID function, read from script
xchg ecx,eax    
lodsd
xchg ecx,eax         ; ECX = CPUID subfunction, read from script 
mov dh,[rsi]         ; DH  = Bit number, read from script
inc rsi
jmp .helperPrefixCpuid
;---------- Detected script command: XCR0 -------------------------------------;
.entryXcr0:
xor eax,eax
lodsb                ; EAX = Tested bit number at XCR0, read from script
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
;------------------------------------------------------------------------------;
HelperEntryCpuid:
push rsi rdi rbp rbx
mov esi,eax     ; ESI = function
mov edi,ecx     ; EDI = subfunction 
mov ebp,edx     ; EBP = bit number : register id
mov eax,esi
and eax,80000000h
cpuid
cmp eax,esi
jb .bitNo
xchg eax,esi
mov ecx,edi
cpuid
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
pop rbx rbp rdi rsi
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
;------------------------------------------------------------------------------;
HelperEntryXcr0:
push rbp rbx
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
pop rbx rbp
jnc .zero
bts ebx,ebp
.zero:
ret

;---------- Detection CPU Caches by CPUID instruction -------------------------;
;                                                                              ;
; INPUT:   RDI = Destination pointer for save cache info                       ;
;                                                                              ;
; OUTPUT:  RDI = Input RDI + 60 , buffer size fixed = 60 bytes                 ;
;          Destination memory at [input RDI] = Results, 0=Not detected         ;
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
; Can destroy registers, volatile by Microsoft x64 calling convention.         ;
;------------------------------------------------------------------------------;
HelperCpuidCache:
cld                            ; Clear direction, because STOSB used
push rbx rsi
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
jz .noMoreDeterministicCaches   ; Go exit if no more caches
mov dl,12-60                   ; DL = Offset for addressing output data
cmp al,022h                    ; AL = Selector
je .storeCache                 ; Go analyse if L1 instruction cache
mov dl,24-60
cmp al,021h
je .storeCache                 ; Go analyse if L1 data cache
mov dl,36-60
cmp al,043h
je .storeCache                  ; Go analyse if L2 Unified cache
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
;---------- Special support for Intel Trace Cache -----------------------------;
.noMoreDeterministicCaches:
xor eax,eax
cpuid
cmp eax,2
jb .doneCaches
mov eax,2       ; EAX = Function = 2 = Get Cache Descriptors
xor ecx,ecx     ; ECX = Subfunction = 0
cpuid
cmp al,1        ; Leaf count must be 1, otherwise skip
jne .doneCaches
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
mov r8d,15      ; 4 registers * 4 bytes - 1 unused (AL)
;---------- Cycle for interpreting cache descriptors --------------------------;
.cacheDescriptors:
shrd eax,ebx,8
shrd ebx,ecx,8
shrd ecx,edx,8
shr edx,8
mov rsi,[APP_DATA.lockedDataIntelCache]
push rcx
.cacheValues:
movzx ecx,byte [rsi]  ; ECX = Cache type tag by NCRB List, or 0 = Terminator
jrcxz .listDone 
cmp al,[rsi + 1]      ; AL = Descriptor byte, compare with ID by INTEL
jne .descMismatch
push rax rcx
movzx eax,word [rsi + 2]       ; EAX = This Cache size, Trace=KuOps, Other=KB
shl eax,10                     ; EAX = Trace Cache size, uOps or Bytes
dec ecx
imul ecx,ecx,12 
.cacheStore:
mov [rdi - 60 + rcx],rax       ; Store Cache size, uOps or Bytes
pop rcx rax
.descMismatch:
add rsi,4
jmp .cacheValues        ; Cycle for possible Cache types IDs  
.listDone:
pop rcx
dec r8d
jnz .cacheDescriptors   ; Cycle for Cache Descriptors array 
;---------- Exit --------------------------------------------------------------;
.doneCaches:
pop rsi rbx
ret
;---------- Start handling detected cache type / level ------------------------;
.storeCache:
movsx rdx,dl                   ; Save address offset
push rdx
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
pop rdx
mov [rdi + rdx],rcx
;---------- Extract threads share cache ---------------------------------------;
mov ebx,eax
shr eax,14                     ; Logical CPUs: 12 bit [26-14] shift to [11-00]
shr ebx,26                     ; CPU Cores: 6 bit [31-26] shift to [05-00]
and ax,0FFFh                   ; Extract Logical CPUs = Threads
and bx,003Fh                   ; Extrach Cores		  
inc eax                        ; This because "-1" encoding
inc ebx                        ; This because "-1" encoding
;---------- Store threads share cache -----------------------------------------;
mov [rdi + rdx + 08],ax
mov [rdi + rdx + 10],bx
;---------- Return to cycle ---------------------------------------------------;
jmp .nextCache


