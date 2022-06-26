;------------------------------------------------------------------------------;
;                                                                              ;
;  Set of dialogue callback routines (window procedures) for application GUI:  ;
;                    connect include files with subroutines.                   ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Parameters shadow addressing --------------------------------------;
PARM_HWNDDLG  EQU  dword [ebp + 08]  
PARM_MSG      EQU  dword [ebp + 12]
PARM_WPARAM   EQU  dword [ebp + 16]
PARM_LPARAM   EQU  dword [ebp + 20]

;---------- Create and show "Please wait" window ------------------------------;
;                                                                              ;
; INPUT:   AL = Selector value for window text:                                ;
;               WAIT_MODE_CPU or WAIT_MODE_MEMORY                              ;
;          EBX = Parent window handle                                          ;
;                                                                              ;
; OUTPUT:  None, "Please wait" window handle updated                           ;
;                                                                              ;
;------------------------------------------------------------------------------;
OpenPleaseWait:
movzx eax,al
push eax                      ; Parm#5 = Init parameter: WAIT_MODE_CPU or WAIT_MODE_MEMORY
push DialogProcChildWait      ; Parm#4 = Pointer to dialog function
push ebx                      ; Parm#3 = Parent window handle
push IDD_CHILD_WAIT           ; Parm#2 = Dialog box resource ID
push [APP_DATA.hResources]    ; Parm#1 = Module handle for dialog box resource
call [CreateDialogParam]
mov [APP_DATA.hPleaseWait],eax
ret

;---------- Remove and destroy "Please wait" window ---------------------------;
;                                                                              ;
; INPUT:   None at registers,                                                  ; 
;          "Please wait" window handle used                                    ;                    
;                                                                              ;
; OUTPUT:  None, "Please wait" window handle updated                           ;
;                                                                              ;
;------------------------------------------------------------------------------;
ClosePleaseWait:
mov ecx,[APP_DATA.hPleaseWait]
jecxz @f
push 0                        ; Parm#4 = lParam, not used here
push 0                        ; Parm#3 = wParam, not used here
push WM_CLOSE                 ; Parm#2 = Message = Close window
push ecx                      ; Parm#1 = Window handle 
call [SendMessage] 
@@:
mov [APP_DATA.hPleaseWait],0
ret

;---------- Callback dialogue routines ----------------------------------------;
include 'dialog_main.inc'
include 'dialog_main_tabs.inc'
include 'dialog_memory_run.inc'
include 'dialog_memory_draw.inc'
include 'dialog_vector_brief.inc'
include 'dialog_about.inc'
include 'dialog_wait.inc'
;---------- Benchmarks scenario routines --------------------------------------;
include 'scenario_memory_run.inc'
include 'scenario_memory_draw.inc'
include 'scenario_vector_brief.inc'
;---------- Save text report --------------------------------------------------;
include 'scenario_save_report.inc'



