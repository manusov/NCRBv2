;---------- Callback dialogue procedures for operations child windows ---------;
;           Handler for "About" window, item in the application main menu.     ;
;                                                                              ;
; INPUT:   RCX = Parm#1 = HWND = Dialog box handle                             ;
;          RDX = Parm#2 = UINT = Message                                       ;
;          R8  = Parm#3 = WPARAM, message-specific                             ;
;          R9  = Parm#4 = LPARAM, message-specific                             ;
;                                                                              ;
; OUTPUT:  RAX = status, TRUE = message recognized and processed               ;
;                        FALSE = not recognized, must be processed by OS,      ;
;                        see MSDN for status exceptions and details            ;
;                                                                              ;
;------------------------------------------------------------------------------;
DialogProcChildAbout:
push rbx rsi rdi rbp r12 r13 r14 r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
xchg eax,edx
mov rbx,rcx
lea r15,[ABOUT_BOX]
;---------- Detect message type -----------------------------------------------;
cmp eax,WM_INITDIALOG
je .wminitdialog
cmp eax,WM_CLOSE
je .wmclose
cmp eax,WM_COMMAND
je .wmcommand
cmp eax,WM_LBUTTONDOWN
je .wmlbuttondown
cmp eax,WM_RBUTTONDOWN
je .wmrbuttondown
cmp eax,WM_LBUTTONUP
je .wmlbuttonup
cmp eax,WM_RBUTTONUP
je .wmrbuttonup
cmp eax,WM_MOUSEMOVE
je .wmmousemove
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
;               WM_INITDIALOG handler: create "About" window.                  ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wminitdialog:
mov ax,BINDER_BUILD_ABOUT
call Binder                  ; Resource binder script
mov rsi,[APP_DATA.lockedStrings]
mov ax,STR_AB_PROGRAM_INFO
call IndexString
mov rdx,rsi
mov rcx,rbx
call [SetWindowText]         ; Window caption string
mov r9,[APP_DATA.hIcon]      ; Parm#4 = R9  = lParam = Icon handle
mov r8d,ICON_SMALL           ; Parm#3 = R8  = wParam = Set small icon message  
mov edx,WM_SETICON           ; Parm#2 = RDX = Msg  
mov rcx,rbx                  ; Parm#1 = RCX = hWnd
call [SendMessage]
mov edx,IDC_HAND             ; Parm#2 = RDX = Cursor name, here constant  
xor ecx,ecx                  ; Parm#1 = RCX = hInstance, here not used
call [LoadCursor]
mov [r15 + ABOUTBOX.hCursor],rax
xor eax,eax                  ; RAX = 0 for compact push 0
push rax                     ; Parm#14 = Pointer to font typename string, here not used
push VARIABLE_PITCH          ; Parm#13 = Font pitch and family
push CLEARTYPE_QUALITY       ; Parm#12 = Output quality
push CLIP_DEFAULT_PRECIS     ; Parm#11 = Clip precision
push OUT_OUTLINE_PRECIS      ; Parm#10 = Output precision
push DEFAULT_CHARSET         ; Parm#9  = Charset
push rax                     ; Parm#8  = Strike, here=0=none
push rax                     ; Parm#7  = Underline, here=0=none
push rax                     ; Parm#6  = Italic, here=0=none
push FW_DONTCARE             ; Parm#5  = Weight of the font
xor r9d,r9d                  ; Parm#4 = R9  = Orientation
xor r8d,r8d                  ; Parm#3 = R8  = Escapment
xor edx,edx                  ; Parm#2 = RDX = Width
mov ecx,16                   ; Parm#1 = RCX = Height
sub rsp,32                   ; Create parameters shadow
call [CreateFont]
add rsp,32 + 80              ; Remove parameters shadow and 10 parameters
mov [r15 + ABOUTBOX.hFont],rax 
jmp .statusOne

;------------------------------------------------------------------------------;
;                                                                              ;
;                     WM_CLOSE handler: close window.                          ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmclose:
mov rcx,[r15 + ABOUTBOX.hFont]  ; Parm#1 = RCX = Object handle
jrcxz @f
call [DeleteObject]
@@:
xor edx,edx                     ; Parm#2 = RDX = nResult, value for return
mov rcx,rbx                     ; Parm#1 = RCX = Window handle
call [EndDialog]
jmp .statusZero

;------------------------------------------------------------------------------;
;                                                                              ;
;                 WM_COMMAND handler: interpreting user input.                 ; 
;                 Detect click "OK" button at "About" window.                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmcommand:
cmp r8w,IDB_ABOUT_OK
je .wmclose
jmp .statusZero

