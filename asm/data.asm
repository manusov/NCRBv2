;=========================================================================================================;
;                                                                                                         ;
; Project NCRB ( NUMA CPU&RAM Benchmarks v2.xx.xx ).                                                      ;
; (C)2021 Ilya Manusov.                                                                                   ;
; manusov1969@gmail.com                                                                                   ;
; Previous version v1.xx.xx                                                                               ; 
; https://github.com/manusov/NumaCpuAndRamBenchmarks                                                      ;
; This version v2.xx.xx ( UNDER CONSTRUCTION )                                                            ;
; https://github.com/manusov/Prototyping                                                                  ; 
;                                                                                                         ;
; DATA.ASM = source file for FASM                                                                         ; 
; DATA.DLL = translation result, universal resource library for Win32 and Win64                           ;
; Note. Resource-only DLLs is universal,                                                                  ; 
; it can be loaded both by ia32 and x64 applications.                                                     ;
; See also other components:                                                                              ;
; NCRB32.ASM, NCRB64.ASM, KMD32.ASM, KMD64.ASM.                                                           ;
;                                                                                                         ;
; Translation by Flat Assembler version 1.73.27 ( Jan 27, 2021 ).                                         ;
; http://flatassembler.net/                                                                               ;
;                                                                                                         ;
; Edit by FASM Editor 2.0.                                                                                ; 
; Use this editor for correct source file tabulations and format. (!)                                     ;
; https://fasmworld.ru/instrumenty/fasm-editor-2-0/                                                       ;
;                                                                                                         ;
; User mode debug by OllyDbg ( 32-bit, actual for module NCRB32.EXE )                                     ;
; http://www.ollydbg.de/version2.html                                                                     ;
;                                                                                                         ;
; User mode debug by FDBG ( 64-bit, actual for module NCRB64.EXE )                                        ;
; https://board.flatassembler.net/topic.php?t=9689&postdays=0&postorder=asc&start=180                     ;
; ( Search for archive fdbg0025.zip )                                                                     ;
;                                                                                                         ;
; Intel Software Development Emulator ( SDE ) used for debug                                              ;
; https://software.intel.com/content/www/us/en/develop/articles/intel-software-development-emulator.html  ;
;                                                                                                         ;
; Icons from open icon library                                                                            ;
; https://sourceforge.net/projects/openiconlibrary/                                                       ;
;                                                                                                         ;
;=========================================================================================================;


