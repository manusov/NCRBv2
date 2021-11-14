;---------- Get Operating System information ----------------------------------;
;                                                                              ;
; INPUT:   None                                                                ;
;                                                                              ;
; OUTPUT:  CF = Error flag, 0(NC) = OK, 1(C) = Error                           ;
;          AL = Error code, valid if CF = 1                                    ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoOperatingSystem:
lea ebx,[DYNA_IMPORT]
lea edi,[OS_DATA]
mov eax,[ebx + DYNAIMPORT._GlobalMemoryStatusEx]
test eax,eax
jz .error     ; Go error if this required function not found
lea ecx,[OS_DATA.memoryStatusEx]
mov [ecx + MEMORYSTATUSEX_DEF.dwLength],sizeof.MEMORYSTATUSEX_DEF
push ecx
call eax 
test eax,eax
jz .error     ; Go error if this required function failed
lea ecx,[OS_DATA.systemInfo]
push ecx
call [GetSystemInfo]
mov eax,[ebx + DYNAIMPORT._GetActiveProcessorGroupCount]
test eax,eax
jz @f
call eax
movzx eax,ax
@@:
mov [OS_DATA.activeProcessorGroupCount],eax
mov eax,[ebx + DYNAIMPORT._GetActiveProcessorCount]
test eax,eax
jz @f
push ALL_PROCESSOR_GROUPS
call eax
@@:
mov [OS_DATA.activeProcessorCount],eax
mov eax,[ebx + DYNAIMPORT._GetNumaHighestNodeNumber]
test eax,eax
jz @f
lea ecx,[OS_DATA.numaNodeCount]
push ecx
call eax
test eax,eax
jz @f
mov eax,[OS_DATA.numaNodeCount]
inc eax
@@:               ; If no data, EAX=0 at this point, means no nodes found
mov [OS_DATA.numaNodeCount],eax
;---------- Get large page size -----------------------------------------------;
mov eax,[ebx + DYNAIMPORT._GetLargePageMinimum]
test eax,eax
jz @f
call eax
@@:
mov [OS_DATA.largePageSize],eax
;---------- Get large page information: try allocation ------------------------;
push esi
sub esp,128
xor eax,eax
mov [OS_DATA.largePageEnable],eax
cmp [ebx + DYNAIMPORT._OpenProcessToken],eax
je .exit
cmp [ebx + DYNAIMPORT._AdjustTokenPrivileges],eax 
je .exit
cmp [OS_DATA.largePageSize],eax 
je .exit
call [GetCurrentProcess]
test eax,eax
jz .skip
lea ebp,[esp + 120]
push ebp 
push MAXIMUM_ALLOWED
push eax
call [ebx + DYNAIMPORT._OpenProcessToken]
test eax,eax 
jz .skip   
xor eax,eax  
mov esi,esp
push eax
push eax
push eax
push esi 
push eax
push dword [ebp]
mov [tp.PrivilegeCount],1 
mov [tp.Privileges.Luid.usedpart],SE_LOCK_MEMORY_PRIVILEGE 
and [tp.Privileges.Luid.ignorehigh32bitpart],0  
mov [tp.Privileges.Attributes],SE_PRIVILEGE_ENABLED
call [ebx + DYNAIMPORT._AdjustTokenPrivileges] 
mov esi,eax 
push dword [ebp] 
call [CloseHandle] 
.skip:
test eax,eax
jz .exit
xor eax,eax
test esi,esi
jz .exit 
call [GetCurrentProcess]
test eax,eax
jz .exit
mov esi,eax
push PAGE_READWRITE
push MEM_COMMIT + MEM_LARGE_PAGES
push [OS_DATA.largePageSize]
push 0
push esi
call [VirtualAllocEx]
test eax,eax
jz @f
push MEM_RELEASE
push 0
push eax
push esi
call [VirtualFreeEx]
@@:
test eax,eax
setnz al
movzx eax,al
mov [OS_DATA.largePageEnable],eax 
.exit:
add esp,128
pop esi
clc
ret
.error:
mov al,MSG_ERROR_MEMORY_API
stc
ret

