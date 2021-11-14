;------------------------------------------------------------------------------;
;                                                                              ;
;                   This performance patterns used in the                      ; 
;                     "Run", "Draw" benchmark scenarios.                       ;
;           Library of performance patterns for latency measurement:           ;
;                    connect include files with subroutines.                   ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Get pointer to performance pattern subroutines by ID --------------;
;                                                                              ;
; INPUT:   AL  = Subroutine ID                                                 ;
;                                                                              ;
; OUTPUT:  RBX = Subroutine #1 entry point, walk list builder                  ;
;          RAX = Subroutine #2 entry point, walker                             ;
;                                                                              ;
;------------------------------------------------------------------------------;

GetLatencyPattern:
movzx eax,al                         ; This clears bits RAX[63-8]
imul eax,eax,6                       ; 6 = sizeof.LATENCY_ENTRY
movzx eax,word [TableLatency + rax]  ; Read 16-bit compact offset
lea rbx,[RoutinesLatency + rax]      ; Calculate 64-bit absolute address
lea rax,[Pattern_Latency_Walk]
ret

;---------- Get pointer to performance pattern cycle body for dump by ID ------;
;    Also returns used instruction operand width, bits                         ;
;                                                                              ;
; INPUT:   AL  = Subroutine ID                                                 ;
;                                                                              ;
; OUTPUT:  RAX = Dump fragment base address                                    ;
;          EDX = Dump fragment length, bytes                                   ;
;          ECX = Used instruction operand width, bits                          ;
;                                                                              ;
;------------------------------------------------------------------------------;

GetLatencyDump:
movzx eax,al
imul eax,eax,6   ; sizeof.LATENCY_ENTRY
movzx ecx,byte [TableLatency + rax + 5]
movzx edx,byte [TableLatency + rax + 4]
movzx eax,word [TableLatency + rax + 2]
add rax,RoutinesLatency
ret

;---------- Pointer for compact 16-bit relative offsets -----------------------;

RoutinesLatency:

;--- Include files connect for performance patterns: build list, walk list ----;

include 'latency_lcm.inc'      ; Build walk list, use linear congruental method
include 'latency_rdrand.inc'   ; Build walk list, use RDRAND instruction
include 'latency_walk.inc'     ; Walk on pre-builded walk list

