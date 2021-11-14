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
; NCRB32.ASM = source file for FASM                                                                       ; 
; NCRB32.EXE = translation result, application NCRB32.EXE main module                                     ;
; See also other components:                                                                              ;
; NCRB64.ASM, DATA.ASM, KMD32.ASM, KMD64.ASM.                                                             ;
;                                                                                                         ;
; Translation by Flat Assembler version 1.73.27 ( Jan 27, 2021 ).                                         ;
; http://flatassembler.net/                                                                               ;
;                                                                                                         ;
; Edit by FASM Editor 2.0.                                                                                ; 
; Use this editor for correct source file tabulations and format. (!)                                     ;
; https://fasmworld.ru/instrumenty/fasm-editor-2-0/                                                       ;
;                                                                                                         ;
; User mode debug by OllyDbg ( 32-bit, actual for this module NCRB32.EXE )                                ;
; http://www.ollydbg.de/version2.html                                                                     ;
;                                                                                                         ;
; User mode debug by FDBG ( 64-bit, actual for other module NCRB64.EXE )                                  ;
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
include 'win32a.inc'               ; FASM definitions
include 'data\data.inc'            ; NCRB project global definitions
;---------- Global application and version description definitions ------------;
RESOURCE_DESCRIPTION    EQU 'NCRB Win32 edition ( UNDER CONSTRUCTION )'
RESOURCE_VERSION        EQU '2.0.7.0'
RESOURCE_COMPANY        EQU 'https://github.com/manusov'
RESOURCE_COPYRIGHT      EQU '(C) 2021 Ilya Manusov'
PROGRAM_NAME_TEXT       EQU 'NUMA CPU&RAM Benchmarks for Win32 ( UNDER CONSTRUCTION )'
ABOUT_CAP_TEXT          EQU 'Program info'
ABOUT_TEXT_1            EQU 'NUMA CPU&RAM Benchmarks'
ABOUT_TEXT_2            EQU 'v2.00.07 for Windows ia32 ( UNDER CONSTRUCTION )'
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
format PE GUI 4.0
entry start
section '.code' code readable executable
start:
;---------- Application entry point, memory allocation for registry -----------;
cld
push PAGE_READWRITE                 ; Parm#4 = memory protection 
push MEM_COMMIT + MEM_RESERVE       ; Parm#3 = allocation type 
push ALLOCATE_MEMORY_SIZE           ; Parm#2 = required block size
push 0                              ; Parm#1 = fixed address, not used = 0
call [VirtualAlloc]
test eax,eax
jz .memoryAllocError                ; Go if memory allocation error
mov [APP_MEMORY],eax         
;---------- Start GUI initialization ------------------------------------------;
lea ebx,[APP_DATA]
push APP_CTRL                       ; Parm#1 = Pointer to structure
call [InitCommonControlsEx]         ; GUI initialization
test eax,eax
jz .initFailed                      ; Go if initialization error detected
;---------- Load resources DLL ------------------------------------------------;
push LOAD_LIBRARY_AS_DATAFILE       ; Parm#3 = Load options, flags
push 0                              ; Parm#2 = Handle, reserved = 0
push NAME_DATA                      ; Parm#1 = Pointer to file name
call [LoadLibraryEx]                ; Load resources DLL
test eax,eax                       
jz .loadFailed                      ; Go if load resources DLL error
mov [ebx + APPDATA.hResources],eax  ; Store resources DLL handle
;---------- Get handle of this application exe file ---------------------------;
push 0                             ; Parm#1 = 0 = means this exe file
call [GetModuleHandle]             ; Get handle of this exe file
test eax,eax
jz .thisFailed                     ; Go if this module handle = NULL
mov [ebx + APPDATA.hInstance],eax  ; Store handle of current module ( exe file ) 
;---------- Get handle of this application icon -------------------------------;
push ID_EXE_ICONS                  ; Parm#2 = Resource ID
push eax                           ; Parm#1 = Module handle for resource  
call [LoadIcon]                    ; Load application icon, from this exe file
test eax,eax
jz .iconFailed                     ; Go if load error, icon handle = NULL
mov [ebx + APPDATA.hIcon],eax      ; Store handle of application icon
;---------- Get handles and address pointers to tabs icons at resources DLL ---; 
mov ebp,ICON_FIRST                  ; EBP = Icons identifiers
lea edi,[ebx + APPDATA.lockedIcons] ; EDI = Pointer to icons pointers list
mov esi,ICON_COUNT                  ; ESI = Number of loaded icons
;---------- Cycle for load icons from resource DLL ----------------------------;
.loadIcons:
push RT_GROUP_ICON                 ; Parm#3 = Resource type
push ebp                           ; Parm#2 = Resource name, used numeric ID
push [ebx + APPDATA.hResources]    ; Parm#1 = Module handle, this load from DLL
call [FindResource]                ; Find resource, get handle of block
test eax,eax                       ; EAX = HRSRC, handle of resource block
jz .iconsPoolFailed                ; Go if handle = NULL, means error
push eax                           ; Parm#2 = Resource name, used numeric ID
push [ebx + APPDATA.hResources]    ; Parm#1 = Module handle, this load from DLL
call [LoadResource]                ; Load resource, get resource handle  
test eax,eax                       ; EAX = HGLOBAL, handle of resource data
jz .iconsPoolFailed                ; Go if handle = NULL, means error
push eax                           ; Parm#1 = Resource handle
call [LockResource]                ; Lock resource, get address pointer
test eax,eax                       ; EAX = LPVOID, pointer to resource
jz .iconsPoolFailed                ; Go if pointer = NULL, means error
stosd                              ; Store pointer to icon                 
inc ebp                            ; EBP = Next icon
dec esi                            ; ESI = Cycle counter  
jnz .loadIcons                     ; Cycle for initializing all icons 
;---------- Get handle and address pointer to raw pools at resources DLL ------;
; Strings located at raw resources part, for compact encoding 1 byte per char,
; note standard string resource use 2 byte per char (UNICODE). 
; Binders located at raw resources part,
; note binders script used for interconnect GUI and System Information routines. 
lea esi,[RAW_LIST]
lea edi,[ebx + APPDATA.lockedStrings]
.loadRaw:
lodsw
movzx ecx,ax
jecxz .endRaw
push RT_RCDATA                     ; Parm#3 = Resource type
push ecx                           ; Parm#2 = Resource name, used numeric ID
push [ebx + APPDATA.hResources]    ; Parm#1 = Module handle, this load from DLL 
call [FindResource]                ; Find resource, get handle of block
test eax,eax                       ; EAX = HRSRC, handle of resource block
jz .rawResourceFailed              ; Go if handle = NULL, means error
push eax                           ; Parm#2 = Handle of resource block  
push [ebx + APPDATA.hResources]    ; Parm#1 = Module handle, this load from DLL
call [LoadResource]                ; Load resource, get resource handle 
test eax,eax                       ; EAX = HGLOBAL, handle of resource data
jz .rawResourceFailed              ; Go if handle = NULL, means error
push eax                           ; Parm#1 = Resource handle
call [LockResource]                ; Lock resource, get address pointer 
test eax,eax                       ; EAX = LPVOID, pointer to resource
jz .rawResourceFailed              ; Go if pointer = NULL, means error
stosd                              ; Store pointer to binders pool
jmp .loadRaw
.endRaw:
;---------- Create fonts ------------------------------------------------------;
mov esi,[ebx + APPDATA.lockedFontList]
lea edi,[ebx + APPDATA.hFont1]
.createFonts:
xor eax,eax
movzx ecx,word [esi + 00]
jecxz .doneFonts
lea edx,word [esi + 16]
push edx
movzx edx,word [esi + 14]
push edx
movzx edx,word [esi + 12]
push edx
movzx edx,word [esi + 10]
push edx
movzx edx,word [esi + 08]
push edx
movzx edx,word [esi + 06]
push edx
push eax
push eax
push eax
movzx edx,word [esi + 04]
push edx
push eax
push eax
push eax
push ecx
call [CreateFont]
test eax,eax
jz .createFontFailed
stosd
add esi,16
@@:
lodsb
cmp al,0
jne @b
jmp .createFonts
.doneFonts:
;---------- Pre-load strings for fast build clks/mbps table texts -------------;
lea edi,[CLKS_MBPS_TEXTS]
mov bp,DRAW_TABLE_COUNT
mov dx,DRAW_TABLE_FIRST_TEXT
@@:
mov eax,edx
mov esi,[APP_DATA.lockedStrings]
call IndexString
xchg eax,esi
stosd
inc edx
dec bp
jnz @b
;--- Pre-load strings for fast build Bytes/KB/MB/GB/TB/MBPS/nanoseconds -------;
lea edi,[UNITS_TEXTS]
mov bp,UNITS_COUNT
mov dx,UNITS_FIRST_TEXT 
@@:
mov eax,edx
mov esi,[APP_DATA.lockedStrings]
call IndexString
xchg eax,esi
stosd
inc edx
dec bp
jnz @b
;---------- Pre-load string for write TSC frequency at drawings window --------;
lea edi,[DRAW_TSC]
mov ax,STR_MD_TSC_CLOCK_MHZ
call PoolStringWrite
;---------- Load configuration file ncrb.inf ----------------------------------; 
; TODO.
;---------- Get system information, user mode routines ------------------------;
call SystemInfo
jc .errorPlatform
;---------- Load kernel mode driver kmd32.sys (Win32) or kmd64.sys (Win64) ----;
; TODO.
; call LoadKernelModeDriver
; call TryKernelModeDriver
; call UnloadKernelModeDriver
;---------- Check dynamical import results, show missing WinAPI warning -------;
; Application can start with this non-fatal warning.
lea edi,[TEMP_BUFFER]
push esi edi
mov edx,esi
mov ax,STR_WARNING_API
call PoolStringWrite 
mov esi,[ebx + APPDATA.lockedImportList]
lea edx,[DYNA_IMPORT]
xor ebp,ebp
.checkImport:
cmp byte [esi],0
je .doneCheckImport
cmp dword [edx],0
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
cmp byte [esi],0
jne .nextImport
inc esi
.nextImport:
add edx,4
jmp .checkImport
.doneCheckImport:
mov al,0
stosb
pop edi esi
test ebp,ebp
jz .doneImport 
push MB_ICONWARNING    ; Parm#4 = Message box icon type
push PROGRAM_NAME      ; Parm#3 = Pointer to caption
push edi               ; Parm#2 = Pointer to string
push 0                 ; Parm#1 = Parent window handle or 0
call [MessageBoxA]
.doneImport:
;---------- Check WoW64 mode (NCRB32 under Win64) and show WoW64 warning ------;
; Application can start with this non-fatal warning.
cmp dword [OS_DATA.isWow64],0
je .doneWow64
mov esi,[APP_DATA.lockedStrings]    ; ESI = Strings pool base 
mov ax,STR_WARNING_WOW
call IndexString       ; Return ESI = Selected string address, warning WoW64 
push MB_ICONWARNING    ; Parm#4 = Message box icon type
push PROGRAM_NAME      ; Parm#3 = Pointer to caption
push esi               ; Parm#2 = Pointer to string
push 0                 ; Parm#1 = Parent window handle or 0
call [MessageBoxA]
.doneWow64:
;---------- Create and show main dialogue window ------------------------------;
push 0                          ; Parm#5 = Pass value
push DialogProcMain             ; Parm#4 = Pointer to dialogue procedure
push HWND_DESKTOP               ; Parm#3 = Owner window handle
push IDD_MAIN                   ; Parm#2 = Resource ( template ) id
push [ebx + APPDATA.hResources] ; Parm#1 = Handle of resource module
call [DialogBoxParam]           ; Create modal dialogue 
test eax,eax
jz .dialogueFailed              ; Go if create dialogue return error 
cmp eax,-1                     
je .dialogueFailed              ; Go if create dialogue return error
;---------- Application exit point with release resource ----------------------; 
xor ebp,ebp                     ; EBP = Exit Code, 0 means no errors
.exitResources:
mov esi,[APP_MEMORY]
test esi,esi
jz .exit
lea ebx,[APP_DATA]
;---------- Delete created fonts ----------------------------------------------;
push esi
mov esi,[ebx + APPDATA.lockedFontList]
test esi,esi
jz .doneDeleteFonts 
lea edi,[ebx + APPDATA.hFont1]
.deleteFonts:
lodsw
test ax,ax
jz .doneDeleteFonts
mov ecx,[edi]
add edi,4
jecxz .skipDelete
push ecx
call [DeleteObject]
.skipDelete:
add esi,14
@@:
lodsb
cmp al,0
jnz @b 
jmp .deleteFonts
.doneDeleteFonts:
pop esi
;---------- Unload resource library -------------------------------------------;
mov ecx,[ebx + APPDATA.hResources]  ; ECX = Library DATA.DLL handle
jecxz .skipUnload                   ; Go skip unload if handle = null
push ecx
call [FreeLibrary]                  ; Unload DATA.DLL
.skipUnload:
;---------- Release memory ----------------------------------------------------;
push MEM_RELEASE                ; Parm#3 = Memory free operation type
push 0                          ; Parm#2 = Size, 0 = by allocated
push esi                        ; Parm#1 = Memory block base address  
call [VirtualFree]              ; Release memory, allocated for registry
;---------- Exit --------------------------------------------------------------;
.exit:                          
push ebp                        ; Parm#1 = exit code           
call [ExitProcess]
;---------- This entry points used if application start failed ----------------;
.dialogueFailed:
mov al,MSG_DIALOGUE_FAILED       ; AL = String pool index for error name
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
lea esi,[MSG_ERRORS]   ; ESI = Strings pool base, AL = String index 
mov ah,0
.errorEntry:
call IndexString       ; Return ESI = Selected string address 
push MB_ICONERROR      ; Parm#4 = Attributes
push PROGRAM_NAME      ; Parm#3 = Pointer to title (caption) string
push esi               ; Parm#2 = Pointer to string: error name 
push 0                 ; Parm#1 = Parent Window = NULL
call [MessageBox]  
mov ebp,1              ; EBP = Exit Code, 1 means error occurred
jmp .exitResources
;---------- Show message box and go epplication termination -------------------;
; This procedure for incompatible platform detected but application integrity
; OK, use strings from resource DLL.
.errorPlatform:       ; Input AX = Error string ID.
mov esi,[APP_DATA.lockedStrings]
movzx eax,al
add eax,STR_ERROR_CPUID
jmp .errorEntry 
;---------- Copy text string terminated by 00h --------------------------------;
; Note last byte 00h not copied.                                               ;
;                                                                              ;
; INPUT:   ESI = Source address                                                ;
;          EDI = Destination address                                           ;
;                                                                              ;
; OUTPUT:  ESI = Modified by copy                                              ;
;          EDI = Modified by copy                                              ;
;          Memory at [Input EDI] modified                                      ;
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
; INPUT:   ESI = Pointer to string pool                                        ;
;          AX  = Index                                                         ;
;                                                                              ;
; OUTPUT:  ESI = Updated pointer to string, selected by index                  ;  
;                                                                              ;
;------------------------------------------------------------------------------;
IndexString:
cld
movzx ecx,ax
jecxz .stop
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
; OUTPUT:  ESI = Updated pointer to string, selected by index                  ;
;          EDI = Modified by copy                                              ;  
;                                                                              ;
;------------------------------------------------------------------------------;
PoolStringWrite:
mov esi,[APP_DATA.lockedStrings]
test esi,esi
jz .exit
call IndexString
call StringWrite
.exit:
ret
;---------- Print 64-bit Hex Number -------------------------------------------;
;                                                                              ;
; INPUT:  EDX:EAX = Number, EDX=High32, EAX=Low32                              ;
;         EDI = Destination Pointer                                            ;
;                                                                              ;
; OUTPUT: EDI = Modify                                                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HexPrint64:
xchg eax,edx
call HexPrint32
xchg eax,edx
; no RET, continue at next subroutine
;---------- Print 32-bit Hex Number -------------------------------------------;
;                                                                              ;
; INPUT:  EAX = Number                                                         ;
;         EDI = Destination Pointer                                            ;
;                                                                              ;
; OUTPUT: EDI = Modify                                                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HexPrint32:
push eax
ror eax,16
call HexPrint16
pop eax
; no RET, continue at next subroutine
;---------- Print 16-bit Hex Number -------------------------------------------;
;                                                                              ;
; INPUT:  AX  = Number                                                         ;
;         EDI = Destination Pointer                                            ;
;                                                                              ;
; OUTPUT: EDI = Modify                                                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HexPrint16:
push eax
xchg al,ah
call HexPrint8
pop eax
; no RET, continue at next subroutine
;---------- Print 8-bit Hex Number --------------------------------------------;
;                                                                              ;
; INPUT:  AL  = Number                                                         ;
;         EDI = Destination Pointer                                            ;
;                                                                              ;
; OUTPUT: EDI = Modify	                                                       ;
;                                                                              ;
;------------------------------------------------------------------------------;
HexPrint8:
push eax
ror al,4
call HexPrint4
pop eax
; no RET, continue at next subroutine
;---------- Print 4-bit Hex Number --------------------------------------------;
;                                                                              ;
; INPUT:  AL  = Number (bits 0-3)                                              ;
;         EDI = Destination Pointer                                            ;
;                                                                              ;
; OUTPUT: EDI = Modify                                                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
HexPrint4:
cld
push eax
and al,0Fh
add al,90h
daa
adc al,40h
daa
stosb
pop eax
ret
;---------- Print 32-bit Decimal Number ---------------------------------------;
;                                                                              ;
; INPUT:   EAX = Number value                                                  ;
;          BL  = Template size, chars. 0=No template                           ;
;          EDI = Destination Pointer (flat)                                    ;
;                                                                              ;
; OUTPUT:  EDI = New Destination Pointer (flat)                                ;
;                modified because string write                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
DecimalPrint32:
pushad
cld
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
push edx              ; Push remainder
xor edx,edx
mov eax,ecx
mov ecx,10
div ecx
mov ecx,eax          ; ECX = Quotient, used as divisor and cycle condition 
pop eax              ; EAX = remainder
inc bh
test ecx,ecx
jnz .mainCycle       ; Cycle if (unsigned) quotient still > 0 
mov [esp],edi
popad
ret
;---------- Print double precision value --------------------------------------;
; x87 FPU used, required x87 presence validation by CPUID before call this.    ;
;                                                                              ;
; INPUT:   EDX:EAX = Double precision number, EDX=High32, EAX=Low32            ;
;          BL  = Number of digits in the INTEGER part,                         ;
;                used for add left non-signed zeroes.                          ;
;                BL=0 means not print left unsigned zeroes.                    ;
;          BH  = Number of digits in the FLOAT part,                           ;
;                used as precision control.                                    ;
;          EDI = Destination text buffer pointer                               ;
;                                                                              ;
; OUTPUT:  EDI = Modified by text string write                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
DoublePrint:
pushad
cld
test eax,eax
jnz @f
mov ecx,07FFFFFFFh
and ecx,edx
jz .fp64_Zero
cmp ecx,07FF80000h
je .fp64_QNAN
cmp ecx,07FF00000h
je .fp64_INF
ja .fp64_NAN
@@:
finit
push edx eax
push eax
fstcw [esp]
pop eax
or ax,0C00h
push eax
fldcw [esp]
pop eax
fld qword [esp]
pop eax edx
fld st0
frndint
fxch
fsub st0,st1
mov eax,1
movzx ecx,bh
jecxz .divisorDone
@@:
imul eax,eax,10
loop @b
.divisorDone:
push eax
fimul dword [esp]
pop eax
sub esp,32
fbstp [esp+00]
fbstp [esp+16]
test byte [esp+16+09],80h
setnz dl
test byte [esp+00+09],80h
setnz dh
test dx,dx
jz @f
mov al,'-'
stosb
@@:
mov cx,20 
mov edx,[esp + 16 + 06]  
mov esi,[esp + 16 + 02] 
mov ebp,[esp + 16 + 00] 
shl ebp,16
and edx,07FFFFFFFh
.cycleInteger:
mov eax,edx
shr eax,28
cmp cl,1
je .store
cmp cl,bl
jbe .store
test ch,ch
jnz .store
test al,al
jz .position 
.store:
mov ch,1
or al,30h
stosb
.position:
shld edx,esi,4
shld esi,ebp,4
shl ebp,4
dec cl
jnz .cycleInteger
test bh,bh
jz .exit
mov al,'.'
stosb
std 
movzx ecx,bh     
lea edi,[edi + ecx]
mov edx,[esp + 00 + 00]  
mov esi,[esp + 00 + 04] 
mov ebp,[esp + 00 + 00] 
push edi
dec edi
.cycleFloat:
mov al,dl
and al,0Fh
or al,30h
stosb
shrd edx,esi,4
shrd esi,ebp,4
shr ebp,4
loop .cycleFloat
pop edi
cld
add esp,32
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
.Error:
mov al,'?'
stosb
.exit:
finit
mov [esp],edi
popad
ret
;---------- Print memory block size as Integer.Float --------------------------;
;                                                                              ;
; INPUT:   EDX:EAX = Number value, EDX = high 32, EAX = low 32, units = Bytes  ;
;          BL  = Force units (override as smallest only)                       ;
;                FF = No force units, auto select                              ;
;                0 = Bytes, 1 = KB, 2 = MB, 3 = GB, 4 = TB                     ;
;          EDI = Destination Pointer (flat)                                    ;
;                                                                              ;
; OUTPUT:  EDI = New Destination Pointer (flat)                                ;
;                modified because string write                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
SizePrint64:
pushad
cld
xor ecx,ecx
test eax,eax
jnz .unitsAutoCycle
test edx,edx
jz .decimalMode
xor ebp,ebp
xor esi,esi
.unitsAutoCycle:
mov ebp,eax
shrd eax,edx,10
shr edx,10
jnz .above32bit 
cmp cl,bl
je .modNonZero
xor esi,esi
shrd esi,ebp,10
shr esi,22
cmp bl,0FFh
jne .above32bit 
test esi,esi
jnz .modNonZero
.above32bit:                
inc ecx
jmp .unitsAutoCycle
.modNonZero:
cmp ecx,4
ja .hexMode
mov eax,ebp
.decimalMode:
push ebx
mov bl,0
call DecimalPrint32
pop ebx
jecxz .afterNumber
cmp bl,0FFh
je .afterNumber
mov al,'.'
stosb
xchg eax,esi
xor edx,edx
mov ebx,102
div ebx
cmp eax,9
jbe .limitDecimal
mov eax,9
.limitDecimal:
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
mov [esp],edi
popad
ret
;---------- Execute binder in the binders pool by index -----------------------;
;                                                                              ;
; INPUT:   EBX = Current window handle for get dialogue items                  ;
;          AX  = Binder index in the binders pool                              ;
;                                                                              ;
; OUTPUT:  None                                                                ;  
;                                                                              ;
;------------------------------------------------------------------------------;
Binder:
push ebx esi edi ebp
cld
mov esi,[APP_DATA.lockedBinders]
movzx ecx,ax
jecxz .found          ; Go if selected first binder, index = 0
.find:
lodsb
add esi,3             ; 3 bytes can not exist, skip (not read by LODSD) it
test al,00111111b
jnz .find             ; Go  continue scan if list terminator not found
sub esi,3             ; Terminator opcode is single-byte, return pointer back
loop .find            ; Cycle for skip required number of binders
.found:               ; Start execute selected binder
lodsd
test al,00111111b
jz .stop
mov edx,eax
mov ecx,eax
shr eax,6
and eax,00001FFFh     ; EAX = first 13-bit parameter
shr edx,6+13
and edx,00001FFFh     ; EDX = second 13-bit parameter
and ecx,00111111b
push esi edi
call [PROC_BINDERS + ecx * 4 - 4]  ; call by ECX = Binder index
pop edi esi
jmp .found            ; cycle for next instruction of binder
.stop:
pop ebp edi esi ebx
ret
;---------- Script handler: bind indexed string from pool to GUI object -------;
BindSetString:        ; EAX = String ID, EDX = Resource ID for GUI item
mov esi,[APP_DATA.lockedStrings]
call IndexString      ; Return ESI = Pointer to selected string
BindEntry:
push edx              ; Parm#2 = Resource ID for GUI item 
push ebx              ; Parm#1 = Parent window handle  
call [GetDlgItem]     ; Return handle of GUI item
test eax,eax
jz .exit              ; Go skip if error, item not found
push esi              ; Parm#4 = lParam = Pointer to string
push 0                ; Parm#3 = wParam = Not used
push WM_SETTEXT       ; Parm#2 = Msg
push eax              ; Parm#1 = hWnd 
call [SendMessage]    ; Set string for GUI item
.exit:
ret
;---------- Script handler: bind string from temporary buffer to GUI object ---;
BindSetInfo:          ; EAX = String offset, EDX = Resource ID for GUI item
lea esi,[BIND_LIST + eax]
jmp BindEntry
;---------- Script handler: bind string referenced by pointer to GUI object ---;
BindSetPtr:           ; EAX = String pointer, EDX = Resource ID for GUI item
lea esi,[BIND_LIST + eax]
mov esi,[esi]
test esi,esi
jnz BindEntry
ret
;---------- Script handler: enable or disable GUI object by binary flag -------;
BindSetBool:          ; EAX = Variable offset, EDX = Resource ID for GUI item
mov ecx,eax
shr eax,3
and ecx,0111b
lea esi,[BIND_LIST]
movzx eax,byte [esi + eax]
bt eax,ecx
setc al
xchg esi,eax
push edx              ; Parm#2 = Resource ID for GUI item 
push ebx              ; Parm#1 = Parent window handle  
call [GetDlgItem]     ; Return handle of GUI item
test eax,eax
jz .exit              ; Go skip if error, item not found
push esi
push eax 
call [EnableWindow]
.exit:
ret
;---------- Script handler: set GUI object enable and checked states ----------l
; This handler use 2-bit field: enable flag and state flag.
BindSetSwitch:     ; EAX = Variable offset:bit, EDX = Resource ID for GUI item
mov ecx,eax
shr eax,3
and ecx,0111b
lea esi,[BIND_LIST]
movzx eax,byte [esi + eax]
xor esi,esi
bt eax,ecx
rcl esi,1
inc ecx
bt eax,ecx
rcl esi,1           ; Bits ESI.0 = value , ESI.1 = enable
push edx            ; Parm#2 = Resource ID for GUI item 
push ebx            ; Parm#1 = Parent window handle  
call [GetDlgItem]   ; Return handle of GUI item
test eax,eax
jz .exit            ; Go skip if error, item not found
xchg edi,eax
mov edx,esi
shr edx,1           
push edx            ; Parm#2 = Data, 0 = disable , 1 = enable
push edi            ; Parm#1 = hWnd 
call [EnableWindow]
push 0              ; Parm#4 = lParam = not used
and esi,1
push esi            ; Parm#3 = wParam: BST_CHECKED = 1, BST_UNCHECKED = 0
push BM_SETCHECK    ; Parm#2 = Msg
push edi            ; Parm#1 = hWnd 
call [SendMessage]  ; Set string for GUI item
.exit:
ret 
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
lea esi,[BIND_LIST]
lea edi,[TEMP_BUFFER]
add esi,eax
push ebx edx edi
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
lodsd
xchg eax,edx
lodsd
xchg eax,edx
call HexPrint64
.printDone:
mov al,0
stosb
pop edi edx ebx
push edx            ; Parm#2 = Resource ID for GUI item 
push ebx            ; Parm#1 = Parent window handle  
call [GetDlgItem]   ; Return handle of GUI item
test eax,eax
jz .exit            ; Go skip if error, item not found
push edi
push 0
push WM_SETTEXT
push eax
call [SendMessage]
.exit:
ret
;---------- Script handler: operations with combo box -------------------------;
BindSetCombo:         ; EAX = Combo offset, EDX = Resource ID for GUI item
lea esi,[BIND_LIST + eax]  ; ESI = Pointer to combo description list
push edx              ; Parm#2 = Resource ID for GUI item 
push ebx              ; Parm#1 = Parent window handle  
call [GetDlgItem]     ; Return handle of GUI item
test eax,eax          ; EAX = Handle of combo box
jz .exit              ; Go skip if error, item not found
xchg edi,eax          ; EDI = Handle of combo box
mov ebp,0FFFF0000h    ; EBP = Store:Counter for selected item
.scan:
lodsb                 ; AL = Tag from combo description list 
movzx eax,al
call [PROC_COMBO + eax * 4]    ; Call handler = F(tag)
inc ebp
jnc .scan             ; Continue if end tag yet not found
shr ebp,16
cmp bp,0FFFFh
je .exit
push 0                ; Parm#4 = lParam = Not used 
push ebp              ; Parm#3 = wParam = Selected item, 0-based 
push CB_SETCURSEL     ; Parm#2 = Msg
push edi              ; Parm#1 = hWnd 
call [SendMessage]    ; Set string for GUI item
.exit:
ret
BindComboStopOn:      ; End of list, combo box enabled
stc
ret
BindComboStopOff:     ; End of list, combo box disabled (gray) 
stc
ret
BindComboCurrent:     ; Add item to list as current selected
call HelperBindCombo
shl ebp,16
clc
ret
BindComboAdd:         ; Add item to list
call HelperBindCombo
clc
ret
BindComboInactive:    ; Add item to list as inactive (gray)
clc
ret
;---------- Script handler: bind font from registry to GUI object -------------;
BindSetFont:          ; EAX = Font number, EDX = Resource ID for GUI item
lea esi,[APP_DATA.hFont1]
mov esi,[esi + eax * 4]
push edx              ; Parm#2 = Resource ID for GUI item 
push ebx              ; Parm#1 = Parent window handle  
call [GetDlgItem]     ; Return handle of GUI item
test eax,eax
jz .exit              ; Go skip if error, item not found
push 1                ; Parm#4 = lParam = redraw flag
push esi              ; Parm#3 = wParam = handle
push WM_SETFONT       ; Parm#2 = Msg
push eax              ; Parm#1 = hWnd
call [SendMessage]
.exit:
ret
;--- Script handler: get state of GUI object and write to bit at buffer -------;
BindGetSwitch:        ; EAX = Variable offs:bit, EDX = Resource ID for GUI item
xchg esi,eax          ; ESI = Destination variable address and bit position
push edx              ; Parm#2 = ID
push ebx              ; Parm#1 = Parent window handle  
call [GetDlgItem]     ; Return handle of GUI item
test eax,eax
jz .exit              ; Go skip if error, item not found
push 0                ; Parm#4 = Not used = 0
push 0                ; Parm#3 = Not used = 0
push BM_GETCHECK      ; Parm#2 = Message
push eax              ; Parm#1 = Handle                     
call [SendMessage]    ; Return RAX = widget state
cmp eax,1             ; Compare with BST_CHECKED = 1, set ZF = 1 if this
pushf
mov ecx,esi
shr esi,3
and ecx,0111b
add esi,BIND_LIST
popf
je .setBit
mov al,11111110b
rol al,cl
and [esi],al 
.exit:
ret
.setBit:
mov al,00000001b
rol al,cl
or [esi],al 
ret
;---------- Script handlers: get decimal and hex number edit field ------------;
BindGetDec32:
xor ebp,ebp
jmp BindGetNumberEntry
BindGetHex32:
mov bp,1
jmp BindGetNumberEntry
BindGetHex64:
mov bp,2
BindGetNumberEntry:
lea esi,[TEMP_BUFFER]
lea edi,[BIND_LIST + eax]
push edx           ; Parm#2 = Resource ID for GUI item 
push ebx           ; Parm#1 = Parent window handle  
call [GetDlgItem]  ; Return handle of GUI item
test eax,eax
jz .exit           ; Go skip if error, item not found
push esi
push 17
push WM_GETTEXT
push eax
call [SendMessage]
test eax,eax
jz .exit           ; Go skip if error, can't read string
test bp,bp
jz .parseDec
xor ecx,ecx
xor edx,edx
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
shld edx,ecx,4
shl ecx,4
or ecx,eax
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
xchg eax,ecx
cmp bp,2
jb .store32
stosd
xchg eax,edx
.store32:
stosd
.exit:
ret
;---------- Helper for add string to combo box list ---------------------------;
;                                                                              ;
; INPUT:   ESI = Pointer to binder script                                      ;
;          EDI = Parent window handle                                          ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperBindCombo:
lodsw
push esi
movzx eax,ax
mov esi,[APP_DATA.lockedStrings]
call IndexString      ; Return ESI = Pointer to selected string
push esi              ; Parm#4 = lParam = Pointer to string
push 0                ; Parm#3 = wParam = Not used
push CB_ADDSTRING     ; Parm#2 = Msg
push edi              ; Parm#1 = hWnd 
call [SendMessage]    ; Set string for GUI item
pop esi
ret
;---------- Include subroutines from modules ----------------------------------;
include 'ncrb32\dialogs\connect_code.inc'
include 'ncrb32\system_info\connect_code.inc'
include 'ncrb32\threads_manager\connect_code.inc'
include 'ncrb32\memory_bandwidth_temporal\connect_code.inc'
include 'ncrb32\memory_bandwidth_non_temporal\connect_code.inc'
include 'ncrb32\memory_latency\connect_code.inc'
include 'ncrb32\math_bandwidth\connect_code.inc'
;------------------------------------------------------------------------------;
;                                                                              ;
;                              Data section.                                   ;        
;                                                                              ;
;------------------------------------------------------------------------------;
section '.data' data readable writeable
;---------- Common data for application ---------------------------------------;
APP_MEMORY     DD  0                        ; Must be 0 for conditional release
APP_CTRL       INITCOMMONCONTROLSEX  8, 0   ; Structure for initialization
;---------- Pointers to procedures of GUI bind scripts interpreter ------------;
PROC_BINDERS   DD  BindSetString
               DD  BindSetInfo
               DD  BindSetPtr
               DD  BindSetBool
               DD  BindSetSwitch
               DD  BindSetDec32
               DD  BindSetHex32
               DD  BindSetHex64              
               DD  BindSetCombo
               DD  BindSetFont
               DD  BindGetSwitch
               DD  BindGetDec32
               DD  BindGetHex32
               DD  BindGetHex64              