;------------------------------------------------------------------------------;
;                                                                              ;
;        WM_LBUTTONDOWN handler: interpreting mouse left button click.         ; 
;          Provide mouse cursor consistency and go web link if click.          ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmlbuttondown:
call HelperCheckLine4
mov ax,STR_AB_GITHUB_LINK
jc .clicked 
call HelperCheckLine5
mov ax,STR_AB_FASM_LINK
jnc .statusZero
.clicked:
mov rsi,[APP_DATA.lockedStrings]
call IndexString
mov rcx,[r15 + ABOUTBOX.hCursor]  ; Parm#1 = RCX = Cursor handle
jrcxz @f
call [SetCursor]
@@:
mov r8,rsi             ; Parm#3 = R8  = Pointer to file or object name
push SW_NORMAL         ; Parm#6 = Command flags     
push 0                 ; Parm#5 = Pointer to directory name, 0 means not used 
xor r9d,r9d            ; Parm#4 = R9  = Pointer to parameter string, 0 means not used  
xor edx,edx            ; Parm#2 = RDX = Operation, 0 means not used
xor ecx,ecx            ; Parm#1 = RCX = hWnd, 0 means not associated   
sub rsp,32
call [ShellExecute]
add rsp,32 + 16
cmp rax,32
ja @f
mov rsi,[APP_DATA.lockedStrings]
mov ax,STR_AB_SHELL_ERROR
call IndexString
mov r9d,MB_ICONERROR   ; Parm#4 = R9  = Attributes
xor r8d,r8d            ; Parm#3 = R8  = Pointer to title (caption) string
mov rdx,rsi            ; Parm#2 = RDX = Pointer to string: error name
xor ecx,ecx            ; Parm#1 = RCX = Parent Window or NULL
call [MessageBox]  
@@:
jmp .statusZero

;------------------------------------------------------------------------------;
;                                                                              ;
;              WM_RBUTTONDOWN, WM_LBUTTONUP, WM_RBUTTONUP handler.             ; 
;    Provide mouse cursor consistency when mouse buttons click and release.    ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmrbuttondown:
.wmlbuttonup:
.wmrbuttonup:
call HelperCheckLine4
jc @f
call HelperCheckLine5
jnc .statusZero
@@:
.change:
mov rcx,[r15 + ABOUTBOX.hCursor]  ; Parm#1 = RCX = Cursor handle
jrcxz @f
call [SetCursor]
@@:
jmp .statusZero

;------------------------------------------------------------------------------;
;                                                                              ;
;              WM_RBUTTONDOWN, WM_LBUTTONUP, WM_RBUTTONUP handler.             ; 
;               Provide mouse cursor consistency when mouse move.              ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmmousemove:
call HelperCheckLine4
jc .change
call HelperCheckLine5
jc .change
jmp .statusZero

