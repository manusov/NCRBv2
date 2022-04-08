;------------------------------------------------------------------------------;
;                                                                              ;
;  Set of dialogue callback routines (window procedures) for application GUI:  ;
;                    connect include files with subroutines.                   ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Parameters shadow addressing --------------------------------------;
PARM_HWNDDLG  EQU  qword [rbp + 40 + 08 + 00]
PARM_MSG      EQU  qword [rbp + 40 + 08 + 08]
PARM_WPARAM   EQU  qword [rbp + 40 + 08 + 16]
PARM_LPARAM   EQU  qword [rbp + 40 + 08 + 24]
LOW_WPARAM    EQU  dword [rbp + 40 + 08 + 16]

;---------- Create and show "Please wait" window ------------------------------;
;                                                                              ;
; INPUT:   AL  = Selector value for window text:                               ;
;                WAIT_MODE_CPU or WAIT_MODE_MEMORY                             ;
;          RBX = Parent window handle                                          ;
;                                                                              ;
; OUTPUT:  None, "Please wait" window handle updated                           ;
;                                                                              ;
;------------------------------------------------------------------------------;
OpenPleaseWait:
push rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
movzx eax,al
push 0                              ; This PUSH for stack alignment
push rax                            ; Parm#5 = Init parameter: WAIT_MODE_CPU or WAIT_MODE_MEMORY 
lea r9,[DialogProcChildWait]        ; Parm#4 = R9  = Pointer to dialog function  
mov r8,rbx                          ; Parm#3 = R8  = Parent window handle
mov edx,IDD_CHILD_WAIT              ; Parm#2 = RDX = Dialog box resource ID
mov rcx,[APP_DATA.hResources]       ; Parm#1 = RCX = Module handle for dialog box resource
sub rsp,32
call [CreateDialogParam]
mov [APP_DATA.hPleaseWait],rax
mov rsp,rbp
pop rbp
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
push rbp
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
xor r9d,r9d                         ; Parm#4 = R9  = lParam, not used here 
xor r8d,r8d                         ; Parm#3 = R8  = wParam, not used here
mov edx,WM_CLOSE                    ; Parm#2 = RDX = Message = Close window
mov rcx,[APP_DATA.hPleaseWait]      ; Parm#1 = RCX  = Window handle
jrcxz @f
call [SendMessage]
@@:
mov [APP_DATA.hPleaseWait],0
mov rsp,rbp
pop rbp
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
