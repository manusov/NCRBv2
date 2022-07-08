;---------- Load Kernel Mode Driver -------------------------------------------;
; This loader for NCRB32 loads 32-bit driver if Win32,                         ;
; or 64-bit driver if Win64 with WoW64 detected (if NCRB32 under Win64).       ;
; This loader for NCRB64 loads 64-bit driver only.                             ;
; Selection by system info variable [OS_DATA.isWow64].                         ;
;                                                                              ;
; INPUT:   ECX = Pointer to error status variables, for error reporting:       ;
;                3 DWORDS, 2 pointers to strings + 1 OS API error code.        ;         
;                                                                              ;
; OUTPUT:  EAX = Win API Status, 0 means error, non zero means no errors.      ;
;          ECX = Win API detail error code, valid if errors (when EAX=0 only). ;
;          DX  = Error name string ID, valid if errors (when EAX=0 only).      ;  
;                                                                              ;
;------------------------------------------------------------------------------;
LoadKernelModeDriver:
push ebx esi edi ebp ecx
;---------- Initializing data structure ---------------------------------------;
cld
mov edi,SCP_DATA
mov ecx,sizeof.SCPDATA
mov al,0
rep stosb
mov eax,TEMP_BUFFER
mov [SCP_DATA.drvPath],eax
mov [SCP_DATA.driver],INVALID_HANDLE_VALUE
;---------- Initializing full path to driver file -----------------------------;
mov esi,[APP_DATA.lockedStrings]
mov ax,STR_NAME_KMD32
cmp [OS_DATA.isWow64],0
je @f
mov ax,STR_NAME_KMD64
@@:
call IndexString
mov eax,SCP_DATA.drvFile 
push eax                     ; Parm#4 = Pointer to pointer to updated path 
push [SCP_DATA.drvPath]  ; Parm#3 = Pointer to buffer
push SCP_PATH_BUFFER_SIZE    ; Parm#2 = Buffer size
push esi                     ; Parm#1 = Pointer to file name
call [GetFullPathNameA]
test eax,eax
jz errorGetPath              ; Go if error status returned when get path 
;---------- Open Service Control Manager (SCM) --------------------------------;
push SC_MANAGER_ALL_ACCESS  ; Parm#3 = Desired access rights  
push 0                      ; Parm#2 = Pointer to database name, 0 = not used
push 0                      ; Parm#1 = Pointer to machine name, 0 = not used
call [OpenSCManagerA]
test eax,eax
jz errorOpenScm             ; Go if error status returned when open SCM
mov [SCP_DATA.manager],eax 
;---------- Try Open Service, check for already exist -------------------------;
push eax
mov esi,[APP_DATA.lockedStrings]
mov ax,STR_DRV_NAME
call IndexString
pop eax
push SERVICE_ALL_ACCESS  ; Parm#3 = Desired access rights
push esi                 ; Parm#2 = Pointer to service name
push eax                 ; Parm#1 = Service handle
call [OpenServiceA]      
test eax,eax
jnz .skipCreate          ; Go if open success, means this service already exist
;---------- Create Service ----------------------------------------------------;
xor eax,eax             ; EAX = 0 for compact push 0
push eax                ; Parm#13 = Pointer to password, 0 = not used
push eax                ; Parm#12 = Pointer to service name, 0 = default
push eax                ; Parm#11 = Pointer to dependencies, 0 = none
push eax                ; Parm#10 = Pointer to Tag ID, 0 = none
push eax                ; Parm#9  = Pointer to Load Order Groups list, 0 = none
push [SCP_DATA.drvPath]   ; Parm#8  = Pointer to driver binary file path
push SERVICE_ERROR_NORMAL   ; Parm#7 = Error control option
push SERVICE_DEMAND_START   ; Parm#6 = Service start option
push SERVICE_KERNEL_DRIVER  ; Parm#5 = Service type
push SERVICE_ALL_ACCESS     ; Parm#4 = Desired access
push esi                 ; Parm#3 = Pointer to display name string
push esi                 ; Parm#2 = Pointer to service name string
push [SCP_DATA.manager]  ; Parm#1 = Handle to SCM database
call [CreateServiceA]
test eax,eax
jz errorCreateService    ; Go if error status returned when create service
;---------- Skip create point -------------------------------------------------;
.skipCreate:
mov [SCP_DATA.service],eax 
;---------- Check service status ----------------------------------------------;
mov edi,SCP_DATA.status
push edi                    ; Parm#2 = Pointer to status structure
push eax                    ; Parm#1 = Handle
call [QueryServiceStatus]
test eax,eax
jz errorGetStatus           ; Go if error status returned when get status
cmp [edi + SERVICE_STATUS.dwCurrentState],SERVICE_RUNNING
je skipStart                ; Go if service already running
;---------- Start service -----------------------------------------------------;
mov eax,SCP_DATA.vectors
push eax                    ; Parm#3 = Pointer to arguments vectors, empty 
push 0                      ; Parm#2 = Arguments count
push [SCP_DATA.service]     ; Parm#1 = Handle
call [StartService]
test eax,eax
jz errorStartService        ; Go if error status returned when start service
;---------- Skip start point --------------------------------------------------;
skipStart:
;---------- Create (open) device file -----------------------------------------;
mov esi,[APP_DATA.lockedStrings]
mov ax,STR_DRV_DEVICE
call IndexString
xor eax,eax                    ; EAX = 0 for compact push 0
push eax                       ; Parm#7 = Template file handle, 0 = not used
push eax                       ; Parm#6 = Flags and attributes, 0 = not used
push OPEN_EXISTING             ; Parm#5 = Creation disposition 
push 0                         ; Parm#4 = Security attributes  
push FILE_SHARE_READ or FILE_SHARE_WRITE  ; Parm#3 = Share mode 
push GENERIC_WRITE or GENERIC_READ        ; Parm#2 = Desired access
push esi                      ; Parm#1 = Pointer to file=device name
call [CreateFile]
test eax,eax
jz errorCreateDevice    ; Go if error status returned when create device file
cmp eax,INVALID_HANDLE_VALUE
je errorCreateDevice    ; Go if invalid handle returned when create device file
mov [SCP_DATA.driver],eax
;---------- Load kernel mode driver done, return status -----------------------;
; This entry points can be used from other subroutines also (unloader).
kmdOk:
mov eax,1                  ; EAX = 1 means kernel mode driver loaded OK
kmdDone:
push eax edx
xor esi,esi                ; Pre-clear OS error code
mov edi,1                  ; Set yet no errors
call HelperOsErrorCode     ; Get OS error code if error detected
mov ecx,esi                ; ECX = OS error code for return
pop edx eax
;---------- This means loader -------------------------------------------------; 
mov bx,STR_DRV_ERROR_LOAD  ; BX = Operation name string ID
;--- Exit with update error details string if error, or blank if no errors ----; 
; This 3 dwords (2 pointers + OS error code) valid only if error, when EAX = 0.
kmdExitStrings:
test eax,eax
jnz kmdReturn     ; Skip error decoding if no errors
mov edi,[esp]
push eax ecx edx
mov [edi + 08],ecx
xchg eax,ebx
mov esi,[APP_DATA.lockedStrings]
push esi
call IndexString
mov [edi + 00],esi
pop esi 
mov dword [edi + 04],0
cmp edx,-1
je @f
xchg eax,edx
call IndexString
mov [edi + 04],esi 
@@:
pop edx ecx eax
;---------- Shared return point -----------------------------------------------;
kmdReturn:
pop ebx ebp edi esi ebx
ret
;---------- Errors Handling ---------------------------------------------------;
; This entry points can be used from other subroutines also (unloader).
errorGetPath:
mov dx,STR_DRE_GET_PATH
jmp kmdFailed
errorOpenScm:
mov dx,STR_DRE_OPEN_SERVICE
jmp kmdFailed
errorCreateService:
mov dx,STR_DRE_CREATE_SERVICE
jmp kmdFailed
errorGetStatus:
mov dx,STR_DRE_GET_STATUS
jmp kmdFailed
errorStartService:
mov dx,STR_DRE_START_SERVICE
jmp kmdFailed
errorCreateDevice:
mov dx,STR_DRE_CREATE_DEVICE
kmdFailed:
xor eax,eax          ; EAX = 0 means error loading kernel mode driver
jmp kmdDone