;------------------------------------------------------------------------------;
;                                                                              ;
;                 WM_PAINT handler. Draw GUI window content.                   ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmpaint:
lea rsi,[r15 + ABOUTBOX.ps]
mov rdx,rsi                   ; Parm#2 = RDX = Pointer to PAINT structure
; rcx valid here              ; Parm#1 = RCX = Window handle 
call [BeginPaint]
test rax,rax
jz .statusZero
xchg r13,rax                  ; R13 = hDC, handle display context
;---------- Get window geometry: width and height -----------------------------;
; Note don't use RECT after BeginPaint,
; it can be partial area, required [GetClientRect].
lea rdx,[r15 + ABOUTBOX.backRect]   ; Parm#2 = RDX = Pointer to RECT structure
mov rcx,rbx                         ; Parm#1 = RCX = Window handle
call [GetClientRect]
lea rdi,[r15 + ABOUTBOX.rect]
push rsi rdi                        ; Don't optimize PUSH because stack alignment
lea rsi,[r15 + ABOUTBOX.backRect]
mov ecx,3
cld
rep movsd
lodsd
sub eax,ABOUT_YBOTTOM
stosd
pop rdi rsi
;---------- Set window transparency for text write ----------------------------;
mov edx,TRANSPARENT                ; Parm#2 = RDX = Mode
mov rcx,r13                        ; Parm#1 = RCX = hDC  
call [SetBkMode]
;---------- Fill window by background color -----------------------------------; 
mov r8d,COLOR_WINDOW + 1           ; Parm#3 = R8  = Color (or HBRUSH)
mov rdx,rdi                        ; Parm#2 = RDX = Pointer to RECT structure
mov rcx,r13                        ; Parm#1 = RCX = hDC  
call [FillRect] 
;---------- Draw central picture ----------------------------------------------;
mov r9,[APP_DATA.createdBigIcons]  ; Parm#4 = R9  = Handle icon
mov r8d,ABOUT_ICONY                ; Parm#3 = R8  = Base Y coordinate
mov edx,ABOUT_ICONX                ; Parm#2 = RDX = Base X coordinate
mov rcx,r13                        ; Parm#1 = RCX = hDC, handle display context
call [DrawIcon]
;---------- Support text output -----------------------------------------------;
xor eax,eax
mov rdx,[r15 + ABOUTBOX.hFont]     ; Parm#2 = RDX = HGDIOBJ, object handle
test rdx,rdx
jz @f
mov rcx,r13                        ; Parm#1 = RCX = hDC, handle display context
call [SelectObject]
@@:
mov [r15 + ABOUTBOX.hFontBack],rax
lea rdx,[r15 + ABOUTBOX.tm]        ; Parm#2 = Pointer to TEXTMETRIC structure
mov rcx,r13                        ; Parm#1 = RCX = hDC, handle display context
call [GetTextMetrics]
;---------- Strings 1-3, without clickable elements ---------------------------;
push rbx rsi
cld
lea rsi,[ABOUT_NAME]
mov ebx,ABOUT_YBASE1
.next1:
mov rdx,rsi
@@:
lodsb
cmp al,0Ah
je @f
cmp al,0Dh
je @f
cmp al,0
jne @b
@@:
lea r8,[rsi - 1]
sub r8,rdx
mov [rdi + RECT.top],ebx
add ebx,ABOUT_YADD1
mov [rdi + RECT.bottom],ebx
call HelperCenterText
dec rsi
@@:
lodsb
cmp al,0Ah
je @b
cmp al,0Dh
je @b
cmp al,0
je .stop1
dec rsi
jmp .next1
.stop1:
pop rsi rbx
;---------- Strings 4-5, with clickable elements ------------------------------;
push rbx rbp    ; 2 PUSH, stack still aligned
sub rsp,32      ; make parameters shadow for WinAPI functions calls
lea r14,[r15 + ABOUTBOX.clickStr1]
mov [r14 + CLICKSTRING.stringId],STR_AB_GITHUB
mov [r14 + CLICKSTRING.fullSize],ABOUT_FULL_SIZE_1
mov [r14 + CLICKSTRING.clickStart],ABOUT_CLICK_START_1
mov [r14 + CLICKSTRING.clickSize],ABOUT_CLICK_SIZE_1
mov [r14 + CLICKSTRING.stringId + sizeof.CLICKSTRING],STR_AB_FASM
mov [r14 + CLICKSTRING.fullSize + sizeof.CLICKSTRING],ABOUT_FULL_SIZE_2
mov [r14 + CLICKSTRING.clickStart + sizeof.CLICKSTRING],ABOUT_CLICK_START_2
mov [r14 + CLICKSTRING.clickSize + sizeof.CLICKSTRING],ABOUT_CLICK_SIZE_2
mov ebp,ABOUT_YBASE2
.clickableStrings:
push rsi
mov rsi,[APP_DATA.lockedStrings]
mov eax,[r14 + CLICKSTRING.stringId]
call IndexString
mov rbx,rsi
pop rsi
mov eax,ebp
mov [r14 + CLICKSTRING.ymin],eax
add eax,[r15 + ABOUTBOX.tm + TEXTMETRIC.tmHeight] 
mov [r14 + CLICKSTRING.ymax],eax
mov rdx,rbx
mov r8d,[r14 + CLICKSTRING.fullSize]
call HelperTextSize            ; Get EAX = full string width
mov edx,[r15 + ABOUTBOX.backRect + RECT.right]
sub edx,[r15 + ABOUTBOX.backRect + RECT.left]
sub edx,eax
shr edx,1
mov r12d,edx
mov [rdi + RECT.left],edx
mov eax,ebp
mov [rdi + RECT.top],eax
add eax,ABOUT_YADD2
mov [rdi + RECT.bottom],eax
mov r8d,[r14 + CLICKSTRING.clickStart]
mov rdx,rbx
call HelperLeftText            ; Write sub-string left from clickable
mov r8d,[r14 + CLICKSTRING.clickStart]
mov rdx,rbx
call HelperTextSize            ; Get EAX = sub-string width left from clickable 
add eax,r12d
mov [rdi + RECT.left],eax
mov [r14 + CLICKSTRING.xmin],eax
mov edx,ABOUT_CLICK_COLOR      ; Parm#2 = RDX = Color code
mov rcx,r13                    ; Parm#1 = RCX = hDC, handle display context
call [SetTextColor]
push rax
mov r8d,[r14 + CLICKSTRING.clickSize]
mov edx,[r14 + CLICKSTRING.clickStart]
add rdx,rbx
call HelperLeftText
pop rdx                        ; Parm#2 = RDX = Color code
mov rcx,r13                    ; Parm#1 = RCX = hDC, handle display context
call [SetTextColor]
mov r8d,[r14 + CLICKSTRING.clickStart]
add r8d,[r14 + CLICKSTRING.clickSize]
mov rdx,rbx
call HelperTextSize
add eax,r12d
mov [rdi + RECT.left],eax
mov [r14 + CLICKSTRING.xmax],eax
mov r8,-1
mov edx,[r14 + CLICKSTRING.clickStart]
add edx,[r14 + CLICKSTRING.clickSize]
add rdx,rbx
call HelperLeftText
add r14,sizeof.CLICKSTRING
add ebp,ABOUT_YADD2
cmp ebp,ABOUT_YBASE2 + ABOUT_YADD2 
jbe .clickableStrings 
add rsp,32
pop rbp rbx
;---------- Paint window elements done ----------------------------------------;
mov rdx,[r15 + ABOUTBOX.hFontBack]
test rdx,rdx
jz @f                         ; Parm#2 = RDX = HGDIOBJ, handle GDI object
mov rcx,r13                   ; Parm#1 = RCX = hDC, handle display context
call [SelectObject]
@@:
mov rdx,rsi                   ; Parm#2 = RDX = Pointer to PAINT structure
mov rcx,rbx                   ; Parm#1 = RCX = Window handle
call [EndPaint]
jmp .statusOne  ; .statusZero , experimentally changed because background redraw when other window overlap this 

