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
; OUTPUT:  RBX = Subroutine entry point                                        ;
;                                                                              ;
;------------------------------------------------------------------------------;
GetBandwidthNonTemporalPattern:
movzx eax,al                                         ; This clears bits RAX[63-8]
imul eax,eax,6                                       ; 6 = sizeof.BANDWIDTH_NON_TEMPORAL
movzx eax,word [TABLE_BANDWIDTH_NON_TEMPORAL + rax]  ; Read 16-bit compact offset
lea rbx,[ROUTINES_BANDWIDTH_NON_TEMPORAL + rax]      ; Calculate 64-bit absolute address      
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
GetBandwidthNonTemporalDump:
movzx eax,al
imul eax,eax,6   ; 6 = sizeof.BANDWIDTH_NON_TEMPORAL
movzx ecx,byte [TABLE_BANDWIDTH_NON_TEMPORAL + rax + 5]
movzx edx,byte [TABLE_BANDWIDTH_NON_TEMPORAL + rax + 4]
movzx eax,word [TABLE_BANDWIDTH_NON_TEMPORAL + rax + 2]
add rax,ROUTINES_BANDWIDTH_NON_TEMPORAL
ret
;---------- Include files connect for performance patterns --------------------;
; Non temporal ( read and write with caching rejection ) mode, 
; non temporal is memory access with caching rejection by non temporal hints
ROUTINES_BANDWIDTH_NON_TEMPORAL:   ; Pointer for compact 16-bit relative offsets
;---------- Non-temporal ( non-cacheable write ) mode -------------------------;
include 'ntwrite_sse128.inc'   ; Write by MOVNTPS (128 bit)
include 'ntcopy_sse128.inc'    ; Copy by PREFETCHNTA + MOVAPS + MOVNTPS (128 bit)
include 'ntwrite_avx256.inc'   ; Write by VMOVNTPD (256 bit)
include 'ntcopy_avx256.inc'    ; Copy by MOVAPD + MOVNTPD (256 bit)
include 'ntwrite_avx512.inc'   ; Write by VMOVNTPD (512 bit)
include 'ntcopy_avx512.inc'    ; Copy by MOVAPD + MOVNTPD (512 bit) 
;---------- Non-temporal ( non-cacheable read and write ) mode ----------------;
include 'ntread_sse128.inc'    ; Read by MOVNTDQA (128 bit) 
include 'ntrcopy_sse128.inc'   ; Copy by MOVNTDQA + MOVNTPD (128 bit)
include 'ntread_avx256.inc'    ; Read by VMOVNTDQA (256 bit)
include 'ntrcopy_avx256.inc'   ; Copy by VMOVNTDQA + VMOVNTPD (256 bit)  
include 'ntread_avx512.inc'    ; Read by VMOVNTDQA (512 bit)
include 'ntrcopy_avx512.inc'   ; Copy by VMOVNTDQA + VMOVNTPD (512 bit)
;---------- Non-temporal ( non-cacheable read-by-prefetch ) mode --------------;
include 'ntpread_sse128.inc'   ; Read by PREFETCHNTA + MOVAPS (128 bit)
include 'ntpread_avx256.inc'   ; Read by PREFETCHNTA + VMOVAPD (256 bit)
include 'ntpread_avx512.inc'   ; Read by PREFETCHNTA + VMOVAPD (512 bit)
;---------- Non-temporal read with medium and extended prefetch distance ------;
include 'ntpread_medium_sse128.inc'   ; Same read with prefetch distance + 512
include 'ntpread_long_sse128.inc'     ; Same read with prefetch distance + 2048
include 'ntpread_medium_avx256.inc'   ; Same read with prefetch distance + 512
include 'ntpread_long_avx256.inc'     ; Same read with prefetch distance + 2048
include 'ntpread_medium_avx512.inc'   ; Same read with prefetch distance + 512
include 'ntpread_long_avx512.inc'     ; Same read with prefetch distance + 2048
;---------- Cache optimized write by AMD CLZERO instruction -------------------;
include 'ntwrite_clzero.inc'          ; Write by CLZERO


