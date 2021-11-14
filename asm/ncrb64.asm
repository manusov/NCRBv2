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
; NCRB64.ASM = source file for FASM                                                                       ; 
; NCRB64.EXE = translation result, application NCRB64.EXE main module                                     ;
; See also other components:                                                                              ;
; NCRB32.ASM, DATA.ASM, KMD32.ASM, KMD64.ASM.                                                             ;
;                                                                                                         ;
; Translation by Flat Assembler version 1.73.27 ( Jan 27, 2021 ).                                         ;
; http://flatassembler.net/                                                                               ;
;                                                                                                         ;
; Edit by FASM Editor 2.0.                                                                                ; 
; Use this editor for correct source file tabulations and format. (!)                                     ;
; https://fasmworld.ru/instrumenty/fasm-editor-2-0/                                                       ;
;                                                                                                         ;
; User mode debug by OllyDbg ( 32-bit, actual for other module NCRB32.EXE )                               ;
; http://www.ollydbg.de/version2.html                                                                     ;
;                                                                                                         ;
; User mode debug by FDBG ( 64-bit, actual for this module NCRB64.EXE )                                   ;
; https://board.flatassembler.net/topic.php?t=9689&postdays=0&postorder=asc&start=180                     ;
; ( Search for archive fdbg0025.zip )                                                                     ;
;                                                                                                         ;
; Intel Software Development Emulator ( SDE ) used for debug                                              ;
; https://software.intel.com/content/www/us/en/develop/articles/intel-software-development-emulator.html  ;
;                                                                                                         ;
; Icons from open icon library                                                                            ;
; https://sourceforge.net/projects/openiconlibrary/                                                       ;
;                                                                                                         ;
;=========================================================================================================;


