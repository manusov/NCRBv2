;---------- Callback dialogue procedures for tab sheets child windows ---------;
;                                                                              ;
; INPUT:   RCX = Parm#1 = HWND = Dialog box handle                             ; 
;          RDX = Parm#2 = UINT = Message                                       ; 
;          R8  = Parm#3 = WPARAM, message-specific                             ;
;          R9  = Parm#4 = LPARAM, message-specific                             ;
;                                                                              ;
; OUTPUT:  RAX = status, TRUE = message recognized and processed               ;
;                        FALSE = not recognized, must be processed by OS,      ;
;                        see MSDN for status exceptions and details.           ;  
;                                                                              ;
;------------------------------------------------------------------------------;
DialogProcSysinfo:
mov al,BINDER_BUILD_SYSINFO
jmp DialogProcEntry
DialogProcMemory:
mov al,BINDER_BUILD_MEMORY
jmp DialogProcEntry
DialogProcOs:
mov al,BINDER_BUILD_OS
jmp DialogProcEntry
DialogProcNativeOs:
mov al,BINDER_BUILD_NATIVE_OS
jmp DialogProcEntry
DialogProcTopology:
mov al,BINDER_BUILD_TOPOLOGY
jmp DialogProcEntry
DialogProcTopologyEx:
mov al,BINDER_BUILD_TOPOLOGY_EX
jmp DialogProcEntry
DialogProcNuma:
mov al,BINDER_BUILD_NUMA
jmp DialogProcEntry
DialogProcGroups:
mov al,BINDER_BUILD_P_GROUPS
jmp DialogProcEntry
DialogProcSmbios:
mov al,BINDER_BUILD_SMBIOS
jmp DialogProcEntry
DialogProcAcpi:
mov al,BINDER_BUILD_ACPI
jmp DialogProcEntry
DialogProcAffCpuid:
mov al,BINDER_BUILD_AFF_CPUID
;---------- Entry point with AL = Binder ID for required child window ---------;
DialogProcEntry:
cld
push rbp rbx rsi rdi r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea r15,[APP_DATA]             ; R15 = Pointer to application data structure
movzx esi,al                   ; ESI = Binder ID for this child window
mov rbx,rcx                    ; RBX = This window handle
;---------- Detect message type -----------------------------------------------;
cmp rdx,0000FFFFh
jae .skip
xchg eax,edx                   ; Use EAX for compact CMP
cmp eax,WM_INITDIALOG
je .wminitdialog               ; Go if dialogue initialization message 
cmp eax,WM_COMMAND
je .wmcommand                  ; Go if command message
cmp eax,WM_CLOSE
je .wmclose                    ; Go if window close message
.skip:
xor eax,eax
jmp .finish                    ; Go exit if unknown event

