;---------- Get Operating System information ----------------------------------;
;                                                                              ;
; INPUT:   None                                                                ;
;                                                                              ;
; OUTPUT:  CF = Error flag, 0(NC) = OK, 1(C) = Error                           ;
;          AL = Error code, valid if CF = 1                                    ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoOperatingSystem:
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea rsi,[DYNA_IMPORT]
lea rdi,[OS_DATA]
mov rax,[rsi + DYNAIMPORT._GlobalMemoryStatusEx]
test rax,rax
jz .error     ; Go error if this required function not found
lea rcx,[rdi + OSDATA.memoryStatusEx]
mov [rcx + MEMORYSTATUSEX_DEF.dwLength],sizeof.MEMORYSTATUSEX_DEF
call rax 
test rax,rax
jz .error     ; Go error if this required function failed
lea rcx,[rdi + OSDATA.systemInfo]
call [GetSystemInfo]
mov rax,[rsi + DYNAIMPORT._GetActiveProcessorGroupCount]
test rax,rax
jz @f
call rax
movzx eax,ax
@@:
mov [rdi + OSDATA.activeProcessorGroupCount],eax
mov rax,[rsi + DYNAIMPORT._GetActiveProcessorCount]
test rax,rax
jz @f
mov ecx,ALL_PROCESSOR_GROUPS
call rax
@@:
mov [rdi + OSDATA.activeProcessorCount],eax
mov rax,[rsi + DYNAIMPORT._GetNumaHighestNodeNumber]
test rax,rax
jz @f
lea rbx,[rdi + OSDATA.numaNodeCount]
mov rcx,rbx
call rax
test rax,rax
jz @f
mov eax,[rbx]
inc eax
@@:               ; If no data, EAX=0 at this point, means no nodes found
mov [rbx],eax
;---------- Get large page size -----------------------------------------------;
mov rax,[rsi + DYNAIMPORT._GetLargePageMinimum]
test rax,rax
jz @f
call rax
@@:
mov [rdi + OSDATA.largePageSize],rax
;---------- Get large page information: try allocation ------------------------;
sub rsp,128 + 32
xor eax,eax
mov [rdi + OSDATA.largePageEnable],eax
cmp [rsi + DYNAIMPORT._OpenProcessToken],rax
je .exit
cmp [rsi + DYNAIMPORT._AdjustTokenPrivileges],rax 
je .exit
cmp [rdi + OSDATA.largePageSize],rax 
je .exit
call [GetCurrentProcess]
test rax,rax
jz .skip
mov rcx,rax
mov edx,MAXIMUM_ALLOWED 
lea r8,[rsp + 120 - 32]
call [rsi + DYNAIMPORT._OpenProcessToken] 
test rax,rax 
jz .skip   
mov rcx,[rsp + 120 - 32]  
xor edx,edx
lea r8,[rsp + 48]
xor r9d,r9d
mov [rsp + 32],r9
mov [rsp + 40],r9
mov [tp.PrivilegeCount],1 
mov [tp.Privileges.Luid.usedpart],SE_LOCK_MEMORY_PRIVILEGE 
and [tp.Privileges.Luid.ignorehigh32bitpart],0  
mov [tp.Privileges.Attributes],SE_PRIVILEGE_ENABLED
call [rsi + DYNAIMPORT._AdjustTokenPrivileges] 
mov rbx,rax 
mov rcx,[rsp + 120 - 32] 
call [CloseHandle] 
.skip:
test rax,rax
jz .exit
xor eax,eax
test rbx,rbx
jz .exit 
call [GetCurrentProcess]
test rax,rax
jz .exit
mov rbx,rax
mov rcx,rax 
xor edx,edx
mov r8,[rdi + OSDATA.largePageSize]
mov r9d,MEM_COMMIT + MEM_LARGE_PAGES
push 0
push PAGE_READWRITE
sub rsp,32
call [VirtualAllocEx]
add rsp,32 + 16
test rax,rax
jz @f
mov rcx,rbx
mov rdx,rax
xor r8d,r8d
mov r9d,MEM_RELEASE
call [VirtualFreeEx]
@@:
test rax,rax
setnz al
movzx eax,al
mov [rdi + OSDATA.largePageEnable],eax 
.exit:
mov rsp,rbp
ret
.error:
mov al,MSG_ERROR_MEMORY_API
stc
jmp .exit