;------------------------------------------------------------------------------;
;                                                                              ;
;                        FASM and NCRB definitions.                            ;        
;                                                                              ;
;------------------------------------------------------------------------------;
include 'win64a.inc'               ; FASM definitions
include 'data\data.inc'            ; NCRB project global definitions
;---------- Global application and version description definitions ------------;
RESOURCE_DESCRIPTION    EQU 'NCRB Win64 edition ( UNDER CONSTRUCTION )'
RESOURCE_VERSION        EQU '2.0.7.0'
RESOURCE_COMPANY        EQU 'https://github.com/manusov'
RESOURCE_COPYRIGHT      EQU '(C) 2021 Ilya Manusov'
PROGRAM_NAME_TEXT       EQU 'NUMA CPU&RAM Benchmarks for Win64 ( UNDER CONSTRUCTION )'
ABOUT_CAP_TEXT          EQU 'Program info'
ABOUT_TEXT_1            EQU 'NUMA CPU&RAM Benchmarks'
ABOUT_TEXT_2            EQU 'v2.00.07 for Windows x64 ( UNDER CONSTRUCTION )'
ABOUT_TEXT_3            EQU RESOURCE_COPYRIGHT 
;---------- Global identifiers definitions ------------------------------------;
ID_EXE_ICON             = 100      ; This application icon
ID_EXE_ICONS            = 101      ; This application icon group
MSG_MEMORY_ALLOC_ERROR  = 0        ; Error messages IDs, from this file
MSG_INIT_FAILED         = 1        ; Note. Resource DLL cannot be used for 
MSG_LOAD_FAILED         = 2        ; this messages:
MSG_HANDLE_NULL         = 3        ; it must be valid before DLL loaded 
MSG_ICON_FAILED         = 4 
MSG_ICONS_POOL_FAILED   = 5
MSG_RAW_RESOURCE_FAILED = 6  
MSG_CREATE_FONT_FAILED  = 7
MSG_DIALOGUE_FAILED     = 8
ALLOCATE_MEMORY_SIZE    = 1024 * 1024
;---------- Application start errors identifiers ------------------------------;
MSG_ERROR_CPUID         = 0 
MSG_ERROR_CPUID_F1      = 1     
MSG_ERROR_X87           = 2 
MSG_ERROR_TSC           = 3 
MSG_ERROR_TSC_FREQ      = 4 
MSG_ERROR_MEMORY_API    = 5 
MSG_ERROR_TOPOLOGY_API  = 6 
;---------- Application runtime errors identifiers ----------------------------;
MSG_RUNTIME_ALLOC       = 0    
MSG_RUNTIME_RELEASE     = 1 
MSG_RUNTIME_TIMINGS     = 2 
MSG_RUNTIME_ADDRESS     = 3 
;------------------------------------------------------------------------------;
;                                                                              ;
;                                Code section.                                 ;        
;                                                                              ;
;------------------------------------------------------------------------------;
format PE64 GUI 5.0
entry start
section '.code' code readable executable
start:
;---------- Application entry point, memory allocation for registry -----------;
sub rsp,8 + 32                     ; Stack alignment + Parameters shadow
cld
mov r9d,PAGE_READWRITE             ; R9  = Parm#4 = memory protection 
mov r8d,MEM_COMMIT + MEM_RESERVE   ; R8  = Parm#3 = allocation type 
mov edx,ALLOCATE_MEMORY_SIZE       ; RDX = Parm#2 = required block size
xor ecx,ecx                        ; RCX = Parm#1 = fixed address, not used = 0
call [VirtualAlloc]
test rax,rax
jz .memoryAllocError               ; Go if memory allocation error
mov [APP_MEMORY],rax
;---------- Start GUI initialization ------------------------------------------;
lea r15,[APP_DATA]
lea rcx,[APP_CTRL]                 ; RCX = Parm#1 = Pointer to structure
call [InitCommonControlsEx]        ; GUI controls initialization
test rax,rax
jz .initFailed                     ; Go if initialization error detected
;---------- Load resources DLL, same data DLL for ia32 and x64 ----------------;
mov r8d,LOAD_LIBRARY_AS_DATAFILE   ; R8  = Parm#3 = Load options, flags
xor edx,edx                        ; RDX = Parm#2 = Handle, reserved = 0 
lea rcx,[NAME_DATA]                ; RCX = Parm#1 = Pointer to file name
call [LoadLibraryEx]               ; Load resources DLL
test rax,rax
jz .loadFailed                     ; Go if load resources DLL error
mov [r15 + APPDATA.hResources],rax ; Store resources DLL handle
;---------- Get handle of this application exe file ---------------------------;
xor ecx,ecx                        ; RCX = Parm#1 = 0 = means this exe file 
call [GetModuleHandle]             ; Get handle of this exe file
test rax,rax
jz .thisFailed                     ; Go if this module handle = NULL 
mov [r15 + APPDATA.hInstance],rax  ; Store handle of current module ( exe file ) 
;---------- Get handle of this application icon -------------------------------;
mov edx,ID_EXE_ICONS               ; RDX = Parm#2 = Resource ID
xchg rcx,rax                       ; RCX = Parm#1 = Module handle for resource 
call [LoadIcon]                    ; Load application icon, from this exe file
test rax,rax
jz .iconFailed                     ; Go if load error, icon handle = NULL
mov [r15 + APPDATA.hIcon],rax      ; Store handle of application icon
;---------- Get handles and address pointers to tabs icons at resources DLL ---;
mov ebx,ICON_FIRST                   ; EBX = Icons identifiers
lea rdi,[r15 + APPDATA.lockedIcons]  ; RDI = Pointer to icons pointers list
mov esi,ICON_COUNT                   ; ESI = Number of loaded icons
;---------- Cycle for load icons from resource DLL ----------------------------;
.loadIcons:
mov r8d,RT_GROUP_ICON
mov edx,ebx
mov rcx,[r15 + APPDATA.hResources]
call [FindResource]
test rax,rax                       ; RAX = HRSRC, handle of resource block
jz .iconsPoolFailed                ; Go if handle = NULL, means error
xchg rdx,rax
mov rcx,[r15 + APPDATA.hResources]
call [LoadResource] 
test rax,rax                       ; RAX = HRSRC, handle of resource block
jz .iconsPoolFailed                ; Go if handle = NULL, means error
xchg rcx,rax
call [LockResource] 
test rax,rax                       ; RAX = HRSRC, handle of resource block
jz .iconsPoolFailed                ; Go if pointer = NULL, means error
stosq                              ; Store pointer to icon                 
inc ebx                            ; EBX = Next icon
dec esi                            ; ESI = Cycle counter  
jnz .loadIcons                     ; Cycle for initializing all icons 
;---------- Get handle and address pointer to raw pools at resources DLL ------;
; Strings located at raw resources part, for compact encoding 1 byte per char,
; note standard string resource use 2 byte per char (UNICODE). 
; Binders located at raw resources part,
; note binders script used for interconnect GUI and System Information routines. 
lea rsi,[RAW_LIST]
lea rdi,[r15 + APPDATA.lockedStrings]
.loadRaw:
mov r8d,RT_RCDATA                  ; R8  = Parm#3 = Resource type
lodsw
movzx edx,ax                       ; RDX = Parm#2 = Res. name, used numeric ID
test edx,edx
jz .endRaw
mov rcx,[r15 + APPDATA.hResources] ; RCX = Parm#1 = Module handle, load from DLL
call [FindResource]                ; Find resource, get handle of block                
test rax,rax                       ; RAX = HRSRC, handle of resource block
jz .rawResourceFailed              ; Go if handle = NULL, means error
xchg rdx,rax                       ; RDX = Parm#2 = Handle of resource block
mov rcx,[r15 + APPDATA.hResources] ; RCX = Parm#1 = Module handle, load from DLL
call [LoadResource]                ; Load resource, get resource handle 
test rax,rax                       ; RAX = HRSRC, handle of resource block
jz .rawResourceFailed              ; Go if handle = NULL, means error
xchg rcx,rax                       ; RCX = Parm#1 = Resource handle
call [LockResource]                ; Lock resource, get address pointer  
test rax,rax                       ; RAX = HRSRC, handle of resource block
jz .rawResourceFailed              ; Go if handle = NULL, means error
stosq                              ; Store pointer to strings pool
jmp .loadRaw
.endRaw:
;---------- Create fonts ------------------------------------------------------;
mov rsi,[r15 + APPDATA.lockedFontList]
lea rdi,[r15 + APPDATA.hFont1]
.createFonts:
xor eax,eax
movzx ecx,word [rsi + 00]
jrcxz .doneFonts
lea rdx,word [rsi + 16]
push rdx
movzx edx,word [rsi + 14]
push rdx
movzx edx,word [rsi + 12]
push rdx
movzx edx,word [rsi + 10]
push rdx
movzx edx,word [rsi + 08]
push rdx
movzx edx,word [rsi + 06]
push rdx
push rax
push rax
push rax
movzx edx,word [rsi + 04]
push rdx
xor r9d,r9d
xor r8d,r8d
xor edx,edx
sub rsp,32
call [CreateFont]
add rsp,32+80
test rax,rax
jz .createFontFailed
stosq
add rsi,16
@@:
lodsb
cmp al,0
jne @b
jmp .createFonts
.doneFonts:
;---------- Pre-load strings for fast build clks/mbps table texts -------------;
lea rdi,[CLKS_MBPS_TEXTS]
mov bp,DRAW_TABLE_COUNT
mov dx,DRAW_TABLE_FIRST_TEXT
@@:
mov eax,edx
mov rsi,[APP_DATA.lockedStrings]
call IndexString
xchg rax,rsi
stosq
inc edx
dec bp
jnz @b
;--- Pre-load strings for fast build Bytes/KB/MB/GB/TB/MBPS/nanoseconds -------;
lea rdi,[UNITS_TEXTS]
mov bp,UNITS_COUNT
mov dx,UNITS_FIRST_TEXT 
@@:
mov eax,edx
mov rsi,[APP_DATA.lockedStrings]
call IndexString
xchg rax,rsi
stosq
inc edx
dec bp
jnz @b
;---------- Pre-load string for write TSC frequency at drawings window --------;
lea rdi,[DRAW_TSC]
mov ax,STR_MD_TSC_CLOCK_MHZ
call PoolStringWrite
;---------- Load configuration file ncrb.inf ----------------------------------; 
; TODO.
;---------- Get system information, user mode routines ------------------------;
call SystemInfo
jc .errorPlatform
;---------- Load kernel mode driver kmd64.sys ---------------------------------;
; TODO.
; call LoadKernelModeDriver
; call TryKernelModeDriver
; call UnloadKernelModeDriver
;---------- Check dynamical import results, show missing WinAPI warning -------;
; Application can start with this non-fatal warning.
lea rdi,[TEMP_BUFFER]
push rdi
mov rdx,rsi
mov ax,STR_WARNING_API
call PoolStringWrite 
mov rsi,[r15 + APPDATA.lockedImportList]
lea rdx,[DYNA_IMPORT]
xor ebp,ebp
.checkImport:
cmp byte [rsi],0
je .doneCheckImport
cmp qword [rdx],0
jne .skipImport
call StringWrite
mov ax,0A0Dh
stosw
inc ebp
jmp .skippedImport
.skipImport:
lodsb
cmp al,0
jne .skipImport
.skippedImport:
cmp byte [rsi],0
jne .nextImport
inc rsi
.nextImport:
add rdx,8
jmp .checkImport
.doneCheckImport:
mov al,0
stosb
pop rdi
test ebp,ebp
jz .doneImport 
mov r9d,MB_ICONWARNING   ; R9  = Parm#4 = Message box icon type
lea r8,[PROGRAM_NAME]    ; R8  = Parm#3 = Pointer to caption
mov rdx,rdi              ; RDX = Parm#2 = Pointer to string
xor ecx,ecx              ; RCX = Parm#1 = Parent window handle or 0
call [MessageBoxA]
.doneImport:
;---------- Create and show main dialogue window ------------------------------; 
push 0 0                       ; Parm#5 = Pass value, plus alignment qword 
lea r9,[DialogProcMain]        ; R9  = Parm#4 = Pointer to dialogue proced.
mov r8d,HWND_DESKTOP           ; R8  = Parm#3 = Owner window handle
mov edx,IDD_MAIN               ; RDX = Parm#2 = Resource ( template ) id
mov rcx,[r15 + APPDATA.hResources]  ; RCX = Parm#1 = Handle of resource module  
sub rsp,32                     ; Allocate parameters shadow
call [DialogBoxParam]          ; Create modal dialogue 
add rsp,32 + 16                ; Remove parameters shadow and 2 parameters
test rax,rax
jz .dialogueFailed             ; Go if create dialogue return error 
cmp rax,-1
je .dialogueFailed             ; Go if create dialogue return error
;---------- Application exit point with release resource ----------------------; 
xor r13d,r13d                      ; R13 = Exit Code, 0 means no errors
.exitResources:
mov r14,[APP_MEMORY]
test r14,r14
jz .exit
lea r15,[APP_DATA]
;---------- Delete created fonts ----------------------------------------------;
mov rsi,[r15 + APPDATA.lockedFontList]
test rsi,rsi
jz .doneDeleteFonts 
lea rdi,[r15 + APPDATA.hFont1]
.deleteFonts:
lodsw
test ax,ax
jz .doneDeleteFonts
mov rcx,[rdi]
add rdi,8
jrcxz .skipDelete
call [DeleteObject]
.skipDelete:
add rsi,14
@@:
lodsb
cmp al,0
jnz @b 
jmp .deleteFonts
.doneDeleteFonts:
;---------- Unload resource library -------------------------------------------; 
mov rcx,[r15 + APPDATA.hResources]  ; RCX = Library DATA.DLL handle
jrcxz .skipUnload                   ; Go skip unload if handle = null
call [FreeLibrary]                  ; Unload DATA.DLL
.skipUnload:
;---------- Release memory ----------------------------------------------------; 
mov r8d,MEM_RELEASE                ; R8  = Parm#3 = Memory free operation type
xor edx,edx                        ; RDX = Parm#2 = Size, 0 = by allocated
mov rcx,r14                        ; RCX = Parm#1 = Memory block base address  
call [VirtualFree]                 ; Release memory, allocated for registry
;---------- Exit --------------------------------------------------------------;
.exit:
mov ecx,r13d                       ; RCX = Parm#1 = exit code           
call [ExitProcess]
;---------- This entry points used if application start failed ----------------; 
.dialogueFailed:
mov al,MSG_DIALOGUE_FAILED     ; AL = String pool index for error name
jmp .errorProgram
.createFontFailed:
mov al,MSG_CREATE_FONT_FAILED
jmp .errorProgram
.rawResourceFailed:
mov al,MSG_RAW_RESOURCE_FAILED
jmp .errorProgram
.iconsPoolFailed:
mov al,MSG_ICONS_POOL_FAILED
jmp .errorProgram
.iconFailed:
mov al,MSG_ICON_FAILED
jmp .errorProgram
.thisFailed:
mov al,MSG_HANDLE_NULL
jmp .errorProgram
.loadFailed:
mov al,MSG_LOAD_FAILED
jmp .errorProgram 
.initFailed:
mov al,MSG_INIT_FAILED
jmp .errorProgram
.memoryAllocError:
mov al,MSG_MEMORY_ALLOC_ERROR
;---------- Show message box and go epplication termination -------------------;
; This procedure for application error, use message strings from exe file,
; can execute if resource DLL not loaded or load failes.
.errorProgram:
lea rsi,[MSG_ERRORS]   ; RSI = Strings pool base, AL = String index 
mov ah,0
.errorEntry:
call IndexString       ; Return ESI = Selected string address 
mov r9d,MB_ICONERROR   ; R9  = Parm#4 = Attributes
lea r8,[PROGRAM_NAME]  ; R8  = Parm#3 = Pointer to title (caption) string
mov rdx,rsi            ; RDX = Parm#2 = Pointer to string: error name 
xor ecx,ecx            ; RCX = Parm#1 = Parent Window = NULL
call [MessageBox]  
mov r13d,1
jmp .exitResources
;---------- Show message box and go epplication termination -------------------;
; This procedure for incompatible platform detected but application integrity
; OK, use strings from resource DLL.
.errorPlatform:       ; Input AX = Error string ID.
mov rsi,[APP_DATA.lockedStrings]
movzx eax,al
add eax,STR_ERROR_CPUID
jmp .errorEntry 
;---------- Copy text string terminated by 00h --------------------------------;
; Note last byte 00h not copied.                                               ;
;                                                                              ;
; INPUT:   RSI = Source address                                                ;
;          RDI = Destination address                                           ;
;                                                                              ;
; OUTPUT:  RSI = Modified by copy                                              ;
;          RDI = Modified by copy                                              ;
;          Memory at [Input RDI] modified                                      ;
;                                                                              ;
;------------------------------------------------------------------------------;
StringWrite:
cld
.cycle:
lodsb
cmp al,0
je .exit
stosb
jmp .cycle
.exit:
ret
;---------- Find string in the pool by index ----------------------------------;
;                                                                              ;
; INPUT:   RSI = Pointer to string pool                                        ;
;          AX  = String index in the strings pool                              ;
;                                                                              ;
; OUTPUT:  RSI = Updated pointer to string, selected by index                  ;  
;                                                                              ;
;------------------------------------------------------------------------------;
IndexString:
cld
movzx rcx,ax
jrcxz .stop
.cycle:
lodsb
cmp al,0
jne .cycle
loop .cycle
.stop:
ret
;---------- Find string in the pool by index and write this string ------------;
;                                                                              ;
; INPUT:   AX  = String index in the application resources strings pool        ;
;                                                                              ;
; OUTPUT:  RSI = Updated pointer to string, selected by index                  ;
;          RDI = Modified by copy                                              ;  
;                                                                              ;
;------------------------------------------------------------------------------;
PoolStringWrite:
mov rsi,[APP_DATA.lockedStrings]
test rsi,rsi
jz .exit
call IndexString
call StringWrite
.exit:
ret
;---------- Print 64-bit Hex Number -------------------------------------------;
;                                                                              ;
; INPUT:  RAX = Number                                                         ;
;         RDI = Destination Pointer                                            ;
;                                                                              ;
; OUTPUT: RDI = Modify                                                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HexPrint64:
push rax
ror rax,32
call HexPrint32
pop rax
; no RET, continue at next subroutine
;---------- Print 32-bit Hex Number -------------------------------------------;
;                                                                              ;
; INPUT:  EAX = Number                                                         ;
;         RDI = Destination Pointer                                            ;
;                                                                              ;
; OUTPUT: RDI = Modify                                                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HexPrint32:
push rax
ror eax,16
call HexPrint16
pop rax
; no RET, continue at next subroutine
;---------- Print 16-bit Hex Number -------------------------------------------;
;                                                                              ;
; INPUT:  AX  = Number                                                         ;
;         RDI = Destination Pointer                                            ;
;                                                                              ;
; OUTPUT: RDI = Modify                                                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HexPrint16:
push rax
xchg al,ah
call HexPrint8
pop rax
; no RET, continue at next subroutine
;---------- Print 8-bit Hex Number --------------------------------------------;
;                                                                              ;
; INPUT:  AL  = Number                                                         ;
;         RDI = Destination Pointer                                            ;
;                                                                              ;
; OUTPUT: RDI = Modify                                                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HexPrint8:
push rax
ror al,4
call HexPrint4
pop rax
; no RET, continue at next subroutine
;---------- Print 4-bit Hex Number --------------------------------------------;
;                                                                              ;
; INPUT:  AL  = Number (bits 0-3)                                              ;
;         RDI = Destination Pointer                                            ;
;                                                                              ;
; OUTPUT: RDI = Modify                                                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HexPrint4:
cld
push rax
and al,0Fh
cmp al,9
ja .modify
add al,'0'
jmp .store
.modify:
add al,'A'-10
.store:
stosb
pop rax
ret
;---------- Print 32-bit Decimal Number ---------------------------------------;
;                                                                              ;
; INPUT:   EAX = Number value                                                  ;
;          BL  = Template size, chars. 0=No template                           ;
;          RDI = Destination Pointer (flat)                                    ;
;                                                                              ;
; OUTPUT:  RDI = New Destination Pointer (flat)                                ;
;                modified because string write                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
DecimalPrint32:
cld
push rax rbx rcx rdx
mov bh,80h-10         ; Bit BH.7 = print zeroes flag
add bh,bl
mov ecx,1000000000    ; ECX = service divisor
.mainCycle:
xor edx,edx
div ecx               ; Produce current digit, EDX:EAX / ECX
and al,0Fh
test bh,bh
js .firstZero
cmp ecx,1
je .firstZero
cmp al,0              ; Not actual left zero ?
jz .skipZero
.firstZero:
mov bh,80h            ; Flag = 1
or al,30h
stosb                 ; Store char
.skipZero:
push rdx              ; Push remainder
xor edx,edx
mov eax,ecx
mov ecx,10
div ecx
mov ecx,eax          ; ECX = Quotient, used as divisor and cycle condition 
pop rax              ; EAX = remainder
inc bh
test ecx,ecx
jnz .mainCycle       ; Cycle if (unsigned) quotient still > 0 
pop rdx rcx rbx rax
ret
;---------- Print double precision value --------------------------------------;
; x87 FPU used, required x87 presence validation by CPUID before call this.    ;
;                                                                              ;
; INPUT:   RAX = Double precision number                                       ;
;          BL  = Number of digits in the INTEGER part,                         ;
;                used for add left non-signed zeroes.                          ;
;                BL=0 means not print left unsigned zeroes.                    ;
;          BH  = Number of digits in the FLOAT part,                           ;
;                used as precision control.                                    ;
;          RDI = Destination text buffer pointer                               ;
;                                                                              ;
; OUTPUT:  RDI = Modified by text string write                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
DoublePrint:
push rax rbx rcx rdx r8 r9 r10 r11
cld
mov rdx,07FFFFFFFFFFFFFFFh
and rdx,rax
jz .fp64_Zero
mov rcx,07FF8000000000000h
cmp rdx,rcx
je .fp64_QNAN
mov rcx,07FF0000000000000h
cmp rdx,rcx
je .fp64_INF
ja .fp64_NAN
finit
push rax
push rax
fstcw [rsp]
pop rax
or ax,0C00h
push rax
fldcw [rsp]
pop rax
fld qword [rsp]
pop rax
fld st0
frndint
fxch
fsub st0,st1
mov eax,1
movzx ecx,bh
jrcxz .orderDetected
@@:
imul rax,rax,10
loop @b
.orderDetected:
push rax
fimul dword [rsp]
pop rax
push rax rax
fbstp [rsp]
pop r8 r9
push rax rax
fbstp [rsp]
pop r10 r11
bt r11,15
setc dl
bt r9,15
setc dh
test dx,dx
jz @f
mov al,'-'
stosb
@@:
mov dl,0
mov ecx,18 
.cycleInteger:
mov al,r11l
shr al,4
cmp cl,1
je .store
cmp cl,bl
jbe .store
test dl,dl
jnz .store
test al,al
jz .position 
.store:
mov dl,1
or al,30h
stosb
.position:
shld r11,r10,4
shl r10,4
loop .cycleInteger
test bh,bh
jz .exit
mov al,'.'
stosb
std 
movzx ecx,bh     
lea rdi,[rdi + rcx]
push rdi
dec rdi
.cycleFloat:
mov al,r8l
and al,0Fh
or al,30h
stosb
shrd r8,r9,4
shr r9,4
loop .cycleFloat
pop rdi
cld
jmp .exit
.fp64_Zero:
mov eax,'0.0 '
jmp .fp64special
.fp64_INF:
mov eax,'INF '
jmp .fp64special
.fp64_NAN:
mov eax,'NAN '
jmp .fp64special
.fp64_QNAN:
mov eax,'QNAN'
.fp64special:
stosd
jmp .exit
.error:
mov al,'?'
stosb
.exit:
finit
pop r11 r10 r9 r8 rdx rcx rbx rax
ret
;---------- Print memory block size as Integer.Float --------------------------;
;                                                                              ;
; INPUT:   RAX = Number value, units = Bytes                                   ;
;          BL  = Force units (override as smallest only)                       ;
;                FF = No force units, auto select                              ;
;                0 = Bytes, 1 = KB, 2 = MB, 3 = GB, 4 = TB                     ;
;          RDI = Destination Pointer (flat)                                    ;
;                                                                              ;
; OUTPUT:  RDI = New Destination Pointer (flat)                                ;
;                modified because string write                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
SizePrint64:
push rax rbx rcx rdx rsi
cld
cmp bl,0FFh
je .autoUnits
mov esi,1
movzx ecx,bl
jrcxz .unitsAdjusted
.unitsCycle:
shl rsi,10
loop .unitsCycle
.unitsAdjusted:
mov cl,bl
xor edx,edx
div rsi
mov bl,0
call DecimalPrint32
imul eax,edx,10
div rsi
cmp cl,0
je .afterNumber
push rax
mov al,'.'
stosb
pop rax
jmp .decimalMode
.autoUnits:
xor ecx,ecx
test rax,rax
jz .decimalMode
.unitsAutoCycle:
mov rbx,rax
xor edx,edx
mov esi,1024                           
div rsi
mov esi,0FFFFFFFFh
cmp rbx,rsi
ja .above32bit
test rdx,rdx
jnz .modNonZero
.above32bit:
inc ecx
jmp .unitsAutoCycle
.modNonZero:
cmp ecx,4
ja .hexMode
mov eax,ebx
.decimalMode:
mov bl,0
call DecimalPrint32
.afterNumber:
mov al,' '
stosb
lea eax,[ecx + STR_UNITS_BYTES]
call PoolStringWrite  
jmp .exit
.hexMode:
call HexPrint64
mov al,'h'
stosb 
.exit:
pop rsi rdx rcx rbx rax
ret
;---------- Execute binder in the binders pool by index -----------------------;
;                                                                              ;
; INPUT:   RBX = Current window handle for get dialogue items                  ;
;          AX  = Binder index in the binders pool                              ;
;                                                                              ;
; OUTPUT:  None                                                                ;  
;                                                                              ;
;------------------------------------------------------------------------------;
Binder:
push rbx rsi rdi rbp r13 r14 r15 
cld
lea r15,[APP_DATA]
mov rsi,[r15 + APPDATA.lockedBinders]
movzx rcx,ax
jrcxz .found            ; Go if selected first binder, index = 0
.find:
lodsb
add rsi,3               ; 3 bytes can not exist, skip (not read by LODSD) it
test al,00111111b
jnz .find               ; Go  continue scan if list terminator not found
sub rsi,3               ; Terminator opcode is single-byte, return pointer back
loop .find              ; Cycle for skip required number of binders
.found:                 ; Start execute selected binder
lodsd
test al,00111111b
jz .stop
mov edx,eax
mov ecx,eax
shr eax,6
and eax,00001FFFh       ; EAX = first 13-bit parameter
shr edx,6+13
and edx,00001FFFh       ; EDX = second 13-bit parameter
and ecx,00111111b
push rsi
call [PROC_BINDERS + rcx * 8 - 8]  ; call by ECX = Binder index
pop rsi
jmp .found              ; cycle for next instruction of binder
.stop:
pop r15 r14 r13 rbp rdi rsi rbx
ret
;---------- Script handler: bind indexed string from pool to GUI object -------;  
BindSetString:      ; EAX = String ID, RDX = Parm#2 = Resource ID for GUI item
mov rsi,[r15 + APPDATA.lockedStrings]
call IndexString    ; Return RSI = Pointer to selected string
BindEntry:
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
mov rcx,rbx           ; RCX = Parm#1 = Parent window handle, RDX = Parm#2 = ID  
call [GetDlgItem]     ; Return handle of GUI item
test rax,rax
jz BindExit           ; Go skip if error, item not found
mov r9,rsi            ; R9  = Parm#4 = lParam = Pointer to string
xor r8d,r8d           ; R8  = Parm#3 = wParam = Not used
mov edx,WM_SETTEXT    ; RDX = Parm#2 = Msg
xchg rcx,rax          ; RCX = Parm#1 = hWnd 
call [SendMessage]    ; Set string for GUI item
BindExit:
mov rsp,rbp
ret
;---------- Script handler: bind string from temporary buffer to GUI object ---;
BindSetInfo:          ; EAX = String offset, RDX = Resource ID for GUI item
lea rsi,[BIND_LIST + rax]
jmp BindEntry
;---------- Script handler: bind string referenced by pointer to GUI object ---;
BindSetPtr:           ; EAX = String pointer, RDX = Resource ID for GUI item
lea rsi,[BIND_LIST + rax]
mov rsi,[rsi]
test rsi,rsi
jnz BindEntry
ret
;---------- Script handler: enable or disable GUI object by binary flag -------;
BindSetBool:       ; EAX = Variable offset:bit, RDX = Resource ID for GUI item
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
mov ecx,eax
shr eax,3
and ecx,0111b
lea r8,[BIND_LIST]
movzx eax,byte [r8 + rax]
bt eax,ecx
setc al
xchg esi,eax
mov rcx,rbx         ; RCX = Parm#1 = Parent window handle, RDX = Parm#2 = ID  
call [GetDlgItem]   ; Return handle of GUI item
test rax,rax
jz BindExit         ; Go skip if error, item not found
mov edx,esi
xchg rcx,rax 
call [EnableWindow]
jmp BindExit
;---------- Script handler: set GUI object enable and checked states ----------l
; This handler use 2-bit field: enable flag and state flag.
BindSetSwitch:      ; EAX = Variable offset:bit, RDX = Resource ID for GUI item
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
mov ecx,eax
shr eax,3
and ecx,0111b
lea r8,[BIND_LIST]
movzx eax,byte [r8 + rax]
xor esi,esi
bt eax,ecx
rcl esi,1
inc ecx
bt eax,ecx
rcl esi,1           ; Bits ESI.0 = value , ESI.1 = enable
mov rcx,rbx         ; RCX = Parm#1 = Parent window handle, RDX = Parm#2 = ID  
call [GetDlgItem]   ; Return handle of GUI item
test rax,rax
jz BindExit         ; Go skip if error, item not found
xchg rdi,rax
mov edx,esi
shr edx,1           ; RDX = Parm#2 = Data, 0 = disable , 1 = enable
mov rcx,rdi         ; RCX = Parm#1 = hWnd 
call [EnableWindow]
xor r9d,r9d         ; R9  = Parm#4 = lParam = not used
and esi,1
mov r8d,esi         ; R8  = Parm#3 = wParam: BST_CHECKED = 1, BST_UNCHECKED = 0
mov edx,BM_SETCHECK ; RDX = Parm#2 = Msg
mov rcx,rdi         ; RCX = Parm#1 = hWnd 
call [SendMessage]  ; Set string for GUI item
jmp BindExit 
;---------- Script handlers: set decimal and hex number edit field ------------;
BindSetDec32:
mov cl,0
jmp BindSetNumberEntry
BindSetHex32:
mov cl,1
jmp BindSetNumberEntry
BindSetHex64:
mov cl,2
BindSetNumberEntry:
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea rsi,[BIND_LIST]
lea rdi,[TEMP_BUFFER]
add rsi,rax
push rbx rdi
dec cl
jz .printHex32
dec cl
jz .printHex64
lodsd
mov bl,0
call DecimalPrint32
jmp .printDone
.printHex32:
lodsd
call HexPrint32
jmp .printDone
.printHex64:
lodsq
call HexPrint64
.printDone:
mov al,0
stosb
pop rdi rbx
mov rcx,rbx         ; RCX = Parm#1 = Parent window handle, RDX = Parm#2 = ID  
call [GetDlgItem]   ; Return handle of GUI item
test rax,rax
jz BindExit         ; Go skip if error, item not found
mov r9,rdi
xor r8d,r8d
mov edx,WM_SETTEXT
xchg rcx,rax
call [SendMessage]
jmp BindExit
;---------- Script handler: operations with combo box -------------------------; 
BindSetCombo:     ; EAX = Combo offset, RDX = Parm#2 = Resource ID for GUI item
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea rsi,[BIND_LIST + rax]  ; RSI = Pointer to combo description list
mov rcx,rbx           ; RCX = Parm#1 = Parent window handle, RDX = Parm#2 = ID  
call [GetDlgItem]     ; Return handle of GUI item
test rax,rax          ; RAX = Handle of combo box
jz .exit              ; Go skip if error, item not found
xchg rdi,rax          ; RDI = Handle of combo box
mov r13d,0FFFF0000h   ; R13D = Store:Counter for selected item
.scan:
lodsb                       ; AL = Tag from combo description list 
movzx rax,al
call [PROC_COMBO + rax * 8]  ; Call handler = F(tag)
inc r13d
jnc .scan                   ; Continue if end tag yet not found
shr r13d,16
cmp r13w,0FFFFh
je .exit
xor r9d,r9d           ; R9  = Parm#4 = lParam = Not used 
mov r8d,r13d          ; R8  =  Parm#3 = wParam = Selected item, 0-based 
mov edx,CB_SETCURSEL  ; RDX = Parm#2 = Msg
mov rcx,rdi           ; RCX = Parm#1 = hWnd 
call [SendMessage]    ; Set string for GUI item
.exit:
mov rsp,rbp
ret
BindComboStopOn:                  ; End of list, combo box enabled
stc
ret
BindComboStopOff:                 ; End of list, combo box disabled (gray) 
stc
ret
BindComboCurrent:                 ; Add item to list as current selected
call HelperBindCombo
shl r13d,16
clc
ret
BindComboAdd:                     ; Add item to list
call HelperBindCombo
clc
ret
BindComboInactive:                ; Add item to list as inactive (gray)
clc
ret
;---------- Script handler: bind font from registry to GUI object -------------;
BindSetFont:          ; EAX = Font number, RDX = Resource ID for GUI item
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea rsi,[r15 + APPDATA.hFont1]
mov rsi,[rsi + rax * 8]
mov rcx,rbx            ; RCX = Parm#1 = Parent window handle, RDX = Parm#2 = ID  
call [GetDlgItem]      ; Return handle of GUI item
test rax,rax
jz BindExit            ; Go skip if error, item not found
mov r9d,1              ; R9  = Parm#4 = lParam = redraw flag
mov r8,rsi             ; R8  = Parm#3 = wParam = handle
mov edx,WM_SETFONT     ; RDX = Parm#2 = Msg
xchg rcx,rax           ; RCX = Parm#1 = hWnd
call [SendMessage]
jmp BindExit
;--- Script handler: get state of GUI object and write to bit at buffer -------;
BindGetSwitch:        ; EAX = Variable offs:bit, RDX = Resource ID for GUI item
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
xchg esi,eax          ; ESI = Destination variable address and bit position
mov rcx,rbx           ; RCX = Parm#1 = Parent window handle, RDX = Parm#2 = ID  
call [GetDlgItem]     ; Return handle of GUI item
test rax,rax
jz BindExit           ; Go skip if error, item not found
xchg rcx,rax          ; RCX = Parm#1 = Handle                     
mov edx,BM_GETCHECK   ; RDX = Parm#2 = Message
xor r8d,r8d           ; R8  = Parm#3 = Not used = 0
xor r9d,r9d           ; R9  = Parm#4 = Not used = 0
call [SendMessage]    ; Return RAX = widget state
cmp rax,1             ; Compare with BST_CHECKED = 1, set ZF = 1 if this
pushf
mov ecx,esi
shr esi,3
and ecx,0111b
add rsi,BIND_LIST
popf
je .setBit
mov al,11111110b
rol al,cl
and [rsi],al 
jmp .done
.setBit:
mov al,00000001b
rol al,cl
or [rsi],al 
.done:
jmp BindExit
;---------- Script handlers: get decimal and hex number edit field ------------;
BindGetDec32:
xor r13d,r13d
jmp BindGetNumberEntry
BindGetHex32:
mov r13b,1
jmp BindGetNumberEntry
BindGetHex64:
mov r13b,2
BindGetNumberEntry:
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea rsi,[TEMP_BUFFER]
lea rdi,[BIND_LIST + rax]
mov rcx,rbx        ; RCX = Parm#1 = Parent window handle, RDX = Parm#2 = ID  
call [GetDlgItem]  ; Return handle of GUI item
test rax,rax
jz .exit           ; Go skip if error, item not found
mov r9,rsi
mov r8d,17
mov edx,WM_GETTEXT
xchg rcx,rax
call [SendMessage]
test rax,rax
jz .exit           ; Go skip if error, can't read string
test r13b,r13b
jz .parseDec
xor ecx,ecx
.parseHex:
xor eax,eax
lodsb
cmp al,'0'
jb .parseDone
cmp al,'9'
ja .tryHex 
and al,0Fh 
jmp .tryDone
.tryHex:
and al,0DFh
sub al,'A' - 10
cmp al,10
jb .parseDone 
.tryDone:
rol rcx,4
or rcx,rax
jmp .parseHex
.parseDec:
xor eax,eax
lodsb
sub al,'0'
jb .parseDone
cmp al,9
ja .parseDone
imul ecx,ecx,10
add ecx,eax
.parseDone:
xchg rax,rcx
cmp r13b,2
jb .store32
stosq
jmp .exit
.store32:
stosd
.exit:
jmp BindExit
;---------- Helper for add string to combo box list ---------------------------;
;                                                                              ;
; INPUT:   RSI = Pointer to binder script                                      ;
;          RDI = Parent window handle                                          ;
;          R15 = Pointer to application registry for global variables access   ; 
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperBindCombo:
lodsw
push rsi rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
mov rsi,[r15 + APPDATA.lockedStrings]
call IndexString                  ; Return RSI = Pointer to selected string
mov r9,rsi                        ; R9  = Parm#4 = lParam = Pointer to string
xor r8d,r8d                       ; R8  = Parm#3 = wParam = Not used
mov edx,CB_ADDSTRING              ; RDX = Parm#2 = Msg
mov rcx,rdi                       ; RCX = Parm#1 = hWnd 
call [SendMessage]                ; Set string for GUI item
mov rsp,rbp
pop rbp rsi
ret
;---------- Include subroutines from modules ----------------------------------;
include 'ncrb64\dialogs\connect_code.inc'
include 'ncrb64\system_info\connect_code.inc'
include 'ncrb64\threads_manager\connect_code.inc'
include 'ncrb64\memory_bandwidth_temporal\connect_code.inc'
include 'ncrb64\memory_bandwidth_non_temporal\connect_code.inc'
include 'ncrb64\memory_latency\connect_code.inc'
include 'ncrb64\math_bandwidth\connect_code.inc'
;------------------------------------------------------------------------------;
;                                                                              ;
;                              Data section.                                   ;        
;                                                                              ;
;------------------------------------------------------------------------------;
section '.data' data readable writeable
;---------- Common data for application ---------------------------------------;
APP_MEMORY     DQ  0                        ; Must be 0 for conditional release
APP_CTRL       INITCOMMONCONTROLSEX  8, 0   ; Structure for initialization
;---------- Pointers to procedures of GUI bind scripts interpreter ------------;
PROC_BINDERS   DQ  BindSetString
               DQ  BindSetInfo
               DQ  BindSetPtr
               DQ  BindSetBool
               DQ  BindSetSwitch
               DQ  BindSetDec32
               DQ  BindSetHex32
               DQ  BindSetHex64              
               DQ  BindSetCombo
               DQ  BindSetFont
               DQ  BindGetSwitch
               DQ  BindGetDec32
               DQ  BindGetHex32
               DQ  BindGetHex64              
