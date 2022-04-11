;------------------------------------------------------------------------------;
;                                                                              ;
;   Library for memory and threads management, include adjust measurements:    ;
;               constants and variables with pre-defined values.               ;
;                                                                              ;
;------------------------------------------------------------------------------;
;---------- Memory size constants ---------------------------------------------;
; Block size for DRAM benchmarks, default values 
; must be corrected by platform configuration checks
DEFAULT_DRAM_BLOCK      =  32 * 1024 * 1024
DEFAULT_CACHE_BLOCK     =  32 * 1024
; Constants for custom block size restrictions
CUSTOM_BLOCK_MIN        =  16 * 1024
CUSTOM_BLOCK_MAX        =  256 * 1024 * 1024 
CUSTOM_BLOCK_ALIGN      =  512
;---------- Memory timings constants ------------------------------------------;
; 10000 milliseconds = 10 seconds, events wait time for
; communication with child thread
THREAD_TIME_LIMIT       =  10000  
; Benchmarks repeat parameters, precision=f(repeats), for Cache&RAM mode
; Calibrate iterations for objects, normal mode
; this constants must be multiple of LATENCY DIVISOR (50)
L1_REPEATS_FAST         =  2000000
L2_REPEATS_FAST         =  50000
L3_REPEATS_FAST         =  1000
L4_REPEATS_FAST         =  200
DRAM_REPEATS_FAST       =  100
CUSTOM_REPEATS_FAST     =  100
; Calibrate iterations for objects, slow mode
; this constants must be multiple of LATENCY DIVISOR (50)
L1_REPEATS_SLOW         =  2500000
L2_REPEATS_SLOW         =  500000
L3_REPEATS_SLOW         =  10000
L4_REPEATS_SLOW         =  2000
DRAM_REPEATS_SLOW       =  200
CUSTOM_REPEATS_SLOW     =  1000000
; Divisor constant for latency mode,
; if Latency mode, make measurement repeats / LATENCY_DIVISOR,
; if Bandwidth mode, no divide
LATENCY_DIVISOR         =  50
; Repeats options definitions: 
; objects entries count (L1-L4, DRAM, Custom),
; bytes per entry (dword=4 bytes), 
; list size
REPEAT_OBJECTS          =  6
REPEAT_ENTRY_BYTES      =  4
REPEAT_SERIES_BYTES     =  REPEAT_OBJECTS * REPEAT_ENTRY_BYTES    
; Constants for adaptive measurement mode, target measurement time, seconds,
; for fast and slow (precision) modes,
; adaptive logic criteria is:
; number of iterations must be adjusted for measurement time equal this time
ADAPTIVE_FAST           =  0.01
ADAPTIVE_SLOW           =  0.05
;------------------------------------------------------------------------------;
;     Benchmarks calibration repeats select table and time intervals table.    ;
;------------------------------------------------------------------------------;
; Number of measurement iterations N = F ( target object , careful mode )
; Target Objects: L1, L2, L3, L4 caches, DRAM, Storage, Custom (different sizes)
; Careful mode: Fast(inaccurate), Slow(precision)
BENCHMARKS_REPEATS:
; Number of measurement iterations for objects, for fast (inaccurate) mode,
; for Cache, RAM, Custom modes
; Group must be 24 bytes size for indexed access, 6 dwords
DD  L1_REPEATS_FAST , L2_REPEATS_FAST , L3_REPEATS_FAST , L4_REPEATS_FAST
DD  DRAM_REPEATS_FAST , CUSTOM_REPEATS_FAST
; Number of measurement iterations for objects, for slow (precision) mode,
; for Cache, RAM, Custom modes
; Group must be 24 bytes size for indexed access, 6 dwords
DD  L1_REPEATS_SLOW , L2_REPEATS_SLOW , L3_REPEATS_SLOW , L4_REPEATS_SLOW
DD  DRAM_REPEATS_SLOW , CUSTOM_REPEATS_SLOW
; Constants for adaptive measurement mode, target measurement time, seconds
; double floating point constants
ADAPTIVE_REPEATS:
DQ  ADAPTIVE_FAST
DQ  ADAPTIVE_SLOW