;--- Build Operating System information for text report and GUI widgets -------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextOperatingSystem:
mov [DYNA_PTR.textOs.objectStart],rbp
mov ax,STR_OS_MEMORY_LOAD                        ; Memory load percentage
call HelperOsInfoString
mov eax,[OS_DATA.memoryStatusEx.dwMemoryLoad]
push rax
mov bl,0
call DecimalPrint32
mov ax,' %'
stosw
pop rax
mov rdi,r8
call HexPrint32 
mov ax,STR_OS_TOTAL_PHYSICAL                     ; Total physical memory
call HelperOsInfoString
mov al,MEMORYSTATUSEX_DEF.ullTotalPhys
call HelperOsInfoSize
mov ax,STR_OS_AVAIL_PHYSICAL                     ; Available physical memory
call HelperOsInfoString
mov al,MEMORYSTATUSEX_DEF.ullAvailPhys
call HelperOsInfoSize
mov ax,STR_OS_TOTAL_PAGE_FILE                    ; Total page file 
call HelperOsInfoString
mov al,MEMORYSTATUSEX_DEF.ullTotalPageFile
call HelperOsInfoSize
mov ax,STR_OS_AVAIL_PAGE_FILE                    ; Available page file
call HelperOsInfoString
mov al,MEMORYSTATUSEX_DEF.ullAvailPageFile
call HelperOsInfoSize
mov ax,STR_OS_TOTAL_VIRTUAL                      ; Total virtual user space
call HelperOsInfoString
mov al,MEMORYSTATUSEX_DEF.ullTotalVirtual
call HelperOsInfoSize
mov ax,STR_OS_AVAIL_VIRTUAL                      ; Available virtual user space
call HelperOsInfoString
mov al,MEMORYSTATUSEX_DEF.ullAvailVirtual
call HelperOsInfoSize
mov ax,STR_OS_EXT_VIRTUAL                        ; Extended virtual
call HelperOsInfoString
mov al,'-'
stosb
mov al,MEMORYSTATUSEX_DEF.ullAvailExtendedVirtual
call HelperOsInfoHex
mov rdi,rbp
mov ax,0A0Dh                                     ; Interval
stosw
mov rbp,rdi 
mov bh,0                                         ; BH = 0 means system info
call HelperSystemInfo
mov rdi,rbp
mov ax,0A0Dh                                     ; Interval
stosw
mov rbp,rdi 
mov ax,STR_OS_PROC_CURRENT                       ; Processors at currebt group
call HelperOsInfoString
mov al,SYSTEM_INFO.dwNumberOfProcessors
call HelperOsInfoNumber32
mov ecx,[OS_DATA.activeProcessorCount] ; [r15 + REGISTRY64.osData.activeProcessorCount]      ; Processors total
jecxz @f
push rcx
mov ax,STR_OS_PROC_TOTAL
call HelperOsInfoString
pop rax
call HelperOsInfoValue32
@@:
mov ecx,[OS_DATA.activeProcessorGroupCount]   ; [r15 + REGISTRY64.osData.activeProcessorGroupCount] ; Processors groups
jecxz @f
push rcx
mov ax,STR_OS_PROC_GROUPS
call HelperOsInfoString
pop rax
call HelperOsInfoValue32
@@:
mov ecx,[OS_DATA.numaNodeCount] ; [r15 + REGISTRY64.osData.numaNodeCount]  ; NUMA domains
jecxz @f
push rcx
mov ax,STR_OS_NUMA_DOMAINS
call HelperOsInfoString
pop rax
call HelperOsInfoValue32
@@:
mov ax,STR_OS_NORMAL_PAGE                        ; Normal page size
call HelperOsInfoString
mov al,SYSTEM_INFO.dwPageSize
call HelperOsInfoSizeAuto32
mov rcx,[OS_DATA.largePageSize] ; [r15 + REGISTRY64.osData.largePageSize]  ; Large page size and status
jrcxz .skipLargePages
push rcx
mov ax,STR_OS_LARGE_PAGE
call HelperOsInfoString
pop rax
push rax
mov bl,0FFh
call SizePrint64
cmp [OS_DATA.largePageEnable],0 ; [r15 + REGISTRY64.osData.largePageEnable],0
mov ax,STR_OS_DISABLED
je @f
mov ax,STR_OS_ENABLED
@@:
call PoolStringWrite
pop rax
mov rdi,r8
call HexPrint64
.skipLargePages:
mov al,0
stosb
mov rbp,rdi
mov [DYNA_PTR.textOs.objectStop],rbp
ret

;--- Pre-blank string with CR, LF, write parameter name, set text pointers ----;
; INPUT:   RBP = Pointer to destination buffer                                 ;
;          AX  = Index for parameter name for write to string                  ; 
; OUTPUT:  RDI = Pointer to parameter value position in the string             ;
;          R8  = Pointer to parameter hex value position in the string         ;
;          RBP = Updated pointer to destination buffer                         ;   
;------------------------------------------------------------------------------;
HelperOsInfoString:
push rsi
mov rdi,rbp
push rax
cld
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
pop rax
mov rbp,rdi
lea rdi,[rbp - 82 + 01]
call PoolStringWrite
lea rdi,[rbp - 82 + 33]
lea r8,[rbp - 82 + 62]
pop rsi
ret

;---------- Write memory block size to pre-blanked string, include hex --------;
;           This variant for units fixed = megabytes.                          ;
;           This variant for memory status structure.                          ;
;           This variant for 64-bit value.                                     ;
; INPUT:   RDI = Pointer to destination buffer, value position                 ;
;          R8  = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the MEMORYSTATUSEX structure                        ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  RDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoSize:
mov bl,2
SizeHelperEntry:
movzx eax,al
cmp bl,0FFh
je .sysinfo
lea rcx,[OS_DATA.memoryStatusEx]
jmp .done
.sysinfo:
test bh,bh
jnz .nativeInfo
lea rcx,[OS_DATA.systemInfo]
jmp .done
.nativeInfo:
lea rcx,[OS_DATA.nativeSystemInfo]
.done:
mov rax,[rcx + rax]
cmp bl,0
je .skip
call SizePrint64
.skip:
mov rdi,r8
jmp HexPrint64