;------------------------------------------------------------------------------;
;                                                                              ;
;                WM_INITDIALOG handler: create sheet window.                   ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wminitdialog:
mov eax,esi
call Binder
xchg eax,esi
cmp al,BINDER_BUILD_MEMORY
jne @f
inc eax
inc eax
call Binder
mov ax,BINDER_SET_MEMORY
call Binder
;--- Mark saved draw and run windows coordinates as invalid if yet not saved --; 
mov eax,-1
mov [SAVE_RUN_POS.top],eax
mov [SAVE_DRAW_POS.top],eax
@@:
jmp .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                 WM_COMMAND handler: interpreting user input.                 ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmcommand:
mov eax,r8d
cmp ax,IDB_SYSINFO_CANCEL    ; Detect click "Exit" button in the child window
je .wmclose
cmp ax,IDB_MEMORY_CANCEL
je .wmclose
cmp ax,IDB_OS_CANCEL
je .wmclose
cmp ax,IDB_NAT_OS_CANCEL
je .wmclose
cmp ax,IDB_TOPOL_CANCEL
je .wmclose
cmp ax,IDB_TOPOL_EX_CANCEL
je .wmclose
cmp ax,IDB_NUMA_CANCEL
je .wmclose
cmp ax,IDB_P_GROUPS_CANCEL
je .wmclose
cmp ax,IDB_ACPI_CANCEL
je .wmclose
cmp ax,IDB_A_CPUID_CANCEL
je .wmclose
cmp ax,IDB_MEMORY_RUN        ; Run simple benchmark
je .memoryRun
cmp ax,IDB_MEMORY_DRAW       ; Run benchmark with drawings
je .memoryDraw
cmp ax,IDB_SYSINFO_VBRF      ; Run "Vector brief" benchmark
je .vectorBriefRun
cmp ax,IDB_MEMORY_DEFAULTS   ; Set defaults for memory benchmark
je .memoryDefaults
;---------- Detect events for make widgets dependency -------------------------;
;---------- Widget "Nontemporal" ----------------------------------------------;
lea rdi,[BIND_LIST]                           ; RDI = Pointer to GUI BINDLIST
mov si,ax                                     ; SI  = Checkbox ID
mov ax,BINDER_GET_MEMORY
call Binder                                   ; Read current state of widgets
cmp si,IDB_MEMORY_ASM_A0
jb @f
cmp si,IDB_MEMORY_ASM_A8
jbe .ntOff
@@:
cmp si,IDB_MEMORY_ASM_B0
jb @f
cmp si,IDB_MEMORY_ASM_B8
jbe .ntOn 
@@:
cmp si,IDB_MEMORY_ASM_C0
jb @f
cmp si,IDB_MEMORY_ASM_C7
jbe .ntOff 
@@:
cmp si,IDB_MEMORY_DRAM
je  .ntOn 
cmp si,IDB_MEMORY_L1
jb @f
cmp si,IDB_MEMORY_CUSTOM
jbe  .ntOff 
@@:
cmp si,IDB_MEMORY_NONTEMP
jne @f
mov cl,[rdi + BINDLIST.getMemOption]
and cl,00000001b
shl cl,1
jz .prOff
jmp .prOn
@@:
jmp .processed
;---------- Continue for widget "Nontemporal", handling events ----------------;
.ntOff:
mov cl,00000000b
test [rdi + BINDLIST.getMemMethod],0003FE00h  ; Check asm method: SSE/AVX/AVX512
jz @f
mov cl,00000001b
jmp @f
.ntOn:
mov cl,00000000b
test [rdi + BINDLIST.getMemMethod],0003FE00h  ; Check asm method: SSE/AVX/AVX512
jz @f
mov cl,00000001b
test [rdi + BINDLIST.getMemObject],00010000b  ; Check object type is DRAM
jz @f
mov cl,00000011b
@@:
mov [rdi + BINDLIST.scratchPad],cl
push rcx
mov ax,BINDER_WIDGET_NONTEMPORAL
call Binder
pop rcx          ; Bits: CL.0 = NTflag availability, CL.1 = NTflag state
;---------- Widget(s) "Prefetch" ----------------------------------------------;
cmp si,IDB_MEMORY_ASM_B0
je .prOn
cmp si,IDB_MEMORY_ASM_B3
je .prOn
cmp si,IDB_MEMORY_ASM_B6
je .prOn
cmp si,IDB_MEMORY_ASM_A0
jb @f
cmp si,IDB_MEMORY_ASM_C7
jbe .prOff
@@:
cmp si,IDB_MEMORY_DRAM
je .prOn
cmp si,IDB_MEMORY_L1
jb @f 
cmp si,IDB_MEMORY_CUSTOM
jbe .prOff
@@:
jmp .processed
;---------- Continue for widget "Prefetch", handling events -------------------;
.prOff:
mov dx,0000000000000010b
jmp @f
.prOn:
mov dx,0000000000000010b
test [rdi + BINDLIST.getMemMethod],00009200h  ; Check Read SSE/AVX/AVX512
jz @f
test cl,00000010b                             ; Check NONTEMPORAL just set/clear
jz @f
mov dx,0000000001011100b
@@:
mov word [rdi + BINDLIST.scratchPad],dx
mov ax,BINDER_WIDGET_PREFETCH
call Binder
;---------- Widget(s) "Custom block size" -------------------------------------;
cmp si,IDB_MEMORY_CUSTOM
je .csOn
cmp si,IDB_MEMORY_L1
jb @f
cmp si,IDB_MEMORY_DRAM
jbe .csOff
@@:
jmp .processed
;---------- Continue for widget "Custom block size", handling events ----------;
.csOff:
mov cl,00000000b
jmp @f
.csOn:
mov cl,00000001b
@@:
mov [rdi + BINDLIST.scratchPad],cl
mov ax,BINDER_WIDGET_CUSTOM_SIZE
call Binder
;---------- Widget "Force 32x2" -----------------------------------------------;
; This widget for ia32 version only, yet not used for x64 (this) version.
jmp .processed
;---------- Handler for "Run" button ------------------------------------------;
.memoryRun:
mov ax,BINDER_GET_MEMORY
call Binder
; TODO. Make one binder per editable string, for skip checks individually
test [BIND_LIST.getMemObject],00100000b  ; Check selected object = custom
jz @f                                    ; Go skip if other selected object 
mov ax,BINDER_GET_MEMORY_CUSTOM     ; Error messages generated by this Binder
call Binder                         ; if user editable strings is incorrect
@@:
call CustomBlockSizeMessage         ; Error message if block size restrictions
mov al,WAIT_MODE_MEMORY
call OpenPleaseWait                 ; Open "Please wait" window
call ScenarioMemoryRun              ; Target measured operation
pushf
push rax
call ClosePleaseWait                ; Close "Please wait" window
pop rax
popf
jc .runtimeError                    ; Go error message box if status bad
lea r9,[DialogProcChildMemoryRun]   ; R9  = Parm#4 = Pointer to dialogue proced.
mov edx,IDD_CHILD_MEMORY_RUN        ; RDX = Parm#2 = Resource ( template ) id
.entryDialog:
push 0 0                            ; Parm#5 = Pass value, plus alignment qword
mov r8,rbx                          ; R8  = Parm#3 = Owner window handle
mov rcx,[r15 + APPDATA.hResources]  ; RCX = Parm#1 = Handle of resource module  
sub rsp,32                          ; Allocate parameters shadow
call [DialogBoxParam]               ; Create modal dialogue 
add rsp,32 + 16                     ; Remove parameters shadow and 2 parameters
jmp .processed
;---------- Handler for "Draw" button -----------------------------------------;
.memoryDraw:
mov ax,BINDER_GET_MEMORY
call Binder
; TODO. Make one binder per editable string, for skip checks individually
test [BIND_LIST.getMemObject],00100000b  ; Check selected object = custom
jz @f                                    ; Go skip if other selected object 
mov ax,BINDER_GET_MEMORY_CUSTOM     ; Error messages generated by this Binder
call Binder                         ; if user editable strings is incorrect
@@:
call CustomBlockSizeMessage         ; Error message if block size restrictions
mov al,WAIT_MODE_MEMORY
call OpenPleaseWait                 ; Open "Please wait" window
call DrawSessionStart               ; Target measured operation
;--- Conditionally wait measurements for all points if Silent mode ------------; 
pushf
push rax
jc .noSilent
test [BIND_LIST.getMemSilent],0001b  ; Check Silent mode
jz .noSilent                         ; Go skip if non silent mode
.waitSilent:
mov ecx,100                          ; Units = milliseconds
sub rsp,32
call [Sleep]
add rsp,32
cmp [DRAW_THREAD_PARMS.measureCounter],DRAW_POINTS_COUNT
jb .waitSilent
.noSilent:
call ClosePleaseWait                ; Close "Please wait" window
pop rax
popf
jc .runtimeError                    ; Go error message box if status bad
;--- Create dialogue window with drawings Performance = F ( Block Size ) ------;
push 0 0                            ; Parm#5 = Pass value, plus alignment qword
lea r9,[DialogProcChildMemoryDraw]
mov r8,rbx                          ; R8  = Parm#3 = Owner window handle
mov edx,IDD_CHILD_MEMORY_DRAW
mov rcx,[r15 + APPDATA.hResources]  ; RCX = Parm#1 = Handle of resource module  
sub rsp,32                          ; Allocate parameters shadow
call [DialogBoxParam]               ; Create modal dialogue 
add rsp,32 + 16                     ; Remove parameters shadow and 2 parameters
call DrawSessionStop
jc .runtimeError                    ; Go error message box if status bad
jmp .processed
;---------- Handler for "Vector brief" button ---------------------------------;
.vectorBriefRun:
mov al,WAIT_MODE_CPU
call OpenPleaseWait                 ; Open "Please wait" window
call ScenarioVectorBrief            ; Target measured operation
pushf
push rax
call ClosePleaseWait                ; Close "Please wait" window
pop rax
popf
jc .runtimeError                    ; Go error message box if status bad
lea r9,[DialogProcChildVectorBrief]
mov edx,IDD_CHILD_VECTOR_BRIEF
jmp .entryDialog
;---------- Handler for "Defaults" button -------------------------------------;
.memoryDefaults:
mov ax,BINDER_SET_MEMORY
call Binder
jmp .processed
;---------- Handler for runtime errors ----------------------------------------;
.runtimeError:
mov rsi,[APP_DATA.lockedStrings]
movzx eax,al
add eax,STR_RUNTIME_ALLOC
call IndexString       ; Return ESI = Selected string address 
mov r9d,MB_ICONERROR   ; R9  = Parm#4 = Attributes
lea r8,[PROGRAM_NAME]  ; R8  = Parm#3 = Pointer to title (caption) string
mov rdx,rsi            ; RDX = Parm#2 = Pointer to string: error name 
mov rcx,rbx            ; RCX = Parm#1 = Parent Window - this window
call [MessageBox]  
jmp .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                     WM_CLOSE handler: close window.                          ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmclose:
mov rcx,[r15 + APPDATA.hMain]
jrcxz .processed
xor r9d,r9d
xor r8d,r8d
mov edx,WM_CLOSE
call [SendMessage]
;---------- Exit points -------------------------------------------------------;
.processed:
mov eax,1
.finish:
mov rsp,rbp
pop r15 rdi rsi rbx rbp
ret

 
