;------------------------------------------------------------------------------;
;                                                                              ;
;                   This performance patterns used in the                      ; 
;                     "Run", "Draw" benchmark scenarios.                       ;
;           Library of performance patterns for latency measurement:           ;
;               constants and variables with pre-defined values.               ;
;                                                                              ;
;------------------------------------------------------------------------------;

; Entries for compact encoding performance method 
; Use 16-bit base-relative offsets instead full 64-bit offsets
; 6 or 2 bytes per entry, compact encoding
MACRO LATENCY_ENTRY  x1, x2, x3, x4   ; 6 bytes per entry, compact encoding
{
dw  x1 - RoutinesLatency   ; Target routine entry point
dw  x2 - RoutinesLatency   ; Dump fragment start, dump show at drawings Y=F(X)
db  x3 - x2                ; Dump fragment size
db  x4                     ; Data Bytes per Instruction,
}                          ; example YMM register op. = 32 bytes (256 bit) 

;--- List of performance patterns: latency measurement: build and walk  -------;
; Note for both fill methods (LCM, RDRAND) dump walk method only

align 2
TableLatency:
LATENCY_ENTRY  Pattern_Latency_LCM    , DumpStart_Latency_Walk , DumpStop_Latency_Walk , 8
LATENCY_ENTRY  Pattern_Latency_RDRAND , DumpStart_Latency_Walk , DumpStop_Latency_Walk , 8  
LATENCY_ENTRY  Pattern_Latency_Walk   , DumpStart_Latency_Walk , DumpStop_Latency_Walk , 8   

