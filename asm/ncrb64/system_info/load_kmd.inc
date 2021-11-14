; UNDER CONSTRUCTION.



LoadKernelModeDriver:
push rbx rsi rdi rbp r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea rbx,[SCP_DATA]
;---------- Initializing data structure ---------------------------------------;
cld
mov rdi,rbx
mov ecx,sizeof.SCPDATA
mov al,0
rep stosb
lea rax,[TEMP_BUFFER]
mov [rbx + SCPDATA.drvPath],rax
mov [rbx + SCPDATA.driver],INVALID_HANDLE_VALUE
;---------- Initializing full path to driver file -----------------------------;
mov rsi,[APP_DATA.lockedStrings]
mov ax,STR_NAME_KMD64
call IndexString
mov rcx,rsi                    ; RCX = Parm#1 = Pointer to file name
mov edx,SCP_PATH_BUFFER_SIZE   ; RDX = Parm#2 = Buffer size 
mov r8,[rbx + SCPDATA.drvPath] ; R8  = Parm#3 = Pointer to buffer
lea r9,[rbx + SCPDATA.drvFile] ; R9  = Parm#4 = Pointer to pointer to file name 
call [GetFullPathNameA]
test rax,rax
jz .errorGetPath               ; Go if error status returned when get path 
;---------- Open Service Control Manager (SCM) --------------------------------;
xor ecx,ecx             ; RCX = Parm#1 = Pointer to machine name, 0 = not used 
xor edx,edx             ; RDX = Parm#2 = Pointer to database name, 0 = not used
mov r8d,SC_MANAGER_ALL_ACCESS  ; R8 = Parm#3 = Desired access rights  
call [OpenSCManagerA]
test rax,rax
jz .errorOpenScm        ; Go if error status returned when open SCM
mov [rbx + SCPDATA.manager],rax 
;---------- Try Open Service, check for already exist -------------------------;
push rax
mov rsi,[APP_DATA.lockedStrings]
mov ax,STR_DRV_NAME
call IndexString
pop rcx                           ; RCX = Parm#1 = Service handle
mov rdx,rsi                       ; RDX = Parm#2 = Pointer to service name 
mov r8d,SERVICE_ALL_ACCESS        ; R8  = Parm#3 = Desired access rights
call [OpenServiceA]
test rax,rax
jnz .skipCreate         ; Go if open success, means this service already exist
;---------- Create Service ----------------------------------------------------;
xor eax,eax             ; RAX = 0 for compact push 0
push rax                ; Alignment
push rax                ; Parm#13 = Pointer to password, 0 = not used
push rax                ; Parm#12 = Pointer to service name, 0 = default
push rax                ; Parm#11 = Pointer to dependencies, 0 = none
push rax                ; Parm#10 = Pointer to Tag ID, 0 = none
push rax                ; Parm#9  = Pointer to Load Order Groups list, 0 = none
push [rbx + SCPDATA.drvPath] ; Parm#8  = Pointer to driver binary file path
push SERVICE_ERROR_NORMAL    ; Parm#7 = Error control option
push SERVICE_DEMAND_START    ; Parm#6 = Service start option
push SERVICE_KERNEL_DRIVER   ; Parm#5 = Service type
mov r9d,SERVICE_ALL_ACCESS   ; Parm#4 = R9  = Desired access
mov r8,rsi                   ; Parm#3 = R8  = Pointer to display name string
mov rdx,r8                   ; Parm#2 = RDX = Pointer to service name string
mov rcx,[rbx + SCPDATA.manager]  ; Parm#1 = RCX = Handle to SCM database
sub rsp,32                   ; Create parameters shadow
call [CreateServiceA]
add rsp,32 + 10 * 8          ; Remove parameters shadow and stack parameters
test rax,rax
jz .errorCreateService  ; Go if error status returned when create service
;---------- Skip create point -------------------------------------------------;
.skipCreate:
mov [rbx + SCPDATA.service],rax 
;---------- Check service status ----------------------------------------------;
xchg rcx,rax            ; RCX = Parm#1 = Handle 
lea rdi,[rbx + SCPDATA.status]
mov rdx,rdi             ; RDX = Parm#2 = Pointer to status structure
call [QueryServiceStatus]
test rax,rax
jz .errorGetStatus      ; Go if error status returned when get status
cmp [rdi + SERVICE_STATUS.dwCurrentState],SERVICE_RUNNING
je .skipStart           ; Go skip start if service already running
;---------- Start service -----------------------------------------------------;
mov rcx,[rbx + SCPDATA.service]  ; RCX = Parm#1 = Handle
xor edx,edx                      ; RDX = Parm#2 = Arguments count
lea r8,[rbx + SCPDATA.vectors]   ; R8  = Parm#3 = Pointer to arguments vectors, empty 
call [StartService]
test rax,rax
jz .errorStartService   ; Go if error status returned when start service
;---------- Skip start point --------------------------------------------------;
.skipStart:
;---------- Create (open) device file -----------------------------------------;
mov rsi,[APP_DATA.lockedStrings]
mov ax,STR_DRV_DEVICE
call IndexString
xor eax,eax                    ; RAX = 0 for compact push 0
push rax                       ; Alignment
push rax                       ; Parm#7 = Template file handle, 0 = not used
push rax                       ; Parm#6 = Flags and attributes, 0 = not used
push OPEN_EXISTING             ; Parm#5 = Creation disposition 
xor r9d,r9d                    ; Parm#4 = R9 = Security attributes  
mov r8d,FILE_SHARE_READ or FILE_SHARE_WRITE  ; Parm#3 = R8  = Share mode 
mov edx,GENERIC_WRITE or GENERIC_READ        ; Parm#2 = RDX = Desired access
mov rcx,rsi                   ; Parm#1 = RCX = Pointer to file=device name
sub rsp,32
call [CreateFile]
add rsp,32 + 4 * 8
test rax,rax
jz .errorCreateDevice   ; Go if error status returned when create device file
cmp rax,INVALID_HANDLE_VALUE
je .errorCreateDevice   ; Go if invalid handle returned when create device file
mov [rbx + SCPDATA.driver],rax

