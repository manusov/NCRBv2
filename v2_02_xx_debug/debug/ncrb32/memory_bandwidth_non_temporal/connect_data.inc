;------------------------------------------------------------------------------;
;                                                                              ;
;                  This performance patterns used in the                       ; 
;                    "Run", "Draw" benchmark scenarios.                        ;
;          Library of performance patterns for bandwidth measurement:          ;
;               constants and variables with pre-defined values.               ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Entry for compact encoding performance method ---------------------; 
; Use 16-bit base-relative offsets instead full 64-bit offsets
; 6 or 2 bytes per entry, compact encoding
MACRO BANDWIDTH_NON_TEMPORAL x1, x2, x3, x4
{
dw  x1 - ROUTINES_BANDWIDTH_NON_TEMPORAL  ; Pointer for compact 16-bit relative offsets  ; Target routine entry point
dw  x2 - ROUTINES_BANDWIDTH_NON_TEMPORAL  ; Dump fragment start, dump show at drawings Y=F(X)
db  x3 - x2                               ; Dump fragment size
db  x4                                    ; Read or Write data bytes per instruction,
}                                         ; example YMM register op. = 32 bytes (256 bit) 
;--- List of performance patterns: bandwidth measur., non temporal access -----;
align 2
TABLE_BANDWIDTH_NON_TEMPORAL:
;---------- Non-temporal (non-cacheable write) mode ---------------------------; 
BANDWIDTH_NON_TEMPORAL  Pattern_NtWrite_SSE128   , DumpStart_NtWrite_SSE128   , DumpStop_NtWrite_SSE128   , 16   ; # 0
BANDWIDTH_NON_TEMPORAL  Pattern_NtCopy_SSE128    , DumpStart_NtCopy_SSE128    , DumpStop_NtCopy_SSE128    , 16   ; This duplicated 1
BANDWIDTH_NON_TEMPORAL  Pattern_NtWrite_AVX256   , DumpStart_NtWrite_AVX256   , DumpStop_NtWrite_AVX256   , 32
BANDWIDTH_NON_TEMPORAL  Pattern_NtCopy_AVX256    , DumpStart_NtCopy_AVX256    , DumpStop_NtCopy_AVX256    , 32
BANDWIDTH_NON_TEMPORAL  Pattern_NtWrite_AVX512   , DumpStart_NtWrite_AVX512   , DumpStop_NtWrite_AVX512   , 64
BANDWIDTH_NON_TEMPORAL  Pattern_NtCopy_AVX512    , DumpStart_NtCopy_AVX512    , DumpStop_NtCopy_AVX512    , 64   ; # 5
;---------- Non-temporal (non-cacheable read and write) mode ------------------;
BANDWIDTH_NON_TEMPORAL  Pattern_NtRead_SSE128    , DumpStart_NtRead_SSE128    , DumpStop_NtRead_SSE128    , 16   ; # 6
BANDWIDTH_NON_TEMPORAL  Pattern_NtRCopy_SSE128   , DumpStart_NtRCopy_SSE128   , DumpStop_NtRCopy_SSE128   , 16
BANDWIDTH_NON_TEMPORAL  Pattern_NtRead_AVX256    , DumpStart_NtRead_AVX256    , DumpStop_NtRead_AVX256    , 32
BANDWIDTH_NON_TEMPORAL  Pattern_NtRCopy_AVX256   , DumpStart_NtRCopy_AVX256   , DumpStop_NtRCopy_AVX256   , 32
BANDWIDTH_NON_TEMPORAL  Pattern_NtRead_AVX512    , DumpStart_NtRead_AVX512    , DumpStop_NtRead_AVX512    , 64   
BANDWIDTH_NON_TEMPORAL  Pattern_NtRCopy_AVX512   , DumpStart_NtRCopy_AVX512   , DumpStop_NtRCopy_AVX512   , 64   ; # 11
;---------- Non-temporal (non-cacheable read-by-prefetch and write) mode ------;
BANDWIDTH_NON_TEMPORAL  Pattern_NtpRead_SSE128   , DumpStart_NtpRead_SSE128   , DumpStop_NtpRead_SSE128   , 16   ; # 12 
BANDWIDTH_NON_TEMPORAL  Pattern_NtCopy_SSE128    , DumpStart_NtCopy_SSE128    , DumpStop_NtCopy_SSE128    , 16   ; This duplicated 1
BANDWIDTH_NON_TEMPORAL  Pattern_NtpRead_AVX256   , DumpStart_NtpRead_AVX256   , DumpStop_NtpRead_AVX256   , 32   ; # 14
;---------- SSE128 additions: medium and Long prefetch distance ---------------;
BANDWIDTH_NON_TEMPORAL  Pattern_NtpRead_M_SSE128 , DumpStart_NtpRead_M_SSE128 , DumpStop_NtpRead_M_SSE128 , 16   ; # 15
BANDWIDTH_NON_TEMPORAL  Pattern_NtpRead_L_SSE128 , DumpStart_NtpRead_L_SSE128 , DumpStop_NtpRead_L_SSE128 , 16   ; # 16
;---------- AVX256 additions: medium and Long prefetch distance ---------------;
BANDWIDTH_NON_TEMPORAL  Pattern_NtpRead_M_AVX256 , DumpStart_NtpRead_M_AVX256 , DumpStop_NtpRead_M_AVX256 , 32   ; # 17
BANDWIDTH_NON_TEMPORAL  Pattern_NtpRead_L_AVX256 , DumpStart_NtpRead_L_AVX256 , DumpStop_NtpRead_L_AVX256 , 32   ; # 18
;---------- AVX512 additions: medium and Long prefetch distance ---------------;
BANDWIDTH_NON_TEMPORAL  Pattern_NtpRead_M_AVX512 , DumpStart_NtpRead_M_AVX512 , DumpStop_NtpRead_M_AVX512 , 64   ; # 19
BANDWIDTH_NON_TEMPORAL  Pattern_NtpRead_L_AVX512 , DumpStart_NtpRead_L_AVX512 , DumpStop_NtpRead_L_AVX512 , 64   ; # 20
;---------- Additions for AMD CLZERO instruction support ----------------------;
BANDWIDTH_NON_TEMPORAL  Pattern_NtWrite_CLZERO   , DumpStart_NtWrite_CLZERO   , DumpStop_NtWrite_CLZERO   , 64   ; # 21
