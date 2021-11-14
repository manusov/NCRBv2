;---------- Callback dialogue procedure for main window -----------------------;
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
DialogProcMain:
cld
push rbp rbx rsi rdi r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h           ; Stack alignment
sub rsp,32                           ; Make parameters shadow for next calls
mov PARM_HWNDDLG,rcx                 ; Save input parameters to shadow 
mov PARM_MSG,rdx
mov PARM_WPARAM,r8
mov PARM_LPARAM,r9
lea r15,[APP_DATA]                   ; R15 = Pointer to application data struc. 
lea rbx,[r15 + APPDATA.tabCtrlItem]  ; RBX = Pointer to tab item structure
lea rsi,[r15 + APPDATA.hTabDlg]      ; RSI = Pointer to sheets handles array
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
cmp eax,WM_NOTIFY
je .tabproc                    ; Go if notification message from child window
.skip:
xor eax,eax
jmp .finish                    ; Go exit if unknown event
;---------- WM_INITDIALOG handler: create main window -------------------------; 
.wminitdialog: 
mov rax,PARM_HWNDDLG
mov [r15 + APPDATA.hMain],rax
mov edx,IDC_TAB                ; RDX = Parm#2 = Item identifier 
mov rcx,PARM_HWNDDLG           ; RCX = Parm#1 = Handle to dialog box
call [GetDlgItem]              ; Get handle
mov [r15 + APPDATA.hTab],rax   ; Store window handle = sheets container handle
;---------- Initializing sheet structure --------------------------------------;
xor eax,eax
mov [rbx + TC_ITEM.mask],TCIF_TEXT + TCIF_IMAGE
mov [rbx + TC_ITEM.lpReserved1],eax
mov [rbx + TC_ITEM.lpReserved2],eax
mov [rbx + TC_ITEM.lParam],eax
mov [rbx + TC_ITEM.cchTextMax],64  ; Maximum text size
;---------- Create image list for icons ---------------------------------------;
push 0 0                         ; Alignment + Parm#5 = cGrow = not used
mov r9d,ICON_COUNT               ; R9  = Parm#4 = Images count
mov r8d,ILC_COLOR32 + ILC_MASK   ; R8  = Parm#3 = Images flags
mov edx,16                       ; RDX = Parm#2 = Y size
mov ecx,16                       ; RCX = Parm#1 = X size
sub rsp,32
call [ImageList_Create]
add rsp,32 + 16
mov [r15 + APPDATA.hImageList],rax   ; Store image list handle 
;---------- Initialize cycle for create icons from resource -------------------;
push rsi rdi
mov edi,ICON_COUNT
lea rsi,[r15 + APPDATA.lockedIcons]
;---------- Cycle for create icons from resource ------------------------------;
.createIcons:
lodsq
push 0                           ; This for stack alignment
push LR_DEFAULTCOLOR             ; Parm#7 = Flags
push 16                          ; Parm#6 = cyDesired
push 16                          ; Parm#5 = cxDesired
mov r9d,30000h                   ; R9  = Parm#4 = Version of icon format
mov r8d,TRUE                     ; R8  = Parm#3 = Icon/Cursor, True means Icon
mov edx,468h                     ; RDX = Parm#2 = dwResSize, bytes 
xchg rcx,rax                     ; RCX = Parm#1 = Pointer to resource bits 
sub rsp,32
call [CreateIconFromResourceEx]     ; Create icon, return handle
add rsp,32 + 32
mov [rsi + ICON_COUNT*8 - 8],rax
xchg rdx,rax                        ; RDX = Parm#2 = Handle to icon
mov rcx,[r15 + APPDATA.hImageList]  ; RCX = Parm#1 = Handle to image list 
sub rsp,32
call [ImageList_AddIcon] 
add rsp,32
dec edi
jnz .createIcons
pop rdi rsi
mov r9,[r15 + APPDATA.hImageList]   ; R9  = Parm#4 = LPARAM = image list handle     
xor r8d,r8d                         ; R8  = Parm#3 = WPARAM = not used = 0                 
mov edx,TCM_SETIMAGELIST            ; RDX = Parm#2 = Message
mov rcx,[r15 + APPDATA.hTab]        ; RCX = Parm#1 = Container window handle 
call [SendMessage]                  ; Link image list with control
;---------- Initialize cycle for insert items with strings and icons ----------;
push rsi
sub rsp,32 + 8                      ; Stack alignment and parameters shadow
mov rsi,[r15 + APPDATA.lockedStrings]
xor edi,edi
;---------- Cycle for insert items to tabbed panel ----------------------------;
.insertSheets:
mov rax,rsi
mov [rbx + TC_ITEM.pszText],rax
mov [rbx + TC_ITEM.iImage],edi
mov r9,rbx                       ; R9  = Parm#4 = LPARAM = pointer to TCITEM
mov r8d,ITEM_COUNT - 1           ; R8  = Parm#3 = WPARAM = index for new tab 
mov edx,TCM_INSERTITEM           ; RDX = Parm#2 = Message
mov rcx,[r15 + APPDATA.hTab]     ; RCX = Parm#1 = Container window handle
call [SendMessage]               ; Add this sheet to tabbed panel
.skipString:
lodsb
cmp al,0
jne .skipString                  ; Cycle for skip string
inc edi
cmp edi,ITEM_COUNT
jb .insertSheets                 ; Cycle for insert all sheets
add rsp,32 + 8
pop rsi
;---------- Set item size for container ---------------------------------------;
mov r9d,( 27 shl 16 + 97 )       ; R9  = Parm#4 = LPARAM = [Ysize][Xsize]
xor r8d,r8d                      ; R8  = Parm#3 = WPARAM = not used = 0
mov edx,TCM_SETITEMSIZE          ; RDX = Parm#2 = Message
mov rcx,[r15 + APPDATA.hTab]     ; RCX = Parm#1 = Container window handle
call [SendMessage]               ; Set sheets size 
;---------- Initializing cycle for create dialogues per sheets ----------------; 
push rsi rsi                     ; Second push for stack alignment
mov rdi,rsi
mov esi,IDD_FIRST
lea rbx,[ProcDialogs]
mov ecx,ITEM_COUNT
;---------- Cycle for create dialogues per sheets -----------------------------;
.createDialogues:
push rcx                         ; RCX saved and stack alignment
push 0                           ; Parm#5 = Passed parameter = not used = 0
mov r9,[rbx]                     ; R9  = Parm#4 = Pointer to callback proc.
mov r8,PARM_HWNDDLG              ; R8  = Parm#3 = Container window handle
mov edx,esi                      ; RDX = Parm#2 = Dialog box resource id 
mov rcx,[r15 + APPDATA.hResources]    ; RCX = Parm#1 = Resource module handle
sub rsp,32
call [CreateDialogParam]         ; Set dialogue with handler for sheet
add rsp,32 + 8
stosq                            ; Store this sheet handle
inc esi
pop rcx
add rbx,8
loop .createDialogues            ; Create dialogues cycle for all sheets 
pop rsi rsi                      ; Second pop for stack alignment
;---------- Cycle for find and select active sheet dialogue window ------------;
mov ecx,ITEM_COUNT
.findActive:
push rcx
mov edx,SW_HIDE                      ; This for all sheets exclude first
cmp ecx,ITEM_COUNT 
jne .showThis
mov edx,SW_SHOWDEFAULT               ; This for first sheet, activate it
.showThis:                           ; RDX = Parm#2 = Window activity mode
lodsq
xchg rcx,rax                         ; RCX = Parm#1 = Window handle 
sub rsp,32 + 8
call [ShowWindow]
add rsp,32 + 8
pop rcx
loop .findActive
;---------- Select active sheet at container ----------------------------------;  
xor r9d,r9d                          ; R9  = Parm#4 = LPARAM = not used = 0 
xor r8d,r8d                          ; R8  = Parm#3 = WPARAM = index
mov [r15 + APPDATA.selectedTab],r8d  ; Active sheet = 0
mov edx,TCM_SETCURSEL                ; RDX = Parm#2 = Message
mov rcx,[r15 + APPDATA.hTab]         ; RCX = Parm#1 = Container window handle 
call [SendMessage]                   ; Set current selected sheet
;---------- Main window icon and text title -----------------------------------; 
mov r9,[r15 + APPDATA.hIcon]         ; R9  = Parm#4 = LPARAM = Icon handle 
mov r8d,ICON_SMALL                   ; R8  = Parm#3 = WPARAM = Icon type 
mov edx,WM_SETICON                   ; RDX = Parm#2 = Message 
mov rcx,PARM_HWNDDLG                 ; RCX = Parm#1 = Window handle
call [SendMessage]                   ; Set main window icon
lea rdx,[PROGRAM_NAME]               ; RDX = Parm#2 = Pointer to text string
mov rcx,PARM_HWNDDLG                 ; RCX = Parm#1 = Window handle
call [SetWindowText]                 ; Set main window text title
jmp .processed
;---------- WM_COMMAND handler: interpreting user input -----------------------; 
.wmcommand:                       ; User input: cancel button or close window
;---------- Detect click "About" item in the main menu ------------------------;
mov eax,LOW_WPARAM
cmp ax,IDM_ABOUT
jne .noabout
;---------- "About" message box and wait user input ---------------------------;
mov r9d,MB_ICONINFORMATION   ; R9  = Parm#4 = Message box icon type = Info
lea r8,[ABOUT_CAP]           ; R8  = Parm#3 = Pointer to caption
lea rdx,[ABOUT_NAME]         ; RDX = Parm#2 = Pointer to "About" string
mov rcx,PARM_HWNDDLG         ; RCX = Parm#1 = Parent window handle
call [MessageBoxA]
jmp .processed
;---------- Detect click "Exit" item in the main menu -------------------------; 
.noabout:
cmp ax,IDM_EXIT
je .wmclose
jmp .processed
;---------- WM_NOTIFY handler: events from child GUI objects ------------------;
.tabproc:                         ; Change sheet selection
cmp LOW_WPARAM,IDC_TAB
jne .skip
mov rax,PARM_LPARAM
cmp [rax + NMHDR.code],TCN_SELCHANGE
jne .skip                         ; Skip if other event, no sheet change
mov eax,[r15 + APPDATA.selectedTab]
mov edx,SW_HIDE                   ; RDX = Parm#2 = Window activity mode  
mov rcx,[rsi + rax*8]             ; RCX = Parm#1 = Window handle 
call [ShowWindow]                 ; Hide current sheet
xor r9d,r9d                       ; R9  = Parm#4 = LPARAM = not used = 0
xor r8d,r8d                       ; R8  = Parm#3 = WPARAM = not used = 0
mov edx,TCM_GETCURSEL             ; RDX = Parm#2 = Message
mov rcx,[r15 + APPDATA.hTab]      ; RCX = Parm#1 = Container window handle
call [SendMessage]                ; Get current selected sheet number 
mov [r15 + APPDATA.selectedTab],eax     ; Update current selected sheet number 
mov edx,SW_SHOWDEFAULT            ; RDX = Parm#2 = Window activity mode
mov rcx,[rsi + rax*8]             ; RCX = Parm#1 = Window handle
call [ShowWindow]                 ; Show current selected sheet
jmp .processed
;---------- WM_CLOSE handler: close window ------------------------------------;
.wmclose:
mov edx,1                         ; RDX = Parm#2 = Result for return
mov rcx,PARM_HWNDDLG              ; RCX = Parm#1 = Window handle
call [EndDialog]
;---------- Exit points -------------------------------------------------------;
.processed:
mov eax,1
.finish:
mov rsp,rbp
pop r15 rdi rsi rbx rbp
ret
