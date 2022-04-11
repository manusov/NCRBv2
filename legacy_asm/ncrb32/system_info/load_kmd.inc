; UNDER CONSTRUCTION.



LoadKernelModeDriver:
push ebx esi edi ebp
;---------- Initializing data structure ---------------------------------------;
cld
lea edi,[SCP_DATA]
mov ecx,sizeof.SCPDATA
mov al,0
rep stosb
lea eax,[TEMP_BUFFER]
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
lea eax,[SCP_DATA.drvFile] 
push eax                     ; Parm#4 = Pointer to pointer to updated path 
push [SCP_DATA.drvPath]  ; Parm#3 = Pointer to buffer
push SCP_PATH_BUFFER_SIZE    ; Parm#2 = Buffer size
push esi                     ; Parm#1 = Pointer to file name
call [GetFullPathNameA]
test eax,eax
jz .errorGetPath             ; Go if error status returned when get path 
;---------- Open Service Control Manager (SCM) --------------------------------;
push SC_MANAGER_ALL_ACCESS  ; Parm#3 = Desired access rights  
push 0                      ; Parm#2 = Pointer to database name, 0 = not used
push 0                      ; Parm#1 = Pointer to machine name, 0 = not used
call [OpenSCManagerA]
test eax,eax
jz .errorOpenScm            ; Go if error status returned when open SCM
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
push esi                ; Parm#3 = Pointer to display name string
push esi                ; Parm#2 = Pointer to service name string
push [SCP_DATA.manager]  ; Parm#1 = Handle to SCM database
call [CreateServiceA]
test eax,eax
jz .errorCreateService  ; Go if error status returned when create service
;---------- Skip create point -------------------------------------------------;
.skipCreate:
mov [SCP_DATA.service],eax 
;---------- Check service status ----------------------------------------------;
lea edi,[SCP_DATA.status]
push edi                    ; Parm#2 = Pointer to status structure
push eax                    ; Parm#1 = Handle
call [QueryServiceStatus]
test eax,eax
jz .errorGetStatus          ; Go if error status returned when get status
cmp [edi + SERVICE_STATUS.dwCurrentState],SERVICE_RUNNING
je .skipStart               ; Go if service already running
;---------- Start service -----------------------------------------------------;
lea eax,[SCP_DATA.vectors]
push eax                    ; Parm#3 = Pointer to arguments vectors, empty 
push 0                      ; Parm#2 = Arguments count
push [SCP_DATA.service]   ; Parm#1 = Handle
call [StartService]
test eax,eax
jz .errorStartService      ; Go if error status returned when start service
;---------- Skip start point --------------------------------------------------;
.skipStart:
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
jz .errorCreateDevice   ; Go if error status returned when create device file
cmp eax,INVALID_HANDLE_VALUE
je .errorCreateDevice   ; Go if invalid handle returned when create device file
mov [SCP_DATA.driver],eax

; TODO. Errors Handling.
.errorGetPath:
.errorOpenScm:
.errorCreateService:
.errorGetStatus:
.errorStartService:
.errorCreateDevice:

pop ebp edi esi ebx
ret



; UNDER CONSTRUCTION.



UnloadKernelModeDriver:
push ebx esi edi ebp
;---------- Close device file, uninstall driver -------------------------------; 
; Note close operations sequence continued even if error, but error logged
; at R15 if R15 = 0 (if yet no errors) TODO. CORRECT, NO R15 FOR IA32. 
mov ecx,[SCP_DATA.driver]
jecxz @f
cmp ecx,INVALID_HANDLE_VALUE
je @f
push ecx                    ; Parm#1 = Manager handle
call [CloseHandle]
test eax,eax
jnz @f                      ; Go if no errors
; lea eax,[ErrorCloseDevice]
; call GetOsErrorCode
@@:

;---------- Stop service ------------------------------------------------------;

mov edi,[SCP_DATA.service]
test edi,edi 
jz .skipStop                ; Go skip if handle = 0
lea eax,[SCP_DATA.status]
push eax                    ; Parm#3 = Pointer to status structure
push SERVICE_CONTROL_STOP   ; Parm#2 = Control code
push edi                    ; Parm#1 = Handle
call [ControlService]
test eax,eax
jnz @f                      ; Go if no errors
; lea eax,[ErrorStopService]
; call GetOsErrorCode
@@:

;---------- Delete service ----------------------------------------------------;

push edi                    ; Parm#1 = Service handle
call [DeleteService]
test eax,eax
jnz @f                      ; Go if no errors
; lea eax,[ErrorDeleteService]
; call GetOsErrorCode
@@:

;---------- Close service handle ----------------------------------------------;

push edi                    ; Parm#1 = Service handle
call [CloseServiceHandle]
test eax,eax
jnz @f                      ; Go if no errors
; lea eax,[ErrorCloseService]
; call GetOsErrorCode
@@:

;---------- Skip stop point ---------------------------------------------------;
.skipStop:
;---------- Close manager handle ----------------------------------------------; 
mov ecx,[SCP_DATA.manager]
jecxz @f
push ecx                    ; Parm#1 = Manager handle
call [CloseServiceHandle]
test eax,eax
jnz @f                      ; Go if no errors
; lea eax,[ErrorCloseManager]
; call GetOsErrorCode
@@:

pop ebp edi esi ebx
ret



; UNDER CONSTRUCTION.



TryKernelModeDriver:
push ebx esi edi ebp
lea ebx,[SCP_DATA]
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
lea eax,[kmdRoutine]
mov dword [edx + SERVICE_QUERY.userproc],eax
mov dword [edx + SERVICE_QUERY.userproc + 4],0   ; Required for WoW 32/64
call [WriteFile]
test eax,eax                   ; EAX = Status
pop eax edx                    ; EDX:EAX = Output variable
jz .errorWriteDevice           ; Go if error status returned when send request

; TODO. Errors Handling.
.errorWriteDevice:

pop ebp edi esi ebx
ret


;------------------------------------------------------------------------------;
;       Fragment executed at kernel mode, as callback from KMD handler.        ;
;               This is debug example: read IA32_APIC_BASE MSR.                ;
;                                                                              ;
; INPUT:     EAX = Pointer to data buffer                                      ;
; OUTPUT:    8 bytes at [EAX] = IA32_APIC_BASE MSR value, 64-bit               ;
; Destroyed: EAX, EBX, ECX, EDX.                                               ; 
;------------------------------------------------------------------------------;
kmdRoutine:
mov ebx,eax          ; EBX = Pointer for store data after read MSR
mov ecx,01Bh         ; ECX = address of IA32_APIC_BASE MSR
rdmsr                ; Read MSR selected by ECX
mov [ebx + 0],eax    ; Store bits [31-00] of MSR
mov [ebx + 4],edx    ; Store bits [63-32] of MSR 
retn