;--- Build Operating System information for text report and GUI widgets -------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextOperatingSystem:
mov [DYNA_PTR.textOs.objectStart],ebp
mov ax,STR_OS_MEMORY_LOAD
call HelperOsInfoString                          ; Memory load percentage
mov eax,[OS_DATA.memoryStatusEx.dwMemoryLoad]
push eax
mov bl,0
call DecimalPrint32
mov ax,' %'
stosw
pop eax
mov edi,ecx
call HexPrint32 
mov ax,STR_OS_TOTAL_PHYSICAL
call HelperOsInfoString                          ; Total physical memory
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
mov edi,ebp
mov ax,0A0Dh                                     ; Interval
stosw
mov ebp,edi 
mov bh,0                                         ; BH = 0 means system info
call HelperSystemInfo
mov edi,ebp
mov ax,0A0Dh                                     ; Interval
stosw
mov ebp,edi 
mov ax,STR_OS_PROC_CURRENT                       ; Processors at currebt group
call HelperOsInfoString
mov al,SYSTEM_INFO.dwNumberOfProcessors
call HelperOsInfoNumber32
mov ecx,[OS_DATA.activeProcessorCount]      ; Processors total
jecxz @f
push ecx
mov ax,STR_OS_PROC_TOTAL
call HelperOsInfoString
pop eax
call HelperOsInfoValue32
@@:
mov ecx,[OS_DATA.activeProcessorGroupCount] ; Processors groups
jecxz @f
push ecx
mov ax,STR_OS_PROC_GROUPS
call HelperOsInfoString
pop eax
call HelperOsInfoValue32
@@:
mov ecx,[OS_DATA.numaNodeCount]  ; NUMA domains
jecxz @f
push ecx
mov ax,STR_OS_NUMA_DOMAINS
call HelperOsInfoString
pop eax
call HelperOsInfoValue32
@@:
mov ax,STR_OS_NORMAL_PAGE                        ; Normal page size
call HelperOsInfoString
mov al,SYSTEM_INFO.dwPageSize
call HelperOsInfoSizeAuto32
mov ecx,[OS_DATA.largePageSize]  ; Large page size and status
jecxz .skipLargePages
push ecx
mov ax,STR_OS_LARGE_PAGE
call HelperOsInfoString
pop eax
push eax ecx esi
mov bl,0FFh
xor edx,edx
call SizePrint64
cmp [OS_DATA.largePageEnable],0
mov ax,STR_OS_DISABLED
je @f
mov ax,STR_OS_ENABLED
@@:
call PoolStringWrite
pop esi edi eax
xor edx,edx
call HexPrint64
.skipLargePages:
mov al,0
stosb
mov ebp,edi
mov [DYNA_PTR.textOs.objectStop],ebp
ret

;--- Pre-blank string with CR, LF, write parameter name, set text pointers ----;
; INPUT:   EBP = Pointer to destination buffer                                 ;
;          AX  = Index for parameter name for write to string                  ; 
; OUTPUT:  EDI = Pointer to parameter value position in the string             ;
;          ECX  = Pointer to parameter hex value position in the string        ;
;          EBP = Updated pointer to destination buffer                         ;   
;------------------------------------------------------------------------------;
HelperOsInfoString:
push esi
mov edi,ebp
push eax
cld
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
pop eax
mov ebp,edi
lea edi,[ebp - 82 + 01]
call PoolStringWrite
lea edi,[ebp - 82 + 33]
lea ecx,[ebp - 82 + 62]
pop esi
ret

;---------- Write memory block size to pre-blanked string, include hex --------;
;           This variant for units fixed = megabytes                           ;
;           This variant for memory status structure.                          ;
;           This variant for 64-bit value.                                     ;
; INPUT:   EDI = Pointer to destination buffer, value position                 ;
;          ECX = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the MEMORYSTATUSEX structure                        ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  EDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoSize:
mov bl,2
SizeHelperEntry:
movzx eax,al
push ecx
cmp bl,0FFh
je .sysinfo
lea ecx,[OS_DATA.memoryStatusEx]
jmp .done
.sysinfo:
test bh,bh
jnz .nativeInfo
lea ecx,[OS_DATA.systemInfo]
jmp .done
.nativeInfo:
lea ecx,[OS_DATA.nativeSystemInfo]
.done:
mov edx,[ecx + eax + 4]
mov eax,[ecx + eax + 0]
cmp bl,0
je .skip
call SizePrint64
.skip:
pop edi
jmp HexPrint64

