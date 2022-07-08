;---------- Callback dialogue procedures for operations child windows ---------;
;           Handler for "Run" button, vector brief mode at "Math" screen       ; 
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
DialogProcChildVectorBrief:
push ebp
mov ebp,esp
push ebx esi edi
mov edi,APP_DATA          ; EDI = Pointer to application data
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

;------------------------------------------------------------------------------;
;                                                                              ;
;                WM_INITDIALOG handler: create child window:                   ; 
;                    vector performance brief report.                          ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wminitdialog:
mov ax,BINDER_BUILD_VECTOR_BRIEF
call Binder
push [edi + APPDATA.createdIcons + VBRF_ICON_NUMBER * 4]
push ICON_SMALL
push WM_SETICON 
push ebx              ; push PARM_HWNDDLG
call [SendMessage]
mov esi,[edi + APPDATA.lockedStrings]
mov ax,STR_CHILD_VECTOR_BRIEF
call IndexString
push esi
push ebx              ; push PARM_HWNDDLG
call [SetWindowText]
jmp .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                 WM_COMMAND handler: interpreting user input.                 ; 
;               Detect click "OK" button at vector brief window.               ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmcommand:
cmp PARM_WPARAM,IDB_VB_OK
jne .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                     WM_CLOSE handler: close window.                          ;
;                                                                              ;
;------------------------------------------------------------------------------;
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
