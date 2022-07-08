;---------- Callback dialogue procedure ---------------------------------------;
;           Handler for "About" window, item in the application main menu.     ; 
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
DialogProcChildAbout:
push ebp
mov ebp,esp
push ebx esi edi
mov eax,PARM_MSG
mov ebx,PARM_HWNDDLG
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
pop ebp edi esi ebx
ret 16

;------------------------------------------------------------------------------;
;                                                                              ;
;               WM_INITDIALOG handler: create "About" window.                  ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wminitdialog:
mov ax,BINDER_BUILD_ABOUT      ; Resource binder script
call Binder
mov esi,[APP_DATA.lockedStrings]
mov ax,STR_AB_PROGRAM_INFO
call IndexString
push esi
push ebx                       ; compact push instead PUSH PARM_HWNDDLG
call [SetWindowText]           ; Window caption string
push [APP_DATA.hIcon]          ; Parm#4 = lParam = Icon handle
push ICON_SMALL                ; Parm#3 = wParam = Set small icon message
push WM_SETICON                ; Parm#2 = Msg 
push ebx                       ; Parm#1 = hWnd , push PARM_HWNDDLG
call [SendMessage]
push IDC_HAND                  ; Parm#2 = Cursor name, here constant
push 0                         ; Parm#1 = hInstance, here not used
call [LoadCursor]
mov [ABOUT_BOX.hCursor],eax
xor eax,eax                    ; EAX = 0 for compact push 0
push eax                       ; Parm#14 = Pointer to font typename string, here not used
push VARIABLE_PITCH            ; Parm#13 = Font pitch and family
push CLEARTYPE_QUALITY         ; Parm#12 = Output quality
push CLIP_DEFAULT_PRECIS       ; Parm#11 = Clip precision
push OUT_OUTLINE_PRECIS        ; Parm#10 = Output precision
push DEFAULT_CHARSET           ; Parm#9  = Charset
push eax                       ; Parm#8  = Strike, here=0=none
push eax                       ; Parm#7  = Underline, here=0=none
push eax                       ; Parm#6  = Italic, here=0=none
push FW_DONTCARE               ; Parm#5  = Weight of the font
push eax                       ; Parm#4 = Orientation
push eax                       ; Parm#3 = Escapment
push eax                       ; Parm#2 = Width
push 16                        ; Parm#1 = Height
call [CreateFont]
mov [ABOUT_BOX.hFont],eax 
jmp .statusOne

;------------------------------------------------------------------------------;
;                                                                              ;
;                     WM_CLOSE handler: close window.                          ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmclose:
mov ecx,[ABOUT_BOX.hFont]
jecxz @f
push ecx                       ; Parm#1 = Object handle
call [DeleteObject]
@@:
push 0                         ; Parm#2 = nResult, value for return
push ebx                       ; Parm#1 = Window handle , push PARM_HWNDDLG
call [EndDialog]
jmp .statusZero

;------------------------------------------------------------------------------;
;                                                                              ;
;                 WM_COMMAND handler: interpreting user input.                 ; 
;                 Detect click "OK" button at "About" window.                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmcommand:
cmp PARM_WPARAM,IDB_ABOUT_OK
je .wmclose
jmp .statusZero

