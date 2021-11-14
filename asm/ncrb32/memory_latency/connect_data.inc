;------------------------------------------------------------------------------;
;                                                                              ;
;                   This performance patterns used in the                      ; 
;                     "Run", "Draw" benchmark scenarios.                       ;
;           Library of performance patterns for latency measurement:           ;
;               constants and variables with pre-defined values.               ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Entry for compact encoding performance method ---------------------; 
; Use 16-bit base-relative offsets instead full 64-bit offsets
; 6 or 2 bytes per entry, compact encoding
MACRO LATENCY_ENTRY  x1, x2, x3, x4   ; 6 bytes per entry, compact encoding
{
dw  x1 - ROUTINES_LATENCY   ; Target routine entry point
dw  x2 - ROUTINES_LATENCY   ; Dump fragment start, dump show at drawings Y=F(X)
db  x3 - x2                 ; Dump fragment size
db  x4                      ; Data Bytes per Instruction,
}                           ; example YMM register op. = 32 bytes (256 bit) 
align 2
;---------- Include files connect for performance patterns --------------------;
; Latency measurement, note for both fill methods (LCM, RDRAND) dump walk method only
; Variant 32
LATENCY_PATTERNS_32:
LATENCY_ENTRY  Pattern_Latency_LCM_32      , DumpStart_Latency_Walk_32   , DumpStop_Latency_Walk_32   , 4   ; ID = 0
LATENCY_ENTRY  Pattern_Latency_RDRAND_32   , DumpStart_Latency_Walk_32   , DumpStop_Latency_Walk_32   , 4   ; ID = 1  
LATENCY_ENTRY  Pattern_Latency_Walk_32     , DumpStart_Latency_Walk_32   , DumpStop_Latency_Walk_32   , 4   ; ID = 2   
; Variant 32 x 2, for match to x64 version
LATENCY_PATTERNS_32x2:
LATENCY_ENTRY  Pattern_Latency_LCM_32x2    , DumpStart_Latency_Walk_32x2 , DumpStop_Latency_Walk_32x2 , 8   ; ID = 3
LATENCY_ENTRY  Pattern_Latency_RDRAND_32x2 , DumpStart_Latency_Walk_32x2 , DumpStop_Latency_Walk_32x2 , 8   ; ID = 4  
LATENCY_ENTRY  Pattern_Latency_Walk_32x2   , DumpStart_Latency_Walk_32x2 , DumpStop_Latency_Walk_32x2 , 8   ; ID = 5   