PROC_COMBO     DQ  BindComboStopOn     ; End of list, combo box enabled
               DQ  BindComboStopOff    ; End of list, combo box disabled (gray) 
               DQ  BindComboCurrent    ; Add item to list as current selected
               DQ  BindComboAdd        ; Add item to list
               DQ  BindComboInactive   ; Add item to list as inactive (gray)
;---------- List for load raw resources ---------------------------------------;
RAW_LIST       DW  IDS_STRINGS_POOL
               DW  IDS_BINDERS_POOL
               DW  IDS_CPU_COMMON_POOL
               DW  IDS_CPU_AVX512_POOL
               DW  IDS_OS_CONTEXT_POOL
               DW  IDS_INTEL_CACHE
               DW  IDS_ACPI_DATA_POOL
               DW  IDS_IMPORT_POOL
               DW  IDS_FONTS_POOL
               DW  IDS_BRUSHES_POOL
               DW  IDS_BITMAP_INFO
               DW  0
;---------- Libraries for dynamical import ------------------------------------;
NAME_KERNEL32  DB  'KERNEL32.DLL' , 0      ; Must be sequental list of WinAPI
NAME_ADVAPI32  DB  'ADVAPI32.DLL' , 0 , 0  ; Two zeroes means end of list
NAME_DATA      DB  'DATA.DLL'     , 0
;---------- Text strings about application ------------------------------------;
PROGRAM_NAME   DB  PROGRAM_NAME_TEXT   , 0
ABOUT_CAP      DB  ABOUT_CAP_TEXT      , 0
ABOUT_NAME     DB  ABOUT_TEXT_1        , 0Dh,0Ah
               DB  ABOUT_TEXT_2        , 0Dh,0Ah
               DB  ABOUT_TEXT_3        , 0Dh,0Ah, 0
