;---------- Callback dialogue procedure ---------------------------------------;
;           Handler for "Please wait" window.                                  ;
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
DialogProcChildWait:
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
;              WM_INITDIALOG handler: create "Please wait" window.             ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wminitdialog:
mov eax,PARM_LPARAM
mov [WAIT_BOX.mode],eax    ; Select CPU or Memory benchmark comment string
;---------- Set window caption text string ------------------------------------;
mov esi,[APP_DATA.lockedStrings]
mov ax,STR_PW_RUNNING
call IndexString
push esi                   ; Parm#2 = Pointer to text string
push ebx                   ; Parm#1 = Window handle , push PARM_HWNDDLG
call [SetWindowText]       ; Set main window text title
;---------- Create font -------------------------------------------------------;
xor eax,eax                ; EAX = 0 for compact push 0
push eax                   ; Parm#14 = Pointer to font typename string, here not used
push VARIABLE_PITCH        ; Parm#13 = Font pitch and family
push CLEARTYPE_QUALITY     ; Parm#12 = Output quality
push CLIP_DEFAULT_PRECIS   ; Parm#11 = Clip precision
push OUT_OUTLINE_PRECIS    ; Parm#10 = Output precision
push DEFAULT_CHARSET       ; Parm#9  = Charset
push eax                   ; Parm#8  = Strike, here=0=none
push eax                   ; Parm#7  = Underline, here=0=none
push TRUE                  ; Parm#6  = Italic, here=0=none
push FW_DONTCARE           ; Parm#5  = Weight of the font
push eax                   ; Parm#4 = Orientation
push eax                   ; Parm#3 = Escapment
push eax                   ; Parm#2 = Width
push 17                    ; Parm#1 = Height
call [CreateFont]
mov [WAIT_BOX.hFont],eax 
jmp .statusOne

;------------------------------------------------------------------------------;
;                                                                              ;
;                     WM_CLOSE handler: close window.                          ;
;   Note this handler is redundant (not take control) in the current design.   ;
;   Note about delete font required. Use WM_CLOSE message externally.          ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmclose:
mov ecx,[WAIT_BOX.hFont]
jecxz @f
push ecx                       ; Parm#1 = Object handle
call [DeleteObject]
@@:
push ebx                       ; Parm#1 = Window handle , push PARM_HWNDDLG 
call [DestroyWindow]
jmp .statusZero

;------------------------------------------------------------------------------;
;                                                                              ;
;                 WM_PAINT handler. Draw GUI window content.                   ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmpaint:
mov esi,WAIT_BOX.ps
push esi                   ; Parm#2 = Pointer to PAINT structure
push ebx                   ; Parm#1 = Window handle
call [BeginPaint]
test eax,eax
jz .statusZero
xchg ebp,eax
;---------- Get window geometry: width and height -----------------------------;
; Note don't use RECT after BeginPaint,
; it can be partial area, required [GetClientRect].
push esi
mov esi,WAIT_BOX.backRect
push esi                       ; Parm#2 = Pointer to RECT structure 
push ebx                       ; Parm#1 = Window handle 
call [GetClientRect]
mov edi,WAIT_BOX.rect
push edi
mov ecx,4
cld
rep movsd
pop edi esi
;---------- Set window transparency for text write ----------------------------;
push TRANSPARENT               ; Parm#2 = Mode
push ebp                       ; Parm#1 = hDC  
call [SetBkMode]
;---------- Fill window by background color -----------------------------------;
push COLOR_HIGHLIGHT ; COLOR_WINDOW + 1  ; Parm#3 = Color (or HBRUSH)
push edi                                 ; Parm#2 = Pointer to RECT structure
push ebp                                 ; Parm#1 = hDC
call [FillRect] 
;---------- Draw central picture ----------------------------------------------;
push [APP_DATA.createdBigIcons + 4]   ; Parm#4 = Handle icon
push WAIT_ICONY                       ; Parm#3 = Base Y coordinate
push WAIT_ICONX                       ; Parm#2 = Base X coordinate 
push ebp                              ; Parm#1 = hDC, handle display context
call [DrawIcon]
;---------- Support text output -----------------------------------------------;
xor eax,eax
mov ecx,[WAIT_BOX.hFont]
jecxz @f
push ecx                          ; Parm#2 = HGDIOBJ, object handle
push ebp                          ; Parm#1 = hDC, handle display context
call [SelectObject]
@@:
mov [WAIT_BOX.hFontBack],eax
;---------- Up string: benchmark name -----------------------------------------;
push esi
mov esi,[APP_DATA.lockedStrings]
push esi
mov eax,STR_PW_CPU_BENCH
add eax,[WAIT_BOX.mode]
call IndexString
mov ecx,esi
@@:
lodsb
cmp al,0
jne @b
lea edx,[esi - 1]
sub edx,ecx
mov [edi + RECT.top],WAIT_YBASE1
mov [edi + RECT.bottom],WAIT_YBASE1 + WAIT_YADD
call HelperCenterText      ; Write upper string: benchmark type
;---------- Down string: "Please wait..." -------------------------------------;
pop esi
mov ax,STR_PW_PLEASE_WAIT
call IndexString
mov ecx,esi
@@:
lodsb
cmp al,0
jne @b
lea edx,[esi - 1]
sub edx,ecx
mov [edi + RECT.top],WAIT_YBASE2
mov [edi + RECT.bottom],WAIT_YBASE2 + WAIT_YADD
call HelperCenterText      ; Write upper string: benchmark type
pop esi
;---------- Paint window elements done ----------------------------------------; 
mov ecx,[WAIT_BOX.hFontBack]
jecxz @f
push ecx                   ; Parm#2 = HGDIOBJ, handle GDI object
push ebp                   ; Parm#1 = hDC, handle display context
call [SelectObject]
@@:
push esi                   ; Parm#2 = Pointer to PAINT structure
push ebx                   ; Parm#1 = Window handle
call [EndPaint]
jmp .statusOne  ; .statusZero , experimentally changed because background redraw when other window overlap this

