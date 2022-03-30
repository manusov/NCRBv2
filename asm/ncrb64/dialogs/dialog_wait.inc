;---------- Callback dialogue procedures for operations child windows ---------;
;           Handler for "Please wait" window.                                  ;
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
DialogProcChildWait:
push rbx rsi rdi rbp r12 r13 r14 r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
xchg eax,edx
mov rbx,rcx
lea r15,[WAIT_BOX]
;---------- Detect message type -----------------------------------------------;
cmp eax,WM_INITDIALOG
je .wminitdialog
cmp eax,WM_CLOSE
je .wmclose
cmp eax,WM_PAINT
je .wmpaint
;---------- Return points -----------------------------------------------------;
.statusZero:
xor eax,eax
jmp .status
.statusOne:
mov eax,1
.status:
mov rsp,rbp
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret

;------------------------------------------------------------------------------;
;                                                                              ;
;             WM_INITDIALOG handler: create "Please wait" window.              ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wminitdialog:
mov [r15 + WAITBOX.mode],r9d ; Select CPU or Memory benchmark comment string
;---------- Set window caption text string ------------------------------------;
mov rsi,[APP_DATA.lockedStrings]
mov ax,STR_PW_RUNNING
call IndexString
mov rdx,rsi                  ; RDX = Parm#2 = Pointer to text string
mov rcx,rbx                  ; RCX = Parm#1 = Window handle
call [SetWindowText]         ; Set main window text title
;---------- Create font -------------------------------------------------------;
xor eax,eax                  ; RAX = 0 for compact push 0
push rax                     ; Parm#14 = Pointer to font typename string, here not used
push VARIABLE_PITCH          ; Parm#13 = Font pitch and family
push CLEARTYPE_QUALITY       ; Parm#12 = Output quality
push CLIP_DEFAULT_PRECIS     ; Parm#11 = Clip precision
push OUT_OUTLINE_PRECIS      ; Parm#10 = Output precision
push DEFAULT_CHARSET         ; Parm#9  = Charset
push rax                     ; Parm#8  = Strike, here=0=none
push rax                     ; Parm#7  = Underline, here=0=none
push TRUE                    ; Parm#6  = Italic, here=0=none
push FW_DONTCARE             ; Parm#5  = Weight of the font
xor r9d,r9d                  ; Parm#4 = R9  = Orientation
xor r8d,r8d                  ; Parm#3 = R8  = Escapment
xor edx,edx                  ; Parm#2 = RDX = Width
mov ecx,17                   ; Parm#1 = RCX = Height
sub rsp,32                   ; Create parameters shadow
call [CreateFont]
add rsp,32 + 80              ; Remove parameters shadow and 10 parameters
mov [r15 + WAITBOX.hFont],rax 
jmp .statusOne

;------------------------------------------------------------------------------;
;                                                                              ;
;                     WM_CLOSE handler: close window.                          ;
;   Note this handler is redundant (not take control) in the current design.   ;
;   Note about delete font required. Use WM_CLOSE message externally.          ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmclose:
mov rcx,[r15 + WAITBOX.hFont]    ; Parm#1 = RCX = Object handle
jrcxz @f
call [DeleteObject]
@@:
mov rcx,[APP_DATA.hPleaseWait]   ; Parm#1 = RCX = Window handle 
call [DestroyWindow]
jmp .statusZero

;------------------------------------------------------------------------------;
;                                                                              ;
;                 WM_PAINT handler. Draw GUI window content.                   ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmpaint:
lea rsi,[r15 + WAITBOX.ps]
mov rdx,rsi                   ; Parm#2 = RDX = Pointer to PAINT structure
; rcx valid here              ; Parm#1 = RCX = Window handle 
call [BeginPaint]
test rax,rax
jz .statusZero
xchg r13,rax                  ; R13 = hDC, handle display context
;---------- Get window geometry: width and height -----------------------------;
; Note don't use RECT after BeginPaint,
; it can be partial area, required [GetClientRect].
lea rdx,[r15 + WAITBOX.backRect]    ; Parm#2 = RDX = Pointer to RECT structure
mov rcx,rbx                         ; Parm#1 = RCX = Window handle
call [GetClientRect]
lea rdi,[r15 + WAITBOX.rect]
push rsi rdi                        ; Don't optimize PUSH because stack alignment
lea rsi,[r15 + WAITBOX.backRect]
mov ecx,4
cld
rep movsd
pop rdi rsi
;---------- Set window transparency for text write ----------------------------;
mov edx,TRANSPARENT                ; Parm#2 = RDX = Mode
mov rcx,r13                        ; Parm#1 = RCX = hDC  
call [SetBkMode]
;---------- Fill window by background color -----------------------------------; 
mov r8d,COLOR_HIGHLIGHT ; COLOR_WINDOW + 1  ; Parm#3 = R8  = Color (or HBRUSH)
mov rdx,rdi                                 ; Parm#2 = RDX = Pointer to RECT structure
mov rcx,r13                                 ; Parm#1 = RCX = hDC  
call [FillRect] 
;---------- Draw central picture ----------------------------------------------;
mov r9,[APP_DATA.createdBigIcons + 8]  ; Parm#4 = R9  = Handle icon
mov r8d,WAIT_ICONY                     ; Parm#3 = R8  = Base Y coordinate
mov edx,WAIT_ICONX                 ; Parm#2 = RDX = Base X coordinate
mov rcx,r13                        ; Parm#1 = RCX = hDC, handle display context
call [DrawIcon]
;---------- Support text output -----------------------------------------------;
xor eax,eax
mov rdx,[r15 + WAITBOX.hFont]      ; Parm#2 = RDX = HGDIOBJ, object handle
test rdx,rdx
jz @f
mov rcx,r13                        ; Parm#1 = RCX = hDC, handle display context
call [SelectObject]
@@:
mov [r15 + WAITBOX.hFontBack],rax
;---------- Up string: benchmark name -----------------------------------------;
push rsi
mov rsi,[APP_DATA.lockedStrings]
push rsi
mov eax,STR_PW_CPU_BENCH
add eax,[r15 + WAITBOX.mode]
call IndexString
mov rdx,rsi
@@:
lodsb
cmp al,0
jne @b
lea r8,[rsi - 1]
sub r8,rdx
mov [rdi + RECT.top],WAIT_YBASE1
mov [rdi + RECT.bottom],WAIT_YBASE1 + WAIT_YADD
call HelperCenterText         ; Write upper string: benchmark type
;---------- Down string: "Please wait..." -------------------------------------;
pop rsi
mov ax,STR_PW_PLEASE_WAIT
call IndexString
mov rdx,rsi
@@:
lodsb
cmp al,0
jne @b
lea r8,[rsi - 1]
sub r8,rdx
mov [rdi + RECT.top],WAIT_YBASE2
mov [rdi + RECT.bottom],WAIT_YBASE2 + WAIT_YADD
call HelperCenterText         ; Write upper string: benchmark type
pop rsi
;---------- Paint window elements done ----------------------------------------;
mov rdx,[r15 + WAITBOX.hFontBack]
test rdx,rdx
jz @f                         ; Parm#2 = RDX = HGDIOBJ, handle GDI object
mov rcx,r13                   ; Parm#1 = RCX = hDC, handle display context
call [SelectObject]
@@:
mov rdx,rsi                   ; Parm#2 = RDX = Pointer to PAINT structure
mov rcx,rbx                   ; Parm#1 = RCX = Window handle
call [EndPaint]
jmp .statusOne  ; .statusZero , experimentally changed because background redraw when other window overlap this 