;---------- Errors messages strings -------------------------------------------;
MSG_ERRORS     DB  'Memory allocation error.'                 , 0      
               DB  'Initialization failed.'                   , 0
               DB  'Load resource library failed.'            , 0
               DB  'Handle of this module is NULL.'           , 0
               DB  'Load application icon failed.'            , 0
               DB  'Load icon data from resource DLL failed.' , 0
               DB  'Load raw data from resource DLL failed.'  , 0
               DB  'Create font failed.'                      , 0
               DB  'Create dialogue window failed.'           , 0
;---------- Include constants and pre-defined variables from modules ----------;
include 'ncrb64\dialogs\connect_data.inc'
include 'ncrb64\system_info\connect_data.inc'
include 'ncrb64\threads_manager\connect_data.inc'
include 'ncrb64\memory_bandwidth_temporal\connect_data.inc'
include 'ncrb64\memory_bandwidth_non_temporal\connect_data.inc'
include 'ncrb64\memory_latency\connect_data.inc'
include 'ncrb64\math_bandwidth\connect_data.inc'
;---------- Operating system definitions --------------------------------------;
ERROR_BUFFER_LIMIT  = 07Ah
VALUE_BUFFER_LIMIT  = 128 * 1024 
;---------- Equations for UPB, IPB, OPB data build and interpreting -----------;
; Assembler Methods (AM) option values count and Bandwidth/Latency criteria
; TODO. Add new methods, update AM_BYTE_COUNT, LATENCY_MODE, plus see below.
AM_BYTE_COUNT       =  26     ; 26 bytes per methods primary list
LATENCY_MODE        =  24     ; modes ID = 24, 25 for latency measurement. TODO. USE TRANSLATION FOR TEMPORAL/NONTEMPORAL, THIS VALUE = 24*2=48.
LATENCY_MODE_COUNT  =  2      ; Force 32x2 addend (this yet for ia32 version only)
READ_SSE128_MODE    =  9      ; for add information about prefetch distance
READ_AVX256_MODE    =  12     ; for add information about prefetch distance
READ_AVX512_MODE    =  15     ; for add information about prefetch distance
; Options limits and values
; 22 codes of assembler option, argument for AM_Selector translation,
; because 22 checkboxes for assembler method select in GUI
ASM_ARGUMENT_LIMIT  =  AM_BYTE_COUNT - 1
; 45 names for assembler methods, result of AM_Selector translation
ASM_RESULT_LIMIT    =  45 + 1
; "Nontemporal" checkbox
NONTEMPORAL_OFF     =  0
NONTEMPORAL_ON      =  1
; "Force 32x2" checkbox
FORCE_32_OFF        =  0
FORCE_32_ON         =  1
; Parallel threads option
PARALLEL_LIMIT      =  2
PARALLEL_NOT_SUP    =  0
PARALLEL_NOT_USED   =  1
PARALLEL_USED       =  2
; Target Object: Cache, DRAM, Custom
TARGET_LIMIT        =  5 
TARGET_L1           =  0
TARGET_L2           =  1
TARGET_L3           =  2
TARGET_L4           =  3
TARGET_DRAM         =  4
TARGET_CUSTOM       =  5
; PD = Prefetch distance
PD_LIMIT            =  3
PD_NOT_USED         =  0
PD_DEFAULT          =  1
PD_MEDIUM           =  2
PD_LONG             =  3
; HT = Hyper Threading
HT_LIMIT            =  2
HT_NOT_SUPPORTED    =  0
HT_NOT_USED         =  1
HT_USED             =  2
; PG = Processor Group
PG_LIMIT            =  2
PG_NOT_SUPPORTED    =  0
PG_NO_CONTROL       =  1
PG_OPTIMAL          =  2
; NUMA topology
NUMA_LIMIT          =  4
NUMA_NOT_SUPPORTED  =  0
NUMA_NO_CONTROL     =  1
NUMA_CURRENT_ONLY   =  2
NUMA_OPTIMAL        =  3
NUMA_NON_OPTIMAL    =  4    
; LP = Large Pages
LP_LIMIT            =  2
LP_NOT_SUPPORTED    =  0
LP_NOT_USED         =  1
LP_USED             =  2
; Measure repeats selection modes
MEASURE_LIMIT       =  3
MEASURE_BRIEF       =  0
MEASURE_CAREFUL     =  1    
MEASURE_B_ADAPTIVE  =  2
MEASURE_C_ADAPTIVE  =  3
; Approximation by X modes
APPROX_LIMIT        =  2
APPROX_NONE         =  0
APPROX_X16          =  1
APPROX_X32          =  2
;---------- Multifunctional buffer definitions --------------------------------;
; Note. This variables without pre-defined values located higher
;       for save EXE file space.  
; Note. Upper case used for labels in this block. 
align 4096
TEMP_SIZE           EQU  48 * 1024  
TEMP_BUFFER         DQ   TEMP_SIZE dup (?)
;---------- Threads and memory management definitions -------------------------;
; Thread control entry, or entire benchmark control if single thread.
; Note keep 128 bytes per entry, see ThreadEntry, fixed coding used.
; Note keep all pairs: Affinity Mask + Affinity Group as 2 sequental qwords,
; this rule required because WinAPI use direct output store to this qwords. 
struct THCTRL
eventStart      dq  ?   ; Event Handle for operation START signal
eventDone       dq  ?   ; Event Handle for operation COMPLETE signal 
threadHandle    dq  ?   ; Thread Handle for execution thread
threadAffinity  dq  ?   ; Affinity Mask = F (True Affinity Mask, Options) 
threadGroup     dq  ?   ; Processor group, associated with affinity mask
entryPoint      dq  ?   ; Entry point to operation subroutine
base1           dq  ?   ; Source for read, destination for write and modify
base2           dq  ?   ; Destination for copy
sizeBytes       dq  ?   ; Block size, units = bytes, for benchmarking, not for memory allocation (REDUNDANT FIELD RESERVED) 
sizeInst        dq  ?   ; Block size, units = instructions, for benchmarking (MUST BE UPDATED PER DRAW ITERATION)
largePages      dq  ?   ; 0=not supported, 1=supported but not used, 2=used (REDUNDANT FIELD RESERVED)
repeats         dq  ?   ; Number of measurement repeats for precision requirements
affinityMode    dq  ?   ; Affinity mode: 0=None, 1=Without groups, 2=With groups 
origBase        dq  ?   ; True (before alignment) memory block base for release (REDUNDANT FIELD RESERVED)
origAffinity    dq  ?   ; Store original affinity mask, also storage for return (STORED BY THREAD, NOT BY INITIALIZER) 
origGroup       dq  ?   ; Store processor group for original offinity mask (STORED BY THREAD, NOT BY INITIALIZER) 
ends
MAX_THREADS            =   256   ; Maximum number of supported threads: total and per processor group
MAX_THREADS_PER_GROUP  =   64
; Thread entry size = 128 bytes, const because optimal shift by 7 can be used
; 32768 bytes, 256 entries * 128 bytes, used for (multi) thread management.
THCTRL_SIZE            =   sizeof.THCTRL
THREAD_LIST_SIZE       =   MAX_THREADS * THCTRL_SIZE
THREAD_LIST            DB  THREAD_LIST_SIZE  dup (?)  ; THCTRL  MAX_THREADS dup (?)
;---------- Event management definitions --------------------------------------;
; Event handle size = 8 bytes (64-bit)
; 2048 bytes, 256 handles * 8 bytes, 
; separate sequental list req. for WaitForMultipleObjects function  
EVENT_SIZE             =   8
EVENT_LIST_SIZE        =   MAX_THREADS * EVENT_SIZE
; Events lists must be sequental after thread list, because relative addressing.
START_EVENTS           DB  EVENT_LIST_SIZE   dup ?  
DONE_EVENTS            DB  EVENT_LIST_SIZE   dup ?  
;---------- NUMA topology management definitions ------------------------------; 
; NUMA node description entry, not a same as thread description enrty
; Note keep 64 bytes per entry, see ThreadEntry, can be non parametrized coding
; Note keep all pairs: Affinity Mask + Affinity Group as 2 sequental qwords,
; this rule required because WinAPI use with direct output store to this qwords 
struct NUMACTRL
nodeID          dq  ?   ; NUMA Node number, if no NUMA, this field = 0 for all entries
nodeAffinity    dq  ?   ; NUMA Node affinity mask
nodeGroup       dq  ?   ; Processors Group number, 0 if single group platform
alignedBase     dq  ?   ; Block size after alignment, for read/write operation
alignedSize     dq  ?   ; Block base address after alignment, for r/w operation 
trueBase        dq  ?   ; Base address, returned when allocated, for release, 0=not used 
reserved        dq  2 dup ?    ; Reserved for alignment
ends
MAX_NODES              =   64  ; Maximum number of supported NUMA nodes
; NUMA node entry size = 64 bytes, const because optimal shift by 6 can be used
; 16384 bytes, 256 entries * 64 bytes,
; for NUMA nodes description, not a same as THREAD
NUMACTRL_SIZE          =   sizeof.NUMACTRL
NUMA_LIST_SIZE         =   MAX_NODES * NUMACTRL_SIZE  
; Events lists must be sequental after event lists, because relative addressing.
NUMA_LIST              DB  NUMA_LIST_SIZE    dup ?
;--- Memory and Cache Benchmark User Parameters Block (UPB) definitions -------;
; This variables used for collect data from GUI objects when benchmark runs.
; Test USER parameters, loaded from GUI widgets settings.
struct MEMUPB
optionAsm         dd  ?    ; Select test ASM method, one of procedures
optionNontemp     dd  ?    ; Checkbox "Nontemporal"
optionForce32     dd  ?    ; Checkbox "Force 32x2"
optionDistance    dd  ?    ; Prefetch distance for non-temporal read: 0=default, 1=medium, 2=long
optionTarget      dd  ?    ; Objects { L1, L2, L3, L4, DRAM, CUSTOM }
optionParallel    dd  ?    ; Parallel { GRAY_NOT_SUP, DISABLED, ENABLED }
optionHT          dd  ?    ; HT { GRAY_NOT_SUP, DISABLED, ENABLED }
optionPG          dd  ?    ; PG { GRAY_NOT_SUP, DISABLED, ENABLED } 
optionNUMA        dd  ?    ; NUMA { GRAY_NOT_SUP, UNAWARE, SINGLE_DOMAIN, FORCE LOCAL, FORCE REMOTE }
optionLP          dd  ?    ; LP { GRAY_NOT_SUP, DISABLED, ENABLED }  
optionMeasure     dd  ?    ; Measurement repeats { 0=fast, 1=slow, 2=fast adaptive, 3=slow adaptive }
optionApprox      dd  ?    ; Approximation for X { 0=none, 1=X16, 2=X32 }
runContext        dd  ?    ; Run context: 0 = "Run simple" , 1 = "Run drawings" 
customBlockStart  dq  ?    ; Override start block size, or 0=default
ends
align 8
MEM_UPB MEMUPB ?
;--- Memory and Cache Benchmark Input Parameters Block (IPB) definitions ------;
; This variables used for build input parameters for Memory/Math performance
; engines procedures. IPB = f(UPB).
; Test INPUT parameters, build IPB = F ( UPB, SYSPARMS ).
struct MEMIPB
; First 8 items of IPB associated with first 8 items of UPB
updatedAsm        dd  ?    ; Routine selector, set after detect features
updatedDistance   dd  ?    ; Prefetch distance for non-temporal read: 0=default, 1=medium, 2=long
updatedTarget     dd  ?    ; Objects { L1, L2, L3, L4, DRAM, CUSTOM }
updatedThreads    dd  ?    ; Number of threads, set after detect features
updatedHT         dd  ?    ; 0=Not sup. by platform, 1=sup. but not used, 2=used
updatedPG         dd  ?    ; 0=Not sup. by platform, 1=sup. but not used, 2=used 
updatedNUMA       dd  ?    ; 0=None, 1=No control, 2-Single domain, 3=Optimal, 4=Non optim. 
updatedLP         dd  ?    ; 0=Not sup. by platform, 1=sup. but not used, 2=used
updatedMeasure    dd  ?    ; 0=fast, 1=slow, 2=fast adaptive, 3=slow adaptive
updatedApprox     dd  ?    ; Approximation for X { 0=none, 1=X16, 2=X32 }  
; Two work blocks used by benchmark scenario
allocatedBlock1   dq  ?
allocatedBlock2   dq  ?
; This items generated as f( first 8 items, platform configuration )
startBlockSize    dq  ?    ; Start block size, or 0=default, unite = bytes
endBlockSize      dq  ?    ; End block size, or 0=default
deltaBlockSize    dq  ?    ; Delta block size, or 0=default
measureRepeats    dq  ?    ; Number of measurement repeats, note 64-bit value
operandWidth      dd  ?    ; Used instructions operand width, bits 
groupsCount       dd  ?    ; Number of Processor Groups
domainsCount      dd  ?    ; Number of NUMA domains
pageSize          dq  ?    ; Memory page size, bytes
; Memory allocation results
memoryTotal       dq  ?    ; Memory allocated, per all threads
memoryPerThread   dq  ?    ; Memory allocated, per each thread 
; Benchmarks select and routine dump support
patternRoutine    dq  ?    ; Pointer to target benchmark routine, this is fill routine for latency mode
walkRoutine       dq  ?    ; Second routine, required for latency measurement
dumpMethodStart   dq  ?    ; Start of fragment, visualized as dump during benchmarks
dumpMethodLength  dd  ?    ; Length of fragment, visualized as dump during benchmarks, bytes
; Adaptive measurement timings support
adaptiveSeconds   dq  ?    ; Target measurement time for stabilization, seconds, floation point
adaptiveProduct   dq  ?    ; Size * Repeats = Product , practic calc: Repeats = Product / Size 
; Additionsl system information and management flags
applicationMode   dd  ?    ; 0 = ia32 under Win32, 1 = x64, 2 = ia32 under Win64
nonTemporalMode   dd  ?    ; 1 means performance pattern replaced by non-temporal patterns list
force32mode       dd  ?    ; 1 means force 32x2 mode for latency measurement (yet for ia32 version only)
threadStop        dd  ?    ; Flag for stop child threads
ends
align 8
MEM_IPB MEMIPB ?
;--- Memory and Cache Benchmark Output Parameters Block (OPB) definitions -----;
; This variables used for collect output results of Memory/Math 
; Performance Engines procedures.
; Test OUTPUT parameters, store OPB = F ( Benchmarks and TSC meas. results ).
struct MEMOPB
deltaTSC          dq  ?    ; TSC measured clock, Hz, 64-bit long integer
tscFrequencyHz    dq  ?    ; TSC frequency, Hz, as double precision 
tscFrequencyMHz   dq  ?    ; TSC frequency, MHz, as double precision
tscPeriodS        dq  ?    ; TSC period, seconds, as double precision
tscPeriodNs       dq  ?    ; TSC period, nanoseconds, as double precision 
osTimerDelta      dq  ?    ; delta time at units = 100 ns, 64-bit long integer 
tscTimerDelta     dq  ?    ; delta time at units = 1 TSC clk., 64-bit long int. 
ends
align 8
MEM_OPB MEMOPB ?
;---------- Vector Brief Output Parameters Block (OPB) definitions ------------;
; Here OPB only, because no input parameters for vector brief.
struct VECBROPB
dtSse128read    dq  ?      ; TSC clocks per SSE128 Read pattern
dtSse128write   dq  ?      ; TSC clocks per SSE128 Write pattern
dtSse128copy    dq  ?      ; TSC clocks per SSE128 Copy pattern 
dtAvx256read    dq  ?      ; TSC clocks per AVX256 Read pattern
dtAvx256write   dq  ?      ; TSC clocks per AVX256 Write pattern
dtAvx256copy    dq  ?      ; TSC clocks per AVX256 Copy pattern 
dtAvx512read    dq  ?      ; TSC clocks per AVX512 Read pattern
dtAvx512write   dq  ?      ; TSC clocks per AVX512 Write pattern
dtAvx512copy    dq  ?      ; TSC clocks per AVX512 Copy pattern 
dtSse128sqrt    dq  ?      ; TSC clocks per SSE128 Square Root pattern
dtAvx256sqrt    dq  ?      ; TSC clocks per AVX256 Square Root pattern
dtAvx512sqrt    dq  ?      ; TSC clocks per AVX512 Square Root pattern
dtX87cos        dq  ?      ; TSC clocks per x87 Cosine (FCOS) pattern
dtX87sincos     dq  ?      ; TSC clocks per x87 Sine+Cosine (FSINCOS) pattern 
ends
align 8
VECBR_OPB VECBROPB  ?
;---------- Key data for GUI application with resources -----------------------;  
struct APPDATA
hResources                 dq ?     ; Resource DLL handle
lockedStrings              dq ?     ; Pointer to strings pool
lockedBinders              dq ?     ; Pointer to binders pool
lockedDataCpuCommon        dq ?     ; Data for build common CPU feature bitmap
lockedDataCpuAvx512        dq ?     ; Data for build AVX512 feature bitmap
lockedDataOsContext        dq ?     ; Data for build OS context bitmap
lockedDataIntelCache       dq ?     ; Data for Intel cache descriptors decode
lockedDataAcpi             dq ?     ; Data base for ACPI tables detection
lockedImportList           dq ?     ; List for WinAPI dynamical import
lockedFontList             dq ?     ; List of fonts names
lockedBrushesList          dq ?     ; List of color brushes
lockedBitmapInfo           dq ?     ; Bitmap info header for draw window
hFont1                     dq ?     ; Handles of created fonts
hFont2                     dq ?
hIcon                      dq ?     ; Application icon handle
hMain                      dq ?     ; Main window handle
hTab                       dq ?     ; Sheets container handle
hImageList                 dq ?     ; Image list handle
selectedTab                dd ?                   ; Current sheet number
tabCtrlItem                TC_ITEM ?              ; Tab item data structure
lockedIcons                dq ICON_COUNT dup ?    ; Pointers to icons resources
createdIcons               dq ICON_COUNT dup ?    ; Pointers to icons 
hTabDlg                    dq ITEM_COUNT dup ?    ; Sheets handles
hInstance                  dq ?                   ; This EXE file handle
ends
APP_DATA APPDATA ?
;---------- Operating system constants and structures definition --------------;
ALL_PROCESSOR_GROUPS   = 0000FFFFh 
struct MEMORYSTATUSEX_DEF
dwLength                   dd ?
dwMemoryLoad               dd ?
ullTotalPhys               dq ?
ullAvailPhys               dq ?
ullTotalPageFile           dq ?
ullAvailPageFile           dq ?
ullTotalVirtual            dq ?
ullAvailVirtual            dq ?
ullAvailExtendedVirtual    dq ?
ends
struct OSDATA
memoryStatusEx             MEMORYSTATUSEX_DEF ?
systemInfo                 SYSTEM_INFO        ?
nativeSystemInfo           SYSTEM_INFO        ?
activeProcessorGroupCount  dd ?
activeProcessorCount       dd ?
numaNodeCount              dd ?
largePageSize              dq ?
largePageEnable            dd ?
ends
OS_DATA OSDATA ?
;---------- Platform topology by OS -------------------------------------------;
struct SUMMARYCACHE  ; Summary cache information: cache size and units count
sizeTrace         dq  ?
countTrace        dd  ?
sizeL1C           dq  ?
countL1C          dd  ?
sizeL1D           dq  ?
countL1D          dd  ?
sizeL2U           dq  ?
countL2U          dd  ?
sizeL3U           dq  ?
countL3U          dd  ?
sizeL4U           dq  ?
countL4U          dd  ?
ends
struct SUMMARYTOPOLOGY  ; Summary topology information by OS
threads           dd  ?
cores             dd  ?
sockets           dd  ?
ends
struct SYSPARMS
applicationMode   dd  ?    ; 0 = ia32 under Win32, 1 = x64, 2 = ia32 under Win64
sseSupported      dd  ?    ; Separate flag for save/restore SSE registers at child thread, 0=No, 1=Yes
summaryCache      SUMMARYCACHE
summaryTopology   SUMMARYTOPOLOGY
ends
SYS_PARMS SYSPARMS ?
;---------- Processor detection results by CPUID and RDTSC --------------------;
struct CPUDATA
vendorString               db 13 dup ?
modelString                db 49 dup ?
cpuSignature               dd ?
extractedFeaturesBitmap    dq ?
extractedAvx512Bitmap      dq ?
extractedContextBitmap     dq ?
tscClockHz                 dq ?
ends
CPU_DATA CPUDATA ?
;---------- Processor detection additional results, cache information ---------;
; Cache info format
; Qword = Size, bytes,
; Word = Maximum threads at this level
; Word = Maximum APIC IDs per package
; For Trace Cache size in micro operations, not bytes
struct CPUCACHEDATA
cpuidTraceCache            dq  ?          ; Instruction trace cache
cpuidTraceSmp              dw  ? , ?
cpuidL1Code                dq  ?          ; L1 instruction cache
cpuidL1Csmp                dw  ? , ?
cpuidL1Data                dq  ?          ; L1 data cache
cpuidL1Dsmp                dw  ? , ?
cpuidL2Unified             dq  ?          ; L2 unified cache
cpuidL2Usmp                dw  ? , ?
cpuidL3Unified             dq  ?          ; L3 unified cache
cpuidL3Usmp                dw  ? , ?
ends
CPU_CACHE_DATA CPUCACHEDATA ?
;---------- Temporary ACPI data for don't use BINDLIST at low level -----------;
; Copies of ACPI-related fields of BINDLIST
struct ACPIDATA
tempAcpiEnable             db            ?    ; D0 = ACPI, D1 = MADT, D2 = SRAT
tempMadt                   BINDACPI      ?
tempSrat                   BINDACPI      ?
ends
ACPI_DATA ACPIDATA ?
;--- Application definitions for Kernel Mode Driver Service Control Program ---;  
SCP_PATH_BUFFER_SIZE = 260     ; Buffer size for driver file string path build
;---------- Kernel Mode Driver (KMD) definitions ------------------------------;
RZ_DRIVER_QUERY_BUFFER_SIZE = 24     ; Buffer size for driver request structure
RZ_REQUEST_CODE             = 41h    ; Driver request code = user routine call 
;---------- Service Control Program (SCP) definitions -------------------------;
SC_MANAGER_ALL_ACCESS = 0000F003Fh   ; Used as desired access rights for SCM
SERVICE_ALL_ACCESS    = 0000F01FFh   ; Used as desired acc. rights for service 
SERVICE_KERNEL_DRIVER = 000000001h   ; Used as service type for service
SERVICE_DEMAND_START  = 000000003h   ; Used as service start option for service
SERVICE_ERROR_NORMAL  = 000000001h   ; Used as error control option for service
SERVICE_CONTROL_STOP  = 000000001h   ; Used as control code for stop service
SERVICE_RUNNING       = 000000004h   ; Used for detect service current state 
;---------- Structure for service request execution status (see MSDN) ---------;
struct SERVICE_STATUS
dwServiceType              dd ?     ; Type of system service
dwCurrentState             dd ?     ; State of service, run/stop/pause
dwControlsAccepted         dd ?     ; Accepted service operations flags
dwWin32ExitCode            dd ?     ; Service unified error code
dwServiceSpecificExitCode  dd ?     ; Service-specific error code 
dwCheckPoint               dd ?     ; Incremnted progress indicator value
dwWaitHint                 dd ?     ; Estimated time of operation for tracking
ends
;---------- Structure for driver query ----------------------------------------;
struct SERVICE_QUERY
iocode    dd ?    ; user I/O code, request type selector
iodata    dd ?    ; user I/O data, request input parameter 
userproc  dq ?    ; procedure offset, callback address
parm1     dq ?    ; parameter A, callback routine optional input parameter 1
parm2     dq ?    ; parameter B, callback routine optional input parameter 2
result    dq ?    ; result, usage example: AL after IN AL,DX
buffer    db RZ_DRIVER_QUERY_BUFFER_SIZE dup ?
ends
;---------- Kernel Mode Driver Service Control Program information ------------;
struct SCPDATA
drvPath   dq ?
drvFile   dq ?               
manager   dq ?               
service   dq ?               
vectors   dq ?               
driver    dq ?                
bytes     dq ?                 
status    SERVICE_STATUS ?   ; Driver status structure
query     SERVICE_QUERY  ?   ; Driver request structure 
ends
SCP_DATA SCPDATA ?
;---------- Dynamical imported WinAPI functions pointers list -----------------;
struct DYNAIMPORT
_IsWow64Process                    dq ?   ; This functions from KERNEL32.DLL
_GlobalMemoryStatusEx              dq ?          
_GetNativeSystemInfo               dq ?
_GetLogicalProcessorInformation    dq ?
_GetLogicalProcessorInformationEx  dq ?
_GetActiveProcessorGroupCount      dq ?  
_GetActiveProcessorCount           dq ?       
_GetLargePageMinimum               dq ?
_GetNumaHighestNodeNumber          dq ?
_GetNumaNodeProcessorMask          dq ?
_GetNumaAvailableMemoryNode        dq ?
_GetNumaNodeProcessorMaskEx        dq ?
_GetNumaAvailableMemoryNodeEx      dq ?
_EnumSystemFirmwareTables          dq ?
_GetSystemFirmwareTable            dq ?
_SetThreadAffinityMask             dq ?
_SetThreadGroupAffinity            dq ?
_VirtualAllocExNuma                dq ?
_OpenProcessToken                  dq ?   ; This functions from ADVAPI32.DLL              
_AdjustTokenPrivileges             dq ?         
ends
align 8
DYNA_IMPORT DYNAIMPORT ?
;---------- GUI objects list --------------------------------------------------;
align 8
BIND_LIST BINDLIST ?
;---------- Benchmarks deault Y-sizing parameters -----------------------------;
; This parameters set for first pass, 
; auto adjusted as F(Maximum Detected Speed or Latency) for next passes,
; if don't close Window 1 and press Run (Resize) button 
; Settings for Cache&RAM mode
; Speed units = MBPS (Megabytes per Second), Latency units = ns (nanoseconds)
Y_RANGE_MAX_BANDWIDTH = 300000
Y_RANGE_MAX_LATENCY = 100
Y_DIV = 10
DEFAULT_Y_MBPS_PER_GRID = Y_RANGE_MAX_BANDWIDTH / Y_DIV  ; Default units per grid Y , megabytes per second
DEFAULT_Y_NS_PER_GRID = Y_RANGE_MAX_LATENCY / Y_DIV      ; Default units per grid Y , nanoseconds
; Benchmarks visualization timings parameters
TIMER_TICK_SHOW    = 50      ; Milliseconds per tick, benchmarks progress timer
TIMER_TICK_SILENT  = 60000   ; 1 revisual per 1 minute, for silent mode
;--- Parallel thread for measurements at draw window, state parameters --------;
; Number of pixels by X, used for drawings, means number of measurements per draw
DRAW_POINTS_COUNT  =  640
; Result of 1000000000 / 1000000 = 1000 , used for convert 
; nanoseconds per instruction (nsPI) to Megabytes per Second (MBPS)
; Decimal megabyte = 1000000 bytes (not binary 1048576)
; Second = 1000000000 nanoseconds
NSPI_TO_MBPS        =  1000
;---------- Draw parameters layout declaration --------------------------------;
; Note optimal layout is qwords alignment 8
; DRPM = Draw Parameters: visualization control
struct DRPM
; Benchmarks units control
; Used indexed access from this base, include next groups, don't reorder variables!
valueGridX       dd  ?      ; Units per horizontal grid cell
valueGridY       dd  ?      ; Units per vertical grid cell
selectUnits      dd  ?      ; Units select: 0=Bytes, 1=Kilobytes, 2=Megabytes
selectMode       dd  ?      ; Measurement mode: 0=Bandwidth, 1=Latency
; Drawings X-counter and X-drawings support
timerCount       dd  ?      ; Timer ticks count ; OLD = Pixels counter for X-progress when drawing
drawPreviousY    dd  ?      ; Previous coordinate for vertical lines draw if required, when ABS(X(i)-X(i+1)) > 1
; Benchmark drawings scale parameters
; Better store value for multiply (not divide) at each iteration, for minimize CPU resources utilization
; A / B  replace to:  A * C , when C = 1/B. Store C.
; Also, vertical offset must be negative, upper means smaller offset, biggest MBPS/ns value
yMultiplier      dq  ?      ; Y pixels scale factor, floating point, double
ends
align 8
DRAW_PARMS DRPM ?
;---------- Benchmarking drawings measurement parallel thread state structure -;
; DTHP = Draw Thread Parameters: handles, measurements, statistics
struct DTHP
eventStart       dq  ?   ; Event handle for signaling this thread starts
eventDone        dq  ?   ; Event handle for signaling this thread terminates 
threadHandle     dq  ?   ; Event handle for this thread 
measureCounter   dd  ?   ; Number of stored results, 0 means no stores before run, maximum MEASURE_POINTS_COUNT (640)
visualCounter    dd  ?   ; Number of visualized results, required because results generation and timer ticks is asynchronous
measureBreak     dd  ?   ; Flag for measurement break, 0=None,   BIDIRECTIONAL SIGNALING IS REJECTED BECAUSE RESIZE BUTTON BUG: 1=Break or Done, signaling is BIDIRECTIONAL: Break and Done
measureAlign     dd  ?   ; This required for QWORD alignment
; Note qwords Min, Max, Average, Median must be SEQUENTAL for pointers advance,
; see gui\win1.inc statistics values write 
; Statistics for CPI (Clocks per Instruction)
statCpiMin       dq  ?   ; CPI minimum 
statCpiMax       dq  ?   ; CPI maximum 
statCpiAverage   dq  ?   ; CPI average value
statCpiMedian    dq  ?   ; CPI median value, detected by numbers ordering
statCpiSum       dq  ?   ; Service parameter: values sum for averaging CPI
; Statistics for nsPI (Nanoseconds per Instruction)
statNspiMin      dq  ?
statNspiMax      dq  ?
statNspiAverage  dq  ?
statNspiMedian   dq  ?
statNspiSum      dq  ?
; Statistics for MBPS (Megabytes per Second)
; Note Min/Max here swapped, because min. time means max. bandwidth   
statMbpsMax      dq  ?
statMbpsMin      dq  ?
statMbpsAverage  dq  ?
statMbpsMedian   dq  ?
statMbpsSum      dq  ?
; Array of measurements results, double precision floating point, 64-bit, [delta TSC]
; under measurement ordered for median, remember it before get value for drawings  
measureArray     dq  DRAW_POINTS_COUNT dup (?)
ends
align 8
DRAW_THREAD_PARMS DTHP ?
;---------- Variables for drawings GUI window management ----------------------;
struct GUIPARMS
childWinHandle   dq  ?    ; Handle for drawings window, used for revisual
silentMode       db  ?    ; Silent mode flag, 1 = Slow screen refresh
childWinRunning  db  ?
ends
align 8
GUI_PARMS GUIPARMS ?
;---------- Variables for graphics controller context -------------------------; 
; Video output control
struct GCPARMS
handleMemDC      dq  ?          ; Handle for Device Context, video controller
bitmapPointer    dq  ?          ; Bitmap pointer
handleBitmap     dq  ?          ; Handle of bitmap for graphics draw
handlesBrushes   dq  4 dup (?)  ; Handle for color brushes
handleFont       dq  ?          ; Handle for font in the drawings window
handleDC         dq  ?          ; Handle Graphical Device Context
ends
align 8
GC_PARMS GCPARMS ?
;---------- Variables for graphics output -------------------------------------;
PAINT_STRUCT     PAINTSTRUCT   ; Paint control
GRAPH_RECT       RECT          ; Rectangle definition for visualized area
;---------- Structure for build clks/mbps table texts -------------------------;
align 8
CLKS_MBPS_TEXTS  dq  DRAW_TABLE_COUNT dup (?)
;---------- Structure for build Bytes/KB/MB/GB/TB/MBPS/nanoseconds texts ------;
struct UNITSTEXTS
bytes            dq  ?
kb               dq  ?
mb               dq  ?
gb               dq  ?
tb               dq  ?
mbps             dq  ?
nanoseconds      dq  ?
ends
UNITS_TEXTS UNITSTEXTS ?
;---------- String for write TSC frequency at drawings window -----------------;
DRAW_TSC       db  DRAW_TSC_STRING_SIZE dup (?)
DRAW_TSC_VALUE =   DRAW_TSC + TSC_VALUE_OFFSET
;---------- Pointers to dynamically allocated memory --------------------------;
struct ALLOCATOR  ; Allocator for data block with variable base address and size
objectStart      dq ?
objectStop       dq ?
ends
struct DYNAPTR
listTopology     ALLOCATOR ?
listTopologyEx   ALLOCATOR ?
listNuma         ALLOCATOR ?
listGroup        ALLOCATOR ?
listAcpi         ALLOCATOR ?
listAffCpuid     ALLOCATOR ?
; This 11 pointers MUST BE SEQUENTAL, because accessed in the cycle
textOs           ALLOCATOR ?
textNativeOs     ALLOCATOR ?
textTopology1    ALLOCATOR ?
textTopology2    ALLOCATOR ?
textTopologyEx1  ALLOCATOR ?
textTopologyEx2  ALLOCATOR ?
textNuma         ALLOCATOR ?
textGroup        ALLOCATOR ?
textAcpi1        ALLOCATOR ?
textAcpi2        ALLOCATOR ?
textAffCpuid     ALLOCATOR ?
; end of sequental group of pointers
ends
align 8
DYNA_PTR DYNAPTR ?
;------------------------------------------------------------------------------;
;                                                                              ;
;                              Import section.                                 ;        
;                                                                              ;
;------------------------------------------------------------------------------;
section '.idata' import data readable writeable
library kernel32 , 'kernel32.dll' , \
        advapi32 , 'advapi32.dll' , \
        user32   , 'user32.dll'   , \
        comctl32 , 'comctl32.dll' , \
        gdi32    , 'gdi32.dll' 