; TODO. Errors Handling.
.errorGetPath:
.errorOpenScm:
.errorCreateService:
.errorGetStatus:
.errorStartService:
.errorCreateDevice:

mov rsp,rbp
pop r15 rbp rdi rsi rbx
ret



; UNDER CONSTRUCTION.



UnloadKernelModeDriver:
push rbx rsi rdi rbp r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea rbx,[SCP_DATA]

; xor r15d,r15d

;---------- Close device file, uninstall driver -------------------------------;
; Note close operations sequence continued even if error, 
; but error logged at R15 if R15 = 0 (if yet no errors)
 
mov rcx,[rbx + SCPDATA.driver]    ; RCX = Parm#1 = Manager handle
jrcxz @f
cmp rcx,INVALID_HANDLE_VALUE
je @f
call [CloseHandle]
test rax,rax
jnz @f                       ; Go skip error log if no errors
; test r15,r15
; jnz @f                     ; Go skip error log if errors from previous steps 
; lea r15,[ErrorCloseDevice]
; call GetOsErrorCode
@@:
;---------- Stop service ------------------------------------------------------;
mov rdi,[rbx + SCPDATA.service]
mov rcx,rdi                     ; RCX = Parm#1 = Handle 
jrcxz .skipStop                 ; Go skip if handle = 0
mov edx,SERVICE_CONTROL_STOP    ; RDX = Parm#2 = Control code
lea r8,[rbx + SCPDATA.status]   ; R8  = Parm#3 = Pointer to status structure
call [ControlService]
test rax,rax
jnz @f                        ; Go skip error log if no errors
; test r15,r15
; jnz @f                      ; Go skip error log if errors from previous steps 
; lea r15,[ErrorStopService]
; call GetOsErrorCode
@@:
;---------- Delete service ----------------------------------------------------;
mov rcx,rdi                   ; RCX = Parm#1 = Service handle
call [DeleteService]
test rax,rax
jnz @f                        ; Go skip error log if no errors
; test r15,r15
; jnz @f                       ; Go skip error log if errors from previous steps 
; lea r15,[ErrorDeleteService]
; call GetOsErrorCode
@@:
;---------- Close service handle ----------------------------------------------;
mov rcx,rdi                  ; RCX = Parm#1 = Service handle
call [CloseServiceHandle]
test rax,rax
jnz @f                       ; Go skip error log if no errors
; test r15,r15
; jnz @f                     ; Go skip error log if errors from previous steps 
; lea r15,[ErrorCloseService]
; call GetOsErrorCode
@@:
;---------- Skip stop point ---------------------------------------------------;
.skipStop:
;---------- Close manager handle ----------------------------------------------;
 
