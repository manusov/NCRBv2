;=========================================================================================================;
;                                                                                                         ;
; Project NCRB ( NUMA CPU&RAM Benchmarks v2.xx.xx ).                                                      ;
; (C)2021 Ilya Manusov.                                                                                   ;
; manusov1969@gmail.com                                                                                   ;
; Previous version v1.xx.xx                                                                               ; 
; https://github.com/manusov/NumaCpuAndRamBenchmarks                                                      ;
; This version v2.xx.xx ( UNDER CONSTRUCTION )                                                            ;
; https://github.com/manusov/Prototyping                                                                  ; 
;                                                                                                         ;
; KMD64.ASM = source file for FASM                                                                        ; 
; KMD64.SYS = translation result, NCRB kernel mode driver for Windows x64                                 ;
; Note. This driver used if run NCRB64.EXE under Windows x64 or                                           ;
; NCRB32.EXE under Windows x64 at WoW64 mode.                                                             ;
; See also other components:                                                                              ;
; NCRB32.ASM, NCRB64.ASM, DATA.ASM, KMD32.ASM.                                                            ;
;                                                                                                         ;
; Translation by Flat Assembler version 1.73.27 ( Jan 27, 2021 ).                                         ;
; http://flatassembler.net/                                                                               ;
;                                                                                                         ;
; Edit by FASM Editor 2.0.                                                                                ; 
; Use this editor for correct source file tabulations and format. (!)                                     ;
; https://fasmworld.ru/instrumenty/fasm-editor-2-0/                                                       ;
;                                                                                                         ;
; User mode debug by OllyDbg ( 32-bit, actual for module NCRB32.EXE )                                     ;
; http://www.ollydbg.de/version2.html                                                                     ;
;                                                                                                         ;
; User mode debug by FDBG ( 64-bit, actual for module NCRB64.EXE )                                        ;
; https://board.flatassembler.net/topic.php?t=9689&postdays=0&postorder=asc&start=180                     ;
; ( Search for archive fdbg0025.zip )                                                                     ;
;                                                                                                         ;
; Intel Software Development Emulator ( SDE ) used for debug ( but not for kernel mode code )             ;
; https://software.intel.com/content/www/us/en/develop/articles/intel-software-development-emulator.html  ;
;                                                                                                         ;
; Icons from open icon library                                                                            ;
; https://sourceforge.net/projects/openiconlibrary/                                                       ;
;                                                                                                         ;
; Special thanks to @L.CHEMIST ( Andrey A. Meshkov )                                                      ;
; for Kernel Mode Driver ( KMD ) and Service Control Program ( SCP ) examples                             ;
; http://maalchemist.narod.ru                                                                             ;
;                                                                                                         ;
;                                                                                                         ;
; IMPORTANT NOTE. NON SIGNED KERNEL MODE DRIVERS LIMITATIONS.                                             ;
; For Windows XP x64 no limitations.                                                                      ;
; For Windows 7 x64 can use "Disable driver signature enforcement".                                       ;
; For Windows 10 x64 required test signing.                                                               ;
; For Windows XP ia32,  Windows 7 ia32,  Windows 10 ia32 no limitations.                                  ;
;                                                                                                         ;
;=========================================================================================================;