;---------- Unload Kernel Mode Driver -----------------------------------------;
;                                                                              ;
; INPUT:   ECX = Pointer to error status variables, for error reporting:       ;
;                3 DWORDS, 2 pointers to strings + 1 OS API error code.        ;         
;                                                                              ;
; OUTPUT:  EAX = Win API Status, 0 means error, non zero means no errors.      ;
;          ECX = Win API detail error code, valid if errors (when EAX=0 only). ;
;          DX  = Reserved.                                                     ;  
;                                                                              ;
;------------------------------------------------------------------------------;
UnloadKernelModeDriver:
push ebx esi edi ebp ecx
lea ebx,[SCP_DATA]
xor esi,esi                    ; Pre-clear OS error code
mov edi,1                      ; Set yet no errors
;---------- Close device file, uninstall driver -------------------------------; 
; Note close operations sequence continued even if error, but error logged
; at ESI, EDI, if EDI = 1 yet no previous errors. 
mov ecx,[ebx + SCPDATA.driver]
jecxz @f
cmp ecx,INVALID_HANDLE_VALUE
je @f
push ecx                       ; Parm#1 = Manager handle
call [CloseHandle]
call HelperOsErrorCode         ; Log OS error code: ESI, EDI
@@:
;---------- Stop service ------------------------------------------------------;
mov ebp,[ebx + SCPDATA.service]
test ebp,ebp 
jz .skipStop                   ; Go skip if handle = 0
lea eax,[ebx + SCPDATA.status]
push eax                       ; Parm#3 = Pointer to status structure
push SERVICE_CONTROL_STOP      ; Parm#2 = Control code
push ebp                       ; Parm#1 = Handle
call [ControlService]
call HelperOsErrorCode         ; Log OS error code: ESI, EDI
;---------- Delete service ----------------------------------------------------;
push ebp                       ; Parm#1 = Service handle
call [DeleteService]
call HelperOsErrorCode         ; Log OS error code: ESI, EDI
;---------- Close service handle ----------------------------------------------;
push ebp                       ; Parm#1 = Service handle
call [CloseServiceHandle]
call HelperOsErrorCode         ; Log OS error code: ESI, EDI
;---------- Skip stop point ---------------------------------------------------;
.skipStop:
;---------- Close manager handle ----------------------------------------------; 
mov ecx,[ebx + SCPDATA.manager]
jecxz @f
push ecx                       ; Parm#1 = Manager handle
call [CloseServiceHandle]
call HelperOsErrorCode         ; Log OS error code: ESI, EDI
@@:
;---------- Make status -------------------------------------------------------;
mov eax,edi                    ; EAX = 0 if error, non zero if no errors
mov ecx,esi                    ; ECX = OS error code, valid if error
mov bx,STR_DRV_ERROR_UNLOAD    ; BX  = Operation name string ID
mov edx,-1                     ; EDX = -1 means not used for unload procedure
jmp kmdExitStrings

