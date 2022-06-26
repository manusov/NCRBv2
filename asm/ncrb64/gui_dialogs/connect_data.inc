;------------------------------------------------------------------------------;
;                                                                              ;
;  Set of dialogue callback routines (window procedures) for application GUI:  ;
;               constants and variables with pre-defined values.               ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Pointers to dialogues callbacks procedures, per each tab sheet ----;
ProcDialogs   DQ  DialogProcSysinfo
              DQ  DialogProcMemory
              DQ  DialogProcOs
              DQ  DialogProcNativeOs
              DQ  DialogProcTopology
              DQ  DialogProcTopologyEx
              DQ  DialogProcNuma
              DQ  DialogProcGroups
              DQ  DialogProcSmbios
              DQ  DialogProcAcpi
              DQ  DialogProcAffCpuid
;---------- Data for Vector Brief test scenario -------------------------------;
; id = routine ID at bits [6-0], bit 7 : 0 = BandwidthTemporalPattern, 1 = Math pattern
; count = instruction count per work cycle = per 4KB block, power of 2
; repeats = measurement cycle count, required * REPEATS_SCALE 
; cpuValid = bit number at CPUDATA.extractedFeaturesBitmap, 0FFh means don't check (x87 check is redundant, because run criteria)
; osValid  = bit mask at CPUDATA.extractedContextBitmap, 0FFh means don't check  (include SSE without XCR0) 
MACRO VECLK id, count, repeats, cpuValid, osValid 
{
DB  id          ; ID of measurement routine
DB  count       ; Instruction count per work cycle = per 4KB block, power of 2
DB  repeats     ; Measurement cycle count, required * REPEATS_SCALE
DB  cpuValid    ; CPU (CPUID) check criteria to run this
DB  osValid     ; OS (XCR0) check criteria to run this
}
;---------- Equations for performance patterns select -------------------------;
ID_READ_SSE128   EQU 00h + 9    
ID_WRITE_SSE128  EQU 00h + 10  
ID_COPY_SSE128   EQU 00h + 11
ID_READ_AVX256   EQU 00h + 12
ID_WRITE_AVX256  EQU 00h + 13
ID_COPY_AVX256   EQU 00h + 14
ID_READ_AVX512   EQU 00h + 15
ID_WRITE_AVX512  EQU 00h + 16
ID_COPY_AVX512   EQU 00h + 17
ID_SQRT_SSE128   EQU 80h + 0   
ID_SQRT_AVX256   EQU 80h + 1
ID_SQRT_AVX512   EQU 80h + 2
ID_COS_X87       EQU 80h + 3
ID_SINCOS_X87    EQU 80h + 4
;---------- Vector brief test scenario encoding -------------------------------;
VECTOR_BRIEF:
VECLK  ID_READ_SSE128  , 8 , 160 , 01 , 0FFh   ; SSE   
VECLK  ID_WRITE_SSE128 , 8 , 160 , 01 , 0FFh   ; SSE  
VECLK  ID_COPY_SSE128  , 7 , 160 , 01 , 0FFh   ; SSE 
VECLK  ID_READ_AVX256  , 7 , 160 , 07 , 003h   ; AVX
VECLK  ID_WRITE_AVX256 , 7 , 160 , 07 , 003h   ; AVX  
VECLK  ID_COPY_AVX256  , 6 , 160 , 07 , 003h   ; AVX  
VECLK  ID_READ_AVX512  , 6 , 160 , 09 , 01Fh   ; AVX512 
VECLK  ID_WRITE_AVX512 , 6 , 160 , 09 , 01Fh   ; AVX512  
VECLK  ID_COPY_AVX512  , 5 , 160 , 09 , 01Fh   ; AVX512   
VECLK  ID_SQRT_SSE128  , 8 ,  40 , 02 , 0FFh   ; SSE2 
VECLK  ID_SQRT_AVX256  , 8 ,  40 , 07 , 003h   ; AVX   
VECLK  ID_SQRT_AVX512  , 8 ,  40 , 09 , 01Fh   ; AVX512 
VECLK  ID_COS_X87      , 8 ,   4 , 16 , 0FFh   ; x87     
VECLK  ID_SINCOS_X87   , 8 ,   4 , 16 , 0FFh   ; x87  
;---------- Vector brief test scenario constants ------------------------------;
VECTOR_BRIEF_DATA_SIZE      =  4096     ; Part of L1 ( bytes ) used as data size
VECTOR_BRIEF_REPEATS_SCALE  =  10000    ; Measurement repeats scale for performance patterns