;------------------------------------------------------------------------------;
;                                                                              ;
;        WM_LBUTTONDOWN handler: interpreting mouse left button click.         ; 
;          Provide mouse cursor consistency and go web link if click.          ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmlbuttondown:
mov eax,PARM_LPARAM
call HelperCheckLine4
mov ax,STR_AB_GITHUB_LINK
jc .clicked 
mov eax,PARM_LPARAM
call HelperCheckLine5
mov ax,STR_AB_FASM_LINK
jnc .statusZero
.clicked:
mov esi,[APP_DATA.lockedStrings]
call IndexString
push esi
mov ecx,[ABOUT_BOX.hCursor]
jecxz @f
push ecx            ; Parm#1 = Cursor handle
call [SetCursor]
@@:
pop ecx
xor eax,eax
push SW_NORMAL      ; Parm#6 = Command flags
push eax            ; Parm#5 = Pointer to directory name, 0 means not used
push eax            ; Parm#4 = Pointer to parameter string, 0 means not used
push ecx            ; Parm#3 = Pointer to file or object name
push eax            ; Parm#2 = Operation, 0 means not used
push eax            ; Parm#1 = hWnd, 0 means not associated
call [ShellExecute]
cmp eax,32
ja @f
mov esi,[APP_DATA.lockedStrings]
mov ax,STR_AB_SHELL_ERROR
call IndexString
push MB_ICONERROR   ; Parm#4 = Attributes
push 0              ; Parm#3 = Pointer to title (caption) string
push esi            ; Parm#2 = Pointer to string: error name 
push 0              ; Parm#1 = Parent Window or NULL
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
mov eax,PARM_LPARAM
call HelperCheckLine4
jc @f
call HelperCheckLine5
jnc .statusZero
@@:
.change:
mov ecx,[ABOUT_BOX.hCursor]
jecxz @f
push ecx            ; Parm#1 = Cursor handle
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
mov eax,PARM_LPARAM
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
mov esi,ABOUT_BOX.ps
push esi                   ; Parm#2 = Pointer to PAINT structure
push ebx                   ; Parm#1 = Window handle
call [BeginPaint]
test eax,eax
jz .statusZero
xchg ebp,eax
xor eax,eax
push eax eax eax eax       ; Reserve 4 dwords at stack frame
;---------- Get window geometry: width and height -----------------------------;
; Note don't use RECT after BeginPaint,
; it can be partial area, required [GetClientRect].
push esi
mov esi,ABOUT_BOX.backRect
push esi                       ; Parm#2 = Pointer to RECT structure 
push ebx                       ; Parm#1 = Window handle 
call [GetClientRect]
mov edi,ABOUT_BOX.rect
push edi
mov ecx,3
cld
rep movsd
lodsd
sub eax,ABOUT_YBOTTOM
stosd
pop edi esi
;---------- Set window transparency for text write ----------------------------;
push TRANSPARENT               ; Parm#2 = Mode
push ebp                       ; Parm#1 = hDC  
call [SetBkMode]
;---------- Fill window by background color -----------------------------------;
push COLOR_WINDOW + 1             ; Parm#3 = Color (or HBRUSH)
push edi                          ; Parm#2 = Pointer to RECT structure
push ebp                          ; Parm#1 = hDC
call [FillRect] 
;---------- Draw central picture ----------------------------------------------;
push [APP_DATA.createdBigIcons]   ; Parm#4 = Handle icon
push ABOUT_ICONY                  ; Parm#3 = Base Y coordinate
push ABOUT_ICONX                  ; Parm#2 = Base X coordinate 
push ebp                          ; Parm#1 = hDC, handle display context
call [DrawIcon]
;---------- Support text output -----------------------------------------------;
xor eax,eax
mov ecx,[ABOUT_BOX.hFont]
jecxz @f
push ecx                          ; Parm#2 = HGDIOBJ, object handle
push ebp                          ; Parm#1 = hDC, handle display context
call [SelectObject]
@@:
mov [ABOUT_BOX.hFontBack],eax
push ABOUT_BOX.tm                 ; Parm#2 = Pointer to TEXTMETRIC structure
push ebp                          ; Parm#1 = hDC, handle display context
call [GetTextMetrics]
;---------- Strings 1-3, without clickable elements ---------------------------;
push ebx esi
cld
mov esi,ABOUT_NAME
mov ebx,ABOUT_YBASE1
.next1:
mov ecx,esi
@@:
lodsb
cmp al,0Ah
je @f
cmp al,0Dh
je @f
cmp al,0
jne @b
@@:
lea edx,[esi - 1]
sub edx,ecx
mov [edi + RECT.top],ebx
add ebx,ABOUT_YADD1
mov [edi + RECT.bottom],ebx
call HelperCenterText
dec esi
@@:
lodsb
cmp al,0Ah
je @b
cmp al,0Dh
je @b
cmp al,0
je .stop1
dec esi
jmp .next1
.stop1:
pop esi ebx
;---------- Strings 4-5, with clickable elements ------------------------------;
push ebx
mov ebx,ABOUT_BOX.clickStr1
mov [ebx + CLICKSTRING.stringId],STR_AB_GITHUB
mov [ebx + CLICKSTRING.fullSize],ABOUT_FULL_SIZE_1
mov [ebx + CLICKSTRING.clickStart],ABOUT_CLICK_START_1
mov [ebx + CLICKSTRING.clickSize],ABOUT_CLICK_SIZE_1
mov [ebx + CLICKSTRING.stringId + sizeof.CLICKSTRING],STR_AB_FASM
mov [ebx + CLICKSTRING.fullSize + sizeof.CLICKSTRING],ABOUT_FULL_SIZE_2
mov [ebx + CLICKSTRING.clickStart + sizeof.CLICKSTRING],ABOUT_CLICK_START_2
mov [ebx + CLICKSTRING.clickSize + sizeof.CLICKSTRING],ABOUT_CLICK_SIZE_2
mov dword [esp + 04],ABOUT_YBASE2
.clickableStrings:
push esi
mov esi,[APP_DATA.lockedStrings]
mov eax,[ebx + CLICKSTRING.stringId]
call IndexString
mov [esp + 08 + 04],esi
mov edx,esi
pop esi
mov eax,[esp + 04]
mov [ebx + CLICKSTRING.ymin],eax
add eax,[ABOUT_BOX.tm + TEXTMETRIC.tmHeight] 
mov [ebx + CLICKSTRING.ymax],eax
mov ecx,[ebx + CLICKSTRING.fullSize]
call HelperTextSize            ; Get EAX = full string width
mov edx,[ABOUT_BOX.backRect + RECT.right]
sub edx,[ABOUT_BOX.backRect + RECT.left]
sub edx,eax
shr edx,1
mov [esp + 12],edx
mov [edi + RECT.left],edx
mov eax,[esp + 04]
mov [edi + RECT.top],eax
add eax,ABOUT_YADD2
mov [edi + RECT.bottom],eax
mov edx,[ebx + CLICKSTRING.clickStart]
mov ecx,[esp + 08]
call HelperLeftText            ; Write sub-string left from clickable
mov ecx,[ebx + CLICKSTRING.clickStart]
mov edx,[esp + 08]
call HelperTextSize            ; Get EAX = sub-string width left from clickable 
add eax,[esp + 12]
mov [edi + RECT.left],eax
mov [ebx + CLICKSTRING.xmin],eax
push ABOUT_CLICK_COLOR         ; Parm#2 = Color code
push ebp                       ; Parm#1 = hDC, handle display context
call [SetTextColor]
push eax                       ; Parm#2 = Color code, for next set color
mov edx,[ebx + CLICKSTRING.clickSize]
mov ecx,[ebx + CLICKSTRING.clickStart]
add ecx,[esp + 08 + 04]
call HelperLeftText
push ebp                       ; Parm#1 = hDC, handle display context
call [SetTextColor]
mov ecx,[ebx + CLICKSTRING.clickStart]
add ecx,[ebx + CLICKSTRING.clickSize]
mov edx,[esp + 08]
call HelperTextSize
add eax,[esp + 12]
mov [edi + RECT.left],eax
mov [ebx + CLICKSTRING.xmax],eax
mov edx,-1
mov ecx,[ebx + CLICKSTRING.clickStart]
add ecx,[ebx + CLICKSTRING.clickSize]
add ecx,[esp + 08]
call HelperLeftText
add ebx,sizeof.CLICKSTRING
add dword [esp + 04],ABOUT_YADD2
cmp dword [esp + 04],ABOUT_YBASE2 + ABOUT_YADD2 
jbe .clickableStrings 
pop ebx
add esp,16                 ; Remove 4 dwords from stack frame
;---------- Paint window elements done ----------------------------------------; 
mov ecx,[ABOUT_BOX.hFontBack]
jecxz @f
push ecx                   ; Parm#2 = HGDIOBJ, handle GDI object
push ebp                   ; Parm#1 = hDC, handle display context
call [SelectObject]
@@:
push esi                   ; Parm#2 = Pointer to PAINT structure
push ebx                   ; Parm#1 = Window handle
call [EndPaint]
jmp .statusOne  ; .statusZero , experimentally changed because background redraw when other window overlap this