;---------- Write memory block size to pre-blanked string, include hex --------;
;           This variant for units auto selected                               ;
;           This variant for system information structure.                     ;
;           This variant for 64-bit value.                                     ;
; INPUT:   RDI = Pointer to destination buffer, value position                 ;
;          R8  = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the SYSTEM_INFO structure                           ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  RDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoSizeAuto:
mov bl,0FFh
jmp SizeHelperEntry

;---------- Write memory block size to pre-blanked, hex only ------------------;
;           This variant for memory status structure.                          ;
;           This variant for 64-bit value.                                     ;
; INPUT:   R8  = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the MEMORYSTATUSEX structure                        ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  RDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoHex:
mov bl,0
jmp SizeHelperEntry 

;---------- Write memory block size to pre-blanked string, include hex --------;
;           This variant for units fixed = megabytes.                          ;
;           This variant for system information structure.                     ;
;           This variant for 32-bit value.                                     ;
; INPUT:   RDI = Pointer to destination buffer, value position                 ;
;          R8  = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the SYSTEM_INFO structure                           ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  RDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoSizeAuto32:
mov bl,0FFh
movzx eax,al
test bh,bh
jnz .nativeInfo
mov eax,[OS_DATA.systemInfo + rax]
jmp .doneInfo
.nativeInfo:
mov eax,[OS_DATA.nativeSystemInfo + rax]
.doneInfo:
push rax
call SizePrint64
pop rax
mov rdi,r8
jmp HexPrint32

;---------- Write 32-bit number to pre-blanked string, decimal and hex --------;
; INPUT:   RDI = Pointer to destination buffer, decimal value position         ;
;          R8  = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the SYSTEM_INFO structure                           ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  RDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoNumber32:
movzx eax,al
test bh,bh
jnz .nativeInfo
mov eax,[OS_DATA.systemInfo + rax]
jmp .doneInfo
.nativeInfo:
mov eax,[OS_DATA.nativeSystemInfo + rax]
.doneInfo:
HelperOsInfoValue32:     ; Entry point for EAX = value, RDI, R8 same usage
push rax
mov bl,0
call DecimalPrint32
pop rax
mov rdi,r8
jmp HexPrint32

;---------- Write 16-bit number to pre-blanked string, decimal and hex --------;
; INPUT:   RDI = Pointer to destination buffer, decimal value position         ;
;          R8  = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the SYSTEM_INFO structure                           ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  RDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoNumber16:
movzx eax,al
test bh,bh
jnz .nativeInfo
movzx eax,word [OS_DATA.systemInfo + rax]
jmp .doneInfo
.nativeInfo:
movzx eax,word [OS_DATA.nativeSystemInfo + rax]
.doneInfo:
push rax
mov bl,0
call DecimalPrint32
pop rax
mov rdi,r8
jmp HexPrint16

;---------- Write system information strings ----------------------------------;
; INPUT:   RBP = Pointer to destination buffer                                 ;
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  RBP = Updated pointer to destination buffer                         ;
;          RDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperSystemInfo:
mov ax,STR_OS_MIN_ADDRESS                        ; Application minimum address
call HelperOsInfoString
mov al,SYSTEM_INFO.lpMinimumApplicationAddress
call HelperOsInfoSizeAuto
mov ax,STR_OS_MAX_ADDRESS                        ; Application maximum address
call HelperOsInfoString
mov al,SYSTEM_INFO.lpMaximumApplicationAddress
call HelperOsInfoSizeAuto
mov ax,STR_OS_PROC_MASK                          ; Active processor mask 
call HelperOsInfoString
mov al,'-'
stosb
test bh,bh
jnz .nativeInfo
mov rax,[OS_DATA.systemInfo.dwActiveProcessorMask]
jmp .doneInfo
.nativeInfo:
mov rax,[OS_DATA.nativeSystemInfo.dwActiveProcessorMask]
.doneInfo:
mov rdi,r8
call HexPrint64 
mov ax,STR_OS_PROC_TYPE                          ; Processor type
call HelperOsInfoString
mov al,SYSTEM_INFO.dwProcessorType
call HelperOsInfoNumber32
mov ax,STR_OS_ALLOC_GRAN                         ; Allocation granularity
call HelperOsInfoString
mov al,SYSTEM_INFO.dwAllocationGranularity
call HelperOsInfoSizeAuto32 ; HelperOsInfoNumber32
mov ax,STR_OS_PROC_LEVEL                         ; Processor level
call HelperOsInfoString
mov al,SYSTEM_INFO.wProcessorLevel
call HelperOsInfoNumber16
mov ax,STR_OS_PROC_REVISION                      ; Processor revision
call HelperOsInfoString
mov al,SYSTEM_INFO.wProcessorRevision
jmp HelperOsInfoNumber16
