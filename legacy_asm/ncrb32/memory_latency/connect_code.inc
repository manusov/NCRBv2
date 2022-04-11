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
; OUTPUT:  EBX = Subroutine #1 entry point, walk list builder                  ;
;          EAX = Subroutine #2 entry point, walker                             ;
;                                                                              ;
;------------------------------------------------------------------------------;
GetLatencyPattern:
push ecx                          ; This routine changes output parameters only
movzx eax,al                      ; This clears bits EAX[31-8] 
imul eax,eax,6                    ; 6 = sizeof.LATENCY_ENTRY 
lea ecx,[LATENCY_PATTERNS_32]     ; ECX = Pointer to set 32 or set 32x2 
cmp al,6                          ; EAX = Pattern index inside set
jbe .x2                           ; Go if first set of patterns (32)
sub al,6*2                        ; This branch for second set (32x2)
lea ecx,[LATENCY_PATTERNS_32x2]
.x2:
movzx ebx,word [ecx + eax]        ; EBX = 16-bit compact offset of Builder  
add ebx,ROUTINES_LATENCY          ; EBX = Pointer to Builder
movzx eax,word [ecx + 6*2]        ; EAX = 16-bit compact offset of Walker 
add eax,ROUTINES_LATENCY          ; EAX = Pointer to Walker
pop ecx
ret
;---------- Get pointer to performance pattern cycle body for dump by ID ------;
;    Also returns used instruction operand width, bits                         ;
;                                                                              ;
; INPUT:   AL  = Subroutine ID                                                 ;
;                                                                              ;
; OUTPUT:  EAX = Dump fragment base address                                    ;
;          EDX = Dump fragment length, bytes                                   ;
;          ECX = Used instruction operand width, bits                          ;
;                                                                              ;
;------------------------------------------------------------------------------;
GetLatencyDump:
movzx eax,al
imul eax,eax,6   ; sizeof.BANDWIDTH_ENTRY
cmp al,6
ja .x2
add eax,LATENCY_PATTERNS_32          ; EAX = Pointer to set 32 
jmp .xLoad
.x2:
add eax,LATENCY_PATTERNS_32x2 - 6*2  ; EAX = Pointer to set 32x2
.xLoad:
movzx ecx,byte [eax + 5]
movzx edx,byte [eax + 4]
movzx eax,word [eax + 2]
add eax,ROUTINES_LATENCY
ret
;--- Include files connect for performance patterns: build list, walk list ----;
ROUTINES_LATENCY:  ; Pointer for compact 16-bit relative offsets
include 'latency_lcm_32.inc'       ; Build walk list, use linear congruental method, for one 32-bit load
include 'latency_rdrand_32.inc'    ; Build walk list, use RDRAND instruction, for one 32-bit load
include 'latency_walk_32.inc'      ; Walk on pre-builded walk list, one 32-bit load
include 'latency_lcm_32x2.inc'     ; Build walk list, use linear congruental method, for two 32-bit loads match x64 version
include 'latency_rdrand_32x2.inc'  ; Build walk list, use RDRAND instruction, for two 32-bit loads match x64 version
include 'latency_walk_32x2.inc'    ; Walk on pre-builded walk list, two 32-bit loads match x64 version
