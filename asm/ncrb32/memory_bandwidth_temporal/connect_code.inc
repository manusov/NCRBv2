;------------------------------------------------------------------------------;
;                                                                              ;
;                  This performance patterns used in the                       ; 
;                    "Run", "Draw" benchmark scenarios.                        ;
;          Library of performance patterns for bandwidth measurement:          ;
;                    connect include files with subroutines.                   ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Get pointer to performance pattern subroutine by ID ---------------;
;                                                                              ;
; INPUT:   AL  = Subroutine ID                                                 ;
;                                                                              ;
; OUTPUT:  EBX = Subroutine entry point                                        ;
;                                                                              ;
;------------------------------------------------------------------------------;
GetBandwidthTemporalPattern:
movzx eax,al                                     ; This clears bits EAX[31-8]
imul eax,eax,6                                   ; 6 = sizeof.BANDWIDTH_TEMPORAL
movzx eax,word [TABLE_BANDWIDTH_TEMPORAL + eax]  ; Read 16-bit compact offset
lea ebx,[ROUTINES_BANDWIDTH_TEMPORAL + eax]      ; Calculate 32-bit absolute address      
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
GetBandwidthTemporalDump:
movzx eax,al
imul eax,eax,6   ; 6 = sizeof.BANDWIDTH_TEMPORAL
movzx ecx,byte [TABLE_BANDWIDTH_TEMPORAL + eax + 5]
movzx edx,byte [TABLE_BANDWIDTH_TEMPORAL + eax + 4]
movzx eax,word [TABLE_BANDWIDTH_TEMPORAL + eax + 2]
add eax,ROUTINES_BANDWIDTH_TEMPORAL
ret
;---------- Include files connect for performance patterns --------------------;
ROUTINES_BANDWIDTH_TEMPORAL:  ; Pointer for compact 16-bit relative offsets
; Temporal ( cacheable read and write ) mode, 
; temporal is default cacheable memory access
include 'read_mov32.inc'       ; Read by MOV (32 bit)
include 'write_mov32.inc'      ; Write by MOV (32 bit) 
include 'copy_mov32.inc'       ; Copy by MOV + MOV (32 bit)
include 'modify_not32.inc'     ; Modify = Read-Modify-Write by NOT (32 bit) 
include 'write_stosd.inc'      ; Write by REP STOSD (32 bit)
include 'copy_movsd.inc'       ; Copy by REP MOVSD (32 bit)
include 'read_mmx64.inc'       ; Read by MOVQ (64 bit)
include 'write_mmx64.inc'      ; Write by MOVQ (64 bit)
include 'copy_mmx64.inc'       ; Copy by MOVQ + MOVQ (64 bit)
include 'read_sse128.inc'      ; Read by MOVAPS (128 bit) 
include 'write_sse128.inc'     ; Write by MOVAPS (128 bit)
include 'copy_sse128.inc'      ; Copy by MOVAPS + MOVAPS (128 bit)
include 'read_avx256.inc'      ; Read by VMOVAPD (256 bit)
include 'write_avx256.inc'     ; Write by VMOVAPD (256 bit)
include 'copy_avx256.inc'      ; Copy by VMOVAPD + VMOVAPD (256 bit)
include 'read_avx512.inc'      ; Read by VMOVAPD (512 bit)  
include 'write_avx512.inc'     ; Write by VMOVAPD (512 bit)
include 'copy_avx512.inc'      ; Copy by VMOVAPD + VMOVAPD (512 bit)
;---------- FMA (Fused Multiply and Add) --------------------------------------;
include 'dot_fma256.inc'       ; Fused multiply and add, VFMADD231PD (256 bit)
include 'dot_fma512.inc'       ; Fused multiply and add, VFMADD231PD (512 bit) 
;---------- Gather-Read and Scatter-Write -------------------------------------;
include 'gather_avx256.inc'    ; Gather Read by VGATHERQPD (256 bit)
include 'gather_avx512.inc'    ; Gather Read by VGATHERQPD (512 bit)
include 'scatter_avx512.inc'   ; Scatter Write by VSCATTERQPD (512 bit) 

