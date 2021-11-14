;---------- Callback dialogue procedures for operations child windows ---------;
;           Handler for "Run" button at "Memory" screen                        ; 
;                                                                              ;
; INPUT:   [esp + 04] = Parm#1 = HWND = Dialog box handle                      ; 
;          [esp + 08] = Parm#2 = UINT = Message                                ; 
;          [esp + 12] = Parm#3 = WPARAM, message-specific                      ;
;          [esp + 16] = Parm#4 = LPARAM, message-specific                      ;
;                                                                              ;
; OUTPUT:  EAX = status, TRUE = message recognized and processed               ;
;                        FALSE = not recognized, must be processed by OS,      ;
;                        see MSDN for status exceptions and details            ;  
;                                                                              ;
;------------------------------------------------------------------------------;
DialogProcChildMemoryRun:
push ebp
mov ebp,esp
push ebx esi edi
lea edi,[APP_DATA]        ; EDI = Pointer to application data
mov ebx,PARM_HWNDDLG      ; EBX = Window handle
;---------- Detect message type -----------------------------------------------;
mov eax,PARM_MSG
cmp eax,WM_INITDIALOG
je .wminitdialog
cmp eax,WM_COMMAND
je .wmcommand
cmp eax,WM_CLOSE
je .wmclose
.skip:
xor eax,eax
jmp .finish
;--- WM_INITDIALOG handler: create child window: memory performance report ----;
.wminitdialog:                   ; Build main window 
mov ax,BINDER_BUILD_MEMORY_RUN
call Binder
mov ax,BINDER_BUILD_MEMORY_MBPS
cmp [MEM_IPB.updatedAsm],LATENCY_MODE
jb @f
mov ax,BINDER_BUILD_MEMORY_NS
@@:
call Binder
push [edi + APPDATA.createdIcons + MEMORY_ICON_NUMBER * 4]
push ICON_SMALL
push WM_SETICON 
push PARM_HWNDDLG
call [SendMessage]
mov esi,[edi + APPDATA.lockedStrings]
mov ax,STR_CHILD_MEM_REPORT
call IndexString
push esi
push PARM_HWNDDLG
call [SetWindowText]
jmp .processed
;---------- WM_COMMAND handler: interpreting user input -----------------------; 
; Detect click "OK" button at results window
.wmcommand:
cmp PARM_WPARAM,IDB_MR_OK 
jne .processed
;---------- WM_CLOSE handler: close window ------------------------------------;
.wmclose:
push 1                           ; Parm#2 = Result for return
push ebx                         ; Parm#1 = Window handle
call [EndDialog]
;---------- Exit points -------------------------------------------------------;
.processed:
mov eax,1
.finish:
pop edi esi ebx ebp
ret 16