include 'api\kernel32.inc'
include 'api\advapi32.inc'
include 'api\user32.inc'
include 'api\comctl32.inc'
include 'api\gdi32.inc'
;------------------------------------------------------------------------------;
;                                                                              ;
;                            Resources section.                                ;        
;                                                                              ;
;------------------------------------------------------------------------------;
section '.rsrc' resource data readable
directory RT_ICON       , icons     , \
          RT_GROUP_ICON , gicons    , \
          RT_MANIFEST   , manifests , \
          RT_VERSION    , version
;---------- Icons resource ----------------------------------------------------;
resource icons  , ID_EXE_ICON  , LANG_NEUTRAL , exeicon
resource gicons , ID_EXE_ICONS , LANG_NEUTRAL , exegicon
icon exegicon, exeicon, 'images\fasm64.ico'
;---------- Manifest resource -------------------------------------------------;
resource manifests, 1, LANG_NEUTRAL, manifest
resdata manifest
db '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
db '<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">'
db '<assemblyIdentity'
db '    name="NCRB"'
db '    processorArchitecture="amd64"'
db '    version="1.0.0.0"'
db '    type="win32"/>'
db '<description>NCRB</description>'
db '<dependency>'
db '    <dependentAssembly>'
db '        <assemblyIdentity'
db '           type="win32"'
db '           name="Microsoft.Windows.Common-Controls"'
db '           version="6.0.0.0"'
db '           processorArchitecture="amd64"'
db '           publicKeyToken="6595b64144ccf1df"'
db '           language="*"'
db '        />'
db '     </dependentAssembly>'
db '  </dependency>'
db '</assembly>'
endres
;---------- Version resource --------------------------------------------------;
resource     version, 1, LANG_NEUTRAL, version_info
versioninfo  version_info, \ 
             VOS__WINDOWS32, VFT_DLL, VFT2_UNKNOWN, LANG_NEUTRAL, 0, \
'FileDescription' , RESOURCE_DESCRIPTION ,\
'FileVersion'     , RESOURCE_VERSION     ,\
'CompanyName'     , RESOURCE_COMPANY     ,\
'LegalCopyright'  , RESOURCE_COPYRIGHT
