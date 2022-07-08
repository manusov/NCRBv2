;---------- Callback dialogue procedures for operations child windows ---------;
;           Handler for "Run" button, vector brief mode at "Math" screen       ;
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
DialogProcChildVectorBrief:
cld
push rbp rbx rsi rdi r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h     ; Stack alignment
sub rsp,32                     ; Make parameters shadow for next calls
mov rbx,rcx
lea r15,[APP_DATA]             ; R15 = Pointer to application data
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
;                WM_INITDIALOG handler: create child window:                   ; 
;                    vector performance brief report.                          ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wminitdialog:
mov ax,BINDER_BUILD_VECTOR_BRIEF
call Binder
mov r9,[r15 + APPDATA.createdIcons + VBRF_ICON_NUMBER * 8] 
mov r8d,ICON_SMALL 
mov edx,WM_SETICON 
mov rcx,rbx            ; RCX = PARM_HWNDDLG
call [SendMessage]
mov rsi,[r15 + APPDATA.lockedStrings]
mov ax,STR_CHILD_VECTOR_BRIEF
call IndexString
mov rdx,rsi
mov rcx,rbx            ; RCX = PARM_HWNDDLG
call [SetWindowText]
jmp .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                 WM_COMMAND handler: interpreting user input.                 ; 
;               Detect click "OK" button at vector brief window.               ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmcommand:
cmp r8w,IDB_VB_OK     ; CMP PARM_WPARAM,IDB_VB_OK 
jne .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                     WM_CLOSE handler: close window.                          ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmclose:
mov edx,1                   ; RDX = Parm#2 = Result for return
mov rcx,rbx                 ; RCX = Parm#1 = Window handle = PARM_HWNDDLG 
call [EndDialog]
;---------- Exit points -------------------------------------------------------;
.processed:
mov eax,1
.finish:
mov rsp,rbp
pop r15 rdi rsi rbx rbp
ret
