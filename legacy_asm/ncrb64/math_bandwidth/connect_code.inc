;------------------------------------------------------------------------------;
;                                                                              ;
;             This library of performance patterns used in the                 ; 
;                  "Vector brief" benchmark scenario.                          ;
;                Connect include files with subroutines.                       ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Get pointer to performance pattern subroutine by ID ---------------;
;                                                                              ;
; INPUT:   AL  = Subroutine ID                                                 ;
;                                                                              ;
; OUTPUT:  RBX = Subroutine entry point                                        ;
;                                                                              ;
;------------------------------------------------------------------------------;
GetMathPattern:
movzx eax,al                           ; This clears bits RAX[63-8]
movzx eax,word [TABLE_MATH + rax*2]    ; Read 16-bit compact offset
lea rbx,[ROUTINES_MATH + rax]          ; Calculate 64-bit absolute address
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