;---------- Translation tables for non-temporal read/write/copy memory --------;
; Value FFh means no translation, use temporal variant
; Values 00h-0FEh can be used for select pattern
; in the non-temporal patterns pool.
; Offset in this table = temporal pattern ID
; Value in this table = non-temporal pattern ID
; for replace if non-temporal access, or FFh if no replace.
NON_TEMPORAL_TRANSLATOR:
;---------- Default prefetch distance -----------------------------------------;
DB  0FFh   ; 0  = Pattern_Read_MOV32    , don't replace
DB  0FFh   ; 1  = Pattern_Write_MOV32   , don't replace 
DB  0FFh   ; 2  = Pattern_Copy_MOV32    , don't replace
DB  0FFh   ; 3  = Pattern_Modify_NOT32  , don't replace
DB  0FFh   ; 4  = Pattern_Write_STOSD   , don't replace
DB  0FFh   ; 5  = Pattern_Copy_MOVSD    , don't replace
DB  0FFh   ; 6  = Pattern_Read_MMX64    , don't replace
DB  0FFh   ; 7  = Pattern_Write_MMX64   , don't replace
DB  0FFh   ; 8  = Pattern_Copy_MMX64    , don't replace
DB  12     ; 9  = Pattern_Read_SSE128   , translate to Pattern_NtpRead_SSE128
DB  0      ; 10 = Pattern_Write_SSE128  , translate to Pattern_NtWrite_SSE128
DB  1      ; 11 = Pattern_Copy_SSE128   , translate to Pattern_NtCopy_SSE128 
DB  14     ; 12 = Pattern_Read_AVX256   , translate to Pattern_NtpRead_AVX256        
DB  2      ; 13 = Pattern_Write_AVX256  , translate to Pattern_NtWrite_AVX256 
DB  3      ; 14 = Pattern_Copy_AVX256   , translate to Pattern_NtCopy_AVX256  
DB  10     ; 15 = Pattern_Read_AVX512   , translate to Pattern_NtRead_AVX512 
DB  4      ; 16 = Pattern_Write_AVX512  , translate to Pattern_NtWrite_AVX512 
DB  5      ; 17 = Pattern_Copy_AVX512   , translate to Pattern_NtCopy_AVX512 
DB  0FFh   ; 18 = Pattern_Dot_FMA256    , don't replace
DB  0FFh   ; 19 = Pattern_Dot_FMA512    , don't replace
DB  0FFh   ; 20 = Reserved for Gather read AVX256             , don't replace 
DB  0FFh   ; 21 = Reserved for Gather read AVX512             , don't replace
DB  0FFh   ; 22 = Reserved for Scatter write AVX512           , don't replace
DB  0FFh   ; 23 = Reserved for Cache optimized write (CLZERO) , don't replace
DB  0FFh   ; 24 = Pattern_Latency_LCM ( LATENCY_MODE )        , don't replace
DB  0FFh   ; 25 = Pattern_Latency_RDRAND ( LATENCY_MODE + 1 ) , don't replace
;---------- Medium prefetch distance ------------------------------------------;
DB  0FFh
DB  0FFh 
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
DB  15     ; 9  = Pattern_Read_SSE128   , translate to Pattern_NtpRead_M_SSE128
DB  0
DB  1 
DB  17     ; 12 = Pattern_Read_AVX256   , translate to Pattern_NtpRead_M_AVX256        
DB  2 
DB  3  
DB  19     ; 15 = Pattern_Read_AVX512   , translate to Pattern_NtRead_M_AVX512 
DB  4 
DB  5 
DB  0FFh
DB  0FFh
DB  0FFh 
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
;---------- Long prefetch distance --------------------------------------------;
DB  0FFh
DB  0FFh 
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
DB  16     ; 9  = Pattern_Read_SSE128   , translate to Pattern_NtpRead_L_SSE128
DB  0
DB  1 
DB  18     ; 12 = Pattern_Read_AVX256   , translate to Pattern_NtpRead_L_AVX256        
DB  2 
DB  3  
DB  20     ; 15 = Pattern_Read_AVX512   , translate to Pattern_NtRead_L_AVX512 
DB  4 
DB  5 
DB  0FFh
DB  0FFh
DB  0FFh 
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
DB  0FFh