;------------------------------------------------------------------------------;
;                                                                              ;
;                        FASM and NCRB definitions.                            ;        
;                                                                              ;
;------------------------------------------------------------------------------;
include 'win64a.inc'   ; FASM definitions
;---------- Global application and version description definitions ------------;
RESOURCE_DESCRIPTION   EQU  'NCRB Kernel Mode Driver for Win64'
RESOURCE_VERSION       EQU  '0.0.0.1'
RESOURCE_COMPANY       EQU  'https://github.com/manusov'
RESOURCE_COPYRIGHT     EQU  '(C) 2021 Ilya Manusov'
;---------- Kernel Mode Driver definitions ------------------------------------;
; Some zero constant values used for XOR/TEST optimizations, 
; carefully inspect code if change.                        ;
RZ_REQUEST_CODE        = 41h         ; Request ID for ring0 callback function
STATUS_SUCCESS         = 0           ; 0 means OK, it used for XOR/TEST
STATUS_UNSUCCESSFUL    = 0C0000001h  ; Driver IO status for operation failed
STATUS_NOT_IMPLEMENTED = 0C0000002h  ; Driver IO status for unknown request
FILE_DEVICE_UNKNOWN    = 0022h       ; Device type for this device
RZ_DRIVER_BUFFER_SIZE  = 1000h       ; Additional buffer size for this device 
IO_NO_INCREMENT        = 0           ; Not change priority, 0 used for XOR/TEST 
;------------------------------------------------------------------------------;
;                                                                              ;
;                                Code section.                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
format PE64 DLL native 5.02 at 10000h
entry DriverLoadEntryPoint
section '.text' code readable executable notpageable
;---------- Driver Load entry point -------------------------------------------;
;                                                                              ;
; Parm#1 = RCX = Pointer to the Driver Object structure                        ;
; Parm#2 = RDX = Pointer to registry key string (UNICODE)                      ;
; Return = RAX = Status by NTDDK.H                                             ;
; Text strings represented as UNICODE_STRING structures, see MSDN.             ;
;                                                                              ;
;------------------------------------------------------------------------------;
align 10h
DriverLoadEntryPoint:
push rsi rdi rbx rbp          ; Save non-volatile but used registers
mov rbp,rsp                   ; RBP used for restore RSP before RET
and rsp,0FFFFFFFFFFFFFFF0h    ; Stack alignment
mov rbx,rsp                   ; RBX = Pointer to aligned stack
sub rsp,32 + 32               ; Create parms. shadow for called + text buffers
mov rdi,rcx                   ; RDI = Pointer to the Driver Object structure 
;---------- Initialize UNICODE string: Device Name ----------------------------;
mov rcx,rsp                   ; Parm#1 = RCX = Destination pointer, [_USDevice]
lea rdx,[DeviceName]          ; Parm#2 = RDX = Source pointer
call [RtlInitUnicodeString]
;---------- Create and initialize Device Object -------------------------------;
mov rcx,rdi                   ; Parm#1 = Pointer to Driver Object
mov edx,RZ_DRIVER_BUFFER_SIZE ; Parm#2 = Device extension size 
mov r8,rsp                    ; Parm#3 = Pointer to struc. with UNICODE string, [_USDevice]
mov r9d,FILE_DEVICE_UNKNOWN   ; Parm#4 = Device type
lea rsi,[DeviceObject]
push rsi                      ; Stack alignment
push rsi                      ; Parm#7 = Pointer to returned Dev.Obj. pointer
xor eax,eax
push rax                      ; Parm#6 = Exclusive access flag
push rax                      ; Parm#5 = Device Characteristics
sub rsp,32                    ; Create parameters shadow
call [IoCreateDevice]
add rsp,32 + 24 + 8           ; Remove parameters shadow, parameters, alignment
test eax,eax                  ; Check status, use fact STATUS_SUCCESS = 0
jnz .return                   ; Go return if error
;---------- Assign device extension variable ----------------------------------;
mov rax,[rsi + 40h]           ; V_DEVICE_OBJECT.DeviceExtension offset 40h
mov [DeviceExtension],rax
;---------- Initialize UNICODE string: Symbolic Link --------------------------;
lea rcx,[rbx - 32 - 16]       ; Parm#1 = RCX = Destination pointer, [_USSymLink]
lea rdx,[SymbolicLinkName]    ; Parm#2 = RDX = Source pointer
call [RtlInitUnicodeString]
;---------- Create symbolic link to the user-visible name (associate it) ------;
lea rcx,[rbx - 32 - 16]       ; Parm#1 = RCX = Pointer to symbolic link name, [_USSymLink] 
mov rdx,rsp                   ; Parm#2 = RDX = Pointer to device name, [_USDevice]
call [IoCreateSymbolicLink]
mov esi,eax                   ; ESI = Save status
test eax,eax                  ; Check status, use fact STATUS_SUCCESS = 0
jz .success                   ; Go continue if no errors
;---------- Delete device object if not successful ----------------------------;
mov rcx,[DeviceObject]        ; Parm#1 = RCX = Pointer to object
call [IoDeleteDevice]
jmp .return
;---------- Branch for continue initialization if success create link ---------;
; Load structure to point to IRP handlers
; IRP = I/O Request Packet
; Create functions pointers list
; RDI = Pointer to driver object
.success:
lea rax,[DriverUnloadEntryPoint]
mov [rdi + 068h],rax          ; V_DRIVER_OBJECT.DriverUnload = 068h
lea rax,[DispatchCreateClose]
mov [rdi + 070h],rax          ; V_DRIVER_OBJECT.MajorFunction + IRP_MJ_CREATE_OFFSET = 070h
mov [rdi + 080h],rax          ; V_DRIVER_OBJECT.MajorFunction + IRP_MJ_CLOSE_OFFSET = 080h
lea rax,[DispatchReadWrite]
mov [rdi + 088h],rax          ; V_DRIVER_OBJECT.MajorFunction + IRP_MJ_READ_OFFSET = 088h
mov [rdi + 090h],rax          ; V_DRIVER_OBJECT.MajorFunction + IRP_MJ_WRITE_OFFSET = 090h
;---------- Exit Load Function ------------------------------------------------;
xchg eax,esi                  ; EAX = Restore status value, previously saved
.return:                      ; XCHG used instead MOV for compact code
mov rsp,rbp
pop rbp rbx rdi rsi
ret
;---------- Driver UnLoad entry point -----------------------------------------;
;                                                                              ;
; Parm#1 = RCX = Pointer to the Driver Object structure                        ;
; No Return                                                                    ;
; Text strings represented as UNICODE_STRING structures, see MSDN.             ;
;                                                                              ;
;------------------------------------------------------------------------------;
align 10h
DriverUnloadEntryPoint:
push rdi rbx rbp             ; Save non-volatile but used registers
mov rbp,rsp                  ; RBP used for restore RSP before RET
and rsp,0FFFFFFFFFFFFFFF0h   ; Stack alignment
mov rbx,rsp                  ; RBX = Pointer to aligned stack
sub rsp,32 + 16              ; Create parms. shadow for called + text buffers
mov rdi,rcx                  ; Save first parameter
;---------- Initialize UNICODE string: Device symbol link (for delete) --------;
mov rcx,rsp                  ; Parm#1 = RCX = Pointer to destination string
lea rdx,[SymbolicLinkName]   ; Parm#2 = RDX = Pointer to name
call [RtlInitUnicodeString]
;---------- Delete symbolic link to the user-visible name ---------------------;
mov rcx,rsp                  ; Parm#1 = RCX = Pointer to destination string
call [IoDeleteSymbolicLink]  ; Return EAX = Status (NTSTATUS)
;---------- Delete device object, exit ----------------------------------------;
mov rcx,[rdi + 8]            ; mov rcx,[rcx + V_DRIVER_OBJECT.DeviceObject]  
call [IoDeleteDevice]        ; Parm#1 = RCX = Pointer to Device Object
mov rsp,rbp
pop rbp rbx rdi
ret
;---------- Driver DispatchCreateClose entry point ----------------------------;
;                                                                              ;
; Parm#1 = RCX = Pointer to the Driver Object structure                        ;
; Parm#2 = RDX = Pointer to IRP (I/O Request Packet)                           ;
; Return = RAX = Status (NTSTATUS.H)                                           ;
;                                                                              ;
;------------------------------------------------------------------------------;
align 10h
DispatchCreateClose:
push rbp                    ; Save non-volatile but used registers
mov rbp,rsp                 ; RBP used for restore RSP before RET
and rsp,0FFFFFFFFFFFFFFF0h  ; Stack alignment
sub rsp,32                  ; Create parameters shadow for called
xor eax,eax
mov [rdx + 030h],eax        ; V_IRP.IoStatus.Status = 030h , STATUS_SUCCESS = 0
mov [rdx + 038h],rax        ; V_IRP.IoStatus.Information = 038h
;---------- Indicate completion, exit -----------------------------------------;
mov rcx,rdx                 ; Parm#1 = RCX = Pointer IRP = Input RDX
xor edx,edx                 ; Parm#2 = Priority Boost = IO_NO_INCREMENT = 0
call [IoCompleteRequest]
xor eax,eax                 ; Use fact STATUS_SUCCESS = 0
mov rsp,rbp
pop rbp
ret
;---------- Driver DispatchReadWrite entry point ------------------------------;
;                                                                              ;
; Parm#1 = RCX = Pointer to the Driver Object structure                        ;
; Parm#2 = RDX = Pointer to IRP (I/O Request Packet)                           ;
; Return = Status (NTSTATUS.H)                                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
align 10h
DispatchReadWrite:
push rbx rsi rdi rbp        ; Save non-volatile but used registers
mov rbp,rsp                 ; RBP used for restore RSP before RET
and rsp,0FFFFFFFFFFFFFFF0h  ; Stack alignment
sub rsp,32                  ; Create parameters shadow for called
mov ebx,STATUS_UNSUCCESSFUL ; EBX = current status, STATUS_UNSUCCESSFUL = 0C0000001h
mov rdi,rdx                 ; RDI = Pointer to IRP
mov qword [rdi + 0038h],0   ; V_IRP.IoStatus.Information = 038h
mov rsi,[rdi + 0B8h]        ; V_IRP.Tail.Overlay.CurrentStackLocation = 0B8h , RSI=PIO_STACK_LOCATION
mov ecx,[rsi + 008h]        ; V_IO_STACK_LOCATION.Parameters.Write.Length = 008h
cmp ecx,RZ_DRIVER_BUFFER_SIZE ; RZ_DRIVER_BUFFER_SIZE = 1000h
ja .done
mov rsi,[rdi + 070h]        ; RSI = Pointer to DriverQuery, V_IRP.UserBuffer = 070h
mov qword [rsi + 020h],0    ; V_RZDriverQuery.RESULT = 020h
mov ecx,[rsi]               ; ECX = User I/O code, V_RZDriverQuery.IOCODE = 000h
;---------- Target operation --------------------------------------------------;
mov ebx,STATUS_NOT_IMPLEMENTED  ; STATUS_NOT_IMPLEMENTED = 0C0000002h
cmp ecx,RZ_REQUEST_CODE
jne .done
mov rax,[rsi + 16]          ; RAX = user parameter
call qword [rsi + 8]        ; call user subroutine
mov ebx,STATUS_SUCCESS
;---------- End of target operation -------------------------------------------;
.done:
mov [rdi + 030h],ebx        ; V_IRP.IoStatus.Status = 030h
;---------- Indicate completion, exit -----------------------------------------;
mov rcx,rdi                 ; Parm#1 = RCX = Pointer IRP = Input RDX
xor edx,edx                 ; Parm#2 = Priority Boost = IO_NO_INCREMENT = 0
call [IoCompleteRequest]
xchg eax,ebx
mov rsp,rbp
pop rbp rdi rsi rbx
ret
;------------------------------------------------------------------------------;
;                                                                              ;
;                              Data section.                                   ;
;                                                                              ;
;------------------------------------------------------------------------------;
section '.data' data readable writeable notpageable
;---------- Device and symbolic link names ------------------------------------; 
; Unified for availability 64-bit KMD under WoW mode ( Win32-on-Window64 ).
align 10h
DeviceName        DU '\Device\ICR0'     , 0
align 10h
SymbolicLinkName  DU '\DosDevices\ICR0' , 0
;---------- Pointers to device data -------------------------------------------;
align 10h
DeviceObject      DQ  0   ; Pointer to device object
DeviceExtension   DQ  0   ; Pointer to device extension ( driver-specific )
;------------------------------------------------------------------------------;
;                                                                              ;
;                              Import section.                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
section 'INIT' data import readable notpageable
library ntoskrnl, 'ntoskrnl.exe'
import  ntoskrnl,\
IoCreateDevice        ,  'IoCreateDevice'        , \
RtlInitUnicodeString  ,  'RtlInitUnicodeString'  , \
IoCreateSymbolicLink  ,  'IoCreateSymbolicLink'  , \
IoDeleteDevice        ,  'IoDeleteDevice'        , \
IoDeleteSymbolicLink  ,  'IoDeleteSymbolicLink'  , \
IoCompleteRequest     ,  'IoCompleteRequest'
;------------------------------------------------------------------------------;
;                                                                              ;
;                             Resources section.                               ;
;                                                                              ;
;------------------------------------------------------------------------------;
section '.rsrc' resource data readable
directory    RT_VERSION , r_version_info
resource     r_version_info, 1, LANG_NEUTRAL, version_info
versioninfo  version_info, \ 
             VOS__WINDOWS32, VFT_DRV, VFT2_UNKNOWN, LANG_NEUTRAL, 0, \
'FileDescription' , RESOURCE_DESCRIPTION ,\
'FileVersion'     , RESOURCE_VERSION     ,\
'CompanyName'     , RESOURCE_COMPANY     ,\
'LegalCopyright'  , RESOURCE_COPYRIGHT
;------------------------------------------------------------------------------;
;                                                                              ;
;                           Relocations section.                               ;
;                                                                              ;
;------------------------------------------------------------------------------;
section '.reloc' fixups data readable discardable
