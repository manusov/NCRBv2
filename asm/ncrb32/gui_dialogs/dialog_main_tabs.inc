;---------- Callback dialogue procedures for tab sheets window ----------------;
; Note. This ESP values at procedure entry, +4 because EIP pushed.             ;
;                                                                              ;
; INPUT:   [esp + 04] = Parm#1 = HWND = Dialog box handle                      ; 
;          [esp + 08] = Parm#2 = UINT = Message                                ; 
;          [esp + 12] = Parm#3 = WPARAM, message-specific                      ;
;          [esp + 16] = Parm#4 = LPARAM, message-specific                      ;
;                                                                              ;
; OUTPUT:  EAX = status, TRUE = message recognized and processed               ;
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
push ebp
mov ebp,esp
push ebx esi edi
movzx esi,al              ; ESI = Binder ID for this child window
mov edi,APP_DATA          ; EDI = Pointer to application data
mov ebx,PARM_HWNDDLG      ; EBX = This window handle
;---------- Detect message type -----------------------------------------------;
mov eax,PARM_MSG
cmp eax,WM_INITDIALOG
je .wminitdialog
cmp eax,WM_COMMAND
je .wmcommand
xor eax,eax
jmp .finish

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
mov eax,PARM_WPARAM
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
mov edi,BIND_LIST                             ; EDI = Pointer to GUI BINDLIST
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
mov cl,[edi + BINDLIST.getMemOption]
and cl,00000001b
shl cl,1
jz .prOff
jmp .prOn
@@:
jmp .processed
;---------- Continue for widget "Nontemporal", handling events ----------------;
.ntOff:
mov cl,00000000b
test [edi + BINDLIST.getMemMethod],0003FE00h  ; Check asm method: SSE/AVX/AVX512
jz @f
mov cl,00000001b
jmp @f
.ntOn:
mov cl,00000000b
test [edi + BINDLIST.getMemMethod],0003FE00h  ; Check asm method: SSE/AVX/AVX512
jz @f
mov cl,00000001b
test [edi + BINDLIST.getMemObject],00010000b  ; Check object type is DRAM
jz @f
mov cl,00000011b
@@:
mov [edi + BINDLIST.scratchPad],cl
push ecx
mov ax,BINDER_WIDGET_NONTEMPORAL
call Binder
pop ecx          ; Bits: CL.0 = NTflag availability, CL.1 = NTflag state
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
test [edi + BINDLIST.getMemMethod],00009200h  ; Check Read SSE/AVX/AVX512
jz @f
test cl,00000010b                             ; Check NONTEMPORAL just set/clear
jz @f
mov dx,0000000001011100b
@@:
mov word [edi + BINDLIST.scratchPad],dx
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
jmp .widgetForce32x2
;---------- Continue for widget "Custom block size", handling events ----------;
.csOff:
mov cl,00000000b
jmp @f
.csOn:
mov cl,00000001b
@@:
mov [edi + BINDLIST.scratchPad],cl
mov ax,BINDER_WIDGET_CUSTOM_SIZE
call Binder
;---------- Widget "Force 32x2" -----------------------------------------------;
.widgetForce32x2:
cmp si,IDB_MEMORY_ASM_C6
je .f32on
cmp si,IDB_MEMORY_ASM_C7
je .f32on 
cmp si,IDB_MEMORY_ASM_A0
jb @f
cmp si,IDB_MEMORY_ASM_C5
jbe .f32off
@@:
jmp .processed
;---------- Continue for widget "Force 32x2", handling events -------------------;
.f32on:
mov cl,00000001b
jmp @f
.f32off:
mov cl,00000000b
@@:
mov [edi + BINDLIST.scratchPad],cl
mov ax,BINDER_WIDGET_FORCE32X2
call Binder
jmp .processed
;---------- Handler for "Run" button ------------------------------------------;
.memoryRun:
mov ax,BINDER_GET_MEMORY
call Binder
; TODO. Make one binder per editable string, for skip checks individually
test [BIND_LIST.getMemObject],00100000b  ; Check selected object = custom
jz @f                                    ; Go skip if other selected object 
mov ax,BINDER_GET_MEMORY_CUSTOM  ; Error messages generated by this Binder
call Binder                      ; if user editable strings is incorrect
@@:
call CustomBlockSizeMessage      ; Error message if block size restrictions
mov al,WAIT_MODE_MEMORY
call OpenPleaseWait              ; Open "Please wait" window
call ScenarioMemoryRun           ; Target measured operation
pushf
push eax
call ClosePleaseWait             ; Close "Please wait" window
pop eax
popf
jc .runtimeError                 ; Go error message box if status bad
push 0                           ; Parm#5 = Pass value
push DialogProcChildMemoryRun    ; Parm#4 = Pointer to dialogue procedure
push ebx                         ; Parm#3 = Owner window handle
push IDD_CHILD_MEMORY_RUN        ; Parm#2 = Resource ( template ) id
.entryDialog:
push [edi + APPDATA.hResources]  ; Parm#1 = Handle of resource module
call [DialogBoxParam]            ; Create modal dialogue 
jmp .processed
;---------- Handler for "Draw" button -----------------------------------------;
.memoryDraw:
mov ax,BINDER_GET_MEMORY
call Binder
; TODO. Make one binder per editable string, for skip checks individually
test [BIND_LIST.getMemObject],00100000b  ; Check selected object = custom
jz @f                                    ; Go skip if other selected object 
mov ax,BINDER_GET_MEMORY_CUSTOM  ; Error messages generated by this Binder
call Binder                      ; if user editable strings is incorrect
@@:
call CustomBlockSizeMessage      ; Error message if block size restrictions
mov al,WAIT_MODE_MEMORY
call OpenPleaseWait              ; Open "Please wait" window
call DrawSessionStart            ; Target measured operation
;--- Conditionally wait measurements for all points if Silent mode ------------; 
pushf
push eax
jc .noSilent
test [BIND_LIST.getMemSilent],0001b  ; Check Silent mode
jz .noSilent                         ; Go skip if non silent mode
.waitSilent:
push 100                             ; Units = milliseconds
call [Sleep]
cmp [DRAW_THREAD_PARMS.measureCounter],DRAW_POINTS_COUNT
jb .waitSilent
.noSilent:
call ClosePleaseWait             ; Close "Please wait" window
pop eax
popf
jc .runtimeError                 ; Go error message box if status bad
;--- Create dialogue window with drawings Performance = F ( Block Size ) ------;
push 0
push DialogProcChildMemoryDraw
push ebx
push IDD_CHILD_MEMORY_DRAW
push [edi + APPDATA.hResources]  ; Parm#1 = Handle of resource module
call [DialogBoxParam]            ; Create modal dialogue 
call DrawSessionStop
jc .runtimeError                 ; Go error message box if status bad
jmp .processed
;---------- Handler for "Vector brief" button ---------------------------------;
.vectorBriefRun:
mov al,WAIT_MODE_CPU
call OpenPleaseWait              ; Open "Please wait" window
call ScenarioVectorBrief         ; Target measured operation
pushf
push eax
call ClosePleaseWait             ; Close "Please wait" window
pop eax
popf
jc .runtimeError                 ; Go error message box if status bad
push 0
push DialogProcChildVectorBrief
push ebx
push IDD_CHILD_VECTOR_BRIEF
jmp .entryDialog
;---------- Handler for "Defaults" button -------------------------------------;
.memoryDefaults:
mov ax,BINDER_SET_MEMORY
call Binder
jmp .processed
;---------- Handler for runtime errors ----------------------------------------;
.runtimeError:
mov esi,[APP_DATA.lockedStrings]
movzx eax,al
add eax,STR_RUNTIME_ALLOC
call IndexString       ; Return ESI = Selected string address 
push MB_ICONERROR      ; Parm#4 = Attributes
push PROGRAM_NAME      ; Parm#3 = Pointer to title (caption) string
push esi               ; Parm#2 = Pointer to string: error name 
push ebx               ; Parm#1 = Parent Window = NULL
call [MessageBox]  
jmp .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                     WM_CLOSE handler: close window.                          ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmclose:
mov ecx,[edi + APPDATA.hMain]
jecxz .processed
push 0
push 0
push WM_CLOSE
push ecx
call [SendMessage]
;---------- Exit points -------------------------------------------------------;
.processed:
mov eax,1
.finish:
pop edi esi ebx ebp
ret 16