PROC_COMBO     DD  BindComboStopOn     ; End of list, combo box enabled
               DD  BindComboStopOff    ; End of list, combo box disabled (gray) 
               DD  BindComboCurrent    ; Add item to list as current selected
               DD  BindComboAdd        ; Add item to list
               DD  BindComboInactive   ; Add item to list as inactive (gray)
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
PROGRAM_NAME   DB  PROGRAM_NAME_TEXT  , 0
ABOUT_CAP      DB  ABOUT_CAP_TEXT     , 0
ABOUT_NAME     DB  ABOUT_TEXT_1       , 0Dh,0Ah
               DB  ABOUT_TEXT_2       , 0Dh,0Ah
               DB  ABOUT_TEXT_3       , 0Dh,0Ah, 0
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
include 'ncrb32\dialogs\connect_data.inc'
include 'ncrb32\system_info\connect_data.inc'
include 'ncrb32\threads_manager\connect_data.inc'
include 'ncrb32\memory_bandwidth_temporal\connect_data.inc'
include 'ncrb32\memory_bandwidth_non_temporal\connect_data.inc'
include 'ncrb32\memory_latency\connect_data.inc'
include 'ncrb32\math_bandwidth\connect_data.inc'
;---------- Operating system definitions --------------------------------------;
ERROR_BUFFER_LIMIT  = 07Ah
VALUE_BUFFER_LIMIT  = 128 * 1024 
;---------- Equations for UPB, IPB, OPB data build and interpreting -----------;
; Assembler Methods (AM) option values count and Bandwidth/Latency criteria
; TODO. Add new methods, update AM_BYTE_COUNT, LATENCY_MODE, plus see below.
AM_BYTE_COUNT       =  26     ; 26 bytes per methods primary list
LATENCY_MODE        =  24     ; modes ID = 24, 25 for latency measurement.  TODO. USE TRANSLATION FOR TEMPORAL/NONTEMPORAL, THIS VALUE = 24*2=48.
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
; for save EXE file space. 
align 4096
TEMP_SIZE           EQU  48 * 1024  
TEMP_BUFFER         DQ   TEMP_SIZE dup (?)
;---------- Threads and memory management definitions -------------------------;
; Thread control entry, or entire benchmark control if single thread
; Note keep 64 bytes per entry, see ThreadEntry, can be non parametrized coding
; Note keep all pairs: Affinity Mask + Affinity Group as 2 sequental qwords,
; this rule required because WinAPI use direct output store to this qwords. 
struct THCTRL
eventStart      dd  ?   ; Event Handle for operation START signal
eventDone       dd  ?   ; Event Handle for operation COMPLETE signal 
threadHandle    dd  ?   ; Thread Handle for execution thread
threadAffinity  dd  ?   ; Affinity Mask = F (True Affinity Mask, Options) 
threadGroup     dd  ?   ; Processor group, associated with affinity mask
entryPoint      dd  ?   ; Entry point to operation subroutine
base1           dd  ?   ; Source for read, destination for write and modify
base2           dd  ?   ; Destination for copy
sizeBytes       dd  ?   ; Block size, units = bytes, for benchmarking, not for memory allocation (REDUNDANT FIELD RESERVED) 
sizeInst        dd  ?   ; Block size, units = instructions, for benchmarking (MUST BE UPDATED PER DRAW ITERATION)
repeats         dq  ?   ; Number of measurement repeats for precision requirements
largePages      dw  ?   ; 0=not supported, 1=supported but not used, 2=used (REDUNDANT FIELD RESERVED)
affinityMode    dw  ?   ; Affinity mode: 0=None, 1=Without groups, 2=With groups 
origBase        dd  ?   ; True (before alignment) memory block base for release (REDUNDANT FIELD RESERVED)
origAffinity    dd  ?   ; Store original affinity mask, also storage for return (STORED BY THREAD, NOT BY INITIALIZER) 
origGroup       dd  ?   ; Store processor group for original offinity mask (STORED BY THREAD, NOT BY INITIALIZER) 
ends
MAX_THREADS            =  256   ; Maximum number of supported threads: total and per processor group
MAX_THREADS_PER_GROUP  =  64
; Thread entry size = 64 bytes, const because optimal shift by 6 can be used
; 16384 bytes, 256 entries * 64 bytes, used for (multi) thread management
THCTRL_SIZE            =  sizeof.THCTRL
THREAD_LIST_SIZE       =  MAX_THREADS * THCTRL_SIZE
THREAD_LIST            DB  THREAD_LIST_SIZE  dup (?)  ; THCTRL  MAX_THREADS dup (?)
;---------- Event management definitions --------------------------------------;
; Event handle size = 4 bytes (32-bit)
; 1024 bytes, 256 handles * 4 bytes, 
; separate sequental list req. for WaitForMultipleObjects function  
EVENT_SIZE             =  4
EVENT_LIST_SIZE        =  MAX_THREADS * EVENT_SIZE
; Events lists must be sequental after thread list, because relative addressing.
START_EVENTS           DB  EVENT_LIST_SIZE   dup ?  
DONE_EVENTS            DB  EVENT_LIST_SIZE   dup ?  
;---------- NUMA topology management definitions ------------------------------;
; NUMA node description entry, not a same as thread description enrty
; Note keep 32 bytes per entry, see ThreadEntry, can be non parametrized coding
; Note keep all pairs: Affinity Mask + Affinity Group as 2 sequental qwords,
; this rule required because WinAPI use with direct output store to this qwords 
struct NUMACTRL
nodeID          dd  ?   ; NUMA Node number, if no NUMA, this field = 0 for all entries
nodeAffinity    dd  ?   ; NUMA Node affinity mask
nodeGroup       dd  ?   ; Processors Group number, 0 if single group platform
alignedBase     dd  ?   ; Block size after alignment, for read/write operation
alignedSize     dd  ?   ; Block base address after alignment, for r/w operation 
trueBase        dd  ?   ; Base address, returned when allocated, for release, 0=not used 
reserved        dd  2 dup ?    ; Reserved for alignment
ends
MAX_NODES              =  64  ; Maximum number of supported NUMA nodes
; NUMA node entry size = 32 bytes, const because optimal shift by 5 can be used
; 8192 bytes, 256 entries * 32 bytes,
; for NUMA nodes description, not a same as THREAD
NUMACTRL_SIZE          =  sizeof.NUMACTRL
NUMA_LIST_SIZE         =  MAX_NODES * NUMACTRL_SIZE  
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
customBlockStart  dd  ?    ; Override start block size, or 0=default
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
allocatedBlock1   dd  ?
allocatedBlock2   dd  ?
; This items generated as f( first 8 items, platform configuration )
startBlockSize    dd  ?    ; Start block size, or 0=default, unite = bytes
endBlockSize      dd  ?    ; End block size, or 0=default
deltaBlockSize    dd  ?    ; Delta block size, or 0=default
measureRepeats    dq  ?    ; Number of measurement repeats, note 64-bit value
operandWidth      dd  ?    ; Used instructions operand width, bits 
groupsCount       dd  ?    ; Number of Processor Groups
domainsCount      dd  ?    ; Number of NUMA domains
pageSize          dd  ?    ; Memory page size, bytes
; Memory allocation results
memoryTotal       dd  ?    ; Memory allocated, per all threads
memoryPerThread   dd  ?    ; Memory allocated, per each thread 
; Benchmarks select and routine dump support
patternRoutine    dd  ?    ; Pointer to target benchmark routine, this is fill routine for latency mode
walkRoutine       dd  ?    ; Second routine, required for latency measurement
dumpMethodStart   dd  ?    ; Start of fragment, visualized as dump during benchmarks
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
dtSse128read      dq  ?    ; TSC clocks per SSE128 Read pattern
dtSse128write     dq  ?    ; TSC clocks per SSE128 Write pattern
dtSse128copy      dq  ?    ; TSC clocks per SSE128 Copy pattern 
dtAvx256read      dq  ?    ; TSC clocks per AVX256 Read pattern
dtAvx256write     dq  ?    ; TSC clocks per AVX256 Write pattern
dtAvx256copy      dq  ?    ; TSC clocks per AVX256 Copy pattern 
dtAvx512read      dq  ?    ; TSC clocks per AVX512 Read pattern
dtAvx512write     dq  ?    ; TSC clocks per AVX512 Write pattern
dtAvx512copy      dq  ?    ; TSC clocks per AVX512 Copy pattern 
dtSse128sqrt      dq  ?    ; TSC clocks per SSE128 Square Root pattern
dtAvx256sqrt      dq  ?    ; TSC clocks per AVX256 Square Root pattern
dtAvx512sqrt      dq  ?    ; TSC clocks per AVX512 Square Root pattern
dtX87cos          dq  ?    ; TSC clocks per x87 Cosine (FCOS) pattern
dtX87sincos       dq  ?    ; TSC clocks per x87 Sine+Cosine (FSINCOS) pattern 
ends
align 8
VECBR_OPB VECBROPB ?
;---------- Key data for GUI application with resources -----------------------;  
struct APPDATA
hResources                 dd ?     ; Resource DLL handle
lockedStrings              dd ?     ; Pointer to strings pool
lockedBinders              dd ?     ; Pointer to binders pool
lockedDataCpuCommon        dd ?     ; Data for build common CPU feature bitmap
lockedDataCpuAvx512        dd ?     ; Data for build AVX512 feature bitmap
lockedDataOsContext        dd ?     ; Data for build OS context bitmap
lockedDataIntelCache       dd ?     ; Data for Intel cache descriptors decode
lockedDataAcpi             dd ?     ; Data base for ACPI tables detection
lockedImportList           dd ?     ; List for WinAPI dynamical import
lockedFontList             dd ?     ; List of fonts names
lockedBrushesList          dd ?     ; List of color brushes
lockedBitmapInfo           dd ?     ; Bitmap info header for draw window
hFont1                     dd ?     ; Handles of created fonts
hFont2                     dd ?
hIcon                      dd ?     ; Application icon handle
hMain                      dd ?     ; Main window handle
hTab                       dd ?     ; Sheets container handle
hImageList                 dd ?     ; Image list handle
selectedTab                dd ?                   ; Current sheet number
tabCtrlItem                TC_ITEM ?              ; Tab item data structure
lockedIcons                dd ICON_COUNT dup ?    ; Pointers to icons resources
createdIcons               dd ICON_COUNT dup ?    ; Pointers to icons
hTabDlg                    dd ITEM_COUNT dup ?    ; Sheets handles
hInstance                  dd ?                   ; This EXE file handle
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
largePageSize              dd ?
largePageEnable            dd ?
isWow64                    dd ?
ends
OS_DATA OSDATA ?
;---------- Platform topology by OS -------------------------------------------;
struct SUMMARYCACHE   ; Summary cache information: cache size and units count
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
struct SUMMARYTOPOLOGY   ; Summary topology information by OS
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
drvPath   dd ?
drvFile   dd ?               
manager   dd ?               
service   dd ?               
vectors   dd ?               
driver    dd ?                
bytes     dd ?                 
status    SERVICE_STATUS ?   ; Driver status structure
query     SERVICE_QUERY  ?   ; Driver request structure 
ends
SCP_DATA SCPDATA ?
;---------- Dynamical imported WinAPI functions pointers list -----------------;
struct DYNAIMPORT
_IsWow64Process                    dd ?   ; This functions from KERNEL32.DLL
_GlobalMemoryStatusEx              dd ?          
_GetNativeSystemInfo               dd ?
_GetLogicalProcessorInformation    dd ?
_GetLogicalProcessorInformationEx  dd ?
_GetActiveProcessorGroupCount      dd ?  
_GetActiveProcessorCount           dd ?       
_GetLargePageMinimum               dd ?
_GetNumaHighestNodeNumber          dd ?
_GetNumaNodeProcessorMask          dd ?
_GetNumaAvailableMemoryNode        dd ?
_GetNumaNodeProcessorMaskEx        dd ?
_GetNumaAvailableMemoryNodeEx      dd ?
_EnumSystemFirmwareTables          dd ?
_GetSystemFirmwareTable            dd ?
_SetThreadAffinityMask             dd ?
_SetThreadGroupAffinity            dd ?
_VirtualAllocExNuma                dd ?
_OpenProcessToken                  dd ?   ; This functions from ADVAPI32.DLL              
_AdjustTokenPrivileges             dd ?         
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
TIMER_TICK_SHOW    = 50     ; Milliseconds per tick, benchmarks progress timer
TIMER_TICK_SILENT  = 60000  ; 1 revisual per 1 minute, for silent mode
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
eventStart       dd  ?   ; Event handle for signaling this thread starts
eventDone        dd  ?   ; Event handle for signaling this thread terminates 
threadHandle     dd  ?   ; Event handle for this thread 
measureCounter   dd  ?   ; Number of stored results, 0 means no stores before run, maximum MEASURE_POINTS_COUNT (640)
visualCounter    dd  ?   ; Number of visualized results, required because results generation and timer ticks is asynchronous
measureBreak     dd  ?   ; Flag for measurement break, 0=None,   BIDIRECTIONAL SIGNALING IS REJECTED BECAUSE RESIZE BUTTON BUG: 1=Break or Done, signaling is BIDIRECTIONAL: Break and Done
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
childWinHandle   dd  ?    ; Handle for drawings window, used for revisual
silentMode       db  ?    ; Silent mode flag, 1 = Slow screen refresh
childWinRunning  db  ?
ends
align 8
GUI_PARMS GUIPARMS ?
;---------- Variables for graphics controller context -------------------------; 
; Video output control
struct GCPARMS
handleMemDC      dd  ?          ; Handle for Device Context, video controller
bitmapPointer    dd  ?          ; Bitmap pointer
handleBitmap     dd  ?          ; Handle of bitmap for graphics draw
handlesBrushes   dd  4 dup (?)  ; Handle for color brushes
handleFont       dd  ?          ; Handle for font in the drawings window
handleDC         dd  ?          ; Handle Graphical Device Context
ends
align 8
GC_PARMS GCPARMS ?
;---------- Variables for graphics output -------------------------------------;
PAINT_STRUCT     PAINTSTRUCT    ; Paint control
GRAPH_RECT       RECT           ; Rectangle definition for visualized area
;---------- Structure for build clks/mbps table texts -------------------------;
align 8
CLKS_MBPS_TEXTS  dd  DRAW_TABLE_COUNT dup (?)
;---------- Structure for build Bytes/KB/MB/GB/TB/MBPS/nanoseconds texts ------;
struct UNITSTEXTS
bytes            dd  ?
kb               dd  ?
mb               dd  ?
gb               dd  ?
tb               dd  ?
mbps             dd  ?
nanoseconds      dd  ?
ends
UNITS_TEXTS UNITSTEXTS ?
;---------- String for write TSC frequency at drawings window -----------------;
DRAW_TSC       db  DRAW_TSC_STRING_SIZE dup (?)
DRAW_TSC_VALUE =   DRAW_TSC + TSC_VALUE_OFFSET
;---------- Pointers to dynamically allocated memory --------------------------;
struct ALLOCATOR  ; Allocator for data block with variable base address and size
objectStart      dd ?
objectStop       dd ?
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
icon exegicon, exeicon, 'images\fasm32.ico'
;---------- Manifest resource -------------------------------------------------;
resource manifests, 1, LANG_NEUTRAL, manifest
resdata manifest
db '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
db '<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">'
db '<assemblyIdentity'
db '    name="NCRB"'
db '    processorArchitecture="x86"'
db '    version="1.0.0.0"'
db '    type="win32"/>'
db '<description>NCRB</description>'
db '<dependency>'
db '    <dependentAssembly>'
db '        <assemblyIdentity'
db '           type="win32"'
db '           name="Microsoft.Windows.Common-Controls"'
db '           version="6.0.0.0"'
db '           processorArchitecture="x86"'
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