;---------- Helper for detect mouse cursor position at clickable text ---------;
;                                                                              ;
; INPUT  :  EAX = Mouse cursor window position, bits[31-16]=Y, bits[15-0]=X    ;
;           Memory structure (LINEVAR) used                                    ; 
;                                                                              ;
; OUTPUT :  CF = result, 0 = Mouse cursor outside, 1 = Indide                  ;
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperCheckLine4:
mov ecx,ABOUT_BOX.clickStr1  ; Pointer for clickable string 1
jmp helperCheckLine
HelperCheckLine5:
mov ecx,ABOUT_BOX.clickStr2  ; Pointer for clickable string 2
helperCheckLine:
push eax
mov edx,eax
shr edx,16
movzx eax,ax
cmp eax,[ecx + CLICKSTRING.xmin]
jb @f
cmp eax,[ecx + CLICKSTRING.xmax]
ja @f
cmp edx,[ecx + CLICKSTRING.ymin]
jb @f
cmp edx,[ecx + CLICKSTRING.ymax]
ja @f
pop eax
stc
ret
@@:
pop eax
clc
ret

;---------- Helper for draw text string ---------------------------------------;
;                                                                              ;
; INPUT  : EBP = Handle Device Context (HDC)                                   ;
;          ECX = Pointer to text string                                        ;
;          EDX = Length of text string or -1 if 0-terminated string            ;
;          EDI = Pointer to RECT structure for string positioning at window    ;
;                                                                              ;
; OUTPUT : None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperLeftText:
mov eax,DT_LEFT + DT_NOPREFIX    ; Entry point for Left-aligned, EDX=Length
jmp helperText
HelperCenterText:
mov eax,DT_CENTER + DT_NOPREFIX  ; Entry point for Center-aligned, 0-terminated
mov edx,-1
helperText:
push eax           ; Parm#5 = Geometry flags 
push edi           ; Parm#4 = Pointer to RECT
push edx           ; Parm#3 = Text length
push ecx           ; Parm#2 = Pointer to text 
push ebp           ; Parm#1 = hDC
call [DrawText]
ret

;---------- Helper for get text string pixel width ----------------------------;
;                                                                              ;
; INPUT  : EBP = Handle Device Context (HDC)                                   ;
;          EDX = Pointer to text string, string without 0 termination          ;
;          ECX = Length of text string                                         ;
;                                                                              ;
; OUTPUT : EAX = pixel width                                                   ;
;          Memory structure (sz) updated                                       ;
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperTextSize:
push ABOUT_BOX.sz              ; Parm#4 = Pointer to SIZE structure
push ecx                       ; Parm#3 = Length
push edx                       ; Parm#2 = Pointer
push ebp                       ; Parm#1 = hDC
call [GetTextExtentPoint32]
mov eax,[ABOUT_BOX.sz + SIZE.cx]
ret