;---------- Write memory block size to pre-blanked string, include hex --------;
;           This variant for units auto selected                               ;
;           This variant for system information structure.                     ;
;           This variant for 64-bit value.                                     ;
; INPUT:   EDI = Pointer to destination buffer, value position                 ;
;          ECX = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the MEMORYSTATUSEX structure                        ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  EDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoSizeAuto:
mov bl,0FFh
jmp SizeHelperEntry

;---------- Write memory block size to pre-blanked, hex only ------------------;
;           This variant for memory status structure.                          ;
;           This variant for 64-bit value.                                     ;
; INPUT:   ECX = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the MEMORYSTATUSEX structure                        ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  EDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoHex:
mov bl,0
jmp SizeHelperEntry 

;---------- Write memory block size to pre-blanked string, include hex --------;
;           This variant for units fixed = megabytes.                          ;
;           This variant for system information structure.                     ;
;           This variant for 32-bit value.                                     ;
; INPUT:   EDI = Pointer to destination buffer, value position                 ;
;          ECX = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the SYSTEM_INFO structure                           ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  EDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoSizeAuto32:
mov bl,0FFh
movzx eax,al
push ecx
test bh,bh
jnz .nativeInfo
mov eax,[OS_DATA.systemInfo + eax]
jmp .doneInfo
.nativeInfo:
mov eax,[OS_DATA.nativeSystemInfo + eax]
.doneInfo:
push eax
xor edx,edx
call SizePrint64
pop eax edi
jmp HexPrint32

;---------- Write 32-bit number to pre-blanked string, decimal and hex --------;
; INPUT:   EDI = Pointer to destination buffer, decimal value position         ;
;          ECX = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the SYSTEM_INFO structure                           ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  EDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoNumber32:
movzx eax,al
push ecx
test bh,bh
jnz .nativeInfo
mov eax,[OS_DATA.systemInfo + eax]
jmp .doneInfo
.nativeInfo:
mov eax,[OS_DATA.nativeSystemInfo + eax]
.doneInfo:
pop ecx
HelperOsInfoValue32:     ; Entry point for EAX = value, EDI, ECX same usage
push eax
mov bl,0
call DecimalPrint32
pop eax
mov edi,ecx
jmp HexPrint32

;---------- Write 16-bit number to pre-blanked string, decimal and hex --------;
; INPUT:   EDI = Pointer to destination buffer, decimal value position         ;
;          ECX = Pointer to destination buffer, hex value position             ; 
;          AL  = Offset in the SYSTEM_INFO structure                           ; 
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  EDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperOsInfoNumber16:
movzx eax,al
push ecx
test bh,bh
jnz .nativeInfo
movzx eax,word [OS_DATA.systemInfo + eax]
jmp .doneInfo
.nativeInfo:
movzx eax,word [OS_DATA.nativeSystemInfo + eax]
.doneInfo:
push eax
mov bl,0
call DecimalPrint32
pop eax edi
jmp HexPrint16

;---------- Write system information strings ----------------------------------;
; INPUT:   EBP = Pointer to destination buffer                                 ;
;          BH = information type: 0 = OS info , 1 = native OS info             ;
; OUTPUT:  EBP = Updated pointer to destination buffer                         ;
;          EDI = Last used and updated destination pointer                     ;
;------------------------------------------------------------------------------;
HelperSystemInfo:
mov ax,STR_OS_MIN_ADDRESS                        ; Application minimum address
call HelperOsInfoString
mov al,SYSTEM_INFO.lpMinimumApplicationAddress
call HelperOsInfoSizeAuto32
mov ax,STR_OS_MAX_ADDRESS                        ; Application maximum address
call HelperOsInfoString
mov al,SYSTEM_INFO.lpMaximumApplicationAddress
call HelperOsInfoSizeAuto32
mov ax,STR_OS_PROC_MASK                          ; Active processor mask 
call HelperOsInfoString
mov al,'-'
stosb
test bh,bh
jnz .nativeInfo
mov eax,[OS_DATA.systemInfo.dwActiveProcessorMask]
jmp .doneInfo
.nativeInfo:
mov eax,[OS_DATA.nativeSystemInfo.dwActiveProcessorMask]
.doneInfo:
mov edi,ecx
call HexPrint32 
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