;---------- Debug Kernel Mode Driver ------------------------------------------;
;                                                                              ;
; INPUT:   ECX = Reserved.                                                     ;   
;                                                                              ;
; OUTPUT:  EAX = Win API Status, 0 means error, non zero means no errors.      ;
;          ECX = Win API detail error code, valid if errors (when EAX=0 only). ;
;          EDX = Pattern callback routine return code.                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
TryKernelModeDriver:
push ebx esi edi ebp ecx
mov ebx,SCP_DATA
;---------- Write to device file, this is requests for driver -----------------;
push 0 0                       ; Variable for output
push 0                         ; Parm#5 = Pointer to overlapped, 0 = not used
lea eax,[ebx + SCPDATA.bytes]
push eax                       ; Parm#4 = Pointer to byte count variable
push sizeof.SERVICE_QUERY      ; Parm#3 = Requested byte count
lea edx,[ebx + SCPDATA.query]
push edx                       ; Parm#2 = Pointer to file buffer=request
push [ebx + SCPDATA.driver]    ; Parm#1 = File=device handle
mov eax,RZ_REQUEST_CODE
mov dword [edx + SERVICE_QUERY.iocode],eax
lea eax,[esp + 20]
mov dword [edx + SERVICE_QUERY.parm1],eax
mov eax,KmdRoutine
mov dword [edx + SERVICE_QUERY.userproc],eax
mov dword [edx + SERVICE_QUERY.userproc + 4],0   ; Required for WoW 32/64
call [WriteFile]
test eax,eax                   ; EAX = Status
pop edx ebx                    ; EBX:EDX = Output variable
;---------- Analusing status and exit -----------------------------------------;
xor esi,esi                    ; Pre-clear OS error code
mov edi,1                      ; Set yet no errors
call HelperOsErrorCode         ; Log OS error code: ESI, EDI
mov eax,edi                    ; EAX = 0 if error, non zero if no errors
mov ecx,esi                    ; ECX = OS error code, valid if error
jmp kmdReturn                  ; DX not used for unload procedure

;---------- Subroutine for kernel mode debug ----------------------------------;
; Fragment executed at kernel mode, as callback from KMD handler.              ;
; This is debug example: read IA32_APIC_BASE MSR.                              ;
;                                                                              ;
; INPUT:     EAX = Pointer to data buffer                                      ;
; OUTPUT:    8 bytes at [EAX] = IA32_APIC_BASE MSR value, 64-bit               ;
; Destroyed: EAX, EBX, ECX, EDX.                                               ; 
;------------------------------------------------------------------------------;
KmdRoutine:
mov ebx,eax          ; EBX = Pointer for store data after read MSR
mov ecx,01Bh         ; ECX = address of IA32_APIC_BASE MSR
rdmsr                ; Read MSR selected by ECX
mov [ebx + 0],eax    ; Store bits [31-00] of MSR
mov [ebx + 4],edx    ; Store bits [63-32] of MSR 
retn

;---------- Get OS error code -------------------------------------------------;
;                                                                              ;
; INPUT:   EAX = Status after WinAPI, 0 if error, non zero if OK               ;   
;          ESI = OS error code, 0 if yet not assigned                          ;
;          EDI = Error status, 0 = error, non zero means OK                    ;
;                                                                              ;
; OUTPUT:  ESI updated by OS error code,                                       ; 
;          if error detected ( ESI = 0 ) and                                   ; 
;          if error code yet not assigned ( input ESI = 0 )                    ;
;          EDI cleared to 0 if error                                           ;
;                                                                              ;
; Must not corrupt all registers, include volatile, because called at          ;
; driver load-unload process.                                                  ;
;------------------------------------------------------------------------------;
HelperOsErrorCode:
test eax,eax
jnz @f                      ; Skip error code assignment if no errors
xor edi,edi                 ; EDI = 0 means error
test esi,esi
jnz @f                      ; Skip error code assignment if already assigned 
push eax ecx edx            ; Save volatile registers
call [GetLastError]
xchg esi,eax
pop edx ecx eax
@@:
ret
