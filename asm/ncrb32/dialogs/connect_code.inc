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
;---------- Callback dialogue routines ----------------------------------------;
include 'dialog_main.inc'
include 'dialog_main_tabs.inc'
include 'dialog_memory_run.inc'
include 'dialog_memory_draw.inc'
include 'dialog_vector_brief.inc'
;---------- Benchmarks scenario routines --------------------------------------;
include 'scenario_memory_run.inc'
include 'scenario_memory_draw.inc'
include 'scenario_vector_brief.inc'



