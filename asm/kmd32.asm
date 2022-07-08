;=========================================================================================================;
;                                                                                                         ;
; Project NCRB ( NUMA CPU&RAM Benchmarks v2.xx.xx ).                                                      ;
; (C)2022 Ilya Manusov.                                                                                   ;
; manusov1969@gmail.com                                                                                   ;
;                                                                                                         ;
; This version v2.xx.xx                                                                                   ;
; https://github.com/manusov/NCRBv2                                                                       ;
; Previous version v1.xx.xx                                                                               ;
; https://github.com/manusov/NumaCpuAndRamBenchmarks                                                      ;
; Prototyping                                                                                             ;
; https://github.com/manusov/Prototyping                                                                  ;
;                                                                                                         ;
; KMD32.ASM = source file for FASM                                                                        ; 
; KMD32.SYS = translation result, NCRB kernel mode driver for Windows ia32                                ;
; Note. This driver used if run NCRB32.EXE under Windows ia32,                                            ;
; but at WoW64 mode for NCRB32.EXE under Windows x64 used KMD64.SYS.                                      ;
; See also other components:                                                                              ;
; NCRB32.ASM, NCRB64.ASM, DATA.ASM, KMD64.ASM.                                                            ;
;                                                                                                         ;
; Translation by Flat Assembler version 1.73.30 (Feb 21, 2022).                                           ;
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
; User mode debug by x64dbg ( 32 and 64-bit, actual for modules NCRB32.EXE, NCRB64.EXE )                  ;
; https://x64dbg.com/                                                                                     ;
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
include 'win32a.inc'   ; FASM definitions
;---------- Global application and version description definitions ------------;
RESOURCE_DESCRIPTION   EQU  'NCRB Kernel Mode Driver for Win32'
RESOURCE_VERSION       EQU  '0.0.0.1'
RESOURCE_COMPANY       EQU  'https://github.com/manusov'
RESOURCE_COPYRIGHT     EQU  '(C) 2022 Ilya Manusov'
;---------- Kernel Mode Driver definitions ------------------------------------;
; Some zero constant values used for XOR/TEST optimizations, 
; carefully inspect code if change.                        ;
RZ_REQUEST_CODE        = 41h         ; Request ID for ring0 callback function 
STATUS_SUCCESS         = 0           ; 0 means OK, it used for XOR/TEST optim. 
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
format PE DLL native 4.0 at 10000h
entry DriverLoadEntryPoint
section '.text' code readable executable notpageable
;---------- Driver Load entry point -------------------------------------------;
;                                                                              ;
; Parm#1 = dword [ebp+08h] = Pointer to the Driver Object structure            ;
; Parm#2 = dword [ebp+0Ch] = Pointer to registry key string (UNICODE)          ;
; Return = EAX = Status by NTDDK.H                                             ;
; Text strings represented as UNICODE_STRING structures, see MSDN.             ;
;                                                                              ;
;------------------------------------------------------------------------------;
align 10h
DriverLoadEntryPoint:
push ebp                      ; because 4(call push eip) + 4(push ebp) = 8,
mov ebp,esp                   ; first parm. means [ebp + 08h]
sub esp,32                    ; Reserve space
push ebx esi edi              ; Save non-volatile but used registers
;---------- Initialize UNICODE string: Device Name ----------------------------;
push usDevice                 ; Parm#2 = Destination pointer
lea eax,[esp + 12 + 04 + 00]
push eax                      ; Parm#1 = Source pointer , push _USD
call [RtlInitUnicodeString]
;---------- Create and initialize device object -------------------------------;
push DeviceObject             ; Parm#7 = Pointer to returned Dev.Obj. pointer 
push FALSE                    ; Parm#6 = Exclusive access flag
push 0                        ; Parm#5 = Device Characteristics
push FILE_DEVICE_UNKNOWN      ; Parm#4 = Device type
lea eax,[esp + 12 + 16 + 00]  ; Required push _USD
push eax                      ; Parm#3 = Pointer to struc. with UNICODE string
push RZ_DRIVER_BUFFER_SIZE    ; Parm#2 = Device extension size
push dword [ebp + 8]          ; Parm#1 = Pointer to Drv. Obj., [lpDriverObject]
call [IoCreateDevice]
test eax,eax                  ; cmp eax,STATUS_SUCCESS
jne .exit                     ; Go exit if error
;---------- Assign device extension variable ----------------------------------;
mov eax,[DeviceObject]
mov eax,[eax + 28h]           ; V_DEVICE_OBJECT.DeviceExtension
mov [DeviceExtension],eax
push usSymbolicLink 
lea eax,[esp + 12 + 04 + 16]
push eax                      ; push _USSL
call [RtlInitUnicodeString]  
;---------- Create symbolic link to the user-visible name ---------------------;
lea eax,[esp + 12 + 00 + 00]
push eax                     ; Parm#2 = Pointer to device name, push _USD 
lea eax,[esp + 12 + 04 + 16] ; Required push _USSL 
push eax                     ; Parm#1 = Pointer to symbolic link name
call [IoCreateSymbolicLink]
;---------- Save status and check result --------------------------------------; 
mov ebx,eax
test eax,eax                 ; cmp eax,STATUS_SUCCESS
je .success                  ; Go continue if no errors
;---------- Delete device object if not successful ----------------------------;
push [DeviceObject] 
call [IoDeleteDevice]
jmp .exit
;---------- Branch for continue initialization if success create link ---------;
; Load structure to point to IRP handlers
; IRP = I/O Request Packet
; Create functions pointers list
; Load structure to point to IRP handlers
.success:
mov eax,[ebp + 8]            ; [lpDriverObject]
lea edx,[DriverUnloadEntryPoint]
mov [eax + 34h],edx          ; V_DRIVER_OBJECT.DriverUnload
lea edx,[DispatchCreateClose]
mov [eax + 38h],edx          ; V_DRIVER_OBJECT.MajorFunction + IRP_MJ_CREATE_OFFSET
mov [eax + 40h],edx          ; V_DRIVER_OBJECT.MajorFunction + IRP_MJ_CLOSE_OFFSET
lea edx,[DispatchReadWrite]
mov [eax + 44h],edx          ; V_DRIVER_OBJECT.MajorFunction + IRP_MJ_READ_OFFSET
mov [eax + 48h],edx          ; V_DRIVER_OBJECT.MajorFunction + IRP_MJ_WRITE_OFFSET
;---------- Assign result, exit -----------------------------------------------;
xchg eax,ebx                 ; XCHG (not MOV) because compact code
.exit:
pop edi esi ebx
leave
ret 8
;---------- Driver UnLoad entry point -----------------------------------------;
;                                                                              ;
; Parm#1 = dword [ebp+08h] = Pointer to the Driver Object structure            ;
; No Return                                                                    ;
; Text strings represented as UNICODE_STRING structures, see MSDN.             ;              
;                                                                              ;
;------------------------------------------------------------------------------;
align 10h
DriverUnloadEntryPoint:
push ebp                      ; because 4(call push eip) + 4(push ebp) = 8,
mov ebp,esp                   ; first parm. means [ebp+8]
sub esp,16
push usSymbolicLink 
lea eax,[esp + 04]
push eax                      ; push _USSL  
call [RtlInitUnicodeString]
;---------- Delete symbolic link to the user-visible name ---------------------;
mov eax,esp
push eax                      ; push _USSL 
call [IoDeleteSymbolicLink]
;---------- Delete device object, exit ----------------------------------------;
mov eax,[ebp + 8]            ; [lpDriverObject]
push dword [eax + 4]         ; V_DRIVER_OBJECT.DeviceObject
call [IoDeleteDevice]
leave
ret 4
;---------- Driver DispatchCreateClose entry point ----------------------------;
;                                                                              ;
; Parm#1 = dword [ebp+08h] = Pointer to the Driver Object structure            ;
; Parm#2 = dword [ebp+0Ch] = Pointer to IRP (I/O Request Packet)               ;
; Return = EAX = Status (NTSTATUS.H)                                           ;
;                                                                              ;
;------------------------------------------------------------------------------;
align 10h
DispatchCreateClose:
push ebp                     ; because 4(call push eip) + 4(push ebp) = 8,
mov ebp,esp                  ; first parm. means [ebp+8]
mov eax,[ebp + 0Ch]          ; [lpIrp]
mov dword [eax + 18h],STATUS_SUCCESS   ; V_IRP.IoStatus.Status
mov dword [eax + 1Ch],0                ; V_IRP.IoStatus.Information
push IO_NO_INCREMENT
push eax                     ; dword [ebp+0Ch]  ; [lpIrp] 
call [IoCompleteRequest]
xor eax,eax                  ; mov eax,STATUS_SUCCESS
leave
ret 8
;---------- Driver DispatchReadWrite entry point ------------------------------;
;                                                                              ;
; Parm#1 = dword [ebp+08h] = Pointer to the Driver Object structure            ;
; Parm#2 = dword [ebp+0Ch] = Pointer to IRP (I/O Request Packet)               ;
; Return = EAX = Status (NTSTATUS.H)                                           ;
;                                                                              ;
;------------------------------------------------------------------------------;
align 10h
DispatchReadWrite:
push ebp                    ; because 4(call push eip) + 4(push ebp) = 8,
mov ebp,esp                 ; first parm. means [ebp+8]
push ebx esi edi
mov ebx,STATUS_UNSUCCESSFUL  ; EBX = current status
mov edi,[ebp + 0Ch]          ; [lpIrp]  ; EDI = PIRP
mov dword [edi + 1Ch],0      ; V_IRP.IoStatus.Information
mov ebx,STATUS_NOT_IMPLEMENTED
mov esi,[edi + 3Ch]          ; ESI = PR0DriverQuery, V_IRP.UserBuffer
mov dword [esi + 20h], 0     ; V_RZDriverQuery.RESULT
mov ecx,dword [esi + 0]      ; ECX = user I/O code, V_RZDriverQuery.IOCODE 
;---------- Detect and execute target operation -------------------------------;
mov ebx,STATUS_NOT_IMPLEMENTED
cmp ecx,RZ_REQUEST_CODE
jne @f
mov eax,[esi + 16]
call dword [esi + 8]
xor ebx,ebx                  ; mov ebx,STATUS_SUCCESS
@@:
;---------- Target operation done, exit ---------------------------------------;
mov [edi + 18h],ebx          ; V_IRP.IoStatus.Status
push IO_NO_INCREMENT 
push edi
call [IoCompleteRequest]
xchg eax,ebx                 ; XCHG (not MOV) because compact code 
pop edi esi ebx
leave
ret 8
;------------------------------------------------------------------------------;
;                                                                              ;
;                              Data section.                                   ;
;                                                                              ;
;------------------------------------------------------------------------------;
section '.data' data readable writeable notpageable
;---------- Device and symbolic link names ------------------------------------;
; Unified for support KMD32 + SCP64 ( WoW 32/64 ).
align 10h
usDevice         DU '\Device\ICR0'     , 0
align 10h
usSymbolicLink   DU '\DosDevices\ICR0' , 0
;---------- Pointers to device data -------------------------------------------;
align 10h
DeviceObject     DD 0   ; Pointer to device object
DeviceExtension  DD 0   ; Pointer to device extension (driver-specific)
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
directory    RT_VERSION, r_version_info
resource     r_version_info, 1, LANG_NEUTRAL, version_info
versioninfo  version_info, \
             VOS__WINDOWS32, VFT_DRV, VFT2_UNKNOWN, LANG_NEUTRAL, 0, \
'FileDescription' , RESOURCE_DESCRIPTION ,\
'FileVersion'     , RESOURCE_VERSION     ,\
'CompanyName'     , RESOURCE_COMPANY     ,\
'LegalCopyright'  , RESOURCE_COPYRIGHT
;------------------------------------------------------------------------------;
;                                                                              ;
;                            Relocations section.                              ;
;                                                                              ;
;------------------------------------------------------------------------------;
section '.reloc' fixups data readable discardable