;------------------------------------------------------------------------------;
;                                                                              ;
;                        FASM and NCRB definitions.                            ;        
;                                                                              ;
;------------------------------------------------------------------------------;
include 'win32a.inc'
include 'data\data.inc'
;---------- Global application and version description definitions ------------;
RESOURCE_DESCRIPTION  EQU  'NCRB universal resource library for Win32 and Win64'
RESOURCE_VERSION      EQU  '2.0.7.0'
RESOURCE_COMPANY      EQU  'https://github.com/manusov'
RESOURCE_COPYRIGHT    EQU  '(C) 2021 Ilya Manusov'
;------------------------------------------------------------------------------;
;                                                                              ;
;                            Resources section.                                ;        
;                                                                              ;
;------------------------------------------------------------------------------;
format PE GUI 4.0 DLL
section '.rsrc' resource data readable
;---------- Root directory of resources ---------------------------------------;
directory \
RT_DIALOG     , dialogs , \ 
RT_MENU       , menus   , \ 
RT_RCDATA     , raws    , \
RT_ICON       , icons   , \
RT_GROUP_ICON , gicons  , \ 
RT_VERSION    , version
;---------- Resources directory for application main window and tabs ----------;
resource dialogs,\
IDD_MAIN               , LANG_ENGLISH + SUBLANG_DEFAULT, mainDialog       , \
IDD_SYSINFO            , LANG_ENGLISH + SUBLANG_DEFAULT, tabSysinfo       , \
IDD_MEMORY             , LANG_ENGLISH + SUBLANG_DEFAULT, tabMemory        , \
IDD_OS                 , LANG_ENGLISH + SUBLANG_DEFAULT, tabOs            , \
IDD_NATIVE_OS          , LANG_ENGLISH + SUBLANG_DEFAULT, tabNativeOs      , \
IDD_TOPOLOGY           , LANG_ENGLISH + SUBLANG_DEFAULT, tabTopology      , \
IDD_TOPOLOGY_EX        , LANG_ENGLISH + SUBLANG_DEFAULT, tabTopologyEx    , \
IDD_NUMA               , LANG_ENGLISH + SUBLANG_DEFAULT, tabNuma          , \
IDD_PGROUPS            , LANG_ENGLISH + SUBLANG_DEFAULT, tabPgroups       , \
IDD_ACPI               , LANG_ENGLISH + SUBLANG_DEFAULT, tabAcpi          , \
IDD_AFF_CPUID          , LANG_ENGLISH + SUBLANG_DEFAULT, tabAffCpuid      , \
IDD_CHILD_MEMORY_RUN   , LANG_ENGLISH + SUBLANG_DEFAULT, childMemoryRun   , \
IDD_CHILD_MEMORY_DRAW  , LANG_ENGLISH + SUBLANG_DEFAULT, childMemoryDraw  , \
IDD_CHILD_VECTOR_BRIEF , LANG_ENGLISH + SUBLANG_DEFAULT, childVectorBrief 
;---------- Application main window as tabbed sheet ---------------------------;
dialog      mainDialog,        '',                      0,   0, 410, 282, DS_CENTER + WS_CAPTION + WS_SYSMENU, 0, IDR_MENU, 'Verdana', 10
dialogitem  'SysTabControl32', '', IDC_TAB          ,   1,   1, 408,  29, WS_VISIBLE + TCS_MULTILINE
enddialog
;---------- Tab 1 = system information ----------------------------------------;
dialog      tabSysinfo    , '',                         2, 30, 403,  253, WS_CHILD + WS_VISIBLE, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_SYSINFO         ,   2,  3, 380,   10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_SYSINFO_VENDOR  ,   1,  17,  55,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_TFMS    ,  58,  17,  70,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_NAME    , 130,  17, 196,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_TSC     , 328,  17,  74,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_CPUID   ,   7,  33,  49,  10, WS_VISIBLE + SS_LEFT + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_MMX     ,  32,  33,  21,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SSE     ,  55,  33,  21,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SSE2    ,  78,  33,  24,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SSE3    , 104,  33,  24,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SSSE3   , 130,  33,  28,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SSE41   , 160,  33,  29,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SSE42   , 191,  33,  29,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_AVX     , 222,  33,  21,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_AVX2    , 245,  33,  24,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_AVX512F , 271,  33,  36,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_RDRAND  , 309,  33,  34,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_VMX_SVM , 345,  33,  21,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_X8664   , 369,  33,  33,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_A0      ,  32,  46,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_A1      ,  94,  46,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_A2      , 156,  46,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_A3      , 218,  46,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_A4      , 280,  46,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_A5      , 342,  46,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_B0      ,  32,  59,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_B1      ,  94,  59,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_B2      , 156,  59,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_B3      , 218,  59,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_B4      , 280,  59,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_B5      , 342,  59,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_C0      ,  32,  72,  91,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_C1      , 125,  72,  91,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_C2      , 218,  72,  91,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_C3      , 311,  72,  91,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_D0      ,  32,  85,  91,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_D1      , 125,  85,  91,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_D2      , 218,  85,  91,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_D3      , 311,  85,  91,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_XCR0    ,   7, 100,  49,  10, WS_VISIBLE + SS_LEFT + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_XMM015  ,  32, 100,  57,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_YMM015  ,  91, 100,  57,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_ZMM015  , 150, 100,  57,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_ZMM1631 , 209, 100,  60,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_K07     , 271, 100,  43,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_BNDREG  , 316, 100,  42,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_BNDCSR  , 360, 100,  42,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_ACPI    ,   7, 115,  49,  10, WS_VISIBLE + SS_LEFT + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_MADT    ,  32, 115,  28,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_MADT_1  ,  62, 115,  36,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_MADT_2  , 100, 115,  44,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_MADT_3  , 146, 115,  85,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_MADT_4  , 233, 115, 169,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SRAT    ,  32, 128,  28,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SRAT_1  ,  62, 128,  36,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SRAT_2  , 100, 128,  44,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SRAT_3  , 146, 128,  85,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SRAT_4  , 233, 128, 169,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_L1C     ,   1, 146,  45,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_L1C_V   ,  48, 146, 105,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_L1D     ,   1, 160,  45,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_L1D_V   ,  48, 160, 105,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_L2U     ,   1, 174,  45,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_L2U_V   ,  48, 174, 105,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_L3U     ,   1, 188,  45,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_L3U_V   ,  48, 188, 105,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_L4U     ,   1, 202,  45,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_L4U_V   ,  48, 202, 105,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_THREADS , 170, 146,  38,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_THR_V   , 210, 146,  29,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_CORES   , 250, 146,  38,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_CORES_V , 290, 146,  29,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SOCKETS , 330, 146,  38,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_SOCK_V  , 370, 146,  32,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_PTOT    , 170, 160, 118,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_PTOT_V  , 290, 160,  29,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_GRP     , 330, 160,  38,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_GRP_V   , 370, 160,  32,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_PCUR    , 170, 174, 118,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_PCUR_V  , 290, 174,  29,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_NUMA    , 170, 188, 118,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_NUMA_V  , 290, 188,  29,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_NUMA_M  , 170, 202, 232,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_MEM     ,   1, 217,  79,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_MEM_V   ,  82, 217,  54,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_MEM_A   , 139, 217,  40,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_MEM_AV  , 181, 217,  54,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_LRPG    , 238, 217,  76,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_LRPG_V  , 316, 217,  32,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'STATIC'      , '', IDC_SYSINFO_LRPG_E  , 350, 217,  52,  10, WS_VISIBLE + SS_SUNKEN + SS_CENTER + SS_CENTERIMAGE
dialogitem  'BUTTON'      , '', IDB_SYSINFO_VBRF    , 303, 234,  58,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
dialogitem  'BUTTON'      , '', IDB_SYSINFO_CANCEL  , 362, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog                                   
;---------- Tab 2 = memory and cache benchmark --------------------------------; 
dialog      tabMemory     , '',                         2,  30, 403, 253, WS_CHILD + WS_VISIBLE, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_MEMORY          ,   2,   3, 380,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MEMORY_FRAME_1  ,   1,  17, 402,  90, WS_VISIBLE + SS_ETCHEDFRAME
dialogitem  'STATIC'      , '', IDC_MEMORY_FRAME_2  ,   1, 109, 402,  55, WS_VISIBLE + SS_ETCHEDFRAME
dialogitem  'STATIC'      , '', IDC_MEMORY_FRAME_3  ,   1, 166,  75,  82, WS_VISIBLE + SS_ETCHEDFRAME
dialogitem  'STATIC'      , '', IDC_MEMORY_FRAME_4  ,  78, 166,  75,  82, WS_VISIBLE + SS_ETCHEDFRAME
dialogitem  'STATIC'      , '', IDC_MEMORY_FRAME_5  , 155, 166,  87,  82, WS_VISIBLE + SS_ETCHEDFRAME
dialogitem  'STATIC'      , '', IDC_MEMORY_FRAME_6  , 244, 166, 159,  65, WS_VISIBLE + SS_ETCHEDFRAME
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_A0   ,   5,  19, 130,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP + WS_GROUP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_A1   ,   5,  28, 130,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_A2   ,   5,  37, 130,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_A3   ,   5,  46, 130,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_A4   ,   5,  55, 130,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_A5   ,   5,  64, 130,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_A6   ,   5,  73, 130,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_A7   ,   5,  82, 130,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_A8   ,   5,  91, 130,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_B0   , 137,  19, 100,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP 
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_B1   , 137,  28, 100,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_B2   , 137,  37, 100,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_B3   , 137,  46, 100,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_B4   , 137,  55, 100,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_B5   , 137,  64, 100,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_B6   , 137,  73, 100,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_B7   , 137,  82, 100,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_B8   , 137,  91, 100,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_C0   , 245,  19, 150,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP 
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_C1   , 245,  28, 150,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_C2   , 245,  37, 150,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_C3   , 245,  46, 150,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_C4   , 245,  55, 150,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_C5   , 245,  64, 150,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_C6   , 245,  82,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_ASM_C7   , 245,  91,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_NONTEMP  , 330,  82,  50,   9, WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_FORCE32  , 330,  91,  50,   9, WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_L1       ,   5, 114,  40,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP + WS_GROUP
dialogitem  'BUTTON'      , '', IDB_MEMORY_L2       ,   5, 123,  40,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_L3       ,   5, 132,  40,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_L4       ,   5, 141,  40,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_DRAM     ,   5, 150,  40,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_CUSTOM   ,  53, 114,  90,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_FILE     ,  53, 125,  90,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_GPU      ,  53, 137,  90,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_PHYSICAL ,  53, 149,  90,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'EDIT'        , '', IDE_MEMORY_B_SIZE   , 147, 113,  77,  10, WS_VISIBLE + WS_CHILD + WS_BORDER + WS_TABSTOP + ES_AUTOHSCROLL + WS_DISABLED
dialogitem  'EDIT'        , '', IDE_MEMORY_F_SIZE   , 147, 125,  77,  10, WS_VISIBLE + WS_CHILD + WS_BORDER + WS_TABSTOP + ES_AUTOHSCROLL + WS_DISABLED
dialogitem  'EDIT'        , '', IDE_MEMORY_G_SIZE   , 147, 137,  77,  10, WS_VISIBLE + WS_CHILD + WS_BORDER + WS_TABSTOP + ES_AUTOHSCROLL + WS_DISABLED
dialogitem  'EDIT'        , '', IDE_MEMORY_M_START  , 147, 149,  77,  10, WS_VISIBLE + WS_CHILD + WS_BORDER + WS_TABSTOP + ES_AUTOHSCROLL + WS_DISABLED
dialogitem  'EDIT'        , '', IDE_MEMORY_M_STOP   , 231, 149,  77,  10, WS_VISIBLE + WS_CHILD + WS_BORDER + WS_TABSTOP + ES_AUTOHSCROLL + WS_DISABLED
dialogitem  'STATIC'      , '', IDC_MEMORY_M_HYPHEN , 226, 150,   5,  10, WS_VISIBLE
dialogitem  'COMBOBOX'    , '', IDC_MEMORY_COMBO_F  , 231, 113,  85,  10, WS_VISIBLE + CBS_DROPDOWNLIST + CBS_HASSTRINGS + WS_VSCROLL + WS_DISABLED 
dialogitem  'COMBOBOX'    , '', IDC_MEMORY_COMBO_F  , 231, 128,  85,  10, WS_VISIBLE + CBS_DROPDOWNLIST + CBS_HASSTRINGS + WS_VSCROLL + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_MEMORY_MTRR_WB  , 327, 114,  68,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP + WS_GROUP 
dialogitem  'BUTTON'      , '', IDB_MEMORY_MTRR_WT  , 327, 123,  68,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_MTRR_WC  , 327, 132,  68,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_MTRR_WP  , 327, 141,  68,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_MTRR_UC  , 327, 150,  68,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_PARALLEL ,   5, 169,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_HT       ,   5, 178,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_PG       ,   5, 187,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_NUMA_U   ,   5, 205,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP + WS_GROUP
dialogitem  'BUTTON'      , '', IDB_MEMORY_NUMA_S   ,   5, 214,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_NUMA_O   ,   5, 223,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_NUMA_N   ,   5, 232,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_NORMAL   ,  82, 169,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP + WS_GROUP
dialogitem  'BUTTON'      , '', IDB_MEMORY_SK_63    ,  82, 178,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_SK_4095  ,  82, 187,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_SK_CSTM  ,  82, 196,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'EDIT'        , '', IDE_MEMORY_SK_SIZE  , 100, 208,  43,  10, WS_VISIBLE + WS_CHILD + WS_BORDER + WS_TABSTOP + ES_AUTOHSCROLL + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_MEMORY_LP       ,  82, 223,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_NO_PF    , 159, 169,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP + WS_GROUP
dialogitem  'BUTTON'      , '', IDB_MEMORY_DEF_PF   , 159, 178,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_MED_PF   , 159, 187,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_LNG_PF   , 159, 196,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_BLK_PF   , 159, 205,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_CST_PF   , 159, 214,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP
dialogitem  'EDIT'        , '', IDE_MEMORY_PF_SIZE  , 177, 226,  43,  10, WS_VISIBLE + WS_CHILD + WS_BORDER + WS_TABSTOP + ES_AUTOHSCROLL + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_MEMORY_BRF      , 248, 169,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP + WS_GROUP  
dialogitem  'BUTTON'      , '', IDB_MEMORY_CRF      , 248, 178,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP 
dialogitem  'BUTTON'      , '', IDB_MEMORY_BRF_A    , 248, 187,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP 
dialogitem  'BUTTON'      , '', IDB_MEMORY_CRF_A    , 248, 196,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP 
dialogitem  'BUTTON'      , '', IDB_MEMORY_ALL_P    , 327, 169,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP + WS_GROUP
dialogitem  'BUTTON'      , '', IDB_MEMORY_X_16     , 327, 178,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP 
dialogitem  'BUTTON'      , '', IDB_MEMORY_X_32     , 327, 187,  70,   9, WS_VISIBLE + WS_CHILD + BS_AUTORADIOBUTTON + WS_TABSTOP 
dialogitem  'BUTTON'      , '', IDB_MEMORY_3D_DRAW  , 248, 214,  150,  9, WS_VISIBLE + WS_CHILD + BS_AUTOCHECKBOX + WS_TABSTOP
dialogitem  'BUTTON'      , '', IDB_MEMORY_DRAW     , 245, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
dialogitem  'BUTTON'      , '', IDB_MEMORY_RUN      , 284, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
dialogitem  'BUTTON'      , '', IDB_MEMORY_DEFAULTS , 323, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
dialogitem  'BUTTON'      , '', IDB_MEMORY_CANCEL   , 362, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog                                   
;---------- Tab 3 = operating system info -------------------------------------; 
dialog      tabOs         , '',                         2,  30, 403, 253, WS_CHILD + WS_VISIBLE, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_OS              ,   2,   3, 380,  10, WS_VISIBLE
dialogitem  'EDIT'        , '', IDE_OS_UP           ,   3,  17, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_OS_TEXT         ,   3,  30, 400, 198, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'BUTTON'      , '', IDB_OS_REPORT       , 323, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_OS_CANCEL       , 362, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog                                   
;---------- Tab 4 = native os info, useable if run ncrb32 under win64 ---------;
dialog      tabNativeOs   , '',                         2,  30, 403, 253, WS_CHILD + WS_VISIBLE, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_NATIVE_OS       ,   2,   3, 380,  10, WS_VISIBLE
dialogitem  'EDIT'        , '', IDE_NATIVE_OS_UP    ,   3,  17, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_NATIVE_OS_TEXT  ,   3,  30, 400, 198, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'BUTTON'      , '', IDB_NAT_OS_REPORT   , 323, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_NAT_OS_CANCEL   , 362, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog                                   
;---------- Tab 5 = platform topology info by winapi --------------------------; 
dialog      tabTopology   , '',                         2,  30, 403, 253, WS_CHILD + WS_VISIBLE, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_TOPOLOGY        ,   2,   3, 380,  10, WS_VISIBLE
dialogitem  'EDIT'        , '', IDE_TOPOL_UP_1      ,   3,  17, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_TOPOL_TEXT_1    ,   3,  30, 400, 129, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'EDIT'        , '', IDE_TOPOL_UP_2      ,   3, 166, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_TOPOL_TEXT_2    ,   3, 179, 400,  49, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'BUTTON'      , '', IDB_TOPOL_REPORT    , 323, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_TOPOL_CANCEL    , 362, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog                                   
;---------- Tab 6 = platform topology info by winapi (ex, extended) -----------; 
dialog      tabTopologyEx , '',                         2,  30, 403, 253, WS_CHILD + WS_VISIBLE, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_TOPOLOGY_EX     ,   2,   3, 380,  10, WS_VISIBLE
dialogitem  'EDIT'        , '', IDE_TOP_EX_UP_1     ,   3,  17, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_TOP_EX_TEXT_1   ,   3,  30, 400, 129, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'EDIT'        , '', IDE_TOP_EX_UP_2     ,   3, 166, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_TOP_EX_TEXT_2   ,   3, 179, 400,  49, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'BUTTON'      , '', IDB_TOPOL_EX_REPORT , 323, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_TOPOL_EX_CANCEL , 362, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog                                   
;---------- Tab 7 = platform NUMA domains list- -------------------------------; 
dialog      tabNuma       , '',                         2,  30, 403, 253, WS_CHILD + WS_VISIBLE, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_NUMA            ,   2,   3, 380,  10, WS_VISIBLE
dialogitem  'EDIT'        , '', IDE_NUMA_UP         ,   3,  17, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_NUMA_TEXT       ,   3,  30, 400, 198, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'BUTTON'      , '', IDB_NUMA_REPORT     , 323, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_NUMA_CANCEL     , 362, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog                                   
;---------- Tab 8 = platform processor groups list ----------------------------; 
dialog      tabPgroups    , '',                         2,  30, 403, 253, WS_CHILD + WS_VISIBLE, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_P_GROUPS        ,   2,   3, 380,  10, WS_VISIBLE
dialogitem  'EDIT'        , '', IDE_P_GROUPS_UP     ,   3,  17, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_P_GROUPS_TEXT   ,   3,  30, 400, 198, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'BUTTON'      , '', IDB_P_GROUPS_REPORT , 323, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_P_GROUPS_CANCEL , 362, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog                                   
;---------- Tab 9 = ACPI tables list ------------------------------------------; 
dialog      tabAcpi       , '',                         2,  30, 403, 253, WS_CHILD + WS_VISIBLE, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_ACPI            ,   2,   3, 380,  10, WS_VISIBLE
dialogitem  'EDIT'        , '', IDE_ACPI_UP_1       ,   3,  17, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_ACPI_TEXT_1     ,   3,  30, 400,  89, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'EDIT'        , '', IDE_ACPI_UP_2       ,   3, 126, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_ACPI_TEXT_2     ,   3, 139, 400,  89, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'BUTTON'      , '', IDB_ACPI_REPORT     , 323, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_ACPI_CANCEL     , 362, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog                                   
;---------- Tab 10 = affinized CPUID dump, per each logical CPU ---------------; 
dialog      tabAffCpuid   , '',                         2,  30, 403, 253, WS_CHILD + WS_VISIBLE, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_AFF_CPUID       ,   2,   3, 380,  10, WS_VISIBLE
dialogitem  'EDIT'        , '', IDE_A_CPUID_UP      ,   3,  17, 400,  10, WS_VISIBLE + WS_BORDER + ES_READONLY
dialogitem  'EDIT'        , '', IDE_A_CPUID_TEXT    ,   3,  30, 400, 198, WS_VISIBLE + WS_BORDER + ES_MULTILINE + ES_AUTOHSCROLL + ES_AUTOVSCROLL + ES_READONLY + WS_VSCROLL
dialogitem  'BUTTON'      , '', IDB_A_CPUID_REPORT  , 323, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT + WS_DISABLED
dialogitem  'BUTTON'      , '', IDB_A_CPUID_CANCEL  , 362, 234,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog                                   
;---------- Child window = memory and cache benchmark run, text results -------; 
dialog      childMemoryRun,    '',                     20,  20, 325, 285, WS_CAPTION + WS_SYSMENU, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_MR_FIRST        ,   7,  10, 205,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_APPLICATION  ,   7,  23,  75,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_METHOD       ,   7,  32,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_WIDTH        ,   7,  41,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_THREADS      ,   7,  50,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_HYPER_THR    ,   7,  59,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_LARGE_PAGES  ,   7,  68,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_NUMA         ,   7,  77,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_P_GROUPS     ,   7,  86,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_TARGET_OBJ   ,   7,  95,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_PREF_DIST    ,   7, 104,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_SIZE_TOTAL   ,   7, 113,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_SIZE_PER_THR ,   7, 122,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_MEASURE_PROF ,   7, 131,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_MEASURE_REP  ,   7, 140,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_MEMORY_ALLOC ,   7, 153, 380,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_BLOCK_1      ,   7, 166,  95,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_BLOCK_2      ,   7, 175,  95,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_MEM_ALC_ALL  ,   7, 184,  95,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_MEM_ALC_THR  ,   7, 193,  95,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_MEAS_RESULTS ,   7, 206, 380,  10, WS_VISIBLE  
dialogitem  'STATIC'      , '', IDC_MR_DT_MS        ,   7, 219, 190,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_DTSC_SEC_MHZ ,   7, 228, 190,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_DTSC_INS_CLK ,   7, 237, 190,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_LAST         ,   7, 253,  75,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_APPLIC_V     ,  94,  23, 230,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_METHOD_V     ,  94,  32, 230,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_WIDTH_V      ,  94,  41, 230,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_THREADS_V    ,  94,  50, 230,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_HYPER_THR_V  ,  94,  59, 230,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_LARGE_PAG_V  ,  94,  68, 230,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_NUMA_V       ,  94,  77, 230,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_P_GROUPS_V   ,  94,  86, 230,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_TARGET_OBJ_V ,  94,  95, 230,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MR_PREF_DIST_V  ,  94, 104, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_SIZE_TOTAL_V ,  94, 113, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_SIZE_PER_T_V ,  94, 122, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_MEASURE_P_V  ,  94, 131, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_MEASURE_R_V  ,  94, 140, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_BLOCK_1_V    ,  94, 166, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_BLOCK_2_V    ,  94, 175, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_MEM_ALC_A_V  ,  94, 184, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_MEM_ALC_T_V  ,  94, 193, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_DT_MS_V      ,  94, 219, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_DTSC_SEC_M_V ,  94, 228, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_DTSC_INS_C_V ,  94, 237, 230,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_MR_LAST_V       ,  94, 253, 230,  10, WS_VISIBLE
dialogitem  'BUTTON'      , '', IDB_MR_OK           , 283, 267,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog
;---------- Child window = memory and cache benchmark run, draw chart ---------;
dialog      childMemoryDraw,   '',                     28,  10, 387, 278, WS_CAPTION + WS_SYSMENU, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_MD_ASM          ,   5, 257, 250,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MD_PREFETCH     , 230, 257,  50,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MD_OBJECT       ,   5, 265,  50,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MD_THREADS      ,  53, 265,  50,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MD_REPEATS      , 111, 265,  60,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MD_PAGES        , 180, 265,  50,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_MD_NUMA         , 230, 265,  50,  10, WS_VISIBLE
dialogitem  'BUTTON'      , '', IDB_MD_RESIZE       , 290, 260,  30,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
dialogitem  'BUTTON'      , '', IDB_MD_SILENT       , 321, 260,  30,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
dialogitem  'BUTTON'      , '', IDB_MD_CANCEL       , 352, 260,  30,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog
;---------- Child window = vector brief benchmark -----------------------------;
dialog      childVectorBrief,  '',                     20,  20, 405, 270, WS_CAPTION + WS_SYSMENU, 0, 0, 'Verdana', 10
dialogitem  'STATIC'      , '', IDC_VB_CPU_NAME     ,   7,  10, 170,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_TSC_CLK      ,   7,  19, 170,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_CPU_FEATURES ,   7,  32, 170,  10, WS_VISIBLE   
dialogitem  'STATIC'      , '', IDC_VB_AVX_256      ,   7,  45, 170,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_AVX2_256     ,   7,  54, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX3_512     ,   7,  63, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512CD     ,   7,  72, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512PF     ,   7,  81, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512ER     ,   7,  90, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512VL     ,   7,  99, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512BW     ,   7, 108, 170,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_AVX512DQ     ,   7, 117, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_IFMA  ,   7, 126, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_VBMI  ,   7, 135, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_VBMI2 ,   7, 144, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_BF16  ,   7, 153, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_VAES  ,   7, 162, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_GFNI  ,   7, 171, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_VNNI  ,   7, 180, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_BTALG ,   7, 189, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_VPOP  ,   7, 198, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_VPCL  ,   7, 207, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_VP2IN ,   7, 216, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_FP16  ,   7, 225, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_4FMAP ,   7, 234, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_AVX512_4VNNI ,   7, 243, 170,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_OS_CONTEXT   , 185,  32, 190,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_SSE128_XMM   , 185,  45, 190,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_AVX256_YMM   , 185,  54, 190,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_AVX512_ZMM_L , 185,  63, 190,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_AVX512_ZMM_H , 185,  72, 190,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_AVX512_K     , 185,  81, 190,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_TIMINGS      , 185, 108, 210,  10, WS_VISIBLE 
dialogitem  'STATIC'      , '', IDC_VB_SSE128_READ  , 185, 126,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_SSE128_WRITE , 185, 135,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_SSE128_COPY  , 185, 144,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_AVX256_READ  , 185, 153,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_AVX256_WRITE , 185, 162,  50,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_AVX256_COPY  , 185, 171,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_AVX512_READ  , 185, 180,  50,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_AVX512_WRITE , 185, 189,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_AVX512_COPY  , 185, 198,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_SQRTPD_XMM   , 185, 207,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_VSQRTPD_YMM  , 185, 216,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_VSQRTPD_ZMM  , 185, 225,  50,  10, WS_VISIBLE
dialogitem  'STATIC'      , '', IDC_VB_FCOS         , 185, 234,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_FSINCOS      , 185, 243,  50,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_READ128_V    , 243, 126, 150,  10, WS_VISIBLE              
dialogitem  'STATIC'      , '', IDC_VB_WRITE128_V   , 243, 135, 150,  10, WS_VISIBLE               
dialogitem  'STATIC'      , '', IDC_VB_COPY128_V    , 243, 144, 150,  10, WS_VISIBLE               
dialogitem  'STATIC'      , '', IDC_VB_READ256_V    , 243, 153, 150,  10, WS_VISIBLE               
dialogitem  'STATIC'      , '', IDC_VB_WRITE256_V   , 243, 162, 150,  10, WS_VISIBLE  
dialogitem  'STATIC'      , '', IDC_VB_COPY256_V    , 243, 171, 150,  10, WS_VISIBLE               
dialogitem  'STATIC'      , '', IDC_VB_READ512_V    , 243, 180, 150,  10, WS_VISIBLE  
dialogitem  'STATIC'      , '', IDC_VB_WRITE512_V   , 243, 189, 150,  10, WS_VISIBLE               
dialogitem  'STATIC'      , '', IDC_VB_COPY512_V    , 243, 198, 150,  10, WS_VISIBLE                 
dialogitem  'STATIC'      , '', IDC_VB_SQRT128_V    , 243, 207, 150,  10, WS_VISIBLE               
dialogitem  'STATIC'      , '', IDC_VB_SQRT256_V    , 243, 216, 150,  10, WS_VISIBLE             
dialogitem  'STATIC'      , '', IDC_VB_SQRT512_V    , 243, 225, 150,  10, WS_VISIBLE  
dialogitem  'STATIC'      , '', IDC_VB_FCOS_V       , 243, 234, 150,  10, WS_VISIBLE               
dialogitem  'STATIC'      , '', IDC_VB_FSINCOS_V    , 243, 243, 150,  10, WS_VISIBLE 
dialogitem  'BUTTON'      , '', IDB_VB_OK           , 360, 250,  38,  13, WS_VISIBLE + BS_DEFPUSHBUTTON + BS_FLAT
enddialog
;---------- Application main menu and service items ---------------------------; 
resource menus, IDR_MENU, LANG_ENGLISH + SUBLANG_DEFAULT, mainMenu
menu mainMenu
menuitem '&File'        , 0 , MFR_POPUP
menuitem '&Save report' , IDM_SAVE_REPORT , 0 , MFS_DISABLED
menuitem 'S&ave image'  , IDM_SAVE_IMAGE  , 0 , MFS_DISABLED
menuseparator
menuitem '&Load report' , IDM_LOAD_REPORT , 0 , MFS_DISABLED
menuseparator
menuitem 'E&xit'        , IDM_EXIT, MFR_END
menuitem '&Help'        , 0 , MFR_POPUP + MFR_END
menuitem '&About...'    , IDM_ABOUT, MFR_END
;---------- Raw resources strings and binder scripts --------------------------;
; Note. Strings represented as raw resources ( not as string resources ) for
; compact encoding: 1 byte per char.  
resource raws, \
IDS_STRINGS_POOL    , LANG_ENGLISH + SUBLANG_DEFAULT , stringsPool       , \
IDS_BINDERS_POOL    , LANG_ENGLISH + SUBLANG_DEFAULT , bindersPool       , \ 
IDS_CPU_COMMON_POOL , LANG_ENGLISH + SUBLANG_DEFAULT , cpuCommonFeatures , \ 
IDS_CPU_AVX512_POOL , LANG_ENGLISH + SUBLANG_DEFAULT , cpuAvx512Features , \
IDS_OS_CONTEXT_POOL , LANG_ENGLISH + SUBLANG_DEFAULT , osContextFeatures , \
IDS_INTEL_CACHE     , LANG_ENGLISH + SUBLANG_DEFAULT , intelCache        , \
IDS_ACPI_DATA_POOL  , LANG_ENGLISH + SUBLANG_DEFAULT , acpiData          , \
IDS_IMPORT_POOL     , LANG_ENGLISH + SUBLANG_DEFAULT , importList        , \ 
IDS_FONTS_POOL      , LANG_ENGLISH + SUBLANG_DEFAULT , fontList          , \
IDS_BRUSHES_POOL    , LANG_ENGLISH + SUBLANG_DEFAULT , brushesList       , \ 
IDS_BITMAP_INFO     , LANG_ENGLISH + SUBLANG_DEFAULT , bitmapInfo
;---------- Raw resource for strings pool -------------------------------------;
resdata stringsPool
;---------- Brief names for application sheets --------------------------------; 
DB  'sysinfo'           , 0
DB  'memory'            , 0
DB  'operating system'  , 0
DB  'native os'         , 0
DB  'topology'          , 0
DB  'extended topology' , 0
DB  'numa domains'      , 0
DB  'processor groups'  , 0
DB  'acpi'              , 0
DB  'affinized cpuid'   , 0
;---------- Full names for application sheets --------------------------------;
DB  'System summary.'                                                                          , 0
DB  'Memory and cache benchmarks, bandwidth (megabytes per second) and latency (nanoseconds).' , 0
DB  'System information by WinAPI.'                                                            , 0
DB  'Native OS information for ia32 application under x64 OS.'                                 , 0
DB  'Platform topology by WinAPI GetLogicalProcessorInformation().'                            , 0
DB  'Platform topology by WinAPI GetLogicalProcessorInformationEx().'                          , 0
DB  'NUMA domains list by WinAPI GetNumaHighestNodeNumber() and other.'                        , 0
DB  'Processor groups list by WinAPI GetActiveProcessorGroupCount() and other.'                , 0
DB  'ACPI tables list by WinAPI EnumSystemFirmwareTables() and other.'                         , 0
DB  'CPUID per each thread affinized by WinAPI SetThreadAffinityMask().'                       , 0
;---------- Title names for child windows -------------------------------------;
DB  'Memory operations performance report'              , 0
DB  'Memory operations performance = f( block size )'   , 0
DB  'Vector brief performance report'                   , 0
;---------- CPUID names for system information --------------------------------;
DB  'CPUID'     , 0
DB  'MMX'       , 0
DB  'SSE'       , 0
DB  'SSE2'      , 0
DB  'SSE3'      , 0
DB  'SSSE3'     , 0
DB  'SSE4.1'    , 0
DB  'SSE4.2'    , 0
DB  'AVX'       , 0
DB  'AVX2'      , 0
DB  'AVX512F'   , 0
DB  'RDRAND'    , 0
DB  'VMX'       , 0
DB  'SVM'       , 0
DB  'x86-64'    , 0
;---------- CPUID names for system information, AVX512 sub-sets ---------------;
DB  'AVX512CD'             , 0
DB  'AVX512PF'             , 0
DB  'AVX512ER'             , 0
DB  'AVX512VL'             , 0
DB  'AVX512BW'             , 0
DB  'AVX512DQ'             , 0
DB  'AVX512_IFMA'          , 0
DB  'AVX512_VBMI'          , 0
DB  'AVX512_VBMI2'         , 0
DB  'AVX512_BF16'          , 0
DB  'AVX512+VAES'          , 0
DB  'AVX512+GFNI'          , 0
DB  'AVX512_VNNI'          , 0
DB  'AVX512_BITALG'        , 0
DB  'AVX512_VPOPCNTDQ'     , 0
DB  'AVX512+VPCLMULQDQ'    , 0
DB  'AVX512_VP2INTERSECT'  , 0
DB  'AVX512_FP16'          , 0
DB  'AVX512_4FMAPS'        , 0
DB  'AVX512_4VNNIW'        , 0
;---------- XCR0 and XGETBV context components names --------------------------;
DB  'XCR0'                 , 0
DB  'XMM[0-15]'            , 0
DB  'YMM[0-15]'            , 0
DB  'ZMM[0-15]'            , 0
DB  'ZMM[16-31]'           , 0
DB  'K[0-7]'               , 0
DB  'BNDREG'               , 0
DB  'BNDCSR'               , 0
;---------- ACPI objects visualized at system information screen --------------;
DB  'ACPI'                 , 0
DB  'MADT'                 , 0
DB  'SRAT'                 , 0
DB  'OEM Rev = '           , 0
DB  'Local APICs = '       , 0
DB  'I/O APICs = '         , 0
DB  'Domains = '           , 0
DB  'CPUs = '              , 0
DB  'RAMs = '              , 0
;---------- Cache memory ------------------------------------------------------;
DB  'L1 Code'              , 0
DB  'L1 Data'              , 0
DB  'L2 Unified'           , 0
DB  'L3 Unified'           , 0
DB  'L4 Unified'           , 0
DB  'Trace cache'          , 0
DB  'KuOps'                , 0
;---------- Platform topology by WinAPI ---------------------------------------;
DB  'Threads'                        , 0
DB  'Cores'                          , 0
DB  'Sockets'                        , 0
DB  'OS processors total'            , 0
DB  'Groups'                         , 0
DB  'OS processors in current group' , 0
DB  'OS NUMA nodes'                  , 0
;---------- Memory information by WinAPI --------------------------------------;
DB  'OS physical memory'   , 0
DB  'Available'            , 0
DB  'Minimum large page'   , 0
;---------- Assembler instructions names for memory and cache benchmarks ------;
; This strings also used as sequental pool when method name visual.
; Part 1 of 3, temporal variant.
DB  'Read x86 (MOV)'                      , 0
DB  'Write x86 (MOV)'                     , 0
DB  'Copy x86 (MOV)'                      , 0
DB  'Modify x86 (NOT)'                    , 0
DB  'Write x86 strings (REP STOSD)'       , 0 
DB  'Copy x86 strings (REP MOVSD)'        , 0
DB  'Read x86-64 (MOV)'                   , 0
DB  'Write x86-64 (MOV)'                  , 0
DB  'Copy x86-64 (MOV)'                   , 0
DB  'Modify x86-64 (NOT)'                 , 0
DB  'Write x86-64 strings (REP STOSQ)'    , 0 
DB  'Copy x86-64 strings (REP MOVSQ)'     , 0
DB  'Read MMX-64 (MOVQ)'                  , 0
DB  'Write MMX-64 (MOVQ)'                 , 0
DB  'Copy MMX-64 (MOVQ)'                  , 0
DB  'Read SSE-128 (MOVAPS)'               , 0  
DB  'Write SSE-128 (MOVAPS)'              , 0
DB  'Copy SSE-128 (MOVAPS)'               , 0
DB  'Read AVX-256 (VMOVAPD)'              , 0
DB  'Write AVX-256 (VMOVAPD)'             , 0
DB  'Copy AVX-256 (VMOVAPD)'              , 0
DB  'Read AVX-512 (VMOVAPD)'              , 0   
DB  'Write AVX-512 (VMOVAPD)'             , 0
DB  'Copy AVX-512 (VMOVAPD)'              , 0
DB  'Dot product FMA-256 (VFMADD231PD)'   , 0 
DB  'Dot product FMA-512 (VFMADD231PD)'   , 0
DB  'Gather read AVX-256 (VGATHERQPD)'    , 0
DB  'Gather read AVX-512 (VGATHERQPD)'    , 0
DB  'Scatter write AVX-512 (VSCATTERQPD)' , 0
DB  'Cache optimized write (CLZERO)'      , 0
DB  'Latency (LCM)'                       , 0  
DB  'Latency (RDRAND)'                    , 0
DB  'Latency 32x2 (LCM)'                  , 0
DB  'Latency 32x2 (RDRAND)'               , 0
;---------- Assembler instructions names for memory and cache benchmarks ------;
; This strings also used as sequental pool when method name visual.
; Part 2 of 3, non-temporal variant, default prefetch distance
DB  'Non-temporal write SSE-128 (MOVNTPS)'                    , 0  ; # 0   
DB  'Non-temporal copy SSE-128 (MOVAPS+MOVNTPS)'              , 0
DB  'Non-temporal write AVX-256 (VMOVNTPD)'                   , 0
DB  'Non-temporal copy AVX-256 (VMOVAPD+VMOVNTPD)'            , 0
DB  'Non-temporal write AVX-512 (VMOVNTPD)'                   , 0
DB  'Non-temporal copy AVX-512 (VMOVAPD+VMOVNTPD)'            , 0
DB  'Non-temporal read SSE-128 (MOVNTDQA)'                    , 0   
DB  'Non-temporal copy SSE-128 (MOVNTDQA+MOVNTPD)'            , 0
DB  'Non-temporal read AVX-256 (VMOVNTDQA)'                   , 0
DB  'Non-temporal copy AVX-256 (VMOVNTDQA+VMOVNTPD)'          , 0
DB  'Non-temporal read AVX-512 (VMOVNTDQA)'                   , 0
DB  'Non-temporal copy AVX-512 (VMOVNTDQA+VMOVNTPD)'          , 0
DB  'Non-temporal read SSE-128 (PREFETCHNTA+MOVAPS)'          , 0   
DB  'Non-temporal copy SSE-128 (PREFETCHNTA+MOVAPS+MOVNTPS)'  , 0
DB  'Non-temporal read AVX-256 (PREFETCHNTA+VMOVAPD)'         , 0  ; # 14
;---------- Assembler instructions names for memory and cache benchmarks ------;
; This strings also used as sequental pool when method name visual.
; Part 3 of 3, non-temporal variant, medium and long prefetch distance
; Note same strings used, 
; prefetch distance comment show as separate additional string
DB  'Non-temporal read SSE-128 (PREFETCHNTA+MOVAPS)'          , 0  ; # 15   
DB  'Non-temporal read SSE-128 (PREFETCHNTA+MOVAPS)'          , 0  ; # 16
DB  'Non-temporal read AVX-256 (PREFETCHNTA+VMOVAPD)'         , 0  ; # 17
DB  'Non-temporal read AVX-256 (PREFETCHNTA+VMOVAPD)'         , 0  ; # 18
DB  'Non-temporal read AVX-512 (PREFETCHNTA+VMOVAPD)'         , 0  ; # 19   
DB  'Non-temporal read AVX-512 (PREFETCHNTA+VMOVAPD)'         , 0  ; # 20 
;---------- Modes names for memory and cache benchmarks -----------------------;
DB  'Nontemporal'                         , 0
DB  'Force 32x2'                          , 0
;---------- Target objects names for memory and cache benchmark ---------------;  
DB  'L1 cache'                 , 0
DB  'L2 cache'                 , 0
DB  'L3 cache'                 , 0
DB  'L4 cache'                 , 0
DB  'DRAM'                     , 0
DB  'Custom block size'        , 0
DB  'Memory mapped file size'  , 0 
DB  'GPU memory block size'    , 0
DB  'Physical map start-stop'  , 0
;---------- Memory status names -----------------------------------------------;
DB  'Write back'               , 0
DB  'Write through'            , 0
DB  'Write combining'          , 0
DB  'Write protected'          , 0
DB  'Uncacheable'              , 0
;---------- Memory access and platform topology options names -----------------;
DB  'Parallel threads'         , 0
DB  'Hyper-threading'          , 0
DB  'Processor groups'         , 0
DB  'NUMA unaware'             , 0
DB  'Single domain'            , 0
DB  'Optimal'                  , 0
DB  'Non optimal'              , 0
DB  'Normal access'            , 0
DB  'One per 64 bytes'         , 0
DB  'One per 4K'               , 0
DB  'One per custom'           , 0
DB  'Large pages'              , 0
;---------- Memory prefetch options names -------------------------------------;
DB  'No soft prefetch'         , 0
DB  'Default distance'         , 0
DB  'Medium'                   , 0
DB  'Long'                     , 0
DB  'Block prefetch'           , 0
DB  'Custom distance'          , 0
;---------- Measurement options names -----------------------------------------;
DB  'Measure brief'                        , 0
DB  'Measure carefull'                     , 0
DB  'Brief adaptive'                       , 0
DB  'Carefull adaptive'                    , 0
DB  'All pixels'                           , 0
DB  'X / 16'                               , 0
DB  'X / 32'                               , 0
DB  'Draw 3D chart by repeat measurements' , 0
;---------- Buttons names -----------------------------------------------------;
DB  'Vector brief' , 0
DB  'Draw'         , 0
DB  'Run'          , 0
DB  'Defaults'     , 0
DB  'Report'       , 0
DB  'Exit'         , 0
DB  'OK'           , 0
;---------- Memory size and speed units, additional information ---------------;
DB  'Bytes'         , 0
DB  'KB'            , 0
DB  'MB'            , 0
DB  'GB'            , 0
DB  'TB'            , 0
DB  'MBPS'          , 0
DB  'nanoseconds'   , 0
DB  'none'          , 0
DB  'mask = '       , 0
DB  'Enabled'       , 0
DB  'Disabled'      , 0
DB  '-'             , 0
DB  'TSC clks'      , 0
DB  'CPU clks'      , 0
DB  'data move'     , 0
DB  'calculation'   , 0
DB  'ns'            , 0
DB  'MHz'           , 0
DB  'Kernel mode'   , 0
DB  'True clock'    , 0
DB  'TFMS='         , 0
DB  'TSC='          , 0
DB  'h'             , 0
DB  'supported'     , 0
DB  'not supported' , 0
DB  'n/a'           , 0
DB  'not supported by CPU' , 0
DB  'not supported by OS'  , 0
;---------- Up strings for GUI tables -----------------------------------------; 
DB  ' Parameter                     | Value                      | Hex'   , 0
DB  ' Parameter                     | Value'                              , 0
DB  ' Topology unit  | Logical CPU affinity | Comments'                   , 0
DB  ' Cache          | Size                 | Count'                      , 0
DB  ' NUMA domain  | Affinity (hex)           | Available memory at node' , 0
DB  ' Group  | Processors count'                                          , 0
DB  ' Sign | OEM ID | OEM Table ID | Creator ID | OEM Rev   | Creator Rev | Rev' , 0
DB  ' Summary'                                                            , 0
DB  ' Thread   | Function   | EAX      | EBX      | ECX      | EDX'       , 0
;---------- Strings for operating system information text ---------------------;
DB  'Memory load'                  , 0
DB  'Total physical memory'        , 0
DB  'Available physical memory'    , 0
DB  'Total page file'              , 0
DB  'Available page file'          , 0
DB  'Total virtual user space'     , 0
DB  'Available virtual user space' , 0
DB  'Extended virtual'             , 0
DB  'Application minimum address'  , 0
DB  'Application maximum address'  , 0
DB  'Active processor mask'        , 0
DB  'Processor type'               , 0
DB  'Allocation granularity'       , 0
DB  'Processor level'              , 0
DB  'Processor revision'           , 0
DB  'Processors at current group'  , 0
DB  'Processors total'             , 0
DB  'Processor groups'             , 0
DB  'NUMA domains'                 , 0
DB  'Normal page size'             , 0
DB  'Minimum large page size'      , 0
DB  ' ( DISABLED )'                , 0
DB  ' ( ENABLED )'                 , 0
;---------- Strings for topology information text -----------------------------;
DB  'CPU core'     , 0
DB  'NUMA node'    , 0
DB  'L'            , 0
DB  'CPU package'  , 0
DB  'Unknown ID'   , 0
DB  'Unified'      , 0
DB  'Instruction'  , 0
DB  'Data'         , 0
DB  'Trace'        , 0
DB  'Unknown'      , 0
DB  ' ...'         , 0
DB  'ht='          , 0
DB  'node='        , 0
DB  'ways='        , 0
DB  'line='        , 0
DB  'size='        , 0
DB  'x '           , 0
;---------- Strings for extended topology information text --------------------;
DB  'Processor group' , 0
DB  'efficiency='     , 0
DB  'smt='            , 0
;---------- Strings for ACPI information text ---------------------------------;
DB  'UNKNOWN table signature' , 0
;---------- Strings for child screen = Memory and cache performance report ----;
DB  'Simple block benchmark, conditions and options settings:', 0 
DB  'application'             , 0
DB  'method'                  , 0
DB  'operand width (bits)'    , 0
DB  'threads'                 , 0
DB  'hyper-threading'         , 0
DB  'large pages'             , 0
DB  'NUMA optimization'       , 0
DB  'processor groups'        , 0
DB  'target object'           , 0
DB  'prefetch distance'       , 0
DB  'data size total'         , 0
DB  'size per thread'         , 0
DB  'measurement profile'     , 0
DB  'measurement repeats'     , 0
DB  'memory allocation:'      , 0
DB  'block #1 base'           , 0
DB  'block #2 base'           , 0
DB  'allocated total'         , 0
DB  'allocated per thread'    , 0
DB  'measurements results:'   , 0
DB  'dT (ms)'                 , 0
DB  'dTSC/Sec (MHz)'          , 0
DB  'dTSC/Instruction (clks)' , 0
DB  'Speed (MBPS)'            , 0
DB  'Latency (ns)'            , 0
DB  'ia32'                                  , 0   ; Application type
DB  'x64'                                   , 0
DB  'ia32 under Win64'                      , 0
DB  'disabled by platform'                  , 0   ; Hyper-threading option strings, for simple result window
DB  'enabled by platform, not used by test' , 0
DB  'enabled by platform, used by test'     , 0
DB  'not available'                         , 0   ; Large pages, for simple result window
DB  'available, not used by test'           , 0
DB  'available, used by test'               , 0
DB  'not supported'                         , 0   ; NUMA option strings
DB  'no control (under OS control)'         , 0
DB  'force current domain only'             , 0
DB  'force optimal (all local domains)'     , 0
DB  'force non-optimal (remote domains)'    , 0
DB  'not supported'                         , 0   ; PG option strings
DB  'no control (current group only)'       , 0
DB  'force optimal (all groups)'            , 0  
DB  'not used by this test'                 , 0   ; Prefetch distance option strings
DB  'default'                               , 0          
DB  'medium'                                , 0
DB  'long'                                  , 0
DB  'brief'                                 , 0   ; Measurement options strings
DB  'careful'                               , 0
DB  'brief adaptive'                        , 0
DB  'careful adaptive'                      , 0
DB  '  + 1000 ms measure CPUCLK + heating'  , 0
;---------- Strings for child screen = Memory and cache performance draw ------;
DB  ' '                   , 0  ; This for prefetch distance not available
DB  'PD default'          , 0
DB  'PD medium'           , 0
DB  'PD long'             , 0
DB  'PD ?'                , 0
DB  'Resize'              , 0
DB  'Silent'              , 0
DB  'Threads='            , 0
DB  ' '                   , 0  ; This for large pages not available
DB  '4K pages'            , 0
DB  'Large pages'         , 0
DB  ' '                   , 0
DB  'NUMA unaware'        , 0
DB  'NUMA single'         , 0
DB  'NUMA local'          , 0
DB  'NUMA remote'         , 0
DB  'CPI'                 , 0
DB  'nsPI'                , 0
DB  'MBPS'                , 0
DB  'minimum'             , 0
DB  'maximum'             , 0
DB  'average'             , 0
DB  'median'              , 0
DB  'TSC clock (MHz)  = ' , 0
;---------- Strings for child screen = Vector brief performance report --------;
DB  'Processor features, detect by CPUID:'            , 0
DB  'AVX 256-bit'                                     , 0
DB  'AVX2 256-bit'                                    , 0
DB  'AVX3 512-bit, AVX512F (Foundation)'              , 0
DB  'AVX512CD (Conflict Detection)'                   , 0
DB  'AVX512PF (Prefetch)'                             , 0
DB  'AVX512ER (Exponential and Reciprocal)'           , 0
DB  'AVX512VL (Vector Length)'                        , 0
DB  'AVX512BW (Byte and Word)'                        , 0
DB  'AVX512DQ (Doubleword and Quadword)'              , 0
DB  'AVX512_IFMA (Integer Fused Multiply and Add)'    , 0
DB  'AVX512_VBMI (Vector Byte Manipulation)'          , 0
DB  'AVX512_VBMI2 (Vector Byte Manipulation 2)'       , 0
DB  'AVX512_BF16 (VNNI BFLOAT16 format)'              , 0
DB  'AVX512+VAES (AES encryption + AVX512)'           , 0
DB  'AVX512+GFNI (Galois fields + AVX512)'            , 0
DB  'AVX512_VNNI (Vector neural network)'             , 0
DB  'AVX512_BITALG (Bit algorithms)'                  , 0
DB  'AVX512_VPOPCNTDQ (Count number of set bits)'     , 0
DB  'AVX512+VPCLMULQDQ (Carry less multiplication)'   , 0
DB  'AVX512_VP2INTERSECT (Compute intersection)'      , 0
DB  'AVX512_FP16 (Floating point 16-bit format)'      , 0
DB  'AVX512_4FMAPS (4 iteration FMA)'                 , 0
DB  'AVX512_4VNNIW (4 iteration VNNI)'                , 0
DB  'OS context management features, detect by XCR0:' , 0
DB  'SSE128 registers XMM[0-15] bits [0-127]'         , 0
DB  'AVX256 registers YMM[0-15] bits [128-255]'       , 0
DB  'AVX512 registers ZMM[0-15] bits[256-511]'        , 0
DB  'AVX512 registers ZMM[16-31] bits[0-511]'         , 0
DB  'AVX512 predicate registers K[0-7]'               , 0
DB  'Instruction timings per 1 core (TSC clocks and nanoseconds):' , 0
DB  'SSE128 read'    , 0             
DB  'SSE128 write'   , 0             
DB  'SSE128 copy'    , 0             
DB  'AVX256 read'    , 0             
DB  'AVX256 write'   , 0
DB  'AVX256 copy'    , 0             
DB  'AVX512 read'    , 0
DB  'AVX512 write'   , 0             
DB  'AVX512 copy'    , 0             
DB  'SQRTPD xmm'     , 0             
DB  'VSQRTPD ymm'    , 0             
DB  'VSQRTPD zmm'    , 0
DB  'FCOS'           , 0             
DB  'FSINCOS'        , 0             
;---------- Strings for fatal error messages, cannot run NCRB -----------------;
; This messages can be generated if resource DLL successfully loaded,
; see also message strings at executeble files NCRB32.ASM, NCRB64.ASM.
DB  'CPUID instruction not supported or locked.'        , 0
DB  'CPUID function 1 not supported or locked.'         , 0
DB  'x87 Floating Point Unit not supported or locked.'  , 0
DB  'Time Stamp Counter not supported or locked.'       , 0
DB  'Error measuring TSC frequency.'                    , 0
DB  'Memory information API failed.'                    , 0
DB  'CPU topological information API failed.'           , 0
;---------- Strings for non-fatal warning messages ----------------------------;
DB  'WARNING: system is not fully NCRB-compatible,' , 0Dh, 0Ah
DB  'missing OS API functions list:'                , 0Dh, 0Ah, 0Dh, 0Ah, 0
DB  'WARNING: NCRB32 runs under Win64,'             , 0Dh, 0Ah
DB  'NCRB64 is optimal for this platform.'          , 0
;---------- Strings for runtime errors ----------------------------------------;
DB  'Benchmarks buffer memory allocation error.'    , 0  
DB  'Benchmarks buffer memory release error.'       , 0
DB  'Benchmarks timings measurement error.'         , 0 
DB  'Benchmarks address arithmetic error.'          , 0
;---------- Strings for Kernel Mode Driver and Service Control Program --------;
DB  'KMD32.SYS'  , 0
DB  'KMD64.SYS'  , 0
DB  'ICR0'       , 0
DB  '\\.\ICR0'   , 0
;---------- Strings for CPU vendors detection ---------------------------------;
DB  'GenuineIntel' , 0
DB  'AuthenticAMD' , 0
endres
;---------- Raw resource for binders pool -------------------------------------;
resdata bindersPool
;---------- GUI binder script for system information screen -------------------;
; This binders for build GUI objects (widgets) by data from buffer (bindlist).
SET_STRING  STR_FULL_SYSINFO     , IDC_SYSINFO   
SET_STRING  STR_CPUID            , IDC_SYSINFO_CPUID  
SET_STRING  STR_MMX              , IDC_SYSINFO_MMX 
SET_STRING  STR_SSE              , IDC_SYSINFO_SSE 
SET_STRING  STR_SSE2             , IDC_SYSINFO_SSE2 
SET_STRING  STR_SSE3             , IDC_SYSINFO_SSE3 
SET_STRING  STR_SSSE3            , IDC_SYSINFO_SSSE3 
SET_STRING  STR_SSE41            , IDC_SYSINFO_SSE41 
SET_STRING  STR_SSE42            , IDC_SYSINFO_SSE42 
SET_STRING  STR_AVX              , IDC_SYSINFO_AVX 
SET_STRING  STR_AVX2             , IDC_SYSINFO_AVX2 
SET_STRING  STR_AVX512F          , IDC_SYSINFO_AVX512F 
SET_STRING  STR_RDRAND           , IDC_SYSINFO_RDRAND 
SET_PTR     BINDLIST.bindCpu.secondVmm , IDC_SYSINFO_VMX_SVM 
SET_STRING  STR_X8664            , IDC_SYSINFO_X8664 
SET_STRING  STR_AVX512CD         , IDC_SYSINFO_A0 
SET_STRING  STR_AVX512PF         , IDC_SYSINFO_A1
SET_STRING  STR_AVX512ER         , IDC_SYSINFO_A2
SET_STRING  STR_AVX512VL         , IDC_SYSINFO_A3
SET_STRING  STR_AVX512BW         , IDC_SYSINFO_A4
SET_STRING  STR_AVX512DQ         , IDC_SYSINFO_A5
SET_STRING  STR_AVX512_IFMA      , IDC_SYSINFO_B0
SET_STRING  STR_AVX512_VBMI      , IDC_SYSINFO_B1
SET_STRING  STR_AVX512_VBMI2     , IDC_SYSINFO_B2
SET_STRING  STR_AVX512_BF16      , IDC_SYSINFO_B3
SET_STRING  STR_AVX512_VAES      , IDC_SYSINFO_B4
SET_STRING  STR_AVX512_GFNI      , IDC_SYSINFO_B5
SET_STRING  STR_AVX512_VNNI      , IDC_SYSINFO_C0
SET_STRING  STR_AVX512_BITALG    , IDC_SYSINFO_C1
SET_STRING  STR_AVX512_VPOPCNTDQ , IDC_SYSINFO_C2
SET_STRING  STR_AVX512_VPCLMULQ  , IDC_SYSINFO_C3
SET_STRING  STR_AVX512_VP2INTERS , IDC_SYSINFO_D0
SET_STRING  STR_AVX512_FP16      , IDC_SYSINFO_D1
SET_STRING  STR_AVX512_4FMAPS    , IDC_SYSINFO_D2
SET_STRING  STR_AVX512_4VNNIW    , IDC_SYSINFO_D3
SET_STRING  STR_XCR0             , IDC_SYSINFO_XCR0 
SET_STRING  STR_XMM_0_15         , IDC_SYSINFO_XMM015  
SET_STRING  STR_YMM_0_15         , IDC_SYSINFO_YMM015 
SET_STRING  STR_ZMM_0_15         , IDC_SYSINFO_ZMM015 
SET_STRING  STR_ZMM_16_31        , IDC_SYSINFO_ZMM1631 
SET_STRING  STR_K_0_7            , IDC_SYSINFO_K07 
SET_STRING  STR_BNDREG           , IDC_SYSINFO_BNDREG 
SET_STRING  STR_BNDCSR           , IDC_SYSINFO_BNDCSR 
SET_STRING  STR_ACPI             , IDC_SYSINFO_ACPI
SET_STRING  STR_MADT             , IDC_SYSINFO_MADT
SET_STRING  STR_SRAT             , IDC_SYSINFO_SRAT
SET_STRING  STR_L1_CODE          , IDC_SYSINFO_L1C
SET_STRING  STR_L1_DATA          , IDC_SYSINFO_L1D
SET_STRING  STR_L2_UNIFIED       , IDC_SYSINFO_L2U
SET_STRING  STR_L3_UNIFIED       , IDC_SYSINFO_L3U
SET_STRING  STR_L4_UNIFIED       , IDC_SYSINFO_L4U
SET_STRING  STR_THREADS          , IDC_SYSINFO_THREADS
SET_STRING  STR_CORES            , IDC_SYSINFO_CORES
SET_STRING  STR_SOCKETS          , IDC_SYSINFO_SOCKETS
SET_STRING  STR_PROC_TOTAL       , IDC_SYSINFO_PTOT
SET_STRING  STR_GRPS             , IDC_SYSINFO_GRP
SET_STRING  STR_PROC_CUR         , IDC_SYSINFO_PCUR
SET_STRING  STR_NUMA_NODES       , IDC_SYSINFO_NUMA
SET_STRING  STR_OS_PHYSICAL      , IDC_SYSINFO_MEM
SET_STRING  STR_OS_AVAILABLE     , IDC_SYSINFO_MEM_A
SET_STRING  STR_OS_MIN_LARGE     , IDC_SYSINFO_LRPG
SET_STRING  STR_VECTOR_BRIEF     , IDB_SYSINFO_VBRF
SET_STRING  STR_EXIT             , IDB_SYSINFO_CANCEL
;---------- CPU main parameters -----------------------------------------------;
SET_PTR     BINDLIST.bindCpu.vendor , IDC_SYSINFO_VENDOR
SET_INFO    BINDLIST.bindCpu.tfms   , IDC_SYSINFO_TFMS
SET_PTR     BINDLIST.bindCpu.name   , IDC_SYSINFO_NAME
SET_INFO    BINDLIST.bindCpu.tsc    , IDC_SYSINFO_TSC      
;---------- CPU common features bitmap ----------------------------------------; 
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 7 , 7 , IDC_SYSINFO_CPUID
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 0 , 0 , IDC_SYSINFO_MMX
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 0 , 1 , IDC_SYSINFO_SSE
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 0 , 2 , IDC_SYSINFO_SSE2
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 0 , 3 , IDC_SYSINFO_SSE3
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 0 , 4 , IDC_SYSINFO_SSSE3
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 0 , 5 , IDC_SYSINFO_SSE41
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 0 , 6 , IDC_SYSINFO_SSE42
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 0 , 7 , IDC_SYSINFO_AVX
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 1 , 0 , IDC_SYSINFO_AVX2
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 1 , 1 , IDC_SYSINFO_AVX512F
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 1 , 2 , IDC_SYSINFO_RDRAND
SET_BOOL    BINDLIST.bindCpu.secondBitmap + 0 , 0 , IDC_SYSINFO_VMX_SVM 
SET_BOOL    BINDLIST.bindCpu.cpuBitmap + 1 , 5 , IDC_SYSINFO_X8664
;---------- CPU AVX512 features bitmap ----------------------------------------;
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 0 , IDC_SYSINFO_A0
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 1 , IDC_SYSINFO_A1
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 2 , IDC_SYSINFO_A2
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 3 , IDC_SYSINFO_A3
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 4 , IDC_SYSINFO_A4
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 5 , IDC_SYSINFO_A5
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 6 , IDC_SYSINFO_B0
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 7 , IDC_SYSINFO_B1
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 0 , IDC_SYSINFO_B2
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 1 , IDC_SYSINFO_B3
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 2 , IDC_SYSINFO_B4
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 3 , IDC_SYSINFO_B5
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 4 , IDC_SYSINFO_C0
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 5 , IDC_SYSINFO_C1
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 6 , IDC_SYSINFO_C2
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 7 , IDC_SYSINFO_C3
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 2 , 0 , IDC_SYSINFO_D0
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 2 , 1 , IDC_SYSINFO_D1
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 2 , 2 , IDC_SYSINFO_D2
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 2 , 3 , IDC_SYSINFO_D3
;---------- OS context management features bitmap -----------------------------;
SET_BOOL    BINDLIST.bindCpu.osBitmap + 7 , 7 , IDC_SYSINFO_XCR0 
SET_BOOL    BINDLIST.bindCpu.osBitmap + 0 , 0 , IDC_SYSINFO_XMM015
SET_BOOL    BINDLIST.bindCpu.osBitmap + 0 , 1 , IDC_SYSINFO_YMM015
SET_BOOL    BINDLIST.bindCpu.osBitmap + 0 , 2 , IDC_SYSINFO_ZMM015
SET_BOOL    BINDLIST.bindCpu.osBitmap + 0 , 3 , IDC_SYSINFO_ZMM1631
SET_BOOL    BINDLIST.bindCpu.osBitmap + 0 , 4 , IDC_SYSINFO_K07
SET_BOOL    BINDLIST.bindCpu.osBitmap + 0 , 5 , IDC_SYSINFO_BNDREG
SET_BOOL    BINDLIST.bindCpu.osBitmap + 0 , 6 , IDC_SYSINFO_BNDCSR
;---------- ACPI tables ( MADT and SRAT ) information -------------------------;
SET_BOOL    BINDLIST.acpiEnable + 0 , 0 , IDC_SYSINFO_ACPI
SET_BOOL    BINDLIST.acpiEnable + 0 , 1 , IDC_SYSINFO_MADT
SET_BOOL    BINDLIST.acpiEnable + 0 , 2 , IDC_SYSINFO_SRAT
SET_BOOL    BINDLIST.acpiEnable + 0 , 1 , IDC_SYSINFO_MADT_1
SET_BOOL    BINDLIST.acpiEnable + 0 , 1 , IDC_SYSINFO_MADT_2   
SET_BOOL    BINDLIST.acpiEnable + 0 , 1 , IDC_SYSINFO_MADT_3   
SET_BOOL    BINDLIST.acpiEnable + 0 , 1 , IDC_SYSINFO_MADT_4   
SET_BOOL    BINDLIST.acpiEnable + 0 , 2 , IDC_SYSINFO_SRAT_1   
SET_BOOL    BINDLIST.acpiEnable + 0 , 2 , IDC_SYSINFO_SRAT_2   
SET_BOOL    BINDLIST.acpiEnable + 0 , 2 , IDC_SYSINFO_SRAT_3   
SET_BOOL    BINDLIST.acpiEnable + 0 , 2 , IDC_SYSINFO_SRAT_4   
SET_INFO    BINDLIST.bindMadt.oem       , IDC_SYSINFO_MADT_1
SET_INFO    BINDLIST.bindMadt.manufact  , IDC_SYSINFO_MADT_2
SET_INFO    BINDLIST.bindMadt.oemRev    , IDC_SYSINFO_MADT_3
SET_INFO    BINDLIST.bindMadt.comment   , IDC_SYSINFO_MADT_4
SET_INFO    BINDLIST.bindSrat.oem       , IDC_SYSINFO_SRAT_1
SET_INFO    BINDLIST.bindSrat.manufact  , IDC_SYSINFO_SRAT_2
SET_INFO    BINDLIST.bindSrat.oemRev    , IDC_SYSINFO_SRAT_3
SET_INFO    BINDLIST.bindSrat.comment   , IDC_SYSINFO_SRAT_4
;---------- Cache information -------------------------------------------------;
SET_BOOL    BINDLIST.bindCache.cacheBitmap + 0 , 0 , IDC_SYSINFO_L1C
SET_BOOL    BINDLIST.bindCache.cacheBitmap + 0 , 0 , IDC_SYSINFO_L1C_V
SET_BOOL    BINDLIST.bindCache.cacheBitmap + 0 , 1 , IDC_SYSINFO_L1D
SET_BOOL    BINDLIST.bindCache.cacheBitmap + 0 , 1 , IDC_SYSINFO_L1D_V
SET_BOOL    BINDLIST.bindCache.cacheBitmap + 0 , 2 , IDC_SYSINFO_L2U
SET_BOOL    BINDLIST.bindCache.cacheBitmap + 0 , 2 , IDC_SYSINFO_L2U_V
SET_BOOL    BINDLIST.bindCache.cacheBitmap + 0 , 3 , IDC_SYSINFO_L3U
SET_BOOL    BINDLIST.bindCache.cacheBitmap + 0 , 3 , IDC_SYSINFO_L3U_V
SET_BOOL    BINDLIST.bindCache.cacheBitmap + 0 , 4 , IDC_SYSINFO_L4U
SET_BOOL    BINDLIST.bindCache.cacheBitmap + 0 , 4 , IDC_SYSINFO_L4U_V
SET_INFO    BINDLIST.bindCache.l1c , IDC_SYSINFO_L1C_V
SET_INFO    BINDLIST.bindCache.l1d , IDC_SYSINFO_L1D_V
SET_INFO    BINDLIST.bindCache.l2u , IDC_SYSINFO_L2U_V
SET_INFO    BINDLIST.bindCache.l3u , IDC_SYSINFO_L3U_V
SET_INFO    BINDLIST.bindCache.l4u , IDC_SYSINFO_L4U_V
;---------- Threads, Cores, Sockets information -------------------------------;
SET_INFO    BINDLIST.bindTopology.threads , IDC_SYSINFO_THR_V
SET_INFO    BINDLIST.bindTopology.cores   , IDC_SYSINFO_CORES_V
SET_INFO    BINDLIST.bindTopology.sockets , IDC_SYSINFO_SOCK_V
;--- System info, processors, processor groups, NUMA, memory, large pahes -----;
SET_INFO    BINDLIST.bindSys.procTotal   , IDC_SYSINFO_PTOT_V
SET_INFO    BINDLIST.bindSys.groups      , IDC_SYSINFO_GRP_V
SET_INFO    BINDLIST.bindSys.procCur     , IDC_SYSINFO_PCUR_V
SET_INFO    BINDLIST.bindSys.numaNodes   , IDC_SYSINFO_NUMA_V    
SET_INFO    BINDLIST.bindSys.memPhys     , IDC_SYSINFO_MEM_V
SET_INFO    BINDLIST.bindSys.memAvail    , IDC_SYSINFO_MEM_AV
SET_INFO    BINDLIST.bindSys.largePage   , IDC_SYSINFO_LRPG_V
SET_INFO    BINDLIST.bindSys.largeEnable , IDC_SYSINFO_LRPG_E
SET_INFO    BINDLIST.bindSys.masksList   , IDC_SYSINFO_NUMA_M     
BIND_STOP
;---------- GUI binder script for memory and cache screen ( both ia32, x64 ) --;
SET_STRING  STR_FULL_MEMORY      , IDC_MEMORY
SET_STRING  STR_AM_06            , IDB_MEMORY_ASM_A6
SET_STRING  STR_AM_07            , IDB_MEMORY_ASM_A7
SET_STRING  STR_AM_08            , IDB_MEMORY_ASM_A8
SET_STRING  STR_AM_09            , IDB_MEMORY_ASM_B0 
SET_STRING  STR_AM_10            , IDB_MEMORY_ASM_B1 
SET_STRING  STR_AM_11            , IDB_MEMORY_ASM_B2 
SET_STRING  STR_AM_12            , IDB_MEMORY_ASM_B3 
SET_STRING  STR_AM_13            , IDB_MEMORY_ASM_B4 
SET_STRING  STR_AM_14            , IDB_MEMORY_ASM_B5 
SET_STRING  STR_AM_15            , IDB_MEMORY_ASM_B6 
SET_STRING  STR_AM_16            , IDB_MEMORY_ASM_B7 
SET_STRING  STR_AM_17            , IDB_MEMORY_ASM_B8 
SET_STRING  STR_AM_18            , IDB_MEMORY_ASM_C0
SET_STRING  STR_AM_19            , IDB_MEMORY_ASM_C1
SET_STRING  STR_AM_20            , IDB_MEMORY_ASM_C2
SET_STRING  STR_AM_21            , IDB_MEMORY_ASM_C3
SET_STRING  STR_AM_22            , IDB_MEMORY_ASM_C4
SET_STRING  STR_AM_23            , IDB_MEMORY_ASM_C5
SET_STRING  STR_AM_LCM           , IDB_MEMORY_ASM_C6
SET_STRING  STR_AM_RDRAND        , IDB_MEMORY_ASM_C7
SET_STRING  STR_NON_TEMPORAL     , IDB_MEMORY_NONTEMP
SET_STRING  STR_32X2             , IDB_MEMORY_FORCE32 
SET_STRING  STR_MEMORY_L1        , IDB_MEMORY_L1
SET_STRING  STR_MEMORY_L2        , IDB_MEMORY_L2
SET_STRING  STR_MEMORY_L3        , IDB_MEMORY_L3
SET_STRING  STR_MEMORY_L4        , IDB_MEMORY_L4
SET_STRING  STR_MEMORY_DRAM      , IDB_MEMORY_DRAM
SET_STRING  STR_MEMORY_CUSTOM    , IDB_MEMORY_CUSTOM
SET_STRING  STR_MEMORY_FILE      , IDB_MEMORY_FILE
SET_STRING  STR_MEMORY_GPU       , IDB_MEMORY_GPU
SET_STRING  STR_MEMORY_PHYSICAL  , IDB_MEMORY_PHYSICAL
SET_STRING  STR_HYPHEN           , IDC_MEMORY_M_HYPHEN
SET_STRING  STR_MEMORY_MTRR_WB   , IDB_MEMORY_MTRR_WB
SET_STRING  STR_MEMORY_MTRR_WT   , IDB_MEMORY_MTRR_WT 
SET_STRING  STR_MEMORY_MTRR_WC   , IDB_MEMORY_MTRR_WC 
SET_STRING  STR_MEMORY_MTRR_WP   , IDB_MEMORY_MTRR_WP 
SET_STRING  STR_MEMORY_MTRR_UC   , IDB_MEMORY_MTRR_UC 
SET_STRING  STR_PARALLEL_THREADS , IDB_MEMORY_PARALLEL 
SET_STRING  STR_HYPER_THREADING  , IDB_MEMORY_HT 
SET_STRING  STR_PROCESSOR_GROUPS , IDB_MEMORY_PG
SET_STRING  STR_NUMA_UNAWARE     , IDB_MEMORY_NUMA_U
SET_STRING  STR_NUMA_SINGLE      , IDB_MEMORY_NUMA_S
SET_STRING  STR_NUMA_OPTIMAL     , IDB_MEMORY_NUMA_O
SET_STRING  STR_NUMA_NO_OPTIMAL  , IDB_MEMORY_NUMA_N
SET_STRING  STR_NORMAL_ACCESS    , IDB_MEMORY_NORMAL
SET_STRING  STR_ACCESS_64B       , IDB_MEMORY_SK_63 
SET_STRING  STR_ACCESS_4096B     , IDB_MEMORY_SK_4095 
SET_STRING  STR_ACCES_CUSTOM     , IDB_MEMORY_SK_CSTM 
SET_STRING  STR_LARGE_PAGES      , IDB_MEMORY_LP
SET_STRING  STR_NO_PREFETCH      , IDB_MEMORY_NO_PF
SET_STRING  STR_DEFAULT_DISTANCE , IDB_MEMORY_DEF_PF
SET_STRING  STR_MEDIUM_DISTANCE  , IDB_MEMORY_MED_PF
SET_STRING  STR_LONG_DISTANCE    , IDB_MEMORY_LNG_PF
SET_STRING  STR_BLOCK_PREFETCH   , IDB_MEMORY_BLK_PF
SET_STRING  STR_CUSTOM_DISTANCE  , IDB_MEMORY_CST_PF
SET_STRING  STR_MEASURE_BRIEF    , IDB_MEMORY_BRF  
SET_STRING  STR_MEASURE_CAREF    , IDB_MEMORY_CRF 
SET_STRING  STR_BRIEFF_ADAPTIVE  , IDB_MEMORY_BRF_A
SET_STRING  STR_CAREF_ADAPTIVE   , IDB_MEMORY_CRF_A
SET_STRING  STR_ALL_POINTS       , IDB_MEMORY_ALL_P
SET_STRING  STR_X_16_POINTS      , IDB_MEMORY_X_16
SET_STRING  STR_X_32_POINTS      , IDB_MEMORY_X_32
SET_STRING  STR_3D_DRAW          , IDB_MEMORY_3D_DRAW 
SET_STRING  STR_DRAW             , IDB_MEMORY_DRAW  
SET_STRING  STR_RUN              , IDB_MEMORY_RUN
SET_STRING  STR_DEFAULTS         , IDB_MEMORY_DEFAULTS
SET_STRING  STR_EXIT             , IDB_MEMORY_CANCEL
BIND_STOP        
;---------- GUI binder script for memory and cache screen ( ia32 only ) -------;
SET_STRING  STR_AM_IA32_00       , IDB_MEMORY_ASM_A0
SET_STRING  STR_AM_IA32_01       , IDB_MEMORY_ASM_A1
SET_STRING  STR_AM_IA32_02       , IDB_MEMORY_ASM_A2
SET_STRING  STR_AM_IA32_03       , IDB_MEMORY_ASM_A3
SET_STRING  STR_AM_IA32_04       , IDB_MEMORY_ASM_A4
SET_STRING  STR_AM_IA32_05       , IDB_MEMORY_ASM_A5
BIND_STOP        
;---------- GUI binder script for memory and cache screen ( x64 only ) --------;
SET_STRING  STR_AM_X64_00        , IDB_MEMORY_ASM_A0
SET_STRING  STR_AM_X64_01        , IDB_MEMORY_ASM_A1
SET_STRING  STR_AM_X64_02        , IDB_MEMORY_ASM_A2
SET_STRING  STR_AM_X64_03        , IDB_MEMORY_ASM_A3
SET_STRING  STR_AM_X64_04        , IDB_MEMORY_ASM_A4
SET_STRING  STR_AM_X64_05        , IDB_MEMORY_ASM_A5
BIND_STOP        
;---------- GUI binder script for operating system screen ---------------------;
SET_STRING  STR_FULL_OS          , IDC_OS
SET_FONT    ID_FONT_2            , IDE_OS_UP
SET_FONT    ID_FONT_2            , IDE_OS_TEXT
SET_STRING  STR_PARM_VALUE_HEX   , IDE_OS_UP
SET_PTR     BINDLIST.viewOs      , IDE_OS_TEXT
SET_STRING  STR_REPORT           , IDB_OS_REPORT
SET_STRING  STR_EXIT             , IDB_OS_CANCEL
BIND_STOP
;---------- GUI binder script for native operating system screen --------------;
SET_STRING  STR_FULL_NATIVE_OS   , IDC_NATIVE_OS
SET_FONT    ID_FONT_2            , IDE_NATIVE_OS_UP
SET_FONT    ID_FONT_2            , IDE_NATIVE_OS_TEXT
SET_STRING  STR_PARM_VALUE_HEX   , IDE_NATIVE_OS_UP
SET_PTR     BINDLIST.viewNative  , IDE_NATIVE_OS_TEXT
SET_STRING  STR_REPORT           , IDB_NAT_OS_REPORT
SET_STRING  STR_EXIT             , IDB_NAT_OS_CANCEL
BIND_STOP
;---------- GUI binder script for topology screen -----------------------------;
SET_STRING  STR_FULL_TOPOLOGY    , IDC_TOPOLOGY
SET_FONT    ID_FONT_2            , IDE_TOPOL_UP_1 
SET_FONT    ID_FONT_2            , IDE_TOPOL_TEXT_1
SET_FONT    ID_FONT_2            , IDE_TOPOL_UP_2
SET_FONT    ID_FONT_2            , IDE_TOPOL_TEXT_2
SET_STRING  STR_TOPOLOGY         , IDE_TOPOL_UP_1
SET_STRING  STR_TOPOLOGY_SUMMARY , IDE_TOPOL_UP_2
SET_PTR     BINDLIST.viewTp      , IDE_TOPOL_TEXT_1
SET_PTR     BINDLIST.viewTpSum   , IDE_TOPOL_TEXT_2
SET_STRING  STR_REPORT           , IDB_TOPOL_REPORT
SET_STRING  STR_EXIT             , IDB_TOPOL_CANCEL
BIND_STOP
;---------- GUI binder script for extended topology screen --------------------;
SET_STRING  STR_FULL_TOPOLOGY_EX , IDC_TOPOLOGY_EX
SET_FONT    ID_FONT_2            , IDE_TOP_EX_UP_1 
SET_FONT    ID_FONT_2            , IDE_TOP_EX_TEXT_1
SET_FONT    ID_FONT_2            , IDE_TOP_EX_UP_2
SET_FONT    ID_FONT_2            , IDE_TOP_EX_TEXT_2
SET_STRING  STR_TOPOLOGY         , IDE_TOP_EX_UP_1
SET_STRING  STR_TOPOLOGY_SUMMARY , IDE_TOP_EX_UP_2
SET_PTR     BINDLIST.viewEt      , IDE_TOP_EX_TEXT_1
SET_PTR     BINDLIST.viewEtSum   , IDE_TOP_EX_TEXT_2
SET_STRING  STR_REPORT           , IDB_TOPOL_EX_REPORT
SET_STRING  STR_EXIT             , IDB_TOPOL_EX_CANCEL
BIND_STOP
;---------- GUI binder script for NUMA nodes list screen ----------------------;
SET_STRING  STR_FULL_NUMA        , IDC_NUMA
SET_FONT    ID_FONT_2            , IDE_NUMA_UP 
SET_FONT    ID_FONT_2            , IDE_NUMA_TEXT
SET_STRING  STR_NUMA             , IDE_NUMA_UP
SET_PTR     BINDLIST.viewNuma    , IDE_NUMA_TEXT 
SET_STRING  STR_REPORT           , IDB_NUMA_REPORT
SET_STRING  STR_EXIT             , IDB_NUMA_CANCEL
BIND_STOP
;---------- GUI binder script for processor groups list screen ----------------;
SET_STRING  STR_FULL_P_GROUPS    , IDC_P_GROUPS
SET_FONT    ID_FONT_2            , IDE_P_GROUPS_UP 
SET_FONT    ID_FONT_2            , IDE_P_GROUPS_TEXT
SET_STRING  STR_GROUPS           , IDE_P_GROUPS_UP
SET_PTR     BINDLIST.viewGroup   , IDE_P_GROUPS_TEXT 
SET_STRING  STR_REPORT           , IDB_P_GROUPS_REPORT
SET_STRING  STR_EXIT             , IDB_P_GROUPS_CANCEL
BIND_STOP
;---------- GUI binder script for ACPI information screen ---------------------;
SET_STRING  STR_FULL_ACPI        , IDC_ACPI
SET_FONT    ID_FONT_2            , IDE_ACPI_UP_1 
SET_FONT    ID_FONT_2            , IDE_ACPI_TEXT_1
SET_FONT    ID_FONT_2            , IDE_ACPI_UP_2 
SET_FONT    ID_FONT_2            , IDE_ACPI_TEXT_2
SET_STRING  STR_ACPI_LIST        , IDE_ACPI_UP_1
SET_STRING  STR_ACPI_SUMMARY     , IDE_ACPI_UP_2
SET_PTR     BINDLIST.viewAcpi    , IDE_ACPI_TEXT_1
SET_PTR     BINDLIST.viewAcpiSum , IDE_ACPI_TEXT_2
SET_STRING  STR_REPORT           , IDB_ACPI_REPORT
SET_STRING  STR_EXIT             , IDB_ACPI_CANCEL
BIND_STOP
;---------- GUI binder script for affinized CPUID dump screen -----------------;
SET_STRING  STR_FULL_AFF_CPUID   , IDC_AFF_CPUID
SET_FONT    ID_FONT_2            , IDE_A_CPUID_UP 
SET_FONT    ID_FONT_2            , IDE_A_CPUID_TEXT
SET_STRING  STR_AFF_CPUID        , IDE_A_CPUID_UP
SET_PTR     BINDLIST.viewAffCpu  , IDE_A_CPUID_TEXT 
SET_STRING  STR_REPORT           , IDB_A_CPUID_REPORT
SET_STRING  STR_EXIT             , IDB_A_CPUID_CANCEL
BIND_STOP
;--- GUI binder script for child screen = Memory and cache perf. report -------;
SET_STRING  STR_MR_FIRST           , IDC_MR_FIRST
SET_STRING  STR_MR_APPLICATION     , IDC_MR_APPLICATION
SET_STRING  STR_MR_METHOD          , IDC_MR_METHOD 
SET_STRING  STR_MR_WIDTH           , IDC_MR_WIDTH 
SET_STRING  STR_MR_THREADS         , IDC_MR_THREADS 
SET_STRING  STR_MR_HYPER_THR       , IDC_MR_HYPER_THR 
SET_STRING  STR_MR_LARGE_PAGES     , IDC_MR_LARGE_PAGES 
SET_STRING  STR_MR_NUMA            , IDC_MR_NUMA 
SET_STRING  STR_MR_P_GROUPS        , IDC_MR_P_GROUPS 
SET_STRING  STR_MR_TARGET_OBJ      , IDC_MR_TARGET_OBJ 
SET_STRING  STR_MR_PREF_DIST       , IDC_MR_PREF_DIST 
SET_STRING  STR_MR_SIZE_TOTAL      , IDC_MR_SIZE_TOTAL 
SET_STRING  STR_MR_SIZE_PER_THR    , IDC_MR_SIZE_PER_THR 
SET_STRING  STR_MR_MEASURE_PROF    , IDC_MR_MEASURE_PROF 
SET_STRING  STR_MR_MEASURE_REP     , IDC_MR_MEASURE_REP 
SET_STRING  STR_MR_MEMORY_ALLOC    , IDC_MR_MEMORY_ALLOC 
SET_STRING  STR_MR_BLOCK_1         , IDC_MR_BLOCK_1 
SET_STRING  STR_MR_BLOCK_2         , IDC_MR_BLOCK_2 
SET_STRING  STR_MR_MEM_ALC_ALL     , IDC_MR_MEM_ALC_ALL 
SET_STRING  STR_MR_MEM_ALC_THR     , IDC_MR_MEM_ALC_THR 
SET_STRING  STR_MR_MEAS_RESULTS    , IDC_MR_MEAS_RESULTS 
SET_STRING  STR_MR_DT_MS           , IDC_MR_DT_MS 
SET_STRING  STR_MR_DTSC_SEC_MHZ    , IDC_MR_DTSC_SEC_MHZ 
SET_STRING  STR_MR_DTSC_INS_CLK    , IDC_MR_DTSC_INS_CLK
SET_INFO    BINDLIST.mrApplication , IDC_MR_APPLIC_V
SET_INFO    BINDLIST.mrMethod      , IDC_MR_METHOD_V
SET_INFO    BINDLIST.mrWidth       , IDC_MR_WIDTH_V
SET_INFO    BINDLIST.mrThreads     , IDC_MR_THREADS_V
SET_INFO    BINDLIST.mrHyperT      , IDC_MR_HYPER_THR_V
SET_INFO    BINDLIST.mrLargeP      , IDC_MR_LARGE_PAG_V
SET_INFO    BINDLIST.mrNuma        , IDC_MR_NUMA_V
SET_INFO    BINDLIST.mrGroups      , IDC_MR_P_GROUPS_V
SET_INFO    BINDLIST.mrTarget      , IDC_MR_TARGET_OBJ_V
SET_INFO    BINDLIST.mrDistance    , IDC_MR_PREF_DIST_V
SET_INFO    BINDLIST.mrSizeTotal   , IDC_MR_SIZE_TOTAL_V
SET_INFO    BINDLIST.mrSizeThread  , IDC_MR_SIZE_PER_T_V
SET_INFO    BINDLIST.mrMeasProf    , IDC_MR_MEASURE_P_V
SET_INFO    BINDLIST.mrMeasRep     , IDC_MR_MEASURE_R_V
SET_INFO    BINDLIST.mrBlock1      , IDC_MR_BLOCK_1_V
SET_INFO    BINDLIST.mrBlock2      , IDC_MR_BLOCK_2_V
SET_INFO    BINDLIST.mrAllocAll    , IDC_MR_MEM_ALC_A_V
SET_INFO    BINDLIST.mrAllocThr    , IDC_MR_MEM_ALC_T_V
SET_INFO    BINDLIST.mrDtMs        , IDC_MR_DT_MS_V
SET_INFO    BINDLIST.mrDtscSec     , IDC_MR_DTSC_SEC_M_V
SET_INFO    BINDLIST.mrDtscIns     , IDC_MR_DTSC_INS_C_V
SET_INFO    BINDLIST.mrLastValue   , IDC_MR_LAST_V 
SET_STRING  STR_OK                 , IDB_MR_OK 
BIND_STOP 
;---------- Result string for bandwidth measurement mode ----------------------;
SET_STRING  STR_MR_SPEED_MBPS      , IDC_MR_LAST 
BIND_STOP
;---------- Result string for latency measurement mode ------------------------;
SET_STRING  STR_MR_LATENCY_NS      , IDC_MR_LAST 
BIND_STOP
;--- GUI binder script for child screen = Memory and cache performance draw ---;
SET_INFO    BINDLIST.mdAsm         , IDC_MD_ASM
SET_INFO    BINDLIST.mdPrefetch    , IDC_MD_PREFETCH
SET_INFO    BINDLIST.mdObject      , IDC_MD_OBJECT
SET_INFO    BINDLIST.mdThreads     , IDC_MD_THREADS
SET_INFO    BINDLIST.mdRepeats     , IDC_MD_REPEATS
SET_INFO    BINDLIST.mdPages       , IDC_MD_PAGES
SET_INFO    BINDLIST.mdNuma        , IDC_MD_NUMA
SET_STRING  STR_MD_RESIZE          , IDB_MD_RESIZE
SET_STRING  STR_MD_SILENT          , IDB_MD_SILENT
SET_STRING  STR_EXIT               , IDB_MD_CANCEL
BIND_STOP
;--- GUI binder script for child screen = Vector brief performance report -----;
SET_PTR     BINDLIST.bindCpu.name                 , IDC_VB_CPU_NAME
SET_INFO    BINDLIST.bindCpu.tsc                  , IDC_VB_TSC_CLK
SET_BOOL    BINDLIST.bindCpu.cpuBitmap    + 0 , 7 , IDC_VB_AVX_256 
SET_BOOL    BINDLIST.bindCpu.cpuBitmap    + 1 , 0 , IDC_VB_AVX2_256 
SET_BOOL    BINDLIST.bindCpu.cpuBitmap    + 1 , 1 , IDC_VB_AVX3_512 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 0 , IDC_VB_AVX512CD 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 1 , IDC_VB_AVX512PF 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 2 , IDC_VB_AVX512ER 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 3 , IDC_VB_AVX512VL 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 4 , IDC_VB_AVX512BW 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 5 , IDC_VB_AVX512DQ 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 6 , IDC_VB_AVX512_IFMA 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 0 , 7 , IDC_VB_AVX512_VBMI 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 0 , IDC_VB_AVX512_VBMI2 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 1 , IDC_VB_AVX512_BF16 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 2 , IDC_VB_AVX512_VAES 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 3 , IDC_VB_AVX512_GFNI
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 4 , IDC_VB_AVX512_VNNI
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 5 , IDC_VB_AVX512_BTALG 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 6 , IDC_VB_AVX512_VPOP 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 1 , 7 , IDC_VB_AVX512_VPCL 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 2 , 0 , IDC_VB_AVX512_VP2IN
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 2 , 1 , IDC_VB_AVX512_FP16 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 2 , 2 , IDC_VB_AVX512_4FMAP 
SET_BOOL    BINDLIST.bindCpu.avx512bitmap + 2 , 3 , IDC_VB_AVX512_4VNNI
SET_BOOL    BINDLIST.bindCpu.osBitmap     + 0 , 0 , IDC_VB_SSE128_XMM 
SET_BOOL    BINDLIST.bindCpu.osBitmap     + 0 , 1 , IDC_VB_AVX256_YMM 
SET_BOOL    BINDLIST.bindCpu.osBitmap     + 0 , 2 , IDC_VB_AVX512_ZMM_L 
SET_BOOL    BINDLIST.bindCpu.osBitmap     + 0 , 3 , IDC_VB_AVX512_ZMM_H 
SET_BOOL    BINDLIST.bindCpu.osBitmap     + 0 , 4 , IDC_VB_AVX512_K
SET_BOOL    BINDLIST.setMemMethod         + 2 , 2 , IDC_VB_SSE128_READ             
SET_BOOL    BINDLIST.setMemMethod         + 2 , 2 , IDC_VB_SSE128_WRITE             
SET_BOOL    BINDLIST.setMemMethod         + 2 , 2 , IDC_VB_SSE128_COPY             
SET_BOOL    BINDLIST.setMemMethod         + 3 , 0 , IDC_VB_AVX256_READ             
SET_BOOL    BINDLIST.setMemMethod         + 3 , 0 , IDC_VB_AVX256_WRITE
SET_BOOL    BINDLIST.setMemMethod         + 3 , 0 , IDC_VB_AVX256_COPY             
SET_BOOL    BINDLIST.setMemMethod         + 3 , 6 , IDC_VB_AVX512_READ
SET_BOOL    BINDLIST.setMemMethod         + 3 , 6 , IDC_VB_AVX512_WRITE             
SET_BOOL    BINDLIST.setMemMethod         + 3 , 6 , IDC_VB_AVX512_COPY             
SET_BOOL    BINDLIST.setMemMethod         + 7 , 4 , IDC_VB_SQRTPD_XMM             
SET_BOOL    BINDLIST.setMemMethod         + 3 , 0 , IDC_VB_VSQRTPD_YMM             
SET_BOOL    BINDLIST.setMemMethod         + 3 , 6 , IDC_VB_VSQRTPD_ZMM
SET_BOOL    BINDLIST.setMemMethod         + 7 , 6 , IDC_VB_FCOS             
SET_BOOL    BINDLIST.setMemMethod         + 7 , 6 , IDC_VB_FSINCOS
SET_BOOL    BINDLIST.setMemMethod         + 2 , 2 , IDC_VB_READ128_V              
SET_BOOL    BINDLIST.setMemMethod         + 2 , 2 , IDC_VB_WRITE128_V               
SET_BOOL    BINDLIST.setMemMethod         + 2 , 2 , IDC_VB_COPY128_V               
SET_BOOL    BINDLIST.setMemMethod         + 3 , 0 , IDC_VB_READ256_V               
SET_BOOL    BINDLIST.setMemMethod         + 3 , 0 , IDC_VB_WRITE256_V  
SET_BOOL    BINDLIST.setMemMethod         + 3 , 0 , IDC_VB_COPY256_V               
SET_BOOL    BINDLIST.setMemMethod         + 3 , 6 , IDC_VB_READ512_V  
SET_BOOL    BINDLIST.setMemMethod         + 3 , 6 , IDC_VB_WRITE512_V               
SET_BOOL    BINDLIST.setMemMethod         + 3 , 6 , IDC_VB_COPY512_V                 
SET_BOOL    BINDLIST.setMemMethod         + 7 , 4 , IDC_VB_SQRT128_V               
SET_BOOL    BINDLIST.setMemMethod         + 3 , 0 , IDC_VB_SQRT256_V             
SET_BOOL    BINDLIST.setMemMethod         + 3 , 6 , IDC_VB_SQRT512_V  
SET_BOOL    BINDLIST.setMemMethod         + 7 , 6 , IDC_VB_FCOS_V               
SET_BOOL    BINDLIST.setMemMethod         + 7 , 6 , IDC_VB_FSINCOS_V 
SET_STRING  STR_VB_CPU_FEATURES                   , IDC_VB_CPU_FEATURES   
SET_STRING  STR_VB_AVX_256                        , IDC_VB_AVX_256 
SET_STRING  STR_VB_AVX2_256                       , IDC_VB_AVX2_256
SET_STRING  STR_VB_AVX3_512                       , IDC_VB_AVX3_512
SET_STRING  STR_VB_AVX512CD                       , IDC_VB_AVX512CD
SET_STRING  STR_VB_AVX512PF                       , IDC_VB_AVX512PF
SET_STRING  STR_VB_AVX512ER                       , IDC_VB_AVX512ER
SET_STRING  STR_VB_AVX512VL                       , IDC_VB_AVX512VL
SET_STRING  STR_VB_AVX512BW                       , IDC_VB_AVX512BW
SET_STRING  STR_VB_AVX512DQ                       , IDC_VB_AVX512DQ
SET_STRING  STR_VB_AVX512_IFMA                    , IDC_VB_AVX512_IFMA
SET_STRING  STR_VB_AVX512_VBMI                    , IDC_VB_AVX512_VBMI
SET_STRING  STR_VB_AVX512_VBMI2                   , IDC_VB_AVX512_VBMI2
SET_STRING  STR_VB_AVX512_BF16                    , IDC_VB_AVX512_BF16
SET_STRING  STR_VB_AVX512_VAES                    , IDC_VB_AVX512_VAES
SET_STRING  STR_VB_AVX512_GFNI                    , IDC_VB_AVX512_GFNI
SET_STRING  STR_VB_AVX512_VNNI                    , IDC_VB_AVX512_VNNI
SET_STRING  STR_VB_AVX512_BTALG                   , IDC_VB_AVX512_BTALG
SET_STRING  STR_VB_AVX512_VPOP                    , IDC_VB_AVX512_VPOP
SET_STRING  STR_VB_AVX512_VPCL                    , IDC_VB_AVX512_VPCL
SET_STRING  STR_VB_AVX512_VP2IN                   , IDC_VB_AVX512_VP2IN
SET_STRING  STR_VB_AVX512_FP16                    , IDC_VB_AVX512_FP16
SET_STRING  STR_VB_AVX512_4FMAP                   , IDC_VB_AVX512_4FMAP
SET_STRING  STR_VB_AVX512_4VNNI                   , IDC_VB_AVX512_4VNNI
SET_STRING  STR_VB_OS_CONTEXT                     , IDC_VB_OS_CONTEXT
SET_STRING  STR_VB_SSE128_XMM                     , IDC_VB_SSE128_XMM
SET_STRING  STR_VB_AVX256_YMM                     , IDC_VB_AVX256_YMM
SET_STRING  STR_VB_AVX512_ZMM_L                   , IDC_VB_AVX512_ZMM_L
SET_STRING  STR_VB_AVX512_ZMM_H                   , IDC_VB_AVX512_ZMM_H
SET_STRING  STR_VB_AVX512_K                       , IDC_VB_AVX512_K
SET_STRING  STR_VB_TIMINGS                        , IDC_VB_TIMINGS
SET_STRING  STR_VB_SSE128_READ                    , IDC_VB_SSE128_READ             
SET_STRING  STR_VB_SSE128_WRITE                   , IDC_VB_SSE128_WRITE             
SET_STRING  STR_VB_SSE128_COPY                    , IDC_VB_SSE128_COPY             
SET_STRING  STR_VB_AVX256_READ                    , IDC_VB_AVX256_READ             
SET_STRING  STR_VB_AVX256_WRITE                   , IDC_VB_AVX256_WRITE
SET_STRING  STR_VB_AVX256_COPY                    , IDC_VB_AVX256_COPY             
SET_STRING  STR_VB_AVX512_READ                    , IDC_VB_AVX512_READ
SET_STRING  STR_VB_AVX512_WRITE                   , IDC_VB_AVX512_WRITE             
SET_STRING  STR_VB_AVX512_COPY                    , IDC_VB_AVX512_COPY             
SET_STRING  STR_VB_SQRTPD_XMM                     , IDC_VB_SQRTPD_XMM             
SET_STRING  STR_VB_VSQRTPD_YMM                    , IDC_VB_VSQRTPD_YMM             
SET_STRING  STR_VB_VSQRTPD_ZMM                    , IDC_VB_VSQRTPD_ZMM
SET_STRING  STR_VB_FCOS                           , IDC_VB_FCOS             
SET_STRING  STR_VB_FSINCOS                        , IDC_VB_FSINCOS
SET_INFO    BINDLIST.vbSse128read                 , IDC_VB_READ128_V              
SET_INFO    BINDLIST.vbSse128write                , IDC_VB_WRITE128_V               
SET_INFO    BINDLIST.vbSse128copy                 , IDC_VB_COPY128_V               
SET_INFO    BINDLIST.vbAvx256read                 , IDC_VB_READ256_V               
SET_INFO    BINDLIST.vbAvx256write                , IDC_VB_WRITE256_V  
SET_INFO    BINDLIST.vbAvx256copy                 , IDC_VB_COPY256_V               
SET_INFO    BINDLIST.vbAvx512read                 , IDC_VB_READ512_V  
SET_INFO    BINDLIST.vbAvx512write                , IDC_VB_WRITE512_V               
SET_INFO    BINDLIST.vbAvx512copy                 , IDC_VB_COPY512_V                 
SET_INFO    BINDLIST.vbSse128sqrt                 , IDC_VB_SQRT128_V               
SET_INFO    BINDLIST.vbAvx256sqrt                 , IDC_VB_SQRT256_V             
SET_INFO    BINDLIST.vbAvx512sqrt                 , IDC_VB_SQRT512_V  
SET_INFO    BINDLIST.vbX87cos                     , IDC_VB_FCOS_V               
SET_INFO    BINDLIST.vbX87sincos                  , IDC_VB_FSINCOS_V 
SET_STRING  STR_OK                                , IDB_VB_OK             
BIND_STOP
;---------- Continue, binders for GUI scripts ---------------------------------;
; This binders for set GUI objects (widgets) state by data from buffer
; (bindlist), separate from build objects, because required set by
; system configuration detection results and by "Set defaults" button.
; Set widgets states for memory and cache benchmark settings by system info.
SET_SWITCH  BINDLIST.setMemMethod + 0 , 0 , IDB_MEMORY_ASM_A0
SET_SWITCH  BINDLIST.setMemMethod + 0 , 2 , IDB_MEMORY_ASM_A1
SET_SWITCH  BINDLIST.setMemMethod + 0 , 4 , IDB_MEMORY_ASM_A2
SET_SWITCH  BINDLIST.setMemMethod + 0 , 6 , IDB_MEMORY_ASM_A3
SET_SWITCH  BINDLIST.setMemMethod + 1 , 0 , IDB_MEMORY_ASM_A4
SET_SWITCH  BINDLIST.setMemMethod + 1 , 2 , IDB_MEMORY_ASM_A5
SET_SWITCH  BINDLIST.setMemMethod + 1 , 4 , IDB_MEMORY_ASM_A6
SET_SWITCH  BINDLIST.setMemMethod + 1 , 6 , IDB_MEMORY_ASM_A7
SET_SWITCH  BINDLIST.setMemMethod + 2 , 0 , IDB_MEMORY_ASM_A8
SET_SWITCH  BINDLIST.setMemMethod + 2 , 2 , IDB_MEMORY_ASM_B0
SET_SWITCH  BINDLIST.setMemMethod + 2 , 4 , IDB_MEMORY_ASM_B1
SET_SWITCH  BINDLIST.setMemMethod + 2 , 6 , IDB_MEMORY_ASM_B2
SET_SWITCH  BINDLIST.setMemMethod + 3 , 0 , IDB_MEMORY_ASM_B3
SET_SWITCH  BINDLIST.setMemMethod + 3 , 2 , IDB_MEMORY_ASM_B4
SET_SWITCH  BINDLIST.setMemMethod + 3 , 4 , IDB_MEMORY_ASM_B5
SET_SWITCH  BINDLIST.setMemMethod + 3 , 6 , IDB_MEMORY_ASM_B6
SET_SWITCH  BINDLIST.setMemMethod + 4 , 0 , IDB_MEMORY_ASM_B7
SET_SWITCH  BINDLIST.setMemMethod + 4 , 2 , IDB_MEMORY_ASM_B8
SET_SWITCH  BINDLIST.setMemMethod + 4 , 4 , IDB_MEMORY_ASM_C0
SET_SWITCH  BINDLIST.setMemMethod + 4 , 6 , IDB_MEMORY_ASM_C1
SET_SWITCH  BINDLIST.setMemMethod + 5 , 0 , IDB_MEMORY_ASM_C2
SET_SWITCH  BINDLIST.setMemMethod + 5 , 2 , IDB_MEMORY_ASM_C3
SET_SWITCH  BINDLIST.setMemMethod + 5 , 4 , IDB_MEMORY_ASM_C4
SET_SWITCH  BINDLIST.setMemMethod + 5 , 6 , IDB_MEMORY_ASM_C5
SET_SWITCH  BINDLIST.setMemMethod + 6 , 0 , IDB_MEMORY_ASM_C6
SET_SWITCH  BINDLIST.setMemMethod + 6 , 2 , IDB_MEMORY_ASM_C7
SET_SWITCH  BINDLIST.setMemOption + 0 , 0 , IDB_MEMORY_NONTEMP
SET_SWITCH  BINDLIST.setMemOption + 0 , 2 , IDB_MEMORY_FORCE32
SET_SWITCH  BINDLIST.setMemObject + 0 , 0 , IDB_MEMORY_L1
SET_SWITCH  BINDLIST.setMemObject + 0 , 2 , IDB_MEMORY_L2
SET_SWITCH  BINDLIST.setMemObject + 0 , 4 , IDB_MEMORY_L3
SET_SWITCH  BINDLIST.setMemObject + 0 , 6 , IDB_MEMORY_L4
SET_SWITCH  BINDLIST.setMemObject + 1 , 0 , IDB_MEMORY_DRAM
SET_SWITCH  BINDLIST.setMemObject + 1 , 2 , IDB_MEMORY_CUSTOM
SET_SWITCH  BINDLIST.setMemObject + 1 , 4 , IDB_MEMORY_FILE
SET_SWITCH  BINDLIST.setMemObject + 1 , 6 , IDB_MEMORY_GPU
SET_SWITCH  BINDLIST.setMemObject + 2 , 0 , IDB_MEMORY_PHYSICAL
SET_SWITCH  BINDLIST.setMemMtrr   + 0 , 0 , IDB_MEMORY_MTRR_WB
SET_SWITCH  BINDLIST.setMemMtrr   + 0 , 2 , IDB_MEMORY_MTRR_WT
SET_SWITCH  BINDLIST.setMemMtrr   + 0 , 4 , IDB_MEMORY_MTRR_WC
SET_SWITCH  BINDLIST.setMemMtrr   + 0 , 6 , IDB_MEMORY_MTRR_WP
SET_SWITCH  BINDLIST.setMemMtrr   + 1 , 0 , IDB_MEMORY_MTRR_UC
SET_SWITCH  BINDLIST.setMemSmp    + 0 , 0 , IDB_MEMORY_PARALLEL  
SET_SWITCH  BINDLIST.setMemSmp    + 0 , 2 , IDB_MEMORY_HT
SET_SWITCH  BINDLIST.setMemSmp    + 0 , 4 , IDB_MEMORY_PG
SET_SWITCH  BINDLIST.setMemNuma   + 0 , 0 , IDB_MEMORY_NUMA_U
SET_SWITCH  BINDLIST.setMemNuma   + 0 , 2 , IDB_MEMORY_NUMA_S
SET_SWITCH  BINDLIST.setMemNuma   + 0 , 4 , IDB_MEMORY_NUMA_O
SET_SWITCH  BINDLIST.setMemNuma   + 0 , 6 , IDB_MEMORY_NUMA_N
SET_SWITCH  BINDLIST.setMemAccess + 0 , 0 , IDB_MEMORY_NORMAL
SET_SWITCH  BINDLIST.setMemAccess + 0 , 2 , IDB_MEMORY_SK_63
SET_SWITCH  BINDLIST.setMemAccess + 0 , 4 , IDB_MEMORY_SK_4095
SET_SWITCH  BINDLIST.setMemAccess + 0 , 6 , IDB_MEMORY_SK_CSTM
SET_SWITCH  BINDLIST.setMemLpages + 0 , 0 , IDB_MEMORY_LP
SET_SWITCH  BINDLIST.setMemPref   + 0 , 0 , IDB_MEMORY_NO_PF
SET_SWITCH  BINDLIST.setMemPref   + 0 , 2 , IDB_MEMORY_DEF_PF
SET_SWITCH  BINDLIST.setMemPref   + 0 , 4 , IDB_MEMORY_MED_PF
SET_SWITCH  BINDLIST.setMemPref   + 0 , 6 , IDB_MEMORY_LNG_PF
SET_SWITCH  BINDLIST.setMemPref   + 1 , 0 , IDB_MEMORY_BLK_PF
SET_SWITCH  BINDLIST.setMemPref   + 1 , 2 , IDB_MEMORY_CST_PF
SET_SWITCH  BINDLIST.setMemMeas   + 0 , 0 , IDB_MEMORY_BRF 
SET_SWITCH  BINDLIST.setMemMeas   + 0 , 2 , IDB_MEMORY_CRF
SET_SWITCH  BINDLIST.setMemMeas   + 0 , 4 , IDB_MEMORY_BRF_A
SET_SWITCH  BINDLIST.setMemMeas   + 0 , 6 , IDB_MEMORY_CRF_A
SET_SWITCH  BINDLIST.setMemPix    + 0 , 0 , IDB_MEMORY_ALL_P
SET_SWITCH  BINDLIST.setMemPix    + 0 , 2 , IDB_MEMORY_X_16
SET_SWITCH  BINDLIST.setMemPix    + 0 , 4 , IDB_MEMORY_X_32
SET_SWITCH  BINDLIST.setMem3d     + 0 , 0 , IDB_MEMORY_3D_DRAW
SET_HEX64   BINDLIST.setBlkCustom         , IDE_MEMORY_B_SIZE
SET_HEX64   BINDLIST.setBlkMmf            , IDE_MEMORY_F_SIZE
SET_HEX64   BINDLIST.setBlkGpu            , IDE_MEMORY_G_SIZE
SET_HEX64   BINDLIST.setBlkStart          , IDE_MEMORY_M_START
SET_HEX64   BINDLIST.setBlkStop           , IDE_MEMORY_M_STOP
SET_DEC32   BINDLIST.setBlkStep           , IDE_MEMORY_SK_SIZE
SET_DEC32   BINDLIST.setBlkDist           , IDE_MEMORY_PF_SIZE
BIND_STOP
;---------- Continue, binders for GUI scripts ---------------------------------;
; This binders for read data from GUI objects (widgets) to buffer (bindlist).
; Get widgets states from memory and cache benchmark settings screen.
GET_SWITCH  IDB_MEMORY_ASM_A0   , BINDLIST.getMemMethod + 0 , 0
GET_SWITCH  IDB_MEMORY_ASM_A1   , BINDLIST.getMemMethod + 0 , 1
GET_SWITCH  IDB_MEMORY_ASM_A2   , BINDLIST.getMemMethod + 0 , 2
GET_SWITCH  IDB_MEMORY_ASM_A3   , BINDLIST.getMemMethod + 0 , 3
GET_SWITCH  IDB_MEMORY_ASM_A4   , BINDLIST.getMemMethod + 0 , 4
GET_SWITCH  IDB_MEMORY_ASM_A5   , BINDLIST.getMemMethod + 0 , 5
GET_SWITCH  IDB_MEMORY_ASM_A6   , BINDLIST.getMemMethod + 0 , 6
GET_SWITCH  IDB_MEMORY_ASM_A7   , BINDLIST.getMemMethod + 0 , 7
GET_SWITCH  IDB_MEMORY_ASM_A8   , BINDLIST.getMemMethod + 1 , 0
GET_SWITCH  IDB_MEMORY_ASM_B0   , BINDLIST.getMemMethod + 1 , 1 
GET_SWITCH  IDB_MEMORY_ASM_B1   , BINDLIST.getMemMethod + 1 , 2
GET_SWITCH  IDB_MEMORY_ASM_B2   , BINDLIST.getMemMethod + 1 , 3
GET_SWITCH  IDB_MEMORY_ASM_B3   , BINDLIST.getMemMethod + 1 , 4
GET_SWITCH  IDB_MEMORY_ASM_B4   , BINDLIST.getMemMethod + 1 , 5
GET_SWITCH  IDB_MEMORY_ASM_B5   , BINDLIST.getMemMethod + 1 , 6
GET_SWITCH  IDB_MEMORY_ASM_B6   , BINDLIST.getMemMethod + 1 , 7
GET_SWITCH  IDB_MEMORY_ASM_B7   , BINDLIST.getMemMethod + 2 , 0
GET_SWITCH  IDB_MEMORY_ASM_B8   , BINDLIST.getMemMethod + 2 , 1
GET_SWITCH  IDB_MEMORY_ASM_C0   , BINDLIST.getMemMethod + 2 , 2 
GET_SWITCH  IDB_MEMORY_ASM_C1   , BINDLIST.getMemMethod + 2 , 3
GET_SWITCH  IDB_MEMORY_ASM_C2   , BINDLIST.getMemMethod + 2 , 4
GET_SWITCH  IDB_MEMORY_ASM_C3   , BINDLIST.getMemMethod + 2 , 5
GET_SWITCH  IDB_MEMORY_ASM_C4   , BINDLIST.getMemMethod + 2 , 6
GET_SWITCH  IDB_MEMORY_ASM_C5   , BINDLIST.getMemMethod + 2 , 7
GET_SWITCH  IDB_MEMORY_ASM_C6   , BINDLIST.getMemMethod + 3 , 0
GET_SWITCH  IDB_MEMORY_ASM_C7   , BINDLIST.getMemMethod + 3 , 1
GET_SWITCH  IDB_MEMORY_NONTEMP  , BINDLIST.getMemOption + 0 , 0
GET_SWITCH  IDB_MEMORY_FORCE32  , BINDLIST.getMemOption + 0 , 1
GET_SWITCH  IDB_MEMORY_L1       , BINDLIST.getMemObject + 0 , 0
GET_SWITCH  IDB_MEMORY_L2       , BINDLIST.getMemObject + 0 , 1
GET_SWITCH  IDB_MEMORY_L3       , BINDLIST.getMemObject + 0 , 2
GET_SWITCH  IDB_MEMORY_L4       , BINDLIST.getMemObject + 0 , 3
GET_SWITCH  IDB_MEMORY_DRAM     , BINDLIST.getMemObject + 0 , 4
GET_SWITCH  IDB_MEMORY_CUSTOM   , BINDLIST.getMemObject + 0 , 5
GET_SWITCH  IDB_MEMORY_FILE     , BINDLIST.getMemObject + 0 , 6
GET_SWITCH  IDB_MEMORY_GPU      , BINDLIST.getMemObject + 0 , 7
GET_SWITCH  IDB_MEMORY_PHYSICAL , BINDLIST.getMemObject + 1 , 0
GET_SWITCH  IDB_MEMORY_MTRR_WB  , BINDLIST.getMemMtrr   + 0 , 0 
GET_SWITCH  IDB_MEMORY_MTRR_WT  , BINDLIST.getMemMtrr   + 0 , 1
GET_SWITCH  IDB_MEMORY_MTRR_WC  , BINDLIST.getMemMtrr   + 0 , 2
GET_SWITCH  IDB_MEMORY_MTRR_WP  , BINDLIST.getMemMtrr   + 0 , 3
GET_SWITCH  IDB_MEMORY_MTRR_UC  , BINDLIST.getMemMtrr   + 0 , 4
GET_SWITCH  IDB_MEMORY_PARALLEL , BINDLIST.getMemSmp    + 0 , 0  
GET_SWITCH  IDB_MEMORY_HT       , BINDLIST.getMemSmp    + 0 , 1
GET_SWITCH  IDB_MEMORY_PG       , BINDLIST.getMemSmp    + 0 , 2
GET_SWITCH  IDB_MEMORY_NUMA_U   , BINDLIST.getMemNuma   + 0 , 0
GET_SWITCH  IDB_MEMORY_NUMA_S   , BINDLIST.getMemNuma   + 0 , 1
GET_SWITCH  IDB_MEMORY_NUMA_O   , BINDLIST.getMemNuma   + 0 , 2
GET_SWITCH  IDB_MEMORY_NUMA_N   , BINDLIST.getMemNuma   + 0 , 3
GET_SWITCH  IDB_MEMORY_NORMAL   , BINDLIST.getMemAccess + 0 , 0
GET_SWITCH  IDB_MEMORY_SK_63    , BINDLIST.getMemAccess + 0 , 1
GET_SWITCH  IDB_MEMORY_SK_4095  , BINDLIST.getMemAccess + 0 , 2
GET_SWITCH  IDB_MEMORY_SK_CSTM  , BINDLIST.getMemAccess + 0 , 3
GET_SWITCH  IDB_MEMORY_LP       , BINDLIST.getMemLpages + 0 , 0
GET_SWITCH  IDB_MEMORY_NO_PF    , BINDLIST.getMemPref   + 0 , 0
GET_SWITCH  IDB_MEMORY_DEF_PF   , BINDLIST.getMemPref   + 0 , 1
GET_SWITCH  IDB_MEMORY_MED_PF   , BINDLIST.getMemPref   + 0 , 2
GET_SWITCH  IDB_MEMORY_LNG_PF   , BINDLIST.getMemPref   + 0 , 3
GET_SWITCH  IDB_MEMORY_BLK_PF   , BINDLIST.getMemPref   + 0 , 4
GET_SWITCH  IDB_MEMORY_CST_PF   , BINDLIST.getMemPref   + 0 , 5
GET_SWITCH  IDB_MEMORY_BRF      , BINDLIST.getMemMeas   + 0 , 0  
GET_SWITCH  IDB_MEMORY_CRF      , BINDLIST.getMemMeas   + 0 , 1 
GET_SWITCH  IDB_MEMORY_BRF_A    , BINDLIST.getMemMeas   + 0 , 2 
GET_SWITCH  IDB_MEMORY_CRF_A    , BINDLIST.getMemMeas   + 0 , 3 
GET_SWITCH  IDB_MEMORY_ALL_P    , BINDLIST.getMemPix    + 0 , 0
GET_SWITCH  IDB_MEMORY_X_16     , BINDLIST.getMemPix    + 0 , 1 
GET_SWITCH  IDB_MEMORY_X_32     , BINDLIST.getMemPix    + 0 , 2 
GET_SWITCH  IDB_MEMORY_3D_DRAW  , BINDLIST.getMem3d     + 0 , 0
GET_HEX64   IDE_MEMORY_B_SIZE   , BINDLIST.getBlkCustom
GET_HEX64   IDE_MEMORY_F_SIZE   , BINDLIST.getBlkMmf
GET_HEX64   IDE_MEMORY_G_SIZE   , BINDLIST.getBlkGpu
GET_HEX64   IDE_MEMORY_M_START  , BINDLIST.getBlkStart
GET_HEX64   IDE_MEMORY_M_STOP   , BINDLIST.getBlkStop
GET_DEC32   IDE_MEMORY_SK_SIZE  , BINDLIST.getBlkStep
GET_DEC32   IDE_MEMORY_PF_SIZE  , BINDLIST.getBlkDist
BIND_STOP
;--- Continue, binder for widgets dependency support: "Nontemporal" widget ----;
SET_SWITCH  BINDLIST.scratchPad + 0 , 0 , IDB_MEMORY_NONTEMP
BIND_STOP
;--- Continue, binder for widgets dependency support: "Force 32x2" widget -----;
SET_SWITCH  BINDLIST.scratchPad + 0 , 0 , IDB_MEMORY_FORCE32
BIND_STOP
;--- Continue, binder for widgets dependency support: "Prefetch" widget(s) ----;
SET_SWITCH  BINDLIST.scratchPad + 0 , 0 , IDB_MEMORY_NO_PF
SET_SWITCH  BINDLIST.scratchPad + 0 , 2 , IDB_MEMORY_DEF_PF
SET_SWITCH  BINDLIST.scratchPad + 0 , 4 , IDB_MEMORY_MED_PF
SET_SWITCH  BINDLIST.scratchPad + 0 , 6 , IDB_MEMORY_LNG_PF
SET_SWITCH  BINDLIST.scratchPad + 1 , 0 , IDB_MEMORY_BLK_PF
SET_SWITCH  BINDLIST.scratchPad + 1 , 2 , IDB_MEMORY_CST_PF
BIND_STOP
;--- Continue, binder for widgets dependency support: "Custom size" widget ----;
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDE_MEMORY_B_SIZE
BIND_STOP
;--- Continue, binder for disable buttons when run bench. and enable after ----;
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_SYSINFO_VBRF
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_SYSINFO_CANCEL
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_MEMORY_DRAW
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_MEMORY_RUN
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_MEMORY_DEFAULTS
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_MEMORY_CANCEL
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_OS_CANCEL
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_NAT_OS_CANCEL
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_TOPOL_CANCEL
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_TOPOL_EX_CANCEL
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_NUMA_CANCEL
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_P_GROUPS_CANCEL
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_ACPI_CANCEL
SET_BOOL    BINDLIST.scratchPad + 0 , 0 , IDB_A_CPUID_CANCEL
BIND_STOP
endres
;---------- CPU common features bitmap builder script -------------------------;
resdata cpuCommonFeatures
ENTRY_CPUID     00000001h             , R_EDX , 23   ; MMX
ENTRY_CPUID     00000001h             , R_EDX , 25   ; SSE  
ENTRY_CPUID     00000001h             , R_EDX , 26   ; SSE2
ENTRY_CPUID     00000001h             , R_ECX , 01   ; SSE3
ENTRY_CPUID     00000001h             , R_ECX , 09   ; SSSE3
ENTRY_CPUID     00000001h             , R_ECX , 19   ; SSE4.1
ENTRY_CPUID     00000001h             , R_ECX , 20   ; SSE4.2
ENTRY_CPUID     00000001h             , R_ECX , 28   ; AVX
ENTRY_CPUID     00000007h             , R_EBX , 05   ; AVX2
ENTRY_CPUID_S   00000007h , 00000000h , R_EBX , 16   ; AVX512F
ENTRY_CPUID     00000001h             , R_ECX , 30   ; RDRAND
ENTRY_CPUID     00000001h             , R_ECX , 05   ; VMX
ENTRY_CPUID     80000001h             , R_ECX , 02   ; SVM
ENTRY_CPUID     80000001h             , R_EDX , 29   ; x86-64
ENTRY_CPUID     00000001h             , R_ECX , 12   ; FMA 256
ENTRY_CPUID     80000008h             , R_EBX , 0    ; CLZERO
ENTRY_CPUID     00000001h             , R_EDX , 0    ; x87 (redundant by run criteria)
ENTRY_STOP
endres  
;---------- CPU AVX512 features bitmap builder script -------------------------;
resdata cpuAvx512Features
ENTRY_CPUID_S   00000007h , 00000000h , R_EBX , 28   ; AVX512CD
ENTRY_CPUID_S   00000007h , 00000000h , R_EBX , 26   ; AVX512PF
ENTRY_CPUID_S   00000007h , 00000000h , R_EBX , 27   ; AVX512ER
ENTRY_CPUID_S   00000007h , 00000000h , R_EBX , 31   ; AVX512VL
ENTRY_CPUID_S   00000007h , 00000000h , R_EBX , 30   ; AVX512BW
ENTRY_CPUID_S   00000007h , 00000000h , R_EBX , 17   ; AVX512DQ
ENTRY_CPUID_S   00000007h , 00000000h , R_EBX , 21   ; AVX512_IFMA
ENTRY_CPUID_S   00000007h , 00000000h , R_ECX , 01   ; AVX512_VBMI
ENTRY_CPUID_S   00000007h , 00000000h , R_ECX , 06   ; AVX512_VBMI2
ENTRY_CPUID_S   00000007h , 00000001h , R_EAX , 05   ; AVX512_BF16
PREFIX_CPUID_S  00000007h , 00000000h , R_EBX , 16   ; Additional check AVX512F
ENTRY_CPUID_S   00000007h , 00000000h , R_ECX , 09   ; AVX512+VAES
PREFIX_CPUID_S  00000007h , 00000000h , R_EBX , 16   ; Additional check AVX512F
ENTRY_CPUID_S   00000007h , 00000000h , R_ECX , 08   ; AVX512+GFNI
ENTRY_CPUID_S   00000007h , 00000000h , R_ECX , 11   ; AVX512_VNNI
ENTRY_CPUID_S   00000007h , 00000000h , R_ECX , 12   ; AVX512_BITALG
ENTRY_CPUID_S   00000007h , 00000000h , R_ECX , 14   ; AVX512_VPOPCNTDQ
PREFIX_CPUID_S  00000007h , 00000000h , R_EBX , 16   ; Additional check AVX512F
ENTRY_CPUID_S   00000007h , 00000000h , R_ECX , 10   ; AVX512+VPCLMULQDQ
ENTRY_CPUID_S   00000007h , 00000000h , R_EDX , 08   ; AVX512_VP2INTERSECT
ENTRY_CPUID_S   00000007h , 00000000h , R_EDX , 23   ; AVX512_FP16
ENTRY_CPUID_S   00000007h , 00000000h , R_EDX , 03   ; AVX512_4FMAPS
ENTRY_CPUID_S   00000007h , 00000000h , R_EDX , 02   ; AVX512_4VNNIW
ENTRY_STOP
endres  
;---------- OS context features bitmap builder script -------------------------;
resdata osContextFeatures
ENTRY_XCR0     01   ; XMM[0-15]  
ENTRY_XCR0     02   ; YMM[0-15] 
ENTRY_XCR0     06   ; ZMM[0-15]
ENTRY_XCR0     07   ; ZMM[16-31]
ENTRY_XCR0     05   ; K[0-7]
ENTRY_XCR0     03   ; BNDREGS
ENTRY_XCR0     04   ; BNDCSR
ENTRY_STOP
endres  
;---------- List of INTEL-specific cache descriptors for caches detection -----; 
resdata intelCache
;---------- List of INTEL-specific cache descriptors for caches detection -----;
; Data structure for Intel Cache Detection 
; This actual for CPUs not support function 
; CPUID 00000004h = Deterministic Cache Info, 
; Note if runs under Windows XP 32-bit, cache information cannot be detected
; by Win API GetLogicalProcessorInformation, means required use:
; CPUID 00000002h = Get Cache Descriptors, with decoding by this data
;--- This original list imported from Java Cpuid utility ----------------------;
;    { 0x00, "null descriptor (=unused descriptor)" } ,
;    { 0x01, "code TLB, 4K pages, 4 ways, 32 entries" } ,
;    { 0x02, "code TLB, 4M pages, fully, 2 entries" } ,
;    { 0x03, "data TLB, 4K pages, 4 ways, 64 entries" } ,
;    { 0x04, "data TLB, 4M pages, 4 ways, 8 entries" } ,
;    { 0x05, "data TLB, 4M pages, 4 ways, 32 entries" } , 
;    { 0x06, "code L1 cache, 8 KB, 4 ways, 32 byte lines" } , 
;    { 0x08, "code L1 cache, 16 KB, 4 ways, 32 byte lines" } , 
;    { 0x09, "code L1 cache, 32 KB, 4 ways, 64 byte lines" } , 
;    { 0x0A, "data L1 cache, 8 KB, 2 ways, 32 byte lines" } , 
;    { 0x0B, "code TLB, 4M pages, 4 ways, 4 entries" } , 
;    { 0x0C, "data L1 cache, 16 KB, 4 ways, 32 byte lines" } ,
;    { 0x0D, "data L1 cache, 16 KB, 4 ways, 64 byte lines (ECC)" } , 
;    { 0x0E, "data L1 cache, 24 KB, 6 ways, 64 byte lines" } , 
;    { 0x10, "data L1 cache, 16 KB, 4 ways, 32 byte lines (IA-64)" } , 
;    { 0x15, "code L1 cache, 16 KB, 4 ways, 32 byte lines (IA-64)" } , 
;    { 0x1A, "code and data L2 cache, 96 KB, 6 ways, 64 byte lines (IA-64)" } , 
;    { 0x1D, "code and data L2 cache, 128 KB, 2 ways, 64 byte lines" } , 
;    { 0x21, "code and data L2 cache, 256 KB, 8 ways, 64 byte lines" } , 
;    { 0x22, "code and data L3 cache, 512 KB, 4 ways (!), 64 byte lines, dual-sectored" } , 
;    { 0x23, "code and data L3 cache, 1024 KB, 8 ways, 64 byte lines, dual-sectored" } , 
;    { 0x24, "code and data L2 cache, 1024 KB, 16 ways, 64 byte lines" } , 
;    { 0x25, "code and data L3 cache, 2048 KB, 8 ways, 64 byte lines, dual-sectored" } , 
;    { 0x29, "code and data L3 cache, 4096 KB, 8 ways, 64 byte lines, dual-sectored" } , 
;    { 0x2C, "data L1 cache, 32 KB, 8 ways, 64 byte lines" } ,  
;    { 0x30, "code L1 cache, 32 KB, 8 ways, 64 byte lines" } ,  
;    { 0x39, "code and data L2 cache, 128 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x3A, "code and data L2 cache, 192 KB, 6 ways, 64 byte lines, sectored" } ,  
;    { 0x3B, "code and data L2 cache, 128 KB, 2 ways, 64 byte lines, sectored" } ,  
;    { 0x3C, "code and data L2 cache, 256 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x3D, "code and data L2 cache, 384 KB, 6 ways, 64 byte lines, sectored" } ,  
;    { 0x3E, "code and data L2 cache, 512 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x40, "no integrated L2 cache (P6 core) or L3 cache (P4 core)" } ,  
;    { 0x41, "code and data L2 cache, 128 KB, 4 ways, 32 byte lines" } ,  
;    { 0x42, "code and data L2 cache, 256 KB, 4 ways, 32 byte lines" } ,  
;    { 0x43, "code and data L2 cache, 512 KB, 4 ways, 32 byte lines" } ,  
;    { 0x44, "code and data L2 cache, 1024 KB, 4 ways, 32 byte lines" } ,  
;    { 0x45, "code and data L2 cache, 2048 KB, 4 ways, 32 byte lines" } ,  
;    { 0x46, "code and data L3 cache, 4096 KB, 4 ways, 64 byte lines" } ,  
;    { 0x47, "code and data L3 cache, 8192 KB, 8 ways, 64 byte lines" } ,  
;    { 0x48, "code and data L2 cache, 3072 KB, 12 ways, 64 byte lines" } ,  
;    { 0x49, "code and data L3 cache, 4096 KB, 16 ways, 64 byte lines (P4) or"
;           + " code and data L2 cache, 4096 KB, 16 ways, 64 byte lines (Core 2)" } ,   
;    { 0x4A, "code and data L3 cache, 6144 KB, 12 ways, 64 byte lines" } ,  
;    { 0x4B, "code and data L3 cache, 8192 KB, 16 ways, 64 byte lines" } ,  
;    { 0x4C, "code and data L3 cache, 12288 KB, 12 ways, 64 byte lines" } ,  
;    { 0x4D, "code and data L3 cache, 16384 KB, 16 ways, 64 byte lines" } ,  
;    { 0x4E, "code and data L2 cache, 6144 KB, 24 ways, 64 byte lines" } ,  
;    { 0x4F, "code TLB, 4K pages, ???, 32 entries" } ,  
;    { 0x50, "code TLB, 4K/4M/2M pages, fully, 64 entries" } ,  
;    { 0x51, "code TLB, 4K/4M/2M pages, fully, 128 entries" } ,  
;    { 0x52, "code TLB, 4K/4M/2M pages, fully, 256 entries" } ,  
;    { 0x55, "code TLB, 2M/4M, fully, 7 entries" } ,  
;    { 0x56, "L0 data TLB, 4M pages, 4 ways, 16 entries" } ,  
;    { 0x57, "L0 data TLB, 4K pages, 4 ways, 16 entries" } ,  
;    { 0x59, "L0 data TLB, 4K pages, fully, 16 entries" } ,  
;    { 0x5A, "L0 data TLB, 2M/4M, 4 ways, 32 entries" } ,  
;    { 0x5B, "data TLB, 4K/4M pages, fully, 64 entries" } ,  
;    { 0x5C, "data TLB, 4K/4M pages, fully, 128 entries" } ,  
;    { 0x5D, "data TLB, 4K/4M pages, fully, 256 entries" } ,  
;    { 0x60, "data L1 cache, 16 KB, 8 ways, 64 byte lines, sectored" } ,  
;    { 0x61, "code TLB, 4K pages, fully, 48 entries" } ,  
;    { 0x63, "data TLB, 2M/4M pages, 4-way, 32-entries, and"
;           + " data TLB, 1G pages, 4-way, 4 entries" } ,   
;    { 0x64, "data TLB, 4K pages, 4-way, 512 entries" } ,  
;    { 0x66, "data L1 cache, 8 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x67, "data L1 cache, 16 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x68, "data L1 cache, 32 KB, 4 ways, 64 byte lines, sectored" } ,  
;    { 0x6A, "L0 data TLB, 4K pages, 8-way, 64 entries" } ,  
;    { 0x6B, "data TLB, 4K pages, 8-way, 256 entries" } ,  
;    { 0x6C, "data TLB, 2M/4M pages, 8-way, 126 entries" } ,  
;    { 0x6D, "data TLB, 1G pages, fully, 16 entries" } ,  
;    { 0x70, "trace L1 cache, 12 K킣Ps, 8 ways" } ,  
;    { 0x71, "trace L1 cache, 16 K킣Ps, 8 ways" } ,  
;    { 0x72, "trace L1 cache, 32 K킣Ps, 8 ways" } ,  
;    { 0x73, "trace L1 cache, 64 K킣Ps, 8 ways" } ,  
;    { 0x76, "code TLB, 2M/4M pages, fully, 8 entries" } ,  
;    { 0x77, "code L1 cache, 16 KB, 4 ways, 64 byte lines, sectored (IA-64)" } ,  
;    { 0x78, "code and data L2 cache, 1024 KB, 4 ways, 64 byte lines" } ,  
;    { 0x79, "code and data L2 cache, 128 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
;    { 0x7A, "code and data L2 cache, 256 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
;    { 0x7B, "code and data L2 cache, 512 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
;    { 0x7C, "code and data L2 cache, 1024 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
;    { 0x7D, "code and data L2 cache, 2048 KB, 8 ways, 64 byte lines" } ,  
;    { 0x7E, "code and data L2 cache, 256 KB, 8 ways, 128 byte lines, sect. (IA-64)" } ,  
;    { 0x7F, "code and data L2 cache, 512 KB, 2 ways, 64 byte lines" } ,  
;    { 0x80, "code and data L2 cache, 512 KB, 8 ways, 64 byte lines" } ,  
;    { 0x81, "code and data L2 cache, 128 KB, 8 ways, 32 byte lines" } ,  
;    { 0x82, "code and data L2 cache, 256 KB, 8 ways, 32 byte lines" } ,  
;    { 0x83, "code and data L2 cache, 512 KB, 8 ways, 32 byte lines" } ,  
;    { 0x84, "code and data L2 cache, 1024 KB, 8 ways, 32 byte lines" } ,  
;    { 0x85, "code and data L2 cache, 2048 KB, 8 ways, 32 byte lines" } ,  
;    { 0x86, "code and data L2 cache, 512 KB, 4 ways, 64 byte lines" } ,  
;    { 0x87, "code and data L2 cache, 1024 KB, 8 ways, 64 byte lines" } ,  
;    { 0x88, "code and data L3 cache, 2048 KB, 4 ways, 64 byte lines (IA-64)" } ,  
;    { 0x89, "code and data L3 cache, 4096 KB, 4 ways, 64 byte lines (IA-64)" } ,  
;    { 0x8A, "code and data L3 cache, 8192 KB, 4 ways, 64 byte lines (IA-64)" } ,  
;    { 0x8D, "code and data L3 cache, 3072 KB, 12 ways, 128 byte lines (IA-64)" } ,  
;    { 0x90, "code TLB, 4K...256M pages, fully, 64 entries (IA-64)" } ,  
;    { 0x96, "data L1 TLB, 4K...256M pages, fully, 32 entries (IA-64)" } ,  
;    { 0x9B, "data L2 TLB, 4K...256M pages, fully, 96 entries (IA-64)" } ,  
;    { 0xA0, "data TLB, 4K pages, fully, 32 entries" } ,  
;    { 0xB0, "code TLB, 4K pages, 4 ways, 128 entries" } ,  
;    { 0xB1, "code TLB, 4M pages, 4 ways, 4 entries and"
;           + " code TLB, 2M pages, 4 ways, 8 entries " } ,  
;    { 0xB2, "code TLB, 4K pages, 4 ways, 64 entries" } ,  
;    { 0xB3, "data TLB, 4K pages, 4 ways, 128 entries" } ,  
;    { 0xB4, "data TLB, 4K pages, 4 ways, 256 entries" } ,  
;    { 0xB5, "code TLB, 4K pages, 8 ways, 64 entries" } ,  
;    { 0xB6, "code TLB, 4K pages, 8 ways, 128 entries" } ,  
;    { 0xBA, "data TLB, 4K pages, 4 ways, 64 entries" } ,  
;    { 0xC0, "data TLB, 4K/4M pages, 4 ways, 8 entries" } ,  
;    { 0xC1, "L2 code and data TLB, 4K/2M pages, 8 ways, 1024 entries" } ,  
;    { 0xC2, "data TLB, 2M/4M pages, 4 ways, 16 entries" } ,  
;    { 0xC3, "L2 code and data TLB, 4K/2M pages, 6 ways, 1536 entries and"
;           + " L2 code and data TLB, 1G pages, 4 ways, 16 entries" } ,   
;    { 0xC4, "data TLB, 2M/4M pages, 4-way, 32 entries" } ,  
;    { 0xCA, "L2 code and data TLB, 4K pages, 4 ways, 512 entries" } ,  
;    { 0xD0, "code and data L3 cache, 512-kb, 4 ways, 64 byte lines" } ,  
;    { 0xD1, "code and data L3 cache, 1024-kb, 4 ways, 64 byte lines" } ,  
;    { 0xD2, "code and data L3 cache, 2048-kb, 4 ways, 64 byte lines" } ,  
;    { 0xD6, "code and data L3 cache, 1024-kb, 8 ways, 64 byte lines" } ,  
;    { 0xD7, "code and data L3 cache, 2048-kb, 8 ways, 64 byte lines" } ,  
;    { 0xD8, "code and data L3 cache, 4096-kb, 8 ways, 64 byte lines" } ,  
;    { 0xDC, "code and data L3 cache, 1536-kb, 12 ways, 64 byte lines" } ,  
;    { 0xDD, "code and data L3 cache, 3072-kb, 12 ways, 64 byte lines" } ,  
;    { 0xDE, "code and data L3 cache, 6144-kb, 12 ways, 64 byte lines" } ,  
;    { 0xE2, "code and data L3 cache, 2048-kb, 16 ways, 64 byte lines" } ,  
;    { 0xE3, "code and data L3 cache, 4096-kb, 16 ways, 64 byte lines" } ,  
;    { 0xE4, "code and data L3 cache, 8192-kb, 16 ways, 64 byte lines" } ,  
;    { 0xEA, "code and data L3 cache, 12288-kb, 24 ways, 64 byte lines" } ,  
;    { 0xEB, "code and data L3 cache, 18432-kb, 24 ways, 64 byte lines" } ,  
;    { 0xEC, "code and data L3 cache, 24576-kb, 24 ways, 64 byte lines" } ,  
;    { 0xF0, "64 byte prefetching" } ,  
;    { 0xF1, "128 byte prefetching" } ,  
;    { 0xFF, "query standard level 0000_0004h instead" } , 
;    };
;--- This list contains cache only information from original Java Cpuid code --;
; TLB and some special information rejected for current purpose.
CacheDescriptorsDecoder:
L1I 006h, 8      ; { 0x06, "code L1 cache, 8 KB, 4 ways, 32 byte lines" } , 
L1I 008h, 16     ; { 0x08, "code L1 cache, 16 KB, 4 ways, 32 byte lines" } , 
L1I 009h, 32     ; { 0x09, "code L1 cache, 32 KB, 4 ways, 64 byte lines" } , 
L1D 00Ah, 8      ; { 0x0A, "data L1 cache, 8 KB, 2 ways, 32 byte lines" } , 
L1D 00Ch, 16     ; { 0x0C, "data L1 cache, 16 KB, 4 ways, 32 byte lines" } ,
L1D 00Dh, 16     ; { 0x0D, "data L1 cache, 16 KB, 4 ways, 64 byte lines (ECC)" } , 
L1D 00Eh, 24     ; { 0x0E, "data L1 cache, 24 KB, 6 ways, 64 byte lines" } , 
L1D 010h, 16     ; { 0x10, "data L1 cache, 16 KB, 4 ways, 32 byte lines (IA-64)" } ,
L1I 015h, 16     ; { 0x15, "code L1 cache, 16 KB, 4 ways, 32 byte lines (IA-64)" } , 
L2U 01Ah, 96     ; { 0x1A, "code and data L2 cache, 96 KB, 6 ways, 64 byte lines (IA-64)" } , 
L2U 01Dh, 128    ; { 0x1D, "code and data L2 cache, 128 KB, 2 ways, 64 byte lines" } , 
L2U 021h, 256    ; { 0x21, "code and data L2 cache, 256 KB, 8 ways, 64 byte lines" } ,
L3U 022h, 512    ; { 0x22, "code and data L3 cache, 512 KB, 4 ways (!), 64 byte lines, dual-sectored" } , 
L3U 023h, 1024   ; { 0x23, "code and data L3 cache, 1024 KB, 8 ways, 64 byte lines, dual-sectored" } ,
L2U 024h, 1024   ; { 0x24, "code and data L2 cache, 1024 KB, 16 ways, 64 byte lines" } , 
L3U 025h, 2048   ; { 0x25, "code and data L3 cache, 2048 KB, 8 ways, 64 byte lines, dual-sectored" } , 
L3U 029h, 4096   ; { 0x29, "code and data L3 cache, 4096 KB, 8 ways, 64 byte lines, dual-sectored" } , 
L1D 02Ch, 32     ; { 0x2C, "data L1 cache, 32 KB, 8 ways, 64 byte lines" } ,  
L1I 030h, 32     ; { 0x30, "code L1 cache, 32 KB, 8 ways, 64 byte lines" } ,  
L2U 039h, 128    ; { 0x39, "code and data L2 cache, 128 KB, 4 ways, 64 byte lines, sectored" } ,  
L2U 03Ah, 192    ; { 0x3A, "code and data L2 cache, 192 KB, 6 ways, 64 byte lines, sectored" } ,  
L2U 03Bh, 128    ; { 0x3B, "code and data L2 cache, 128 KB, 2 ways, 64 byte lines, sectored" } ,  
L2U 03Ch, 256    ; { 0x3C, "code and data L2 cache, 256 KB, 4 ways, 64 byte lines, sectored" } ,  
L2U 03Dh, 384    ; { 0x3D, "code and data L2 cache, 384 KB, 6 ways, 64 byte lines, sectored" } ,  
L2U 03Eh, 512    ; { 0x3E, "code and data L2 cache, 512 KB, 4 ways, 64 byte lines, sectored" } ,  
L2U 041h, 128    ; { 0x41, "code and data L2 cache, 128 KB, 4 ways, 32 byte lines" } ,  
L2U 042h, 256    ; { 0x42, "code and data L2 cache, 256 KB, 4 ways, 32 byte lines" } ,  
L2U 043h, 512    ; { 0x43, "code and data L2 cache, 512 KB, 4 ways, 32 byte lines" } ,  
L2U 044h, 1024   ; { 0x44, "code and data L2 cache, 1024 KB, 4 ways, 32 byte lines" } ,  
L2U 045h, 2048   ; { 0x45, "code and data L2 cache, 2048 KB, 4 ways, 32 byte lines" } ,  
L3U 046h, 4096   ; { 0x46, "code and data L3 cache, 4096 KB, 4 ways, 64 byte lines" } ,  
L3U 047h, 8192   ; { 0x47, "code and data L3 cache, 8192 KB, 8 ways, 64 byte lines" } ,  
L2U 048h, 3072   ; { 0x48, "code and data L2 cache, 3072 KB, 12 ways, 64 byte lines" } ,  
; L3U 049h, 4096   ; { 0x49, "code and data L3 cache, 4096 KB, 16 ways, 64 byte lines (P4) or"
;                  ;  + " code and data L2 cache, 4096 KB, 16 ways, 64 byte lines (Core 2)" } ,   
L3U 04Ah, 6144   ; { 0x4A, "code and data L3 cache, 6144 KB, 12 ways, 64 byte lines" } ,  
L3U 04Bh, 8192   ; { 0x4B, "code and data L3 cache, 8192 KB, 16 ways, 64 byte lines" } ,  
L3U 04Ch, 12288  ; { 0x4C, "code and data L3 cache, 12288 KB, 12 ways, 64 byte lines" } ,  
L3U 04Dh, 16384  ; { 0x4D, "code and data L3 cache, 16384 KB, 16 ways, 64 byte lines" } ,
L2U 04Eh, 6144   ; { 0x4E, "code and data L2 cache, 6144 KB, 24 ways, 64 byte lines" } ,  
L1D 060h, 16     ; { 0x60, "data L1 cache, 16 KB, 8 ways, 64 byte lines, sectored" } ,  
L1D 066h, 8      ; { 0x66, "data L1 cache, 8 KB, 4 ways, 64 byte lines, sectored" } ,  
L1D 067h, 16     ; { 0x67, "data L1 cache, 16 KB, 4 ways, 64 byte lines, sectored" } ,  
L1D 068h, 32     ; { 0x68, "data L1 cache, 32 KB, 4 ways, 64 byte lines, sectored" } ,  
L1T 070h, 12     ; { 0x70, "trace L1 cache, 12 K킣Ps, 8 ways" } ,  
L1T 071h, 16     ; { 0x71, "trace L1 cache, 16 K킣Ps, 8 ways" } ,  
L1T 072h, 32     ; { 0x72, "trace L1 cache, 32 K킣Ps, 8 ways" } ,  
L1T 073h, 64     ; { 0x73, "trace L1 cache, 64 K킣Ps, 8 ways" } ,  
L1I 077h, 16     ; { 0x77, "code L1 cache, 16 KB, 4 ways, 64 byte lines, sectored (IA-64)" } ,  
L2U 078h, 1024   ; { 0x78, "code and data L2 cache, 1024 KB, 4 ways, 64 byte lines" } ,  
L2U 079h, 128    ; { 0x79, "code and data L2 cache, 128 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
L2U 07Ah, 256    ; { 0x7A, "code and data L2 cache, 256 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
L2U 07Bh, 512    ; { 0x7B, "code and data L2 cache, 512 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
L2U 07Ch, 1024   ; { 0x7C, "code and data L2 cache, 1024 KB, 8 ways, 64 byte lines, dual-sectored" } ,  
L2U 07Dh, 2048   ; { 0x7D, "code and data L2 cache, 2048 KB, 8 ways, 64 byte lines" } ,  
L2U 07Eh, 256    ; { 0x7E, "code and data L2 cache, 256 KB, 8 ways, 128 byte lines, sect. (IA-64)" } ,  
L2U 07Fh, 512    ; { 0x7F, "code and data L2 cache, 512 KB, 2 ways, 64 byte lines" } ,  
L2U 080h, 512    ; { 0x80, "code and data L2 cache, 512 KB, 8 ways, 64 byte lines" } ,  
L2U 081h, 128    ; { 0x81, "code and data L2 cache, 128 KB, 8 ways, 32 byte lines" } ,  
L2U 082h, 256    ; { 0x82, "code and data L2 cache, 256 KB, 8 ways, 32 byte lines" } ,  
L2U 083h, 512    ; { 0x83, "code and data L2 cache, 512 KB, 8 ways, 32 byte lines" } ,  
L2U 084h, 1024   ; { 0x84, "code and data L2 cache, 1024 KB, 8 ways, 32 byte lines" } ,  
L2U 085h, 2048   ; { 0x85, "code and data L2 cache, 2048 KB, 8 ways, 32 byte lines" } ,  
L2U 086h, 512    ; { 0x86, "code and data L2 cache, 512 KB, 4 ways, 64 byte lines" } ,  
L2U 087h, 1024   ; { 0x87, "code and data L2 cache, 1024 KB, 8 ways, 64 byte lines" } ,
L3U 088h, 2048   ; { 0x88, "code and data L3 cache, 2048 KB, 4 ways, 64 byte lines (IA-64)" } ,  
L3U 089h, 4096   ; { 0x89, "code and data L3 cache, 4096 KB, 4 ways, 64 byte lines (IA-64)" } ,  
L3U 08Ah, 8192   ; { 0x8A, "code and data L3 cache, 8192 KB, 4 ways, 64 byte lines (IA-64)" } ,  
L3U 08Dh, 3072   ; { 0x8D, "code and data L3 cache, 3072 KB, 12 ways, 128 byte lines (IA-64)" } ,  
L3U 0D0h, 512    ; { 0xD0, "code and data L3 cache, 512-kb, 4 ways, 64 byte lines" } ,  
L3U 0D1h, 1024   ; { 0xD1, "code and data L3 cache, 1024-kb, 4 ways, 64 byte lines" } ,  
L3U 0D2h, 2048   ; { 0xD2, "code and data L3 cache, 2048-kb, 4 ways, 64 byte lines" } ,  
L3U 0D6h, 1024   ; { 0xD6, "code and data L3 cache, 1024-kb, 8 ways, 64 byte lines" } ,  
L3U 0D7h, 2048   ; { 0xD7, "code and data L3 cache, 2048-kb, 8 ways, 64 byte lines" } ,  
L3U 0D8h, 4096   ; { 0xD8, "code and data L3 cache, 4096-kb, 8 ways, 64 byte lines" } ,  
L3U 0DCh, 1536   ; { 0xDC, "code and data L3 cache, 1536-kb, 12 ways, 64 byte lines" } ,  
L3U 0DDh, 3072   ; { 0xDD, "code and data L3 cache, 3072-kb, 12 ways, 64 byte lines" } ,  
L3U 0DEh, 6144   ; { 0xDE, "code and data L3 cache, 6144-kb, 12 ways, 64 byte lines" } ,  
L3U 0E2h, 2048   ; { 0xE2, "code and data L3 cache, 2048-kb, 16 ways, 64 byte lines" } ,  
L3U 0E3h, 4096   ; { 0xE3, "code and data L3 cache, 4096-kb, 16 ways, 64 byte lines" } ,  
L3U 0E4h, 8192   ; { 0xE4, "code and data L3 cache, 8192-kb, 16 ways, 64 byte lines" } ,  
L3U 0EAh, 12288  ; { 0xEA, "code and data L3 cache, 12288-kb, 24 ways, 64 byte lines" } ,  
L3U 0EBh, 18432  ; { 0xEB, "code and data L3 cache, 18432-kb, 24 ways, 64 byte lines" } ,  
L3U 0ECh, 24576  ; { 0xEC, "code and data L3 cache, 24576-kb, 24 ways, 64 byte lines" } ,  
END_CACHE        ; List terminator
endres
;---------- ACPI tables data base ---------------------------------------------; 
resdata acpiData
DB  'AEST' , 'Arm Error Source'                                 , 0
DB  'APIC' , 'Multiple APIC Description'                        , 0
DB  'BDAT' , 'BIOS Data ACPI'                                   , 0
DB  'BERT' , 'Boot Error Record'                                , 0
DB  'BGRT' , 'Boot Graphics Resource'                           , 0
DB  'BOOT' , 'Simple Boot Flag'                                 , 0
DB  'CDIT' , 'Component Distance Information'                   , 0
DB  'CEDT' , 'CXL Early Discovery'                              , 0
DB  'CPEP' , 'Corrected Platform Error Polling'                 , 0
DB  'CRAT' , 'Component Resource Attribute'                     , 0
DB  'CSRT' , 'Core System Resource'                             , 0
DB  'DBGP' , 'Debug Port'                                       , 0
DB  'DMAR' , 'DMA Remapping'                                    , 0
DB  'DSDT' , 'Differentiated System Description'                , 0
DB  'DPPT' , 'DMA Protection Policy'                            , 0
DB  'DRTM' , 'Dynamic Root of Trust for Measurement'            , 0
DB  'ECDT' , 'Embedded Controller Boot Resources'               , 0
DB  'EINJ' , 'Error Injection'                                  , 0
DB  'ERST' , 'Error Record Serialization'                       , 0
DB  'ETDT' , 'Event Timer Description'                          , 0
DB  'FACP' , 'Fixed ACPI Description'                           , 0
DB  'FACS' , 'Firmware ACPI Control Structure'                  , 0
DB  'FPDT' , 'Firmware Performance Data'                        , 0
DB  'GTDT' , 'Generic Timer Description'                        , 0
DB  'HEST' , 'Hardware Error Source'                            , 0
DB  'HPET' , 'High Precision Event Timer'                       , 0
DB  'IBFT' , 'iSCSI Boot Firmware'                              , 0
DB  'IORT' , 'I/O Remapping'                                    , 0
DB  'IVRS' , 'I/O Virtualization Reporting'                     , 0
DB  'LPIT' , 'Low Power Idle'                                   , 0
DB  'MCFG' , 'Memory Mapped Configuration'                      , 0
DB  'MCHI' , 'Management Controller Host Interface'             , 0
DB  'MPAM' , 'Arm Memory Partitioning and Monitoring'           , 0
DB  'MSDM' , 'Microsoft Data Management'                        , 0
DB  'MSCT' , 'Maximum System Characteristics'                   , 0
DB  'MPST' , 'Memory Power State'                               , 0
DB  'NFIT' , 'NVDIMM Firmware Interface'                        , 0
DB  'OEMx' , 'OEM Specific Information'                         , 0
DB  'PHAT' , 'Platform Health Assessment'                       , 0
DB  'PCCT' , 'Platform Communications Channel'                  , 0
DB  'PMTT' , 'Platform Memory Topology'                         , 0
DB  'PSDT' , 'Persistent System Description'                    , 0
DB  'PRMT' , 'Platform Runtime Mechanism Table'                 , 0
DB  'RASF' , 'ACPI RAS Feature'                                 , 0
DB  'RGRT' , 'Regulatory Graphics Resource Table'               , 0
DB  'RSDT' , 'Root System Description'                          , 0
DB  'SBST' , 'Smart Battery Specification'                      , 0
DB  'SDEI' , 'Software Delegated Exceptions Interface'          , 0
DB  'SDEV' , 'Secure Devices Table'                             , 0
DB  'SLIC' , 'Microsoft Software Licensing'                     , 0
DB  'SLIT' , 'System Locality Distance Information'             , 0
DB  'SRAT' , 'Static/System Resource Affinity'                  , 0
DB  'SSDT' , 'Secondary System Description'                     , 0
DB  'SPCR' , 'Serial Port Console Redirection'                  , 0
DB  'SPMI' , 'Server Platform Management Interface'             , 0
DB  'STAO' , '_STA Override'                                    , 0
DB  'SVKL' , 'Storage Volume Key Data'                          , 0
DB  'TCPA' , 'Trusted Computing Platform Alliance Capabilities' , 0
DB  'TPM2' , 'Trusted Platform Module 2'                        , 0
DB  'UEFI' , 'Unified Extensible Firmware Interface'            , 0
DB  'WAET' , 'Windows ACPI Emulated Devices'                    , 0
DB  'WDAT' , 'Watch Dog Action Table'                           , 0
DB  'WDRT' , 'Watch Dog Resource Table'                         , 0
DB  'WPBT' , 'Windows Platform Binary'                          , 0
DB  'WSMT' , 'Windows Security Mitigations'                     , 0
DB  'XENV' , 'Xen Project'                                      , 0
DB  'XSDT' , 'Extended System Description'                      , 0
DB  0
endres  
;---------- Raw resource for dynamical imported functions list ----------------;
resdata importList
DB  'IsWow64Process'               , 0      ; This functions from KERNEL32.DLL
DB  'GlobalMemoryStatusEx'         , 0          
DB  'GetNativeSystemInfo'          , 0
DB  'GetLogicalProcessorInformation'   , 0
DB  'GetLogicalProcessorInformationEx' , 0
DB  'GetActiveProcessorGroupCount' , 0    
DB  'GetActiveProcessorCount'      , 0              
DB  'GetLargePageMinimum'          , 0
DB  'GetNumaHighestNodeNumber'     , 0
DB  'GetNumaNodeProcessorMask'     , 0
DB  'GetNumaAvailableMemoryNode'   , 0
DB  'GetNumaNodeProcessorMaskEx'   , 0
DB  'GetNumaAvailableMemoryNodeEx' , 0
DB  'EnumSystemFirmwareTables'     , 0
DB  'GetSystemFirmwareTable'       , 0      
DB  'SetThreadAffinityMask'        , 0
DB  'SetThreadGroupAffinity'       , 0
DB  'VirtualAllocExNuma'           , 0 , 0  ; Two zeroes means end of sub-list
DB  'OpenProcessToken'             , 0      ; This functions from ADVAPI32.DLL              
DB  'AdjustTokenPrivileges'        , 0 , 0  ; Two zeroes means end of sub-list
DB  0                                       ; Third zero means end of list                  
endres
;---------- Raw resource for dynamical created fonts list ---------------------;
resdata fontList
; parameters sequence:
; cHeight, cWidth, cWeight, iCharset, iOutPrecision,
; iClipPrecision, iQuality, iPitchAndFamily  
DW  17 , 10 , FW_DONTCARE , DEFAULT_CHARSET
DW  OUT_TT_ONLY_PRECIS  , CLIP_DEFAULT_PRECIS , CLEARTYPE_QUALITY , FIXED_PITCH
DB  'Verdana' , 0
DW  16 , 40 , FW_DONTCARE , DEFAULT_CHARSET
DW  OUT_TT_ONLY_PRECIS  , CLIP_DEFAULT_PRECIS , CLEARTYPE_QUALITY , FIXED_PITCH
DB  'System monospace' , 0
DW  0
endres
;---------- Raw resource for color brushes used in the drawings window --------;
resdata brushesList
; Brush color values = 00bbggrrh, bb=blue, gg=green, rr=red, 1 byte per color
; Used for graphics window, drawings Y=F(X)
DD  BRUSH_GRID         ; Grid with horizontal and vertical lines 
DD  BRUSH_LINE         ; Draw Line Speed = F (Block Size)
DD  BRUSH_BACKGROUND   ; Draw window background
DD  BRUSH_STATISTICS   ; Statistics table lines
endres
;---------- Raw resource for bitmap used in the drawings window ---------------;
resdata bitmapInfo
; BITMAPINFOHEADER descriptor structure ( for Win32/Win64 )
; This structure used when initializing graphics window context, drawings Y=F(X)
;--------------------------------------------------------------
; Name		          Size    Comments
;--------------------------------------------------------------
; biSize           dd ?   Number of bytes
; biWidth          dd ?   Width
; biHeight         dd ?   Height
; biPlanes         dw ?   Bit Planes
; biBitCount       dw ?   Bits Per Pixel
; biCompression    dd ?   Compression Mode
; biSizeImage      dd ?   Image size, not required for uncompressed
; biXPelsPerMeter  dd ?   X pixels per meter, here not required
; biYPelsPerMeter  dd ?   Y pixels per meter, here not required
; biClrUsed        dd ?   Number of colors used, here not required (true color)
; biClrImportant   dd ?   Number of important colors, 0=All, here not used
;                         Important colors - used for visualization.
;                         0 means all colors used for visualization.
;--------------------------------------------------------------
BITMAPINFOHEADER 40,SUBWINX,SUBWINY,1,32,BI_RGB,0,0,0,0,0
endres
;---------- Directory of icon resources ---------------------------------------; 
resource icons, \
IDI_SYSINFO     , LANG_NEUTRAL , iSysinfo    , \
IDI_MEMORY      , LANG_NEUTRAL , iMemory     , \
IDI_OS          , LANG_NEUTRAL , iOs         , \
IDI_NATIVE_OS   , LANG_NEUTRAL , iNativeOs   , \
IDI_TOPOLOGY    , LANG_NEUTRAL , iTopology   , \
IDI_TOPOLOGY_EX , LANG_NEUTRAL , iTopologyEx , \
IDI_NUMA        , LANG_NEUTRAL , iNuma       , \
IDI_P_GROUPS    , LANG_NEUTRAL , iPgroups    , \
IDI_ACPI        , LANG_NEUTRAL , iAcpi       , \
IDI_AFF_CPUID   , LANG_NEUTRAL , iAffCpuid   ; , \
;---------- Directory of group icon resources ---------------------------------;
resource gicons, \
IDG_SYSINFO     , LANG_NEUTRAL , gSysinfo    , \
IDG_MEMORY      , LANG_NEUTRAL , gMemory     , \
IDG_OS          , LANG_NEUTRAL , gOs         , \
IDG_NATIVE_OS   , LANG_NEUTRAL , gNativeOs   , \
IDG_TOPOLOGY    , LANG_NEUTRAL , gTopology   , \
IDG_TOPOLOGY_EX , LANG_NEUTRAL , gTopologyEx , \
IDG_NUMA        , LANG_NEUTRAL , gNuma       , \
IDG_P_GROUPS    , LANG_NEUTRAL , gPgroups    , \
IDG_ACPI        , LANG_NEUTRAL , gAcpi       , \
IDG_AFF_CPUID   , LANG_NEUTRAL , gAffCpuid   ; , \
;---------- Icon resources ----------------------------------------------------;
icon iSysinfo    , gSysinfo    , 'images\sysinfo.ico'
icon iMemory     , gMemory     , 'images\memory.ico'
icon iOs         , gOs         , 'images\os.ico'
icon iNativeOs   , gNativeOs   , 'images\nativeos.ico'
icon iTopology   , gTopology   , 'images\topology.ico'
icon iTopologyEx , gTopologyEx , 'images\topologyex.ico'
icon iNuma       , gNuma       , 'images\numa.ico'
icon iPgroups    , gPgroups    , 'images\pgroups.ico'
icon iAcpi       , gAcpi       , 'images\acpi.ico'
icon iAffCpuid   , gAffCpuid   , 'images\affcpuid.ico'
;---------- Version resources -------------------------------------------------;
resource     version, 1, LANG_NEUTRAL, version_info
versioninfo  version_info, \ 
             VOS__WINDOWS32, VFT_DLL, VFT2_UNKNOWN, LANG_NEUTRAL, 0, \
'FileDescription' , RESOURCE_DESCRIPTION ,\
'FileVersion'     , RESOURCE_VERSION     ,\
'CompanyName'     , RESOURCE_COMPANY     ,\
'LegalCopyright'  , RESOURCE_COPYRIGHT

