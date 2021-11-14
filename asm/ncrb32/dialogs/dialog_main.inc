;---------- Callback dialogue procedure for main window -----------------------;
; Note. This ESP values at procedure entry, +4 because EIP pushed.             ;
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
DialogProcMain:
push ebp
mov ebp,esp
push ebx esi edi
lea edi,[APP_DATA]                   ; EDI = Pointer to application data
lea ebx,[edi + APPDATA.tabCtrlItem]  ; EBX = Pointer to tab item structure
lea esi,[edi + APPDATA.hTabDlg]      ; ESI = Pointer to sheets handles array
;---------- Detect message type -----------------------------------------------;
mov eax,PARM_MSG
cmp eax,WM_INITDIALOG
je .wminitdialog
cmp eax,WM_COMMAND
je .wmcommand
cmp eax,WM_CLOSE
je .wmclose
cmp eax,WM_NOTIFY
je .tabproc
.skip:
xor eax,eax
jmp .finish
;---------- WM_INITDIALOG handler: create main window -------------------------;
.wminitdialog:                   ; Build main window 
mov eax,PARM_HWNDDLG
mov [edi + APPDATA.hMain],eax
push IDC_TAB                         ; Parm#2 = Item identifier
push PARM_HWNDDLG                    ; Parm#1 = Handle to dialog box
call [GetDlgItem]                    ; Get handle
mov [edi + APPDATA.hTab],eax         ; Store sheets container handle
;---------- Initializing sheet structure --------------------------------------;
xor eax,eax
mov [ebx + TC_ITEM.mask],TCIF_TEXT + TCIF_IMAGE
mov [ebx + TC_ITEM.lpReserved1],eax
mov [ebx + TC_ITEM.lpReserved2],eax
mov [ebx + TC_ITEM.lParam],eax
mov [ebx + TC_ITEM.cchTextMax],64  ; Maximum text size
;---------- Create image list for icons ---------------------------------------;
push 0                           ; Parm#5 = cGrow = not used
push ICON_COUNT                  ; Parm#4 = Images count 
push ILC_COLOR32 + ILC_MASK      ; Parm#3 = Images flags
push 16                          ; Parm#2 = Y size
push 16                          ; Parm#1 = X size 
call [ImageList_Create]
mov [edi + APPDATA.hImageList],eax  ; Store image list handle 
;---------- Initialize cycle for create icons from resource -------------------;
push esi edi
lea esi,[edi + APPDATA.lockedIcons]
mov edi,ICON_COUNT
;---------- Cycle for create icons from resource ------------------------------;
.createIcons:
lodsd
push LR_DEFAULTCOLOR             ; Parm#7 = Flags
push 16                          ; Parm#6 = cyDesired
push 16                          ; Parm#5 = cxDesired
push 30000h                      ; Parm#4 = Version of icon format
push TRUE                        ; Parm#3 = Icon/Cursor, True means Icon
push 468h                        ; Parm#2 = dwResSize, bytes 
push eax                         ; Parm#1 = Pointer to resource bits
call [CreateIconFromResourceEx]  ; Create icon, return handle
mov [esi + ICON_COUNT*4 - 4],eax
push eax                         ; Parm#2 = Handle to icon
push [APP_DATA.hImageList]       ; Parm#1 = Handle to image list
call [ImageList_AddIcon]
dec edi
jnz .createIcons
pop edi esi
push [edi + APPDATA.hImageList]  ; Parm#4 = LPARAM = image list handle
push 0                           ; Parm#3 = WPARAM = not used = 0
push TCM_SETIMAGELIST            ; Parm#2 = Message
push [edi + APPDATA.hTab]        ; Parm#1 = Container window handle
call [SendMessage]               ; Link image list with control
;---------- Initialize cycle for insert items with strings and icons ----------;
push esi edi
mov esi,[edi + APPDATA.lockedStrings]
xor edi,edi
;---------- Cycle for insert items to tabbed panel ----------------------------;
.insertSheets:
mov eax,esi
mov [ebx + TC_ITEM.pszText],eax
mov [ebx + TC_ITEM.iImage],edi
push ebx                              ; Parm#4 = LPARAM = pointer to TCITEM
push ITEM_COUNT - 1                   ; Parm#3 = WPARAM = index for new tab
push TCM_INSERTITEM                   ; Parm#2 = Message
push [APP_DATA.hTab]  ; Parm#1 = Container window handle
call [SendMessage]                    ; Add third sheet to tabbed panel
cld
.skipString:
lodsb
cmp al,0
jne .skipString                  ; Cycle for skip string
inc edi
cmp edi,ITEM_COUNT
jb .insertSheets                 ; Cycle for insert all sheets
pop edi esi
;---------- Set item size for container ---------------------------------------;
push ( 27 shl 16 + 97 )          ; Parm#4 = LPARAM = [Ysize][Xsize]
push 0                           ; Parm#3 = WPARAM = not used = 0
push TCM_SETITEMSIZE             ; Parm#2 = Message
push [edi + APPDATA.hTab]        ; Parm#1 = Container window handle
call [SendMessage]               ; Set sheets size 
;---------- Initializing cycle for create dialogues per sheets ----------------;
push esi edi
mov edi,esi
mov esi,IDD_FIRST
lea ebx,[ProcDialogs]
mov ecx,ITEM_COUNT
;---------- Cycle for create dialogues per sheets -----------------------------;
.createDialogues:
push ecx
push 0                            ; Parm#5 = Passed parameter = not used = 0
push dword [ebx]                  ; Parm#4 = Pointer to callback procedure
push PARM_HWNDDLG                 ; Parm#3 = Container window handle
push esi                          ; Parm#2 = Dialog box resource id
push [APP_DATA.hResources]   ; Parm#1 = Resource module handle
call [CreateDialogParam]          ; Set dialogue with handler for third sheet
stosd                             ; Store third sheet handle
inc esi
pop ecx
add ebx,4
loop .createDialogues             ; Create dialogues cycle for all sheets
pop edi esi
;---------- Cycle for find and select active sheet dialogue window ------------;
mov ecx,ITEM_COUNT
.findActive:
push ecx
mov eax,SW_HIDE                    ; This for all sheets exclude first
cmp ecx,ITEM_COUNT
jne .showThis
mov eax,SW_SHOWDEFAULT             ; This for first sheet, activate it
.showThis:
push eax                           ; Parm#2 = Window activity mode
lodsd
push eax                           ; Parm#1 = Window handle
call [ShowWindow]
pop ecx
loop .findActive
;---------- Select active sheet at container ----------------------------------;
xor eax,eax                          ; EAX = 0 for compact PUSH 0
mov [edi + APPDATA.selectedTab],eax  ; Active sheet = 0  
push eax                             ; Parm#4 = LPARAM = not used = 0 
push eax                             ; Parm#3 = WPARAM = index = [selectedTab]
push TCM_SETCURSEL                   ; Parm#2 = Message
push [edi + APPDATA.hTab]            ; Parm#1 = Container window handle
call [SendMessage]                   ; Set current selected sheet
;---------- Main window icon and text title -----------------------------------;
push [edi + APPDATA.hIcon]        ; Parm#4 = LPARAM = Icon handle
push ICON_SMALL                   ; Parm#3 = WPARAM = Icon type
push WM_SETICON                   ; Parm#2 = Message 
push PARM_HWNDDLG                 ; Parm#1 = Window handle
call [SendMessage]                ; Set main window icon
push PROGRAM_NAME                 ; Parm#2 = Pointer to text string
push PARM_HWNDDLG                 ; Parm#1 = Window handle
call [SetWindowText]              ; Set main window text title
jmp .processed
;---------- WM_COMMAND handler: interpreting user input -----------------------;
.wmcommand:                       ; User input: cancel button or close window
;---------- Detect click "About" item in the main menu ------------------------;
mov eax,PARM_WPARAM
cmp ax,IDM_ABOUT
jne .noabout
;---------- "About" message box and wait user input ---------------------------;
push MB_ICONINFORMATION   ; Parm#4 = Message box icon type = Info
push ABOUT_CAP            ; Parm#3 = Pointer to caption
push ABOUT_NAME           ; Parm#2 = Pointer to "About" string
push PARM_HWNDDLG         ; Parm#1 = Parent window handle
call [MessageBoxA]
jmp .processed
.noabout:
;---------- Detect click "Exit" item in the main menu -------------------------; 
cmp ax,IDM_EXIT
je .wmclose
jmp .processed
;---------- WM_NOTIFY handler: events from child GUI objects ------------------;
.tabproc:                         ; Change sheet selection
cmp PARM_WPARAM,IDC_TAB
jne .skip
mov eax,PARM_LPARAM
cmp [eax + NMHDR.code],TCN_SELCHANGE
jne .skip                            ; Skip if other event, no sheet change
mov eax,[edi + APPDATA.selectedTab]
push SW_HIDE                         ; Parm#2 = Window activity mode 
push dword [esi + eax*4]             ; Parm#1 = Window handle 
call [ShowWindow]                    ; Hide current sheet
push 0                               ; Parm#4 = LPARAM = not used = 0
push 0                               ; Parm#3 = WPARAM = not used = 0
push TCM_GETCURSEL                   ; Parm#2 = Message
push [edi + APPDATA.hTab]            ; Parm#1 = Container window handle
call [SendMessage]                   ; Get current selected sheet number 
mov [edi + APPDATA.selectedTab],eax  ; Update current selected sheet number 
push SW_SHOWDEFAULT
push dword [esi + eax*4]
call [ShowWindow]                    ; Show current selected sheet
jmp .processed
;---------- WM_CLOSE handler: close window ------------------------------------;
.wmclose:
push 1                            ; Parm#2 = Result for return
push PARM_HWNDDLG                 ; Parm#1 = Window handle
call [EndDialog]
;---------- Exit points -------------------------------------------------------;
.processed:
mov eax,1
.finish:
pop edi esi ebx ebp
ret 16