;---------- Helper for detect mouse cursor position at clickable text ---------;
;                                                                              ;
; INPUT  :  R9D = Mouse cursor window position, bits[31-16]=Y, bits[15-0]=X    ;
;           R15 = Pointer to ABOUT_BOX data structure                          ;
;           Memory structure (LINEVAR) used                                    ; 
;                                                                              ;
; OUTPUT :  CF = result, 0 = Mouse cursor outside, 1 = Indide                  ;
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperCheckLine4:
lea rcx,[r15 + ABOUTBOX.clickStr1]  ; Pointer for clickable string 1
jmp helperCheckLine
HelperCheckLine5:
lea rcx,[r15 + ABOUTBOX.clickStr2]  ; Pointer for clickable string 2
helperCheckLine:
movzx eax,r9w
mov edx,r9d
shr edx,16
cmp eax,[rcx + CLICKSTRING.xmin]
jb @f
cmp eax,[rcx + CLICKSTRING.xmax]
ja @f
cmp edx,[rcx + CLICKSTRING.ymin]
jb @f
cmp edx,[rcx + CLICKSTRING.ymax]
ja @f
stc
ret
@@:
clc
ret

;---------- Helper for draw text string ---------------------------------------;
;                                                                              ;
; INPUT  : R13 = Handle Device Context (HDC)                                   ;
;          RDX = Pointer to text string                                        ;
;          R8  = Length of text string or -1 if 0-terminated string            ;
;          RDI = Pointer to RECT structure for string positioning at window    ;
;                                                                              ;
; OUTPUT : None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperLeftText:
mov eax,DT_LEFT + DT_NOPREFIX    ; Entry point for Left-aligned, R8=Length
jmp helperText
HelperCenterText:
mov eax,DT_CENTER + DT_NOPREFIX  ; Entry point for Center-aligned, 0-terminated
mov r8d,-1
helperText:
push rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
push 0             ; This PUSH 0 for alignment
push rax           ; Parm#5 = Geometry flags 
sub rsp,32
mov r9,rdi         ; Parm#3 = R8 = Text length , Parm#4 = R9 = Pointer to RECT 
mov rcx,r13        ; Parm#1 = RCX = hDC , Parm#2 = RDX = Pointer to text
call [DrawText]
ExitRbp:
mov rsp,rbp
pop rbp
ret

;---------- Helper for get text string pixel width ----------------------------;
;                                                                              ;
; INPUT  : R13 = Handle Device Context (HDC)                                   ;
;          RDX = Pointer to text string, string without 0 termination          ;
;          R8  = Length of text string                                         ;
;          R15 = Pointer to ABOUT_BOX data structure                           ;
;                                                                              ;
; OUTPUT : EAX = pixel width                                                   ;
;          Memory structure (sz) updated                                       ;
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperTextSize:
push rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
lea r9,[r15 + ABOUTBOX.sz]    ; Parm#3 = R8 = Length , Parm#4 = R9 = Pointer to SIZE structure
mov rcx,r13                   ; Parm#1 = RCX = hDC   , Parm#2 = RDX = Pointer
call [GetTextExtentPoint32]
mov eax,[r15 + ABOUTBOX.sz + SIZE.cx]
jmp ExitRbp