mov rcx,[rbx + SCPDATA.manager]    ; RCX = Parm#1 = Manager handle
jrcxz @f
call [CloseServiceHandle]
test rax,rax
jnz @f                       ; Go skip error log if no errors
; test r15,r15
; jnz @f                     ; Go skip error log if errors from previous steps 
; lea r15,[ErrorCloseManager]
; call GetOsErrorCode
@@:
mov rsp,rbp
pop r15 rbp rdi rsi rbx
ret



; UNDER CONSTRUCTION.



TryKernelModeDriver:
push rbx rsi rdi rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
lea rbx,[SCP_DATA]
;---------- Write to device file, this is requests for driver -----------------;
push 0                         ; Alignment and variable
push 0                         ; Parm#5 = Pointer to overlapped, 0 = not used
lea r9,[rbx + SCPDATA.bytes]   ; Parm#4 = R9  = Pointer to byte count variable
mov r8d,sizeof.SERVICE_QUERY   ; Parm#3 = R8  = Requested byte count
lea rdx,[rbx + SCPDATA.query]  ; Parm#2 = RDX = Pointer to file buffer=request
mov rcx,[rbx + SCPDATA.driver] ; Parm#1 = RCX = File=device handle
mov eax,RZ_REQUEST_CODE
mov qword [rdx + SERVICE_QUERY.iocode],rax    ; Request code
lea rax,[rsp + 8]
mov qword [rdx + SERVICE_QUERY.parm1],rax     ; Parameter = destination pointer
lea rax,[kmdRoutine]
mov qword [rdx + SERVICE_QUERY.userproc],rax  ; Callback pointer
sub rsp,32
call [WriteFile]
add rsp,32 + 8
test rax,rax                   ; RAX = Status
pop rax                        ; RAX = Output variable
jz .errorWriteDevice           ; Go if error status returned when send request

; TODO. Errors Handling.
.errorWriteDevice:

mov rsp,rbp
pop rbp rdi rsi rbx
ret

;------------------------------------------------------------------------------;
;       Fragment executed at kernel mode, as callback from KMD handler.        ;
;               This is debug example: read IA32_APIC_BASE MSR.                ;
;                                                                              ;
; INPUT:     RAX = Pointer to data buffer                                      ;
; OUTPUT:    8 bytes at [RAX] = IA32_APIC_BASE MSR value, 64-bit               ;
; Destroyed: RAX, RBX, RCX, RDX.                                               ; 
;------------------------------------------------------------------------------;
kmdRoutine:
mov rbx,rax          ; RBX = Pointer for store data after read MSR
mov ecx,01Bh         ; ECX = address of IA32_APIC_BASE MSR
rdmsr                ; Read MSR selected by ECX
mov [rbx + 0],eax    ; Store bits [31-00] of MSR
mov [rbx + 4],edx    ; Store bits [63-32] of MSR 
retn


