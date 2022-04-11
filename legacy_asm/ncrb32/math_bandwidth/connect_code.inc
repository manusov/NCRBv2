;------------------------------------------------------------------------------;
;                                                                              ;
;        This performance patterns used in the "Math" benchmark scenario.      ;
;           Library of performance patterns for mathematics benchmarks:        ;
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
GetMathPattern:
movzx eax,al                           ; This clears bits EAX[31-8]
movzx eax,word [TABLE_MATH + eax*2]    ; Read 16-bit compact offset
lea ebx,[ROUTINES_MATH + eax]          ; Calculate 32-bit absolute address
ret
;---------- Math performance bandwidth measurement ----------------------------;
; Routines for mathematics instructions bandwidth benchmarks,
; for register operands
ROUTINES_MATH:                  ; Start offset for compact offsets encoding
include 'sqrt_sse128.inc'       ; Square root, SSE128, 2 x double 
include 'sqrt_avx256.inc'       ; Square root, AVX256, 4 x double
include 'sqrt_avx512.inc'       ; Square root, AVX512, 8 x double
include 'cos_x87.inc'           ; Cosine, x87
include 'sincos_x87.inc'        ; Sine + Cosine, x87


