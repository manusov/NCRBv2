;------------------------------------------------------------------------------;
;                                                                              ;
;               Global definitions for all components of project.              ;
;              declare objects in the resources library DATA.DLL.              ;
;                         Both for ia32 and x64 code.                          ;
;                                                                              ;
;------------------------------------------------------------------------------;

;---------- Identifiers for application main window and tabs ------------------; 
IDD_MAIN                = 100
IDD_SYSINFO             = 101
IDD_MEMORY              = 102
IDD_OS                  = 103
IDD_NATIVE_OS           = 104
IDD_TOPOLOGY            = 105
IDD_TOPOLOGY_EX         = 106
IDD_NUMA                = 107
IDD_PGROUPS             = 108
IDD_SMBIOS              = 109
IDD_ACPI                = 110
IDD_AFF_CPUID           = 111
;---------- Identifiers for child windows -------------------------------------;
IDD_CHILD_MEMORY_RUN    = 120
IDD_CHILD_MEMORY_DRAW   = 121
IDD_CHILD_VECTOR_BRIEF  = 122
IDD_CHILD_ABOUT         = 123
IDD_CHILD_WAIT          = 124
;---------- Identifiers for application main menu and menu items --------------; 
IDR_MENU                = 130
IDM_SAVE_REPORT         = 131 
IDM_LOAD_REPORT         = 132
IDM_SAVE_IMAGE          = 133
IDM_ABOUT               = 134
IDM_EXIT                = 135
;---------- Identifiers for raw resources: strings and binder scripts ---------;
; Note. Strings represented as raw resources (not as UNICODE string resources)
; for compact encoding: 1 byte per char.  
IDS_STRINGS_POOL        = 140
IDS_BINDERS_POOL        = 141
IDS_CPU_COMMON_POOL     = 142
IDS_CPU_AVX512_POOL     = 143
IDS_OS_CONTEXT_POOL     = 144
IDS_INTEL_CACHE         = 145
IDS_SMBIOS_DATA_POOL    = 146
IDS_ACPI_DATA_POOL      = 147
IDS_IMPORT_POOL         = 148
IDS_FONTS_POOL          = 149
IDS_BRUSHES_POOL        = 150
IDS_PENS_POOL           = 151
IDS_BITMAP_INFO         = 152
IDS_REPORT_INFO         = 153
IDS_CONFIG_INFO         = 154
;---------- Identifiers for icons ---------------------------------------------;
IDI_SYSINFO             = 160    ; First tab sheet icon
IDI_MEMORY              = 161
IDI_OS                  = 162
IDI_NATIVE_OS           = 163
IDI_TOPOLOGY            = 164
IDI_TOPOLOGY_EX         = 165
IDI_NUMA                = 166
IDI_P_GROUPS            = 167
IDI_SMBIOS              = 168
IDI_ACPI                = 169
IDI_AFF_CPUID           = 170    ; Last tab sheet icon
IDI_ABOUT_BOX           = 171    ; Picture for "About" box icon
IDI_WAIT_BOX            = 172    ; Picture for "Wait" box icon 
;---------- Identifiers for group icons ---------------------------------------;
IDG_SYSINFO             = 180    ; First tab sheet group icon
IDG_MEMORY              = 181
IDG_OS                  = 182
IDG_NATIVE_OS           = 183
IDG_TOPOLOGY            = 184
IDG_TOPOLOGY_EX         = 185
IDG_NUMA                = 186
IDG_P_GROUPS            = 187
IDG_SMBIOS              = 188
IDG_ACPI                = 189
IDG_AFF_CPUID           = 190    ; Last tab sheet group icon
IDG_ABOUT_BOX           = 191    ; Picture for "About" box group icon
IDG_WAIT_BOX            = 192    ; Picture for "Wait" box group icon 
;---------- Initial values and counters for build GUI objects -----------------; 
ICON_FIRST              = IDG_SYSINFO
IDD_FIRST               = IDD_SYSINFO
ITEM_COUNT              = 11
ICON_COUNT              = ITEM_COUNT
BIG_ICON_COUNT          = 2
MEMORY_ICON_NUMBER      = 1
VBRF_ICON_NUMBER        = 0
;---------- Identifier for tabbed sheet component -----------------------------;
IDC_TAB                 = 200
;---------- Identifiers for tab #1 = System information sheet -----------------;
IDC_SYSINFO             = 300 
IDC_SYSINFO_VENDOR      = 301
IDC_SYSINFO_TFMS        = 302
IDC_SYSINFO_NAME        = 303
IDC_SYSINFO_TSC         = 304
IDC_SYSINFO_CPUID       = 305
IDC_SYSINFO_MMX         = 306
IDC_SYSINFO_SSE         = 307
IDC_SYSINFO_SSE2        = 308
IDC_SYSINFO_SSE3        = 309
IDC_SYSINFO_SSSE3       = 310
IDC_SYSINFO_SSE41       = 311
IDC_SYSINFO_SSE42       = 312
IDC_SYSINFO_AVX         = 313
IDC_SYSINFO_AVX2        = 314
IDC_SYSINFO_AVX512F     = 315
IDC_SYSINFO_RDRAND      = 316
IDC_SYSINFO_VMX_SVM     = 317
IDC_SYSINFO_X8664       = 318
IDC_SYSINFO_A0          = 319
IDC_SYSINFO_A1          = 320
IDC_SYSINFO_A2          = 321
IDC_SYSINFO_A3          = 322
IDC_SYSINFO_A4          = 323
IDC_SYSINFO_A5          = 324
IDC_SYSINFO_B0          = 325
IDC_SYSINFO_B1          = 326
IDC_SYSINFO_B2          = 327
IDC_SYSINFO_B3          = 328
IDC_SYSINFO_B4          = 329
IDC_SYSINFO_B5          = 330
IDC_SYSINFO_C0          = 331
IDC_SYSINFO_C1          = 332
IDC_SYSINFO_C2          = 333
IDC_SYSINFO_C3          = 334
IDC_SYSINFO_D0          = 335
IDC_SYSINFO_D1          = 336
IDC_SYSINFO_D2          = 337
IDC_SYSINFO_D3          = 338
IDC_SYSINFO_XCR0        = 339 
IDC_SYSINFO_XMM015      = 340
IDC_SYSINFO_YMM015      = 341
IDC_SYSINFO_ZMM015      = 342
IDC_SYSINFO_ZMM1631     = 343
IDC_SYSINFO_K07         = 344
IDC_SYSINFO_BNDREG      = 345
IDC_SYSINFO_BNDCSR      = 346
IDC_SYSINFO_ACPI        = 347
IDC_SYSINFO_MADT        = 348
IDC_SYSINFO_MADT_1      = 349
IDC_SYSINFO_MADT_2      = 350
IDC_SYSINFO_MADT_3      = 351
IDC_SYSINFO_MADT_4      = 352
IDC_SYSINFO_SRAT        = 353
IDC_SYSINFO_SRAT_1      = 354
IDC_SYSINFO_SRAT_2      = 355
IDC_SYSINFO_SRAT_3      = 356
IDC_SYSINFO_SRAT_4      = 357
IDC_SYSINFO_L1C         = 358
IDC_SYSINFO_L1C_V       = 359
IDC_SYSINFO_L1D         = 360
IDC_SYSINFO_L1D_V       = 361
IDC_SYSINFO_L2U         = 362
IDC_SYSINFO_L2U_V       = 363
IDC_SYSINFO_L3U         = 364
IDC_SYSINFO_L3U_V       = 365
IDC_SYSINFO_L4U         = 366
IDC_SYSINFO_L4U_V       = 367
IDC_SYSINFO_THREADS     = 368
IDC_SYSINFO_THR_V       = 369
IDC_SYSINFO_CORES       = 370
IDC_SYSINFO_CORES_V     = 371
IDC_SYSINFO_SOCKETS     = 372
IDC_SYSINFO_SOCK_V      = 373

IDC_SYSINFO_PTOT        = 374
IDC_SYSINFO_PTOT_V      = 375
IDC_SYSINFO_GRP         = 376
IDC_SYSINFO_GRP_V       = 377
IDC_SYSINFO_PCUR        = 378
IDC_SYSINFO_PCUR_V      = 379
IDC_SYSINFO_NUMA        = 380
IDC_SYSINFO_NUMA_V      = 381

IDC_SYSINFO_PCORES      = 382
IDC_SYSINFO_PCOR_V      = 383
IDC_SYSINFO_ECORES      = 384
IDC_SYSINFO_ECOR_V      = 385


IDC_SYSINFO_NUMA_M      = 386
IDC_SYSINFO_MEM         = 387
IDC_SYSINFO_MEM_V       = 388 
IDC_SYSINFO_MEM_A       = 389 
IDC_SYSINFO_MEM_AV      = 390 
IDC_SYSINFO_LRPG        = 391 
IDC_SYSINFO_LRPG_V      = 392 
IDC_SYSINFO_LRPG_E      = 393
IDB_SYSINFO_VBRF        = 394 
IDB_SYSINFO_CANCEL      = 395
;---------- Identifiers for tab #2 = Memory and cache benchmarks sheet --------;
IDC_MEMORY              = 500
IDC_MEMORY_FRAME_1      = 501
IDC_MEMORY_FRAME_2      = 502
IDC_MEMORY_FRAME_3      = 503
IDC_MEMORY_FRAME_4      = 504
IDC_MEMORY_FRAME_5      = 505
IDC_MEMORY_FRAME_6      = 506
IDC_MEMORY_FRAME_7      = 507
IDB_MEMORY_ASM_A0       = 508
IDB_MEMORY_ASM_A1       = 509
IDB_MEMORY_ASM_A2       = 510
IDB_MEMORY_ASM_A3       = 511
IDB_MEMORY_ASM_A4       = 512
IDB_MEMORY_ASM_A5       = 513
IDB_MEMORY_ASM_A6       = 514
IDB_MEMORY_ASM_A7       = 515
IDB_MEMORY_ASM_A8       = 516
IDB_MEMORY_ASM_B0       = 517 
IDB_MEMORY_ASM_B1       = 518
IDB_MEMORY_ASM_B2       = 519
IDB_MEMORY_ASM_B3       = 520
IDB_MEMORY_ASM_B4       = 521
IDB_MEMORY_ASM_B5       = 522
IDB_MEMORY_ASM_B6       = 523
IDB_MEMORY_ASM_B7       = 524
IDB_MEMORY_ASM_B8       = 525
IDB_MEMORY_ASM_C0       = 526 
IDB_MEMORY_ASM_C1       = 527
IDB_MEMORY_ASM_C2       = 528
IDB_MEMORY_ASM_C3       = 529
IDB_MEMORY_ASM_C4       = 530
IDB_MEMORY_ASM_C5       = 531
IDB_MEMORY_ASM_C6       = 532
IDB_MEMORY_ASM_C7       = 533
IDB_MEMORY_NONTEMP      = 534
IDB_MEMORY_FORCE32      = 535
IDB_MEMORY_L1           = 536
IDB_MEMORY_L2           = 537
IDB_MEMORY_L3           = 538
IDB_MEMORY_L4           = 539
IDB_MEMORY_DRAM         = 540
IDB_MEMORY_CUSTOM       = 541
IDB_MEMORY_FILE         = 542
IDB_MEMORY_GPU          = 543
IDB_MEMORY_PHYSICAL     = 544
IDE_MEMORY_B_SIZE       = 545
IDE_MEMORY_F_SIZE       = 546
IDE_MEMORY_G_SIZE       = 547
IDE_MEMORY_M_START      = 548
IDE_MEMORY_M_STOP       = 549
IDC_MEMORY_M_HYPHEN     = 550
IDC_MEMORY_COMBO_F      = 551
IDC_MEMORY_COMBO_F      = 552
IDB_HYBRID_UNAWARE      = 553 
IDB_HYBRID_P_PLUS_E     = 554
IDB_HYBRID_PERF         = 555
IDB_HYBRID_EFF          = 556
IDB_MEMORY_PARALLEL     = 557
IDB_MEMORY_HT           = 558
IDB_MEMORY_PG           = 559
IDB_MEMORY_NUMA_U       = 560
IDB_MEMORY_NUMA_S       = 561
IDB_MEMORY_NUMA_O       = 562
IDB_MEMORY_NUMA_N       = 563
IDB_MEMORY_NORMAL       = 564
IDB_MEMORY_SK_63        = 565
IDB_MEMORY_SK_4095      = 566
IDB_MEMORY_SK_CSTM      = 567
IDE_MEMORY_SK_SIZE      = 568
IDB_MEMORY_LP           = 569
IDB_MEMORY_NO_PF        = 570
IDB_MEMORY_DEF_PF       = 571
IDB_MEMORY_MED_PF       = 572
IDB_MEMORY_LNG_PF       = 573
IDB_MEMORY_BLK_PF       = 574
IDB_MEMORY_CST_PF       = 575
IDE_MEMORY_PF_SIZE      = 576
IDB_MEMORY_BRF          = 577  
IDB_MEMORY_CRF          = 578 
IDB_MEMORY_BRF_A        = 579 
IDB_MEMORY_CRF_A        = 580 
IDB_MEMORY_ALL_P        = 581
IDB_MEMORY_X_16         = 582 
IDB_MEMORY_X_32         = 583 
IDB_MEMORY_SILENT       = 584
IDB_MEMORY_DRAW         = 585
IDB_MEMORY_RUN          = 586
IDB_MEMORY_DEFAULTS     = 587
IDB_MEMORY_CANCEL       = 588
;---------- Identifiers for tab #3 = Operating system sheet -------------------; 
IDC_OS                  = 900
IDE_OS_UP               = 901
IDE_OS_TEXT             = 902
IDB_OS_REPORT           = 903
IDB_OS_CANCEL           = 904
;---------- Identifiers for tab #4 = Native os ( for WoW64 ) sheet ------------;
IDC_NATIVE_OS           = 1100
IDE_NATIVE_OS_UP        = 1101
IDE_NATIVE_OS_TEXT      = 1102
IDB_NAT_OS_REPORT       = 1103
IDB_NAT_OS_CANCEL       = 1104
;---------- Identifiers for tab #5 = Topology information sheet ---------------; 
IDC_TOPOLOGY            = 1500
IDE_TOPOL_UP_1          = 1501
IDE_TOPOL_TEXT_1        = 1502
IDE_TOPOL_UP_2          = 1503
IDE_TOPOL_TEXT_2        = 1504
IDB_TOPOL_REPORT        = 1505
IDB_TOPOL_CANCEL        = 1506
;---------- Identifiers for tab #6 = Extended topology information sheet ------;
IDC_TOPOLOGY_EX         = 1700
IDE_TOP_EX_UP_1         = 1701
IDE_TOP_EX_TEXT_1       = 1702
IDE_TOP_EX_UP_2         = 1703
IDE_TOP_EX_TEXT_2       = 1704
IDB_TOPOL_EX_REPORT     = 1705
IDB_TOPOL_EX_CANCEL     = 1706
;---------- Identifiers for tab #7 = NUMA domains sheet -----------------------; 
IDC_NUMA                = 1900
IDE_NUMA_UP             = 1901
IDE_NUMA_TEXT           = 1902
IDB_NUMA_REPORT         = 1903
IDB_NUMA_CANCEL         = 1904
;---------- Identifiers for tab #8 = Processor groups sheet -------------------;
IDC_P_GROUPS            = 2100
IDE_P_GROUPS_UP         = 2101
IDE_P_GROUPS_TEXT       = 2102
IDB_P_GROUPS_REPORT     = 2103
IDB_P_GROUPS_CANCEL     = 2104
;---------- Identifiers for tab #9 = SMBIOS information sheet -----------------;
IDC_SMBIOS              = 2300
IDE_SMBIOS_UP           = 2301
IDE_SMBIOS_TEXT         = 2302
IDB_SMBIOS_REPORT       = 2303
IDB_SMBIOS_CANCEL       = 2304
;---------- Identifiers for tab #10 = ACPI information sheet ------------------;  
IDC_ACPI                = 2500
IDE_ACPI_UP_1           = 2501
IDE_ACPI_TEXT_1         = 2502
IDE_ACPI_UP_2           = 2503
IDE_ACPI_TEXT_2         = 2504
IDB_ACPI_REPORT         = 2505
IDB_ACPI_CANCEL         = 2506
;---------- Identifiers for tab #11 = Affinized CPUID dump sheet --------------; 
IDC_AFF_CPUID           = 2700
IDE_A_CPUID_UP          = 2701
IDE_A_CPUID_TEXT        = 2702
IDB_A_CPUID_REPORT      = 2703
IDB_A_CPUID_CANCEL      = 2704
;--- Identifiers for child screen = Memory and cache performance report -------;
IDC_MR_FIRST            = 2900 
IDC_MR_APPLICATION      = 2901  
IDC_MR_METHOD           = 2902 
IDC_MR_WIDTH            = 2903 
IDC_MR_THREADS          = 2904 
IDC_MR_HYPER_THR        = 2905 
IDC_MR_LARGE_PAGES      = 2906 
IDC_MR_NUMA             = 2907 
IDC_MR_P_GROUPS         = 2908
IDC_MR_HYBRID           = 2909 
IDC_MR_TARGET_OBJ       = 2910 
IDC_MR_PREF_DIST        = 2911 
IDC_MR_SIZE_TOTAL       = 2912 
IDC_MR_SIZE_PER_THR     = 2913 
IDC_MR_MEASURE_PROF     = 2914 
IDC_MR_MEASURE_REP      = 2915 
IDC_MR_MEMORY_ALLOC     = 2916 
IDC_MR_BLOCK_1          = 2917 
IDC_MR_BLOCK_2          = 2918 
IDC_MR_MEM_ALC_ALL      = 2919 
IDC_MR_MEM_ALC_THR      = 2920 
IDC_MR_MEAS_RESULTS     = 2921 
IDC_MR_DT_MS            = 2922 
IDC_MR_DTSC_SEC_MHZ     = 2923 
IDC_MR_DTSC_INS_CLK     = 2924 
IDC_MR_LAST             = 2925
IDC_MR_APPLIC_V         = 2926  
IDC_MR_METHOD_V         = 2927 
IDC_MR_WIDTH_V          = 2928 
IDC_MR_THREADS_V        = 2929 
IDC_MR_HYPER_THR_V      = 2930 
IDC_MR_LARGE_PAG_V      = 2931 
IDC_MR_NUMA_V           = 2932 
IDC_MR_P_GROUPS_V       = 2933
IDC_MR_HYBRID_V         = 2934 
IDC_MR_TARGET_OBJ_V     = 2935 
IDC_MR_PREF_DIST_V      = 2936
IDC_MR_SIZE_TOTAL_V     = 2937 
IDC_MR_SIZE_PER_T_V     = 2938 
IDC_MR_MEASURE_P_V      = 2939 
IDC_MR_MEASURE_R_V      = 2940 
IDC_MR_BLOCK_1_V        = 2941 
IDC_MR_BLOCK_2_V        = 2942 
IDC_MR_MEM_ALC_A_V      = 2943 
IDC_MR_MEM_ALC_T_V      = 2944 
IDC_MR_DT_MS_V          = 2945 
IDC_MR_DTSC_SEC_M_V     = 2946 
IDC_MR_DTSC_INS_C_V     = 2947 
IDC_MR_LAST_V           = 2948
IDB_MR_OK               = 2949
;--- Identifiers for child screen = Memory and cache performance draw ---------;
IDC_MD_ASM              = 3100
IDC_MD_PREFETCH         = 3101
IDC_MD_OBJECT           = 3102
IDC_MD_THREADS          = 3103
IDC_MD_REPEATS          = 3104
IDC_MD_PAGES            = 3105
IDC_MD_NUMA             = 3106
IDB_MD_REFRESH          = 3107
IDB_MD_CANCEL           = 3108
;--- Identifiers for child screen = Vector brief performance report -----------;
IDC_VB_CPU_NAME         = 3300
IDC_VB_TSC_CLK          = 3301
IDC_VB_CPU_FEATURES     = 3302  
IDC_VB_AVX_256          = 3303 
IDC_VB_AVX2_256         = 3304 
IDC_VB_AVX3_512         = 3305 
IDC_VB_AVX512CD         = 3306 
IDC_VB_AVX512PF         = 3307 
IDC_VB_AVX512ER         = 3308 
IDC_VB_AVX512VL         = 3309 
IDC_VB_AVX512BW         = 3310
IDC_VB_AVX512DQ         = 3311 
IDC_VB_AVX512_IFMA      = 3312 
IDC_VB_AVX512_VBMI      = 3313 
IDC_VB_AVX512_VBMI2     = 3314 
IDC_VB_AVX512_BF16      = 3315 
IDC_VB_AVX512_VAES      = 3316 
IDC_VB_AVX512_GFNI      = 3317 
IDC_VB_AVX512_VNNI      = 3318 
IDC_VB_AVX512_BTALG     = 3319 
IDC_VB_AVX512_VPOP      = 3320 
IDC_VB_AVX512_VPCL      = 3321 
IDC_VB_AVX512_VP2IN     = 3322 
IDC_VB_AVX512_FP16      = 3323 
IDC_VB_AVX512_4FMAP     = 3324 
IDC_VB_AVX512_4VNNI     = 3325 
IDC_VB_OS_CONTEXT       = 3326 
IDC_VB_SSE128_XMM       = 3327 
IDC_VB_AVX256_YMM       = 3328 
IDC_VB_AVX512_ZMM_L     = 3329 
IDC_VB_AVX512_ZMM_H     = 3330 
IDC_VB_AVX512_K         = 3331 
IDC_VB_TIMINGS          = 3332 
IDC_VB_SSE128_READ      = 3333              
IDC_VB_SSE128_WRITE     = 3334              
IDC_VB_SSE128_COPY      = 3335              
IDC_VB_AVX256_READ      = 3336              
IDC_VB_AVX256_WRITE     = 3337 
IDC_VB_AVX256_COPY      = 3338              
IDC_VB_AVX512_READ      = 3339 
IDC_VB_AVX512_WRITE     = 3340              
IDC_VB_AVX512_COPY      = 3341              
IDC_VB_SQRTPD_XMM       = 3342              
IDC_VB_VSQRTPD_YMM      = 3343              
IDC_VB_VSQRTPD_ZMM      = 3344 
IDC_VB_FCOS             = 3345              
IDC_VB_FSINCOS          = 3346
IDC_VB_READ128_V        = 3347              
IDC_VB_WRITE128_V       = 3348               
IDC_VB_COPY128_V        = 3349               
IDC_VB_READ256_V        = 3350               
IDC_VB_WRITE256_V       = 3351  
IDC_VB_COPY256_V        = 3352               
IDC_VB_READ512_V        = 3353  
IDC_VB_WRITE512_V       = 3354               
IDC_VB_COPY512_V        = 3355               
IDC_VB_SQRT128_V        = 3356               
IDC_VB_SQRT256_V        = 3357               
IDC_VB_SQRT512_V        = 3358  
IDC_VB_FCOS_V           = 3359               
IDC_VB_FSINCOS_V        = 3360 
IDB_VB_OK               = 3361
;--- Identifiers for child screen = "About" box -------------------------------;
IDB_ABOUT_OK            = 3500               
;---------- Strings numbers in the raw strings pool ---------------------------;
; Note. Strings enumerated by zero-terminations.
;---------- Strings numbers for application sheets brief names ----------------;
STR_BRIEF_SYSINFO      = 0   
STR_BRIEF_MEMORY       = 1
STR_BRIEF_OS           = 2
STR_BRIEF_NATIVE_OS    = 3
STR_BRIEF_TOPOLOGY     = 4
STR_BRIEF_TOPOLOGY_EX  = 5
STR_BRIEF_NUMA         = 6
STR_BRIEF_P_GROUPS     = 7
STR_BRIEF_SMBIOS       = 8
STR_BRIEF_ACPI         = 9
STR_BRIEF_AFF_CPUID    = 10
;---------- Strings numbers for application sheets full names -----------------;
STR_FULL_SYSINFO       = 11   
STR_FULL_MEMORY        = 12
STR_FULL_OS            = 13
STR_FULL_NATIVE_OS     = 14
STR_FULL_TOPOLOGY      = 15
STR_FULL_TOPOLOGY_EX   = 16
STR_FULL_NUMA          = 17
STR_FULL_P_GROUPS      = 18
STR_FULL_SMBIOS        = 19
STR_FULL_ACPI          = 20
STR_FULL_AFF_CPUID     = 21
;---------- Title names for child windows -------------------------------------;
STR_CHILD_MEM_REPORT   = 22
STR_CHILD_MEM_DRAW     = 23
STR_CHILD_VECTOR_BRIEF = 24
;---------- CPUID names for system information --------------------------------;
STR_CPUID              = STR_CHILD_VECTOR_BRIEF + 1
STR_MMX                = STR_CPUID + 1
STR_SSE                = STR_CPUID + 2
STR_SSE2               = STR_CPUID + 3
STR_SSE3               = STR_CPUID + 4
STR_SSSE3              = STR_CPUID + 5
STR_SSE41              = STR_CPUID + 6
STR_SSE42              = STR_CPUID + 7
STR_AVX                = STR_CPUID + 8
STR_AVX2               = STR_CPUID + 9
STR_AVX512F            = STR_CPUID + 10
STR_RDRAND             = STR_CPUID + 11
STR_VMX                = STR_CPUID + 12
STR_SVM                = STR_CPUID + 13
STR_X8664              = STR_CPUID + 14
;---------- CPUID names for system information, AVX512 sub-sets ---------------;
STR_AVX512CD           = STR_X8664 + 1 
STR_AVX512PF           = STR_AVX512CD + 1
STR_AVX512ER           = STR_AVX512CD + 2
STR_AVX512VL           = STR_AVX512CD + 3
STR_AVX512BW           = STR_AVX512CD + 4
STR_AVX512DQ           = STR_AVX512CD + 5
STR_AVX512_IFMA        = STR_AVX512CD + 6
STR_AVX512_VBMI        = STR_AVX512CD + 7
STR_AVX512_VBMI2       = STR_AVX512CD + 8
STR_AVX512_BF16        = STR_AVX512CD + 9
STR_AVX512_VAES        = STR_AVX512CD + 10
STR_AVX512_GFNI        = STR_AVX512CD + 11
STR_AVX512_VNNI        = STR_AVX512CD + 12
STR_AVX512_BITALG      = STR_AVX512CD + 13
STR_AVX512_VPOPCNTDQ   = STR_AVX512CD + 14
STR_AVX512_VPCLMULQ    = STR_AVX512CD + 15
STR_AVX512_VP2INTERS   = STR_AVX512CD + 16
STR_AVX512_FP16        = STR_AVX512CD + 17 
STR_AVX512_4FMAPS      = STR_AVX512CD + 18
STR_AVX512_4VNNIW      = STR_AVX512CD + 19
;---------- XCR0 and XGETBV context components names --------------------------;
STR_XCR0               = STR_AVX512_4VNNIW + 1 
STR_XMM_0_15           = STR_XCR0 + 1 
STR_YMM_0_15           = STR_XCR0 + 2
STR_ZMM_0_15           = STR_XCR0 + 3
STR_ZMM_16_31          = STR_XCR0 + 4
STR_K_0_7              = STR_XCR0 + 5
STR_BNDREG             = STR_XCR0 + 6
STR_BNDCSR             = STR_XCR0 + 7
;---------- ACPI objects visualized at system information screen --------------;
STR_ACPI               = STR_BNDCSR + 1
STR_MADT               = STR_ACPI + 1
STR_SRAT               = STR_ACPI + 2
STR_OEM_REV            = STR_ACPI + 3
STR_LOCAL_APICS        = STR_ACPI + 4
STR_IO_APICS           = STR_ACPI + 5
STR_DOMAINS            = STR_ACPI + 6
STR_CPUS               = STR_ACPI + 7
STR_RAMS               = STR_ACPI + 8
;---------- Cache memory ------------------------------------------------------;
STR_L1_CODE            = STR_RAMS + 1
STR_L1_DATA            = STR_L1_CODE + 1
STR_L2_UNIFIED         = STR_L1_CODE + 2
STR_L3_UNIFIED         = STR_L1_CODE + 3
STR_L4_UNIFIED         = STR_L1_CODE + 4
STR_TRACE              = STR_L1_CODE + 5              
STR_KUOPS              = STR_L1_CODE + 6
;---------- Platform topology by WinAPI ---------------------------------------;
STR_THREADS            = STR_KUOPS + 1
STR_CORES              = STR_THREADS + 1
STR_SOCKETS            = STR_THREADS + 2
STR_PROC_TOTAL         = STR_THREADS + 3
STR_GRPS               = STR_THREADS + 4
STR_PROC_CUR           = STR_THREADS + 5
STR_NUMA_NODES         = STR_THREADS + 6
STR_P_CORES            = STR_THREADS + 7
STR_E_CORES            = STR_THREADS + 8
;---------- Memory information by WinAPI --------------------------------------;
STR_OS_PHYSICAL        = STR_E_CORES + 1
STR_OS_AVAILABLE       = STR_OS_PHYSICAL + 1
STR_OS_MIN_LARGE       = STR_OS_PHYSICAL + 2
;---------- Assembler instructions names for memory and cache benchmarks ------;
; This strings also used as sequental pool when method name visual.
; Part 1 of 3, temporal variant.
STR_AM_IA32_00         = STR_OS_MIN_LARGE + 1  
STR_AM_IA32_01         = STR_AM_IA32_00 + 1   
STR_AM_IA32_02         = STR_AM_IA32_00 + 2 
STR_AM_IA32_03         = STR_AM_IA32_00 + 3 
STR_AM_IA32_04         = STR_AM_IA32_00 + 4 
STR_AM_IA32_05         = STR_AM_IA32_00 + 5 
STR_AM_X64_00          = STR_AM_IA32_00 + 6 
STR_AM_X64_01          = STR_AM_IA32_00 + 7  
STR_AM_X64_02          = STR_AM_IA32_00 + 8
STR_AM_X64_03          = STR_AM_IA32_00 + 9
STR_AM_X64_04          = STR_AM_IA32_00 + 10
STR_AM_X64_05          = STR_AM_IA32_00 + 11
STR_AM_06              = STR_AM_IA32_00 + 12
STR_AM_07              = STR_AM_IA32_00 + 13
STR_AM_08              = STR_AM_IA32_00 + 14
STR_AM_09              = STR_AM_IA32_00 + 15
STR_AM_10              = STR_AM_IA32_00 + 16
STR_AM_11              = STR_AM_IA32_00 + 17
STR_AM_12              = STR_AM_IA32_00 + 18
STR_AM_13              = STR_AM_IA32_00 + 19
STR_AM_14              = STR_AM_IA32_00 + 20
STR_AM_15              = STR_AM_IA32_00 + 21
STR_AM_16              = STR_AM_IA32_00 + 22
STR_AM_17              = STR_AM_IA32_00 + 23
STR_AM_18              = STR_AM_IA32_00 + 24
STR_AM_19              = STR_AM_IA32_00 + 25
STR_AM_20              = STR_AM_IA32_00 + 26
STR_AM_21              = STR_AM_IA32_00 + 27
STR_AM_22              = STR_AM_IA32_00 + 28
STR_AM_23              = STR_AM_IA32_00 + 29
STR_AM_LCM             = STR_AM_IA32_00 + 30
STR_AM_RDRAND          = STR_AM_IA32_00 + 31
STR_AM_LCM_32X2        = STR_AM_IA32_00 + 32
STR_AM_RDRAND_32X2     = STR_AM_IA32_00 + 33
;---------- Assembler instructions names for memory and cache benchmarks ------;
; This strings also used as sequental pool when method name visual.
; Part 2 of 3, non-temporal variant.
STR_AM_NT_00           = STR_AM_RDRAND_32X2 + 1  ; # 0
STR_AM_NT_01           = STR_AM_NT_00 + 1
STR_AM_NT_02           = STR_AM_NT_00 + 2
STR_AM_NT_03           = STR_AM_NT_00 + 3
STR_AM_NT_04           = STR_AM_NT_00 + 4
STR_AM_NT_05           = STR_AM_NT_00 + 5
STR_AM_NT_06           = STR_AM_NT_00 + 6
STR_AM_NT_07           = STR_AM_NT_00 + 7
STR_AM_NT_08           = STR_AM_NT_00 + 8
STR_AM_NT_09           = STR_AM_NT_00 + 9
STR_AM_NT_10           = STR_AM_NT_00 + 10
STR_AM_NT_11           = STR_AM_NT_00 + 11
STR_AM_NT_12           = STR_AM_NT_00 + 12
STR_AM_NT_13           = STR_AM_NT_00 + 13
STR_AM_NT_14           = STR_AM_NT_00 + 14       ; # 14
;---------- Assembler instructions names for memory and cache benchmarks ------;
; This strings also used as sequental pool when method name visual.
; Part 3 of 3, non-temporal variant, medium and long prefetch distance
; Note same strings used, 
; prefetch distance comment show as separate additional string
STR_AM_NT_15           = STR_AM_NT_00 + 15       ; # 15
STR_AM_NT_16           = STR_AM_NT_00 + 16
STR_AM_NT_17           = STR_AM_NT_00 + 17
STR_AM_NT_18           = STR_AM_NT_00 + 18
STR_AM_NT_19           = STR_AM_NT_00 + 19
STR_AM_NT_20           = STR_AM_NT_00 + 20       ; # 20       
;---------- Modes names for memory and cache benchmarks -----------------------;
STR_NON_TEMPORAL       = STR_AM_NT_20 + 1
STR_32X2               = STR_NON_TEMPORAL + 1
;---------- Target objects names for memory and cache benchmark ---------------;
STR_MEMORY_L1          = STR_32X2 + 1
STR_MEMORY_L2          = STR_MEMORY_L1 + 1 
STR_MEMORY_L3          = STR_MEMORY_L1 + 2
STR_MEMORY_L4          = STR_MEMORY_L1 + 3
STR_MEMORY_DRAM        = STR_MEMORY_L1 + 4
STR_MEMORY_CUSTOM      = STR_MEMORY_L1 + 5   ; This for Simple run screen
STR_MEMORY_FILE        = STR_MEMORY_L1 + 6
STR_MEMORY_GPU         = STR_MEMORY_L1 + 7
STR_MEMORY_PHYSICAL    = STR_MEMORY_L1 + 8
STR_MEMORY_CUSTOM_DRAW = STR_MEMORY_L1 + 9   ; This for Draw screen 
;---------- Hybrid cores names ------------------------------------------------;
STR_HYBRID_UNAWARE     = STR_MEMORY_CUSTOM_DRAW + 1
STR_HYBRID_P_PLUS_E    = STR_HYBRID_UNAWARE + 1 
STR_HYBRID_PERF        = STR_HYBRID_UNAWARE + 2 
STR_HYBRID_EFF         = STR_HYBRID_UNAWARE + 3 
;---------- Memory access and platform topology options names -----------------;
STR_PARALLEL_THREADS   = STR_HYBRID_EFF + 1 
STR_HYPER_THREADING    = STR_PARALLEL_THREADS + 1 
STR_PROCESSOR_GROUPS   = STR_PARALLEL_THREADS + 2
STR_NUMA_UNAWARE       = STR_PARALLEL_THREADS + 3
STR_NUMA_SINGLE        = STR_PARALLEL_THREADS + 4
STR_NUMA_OPTIMAL       = STR_PARALLEL_THREADS + 5
STR_NUMA_NO_OPTIMAL    = STR_PARALLEL_THREADS + 6
STR_NORMAL_ACCESS      = STR_PARALLEL_THREADS + 7 
STR_ACCESS_64B         = STR_PARALLEL_THREADS + 8 
STR_ACCESS_4096B       = STR_PARALLEL_THREADS + 9 
STR_ACCES_CUSTOM       = STR_PARALLEL_THREADS + 10 
STR_LARGE_PAGES        = STR_PARALLEL_THREADS + 11
;---------- Memory prefetch options names -------------------------------------;
STR_NO_PREFETCH        = STR_LARGE_PAGES + 1
STR_DEFAULT_DISTANCE   = STR_NO_PREFETCH + 1
STR_MEDIUM_DISTANCE    = STR_NO_PREFETCH + 2
STR_LONG_DISTANCE      = STR_NO_PREFETCH + 3
STR_BLOCK_PREFETCH     = STR_NO_PREFETCH + 4
STR_CUSTOM_DISTANCE    = STR_NO_PREFETCH + 5
;---------- Measurement options names -----------------------------------------;
STR_MEASURE_BRIEF      = STR_CUSTOM_DISTANCE + 1  
STR_MEASURE_CAREF      = STR_MEASURE_BRIEF + 1 
STR_BRIEFF_ADAPTIVE    = STR_MEASURE_BRIEF + 2
STR_CAREF_ADAPTIVE     = STR_MEASURE_BRIEF + 3
STR_ALL_POINTS         = STR_MEASURE_BRIEF + 4
STR_X_16_POINTS        = STR_MEASURE_BRIEF + 5
STR_X_32_POINTS        = STR_MEASURE_BRIEF + 6
STR_SILENT_DRAW        = STR_MEASURE_BRIEF + 7 
;---------- Actions buttons names ---------------------------------------------;
STR_VECTOR_BRIEF       = STR_SILENT_DRAW + 1
STR_DRAW               = STR_VECTOR_BRIEF + 1  
STR_RUN                = STR_VECTOR_BRIEF + 2
STR_DEFAULTS           = STR_VECTOR_BRIEF + 3
STR_REPORT             = STR_VECTOR_BRIEF + 4
STR_EXIT               = STR_VECTOR_BRIEF + 5
STR_OK                 = STR_VECTOR_BRIEF + 6
;---------- Memory size and speed units, additional information ---------------;
STR_UNITS_BYTES        = STR_OK + 1   
STR_UNITS_KB           = STR_UNITS_BYTES + 1 
STR_UNITS_MB           = STR_UNITS_BYTES + 2
STR_UNITS_GB           = STR_UNITS_BYTES + 3
STR_UNITS_TB           = STR_UNITS_BYTES + 4
STR_UNITS_MBPS         = STR_UNITS_BYTES + 5
STR_UNITS_NS           = STR_UNITS_BYTES + 6
STR_NONE               = STR_UNITS_BYTES + 7
STR_MASK               = STR_UNITS_BYTES + 8
STR_ENABLED            = STR_UNITS_BYTES + 9
STR_DISABLED           = STR_UNITS_BYTES + 10 
STR_HYPHEN             = STR_UNITS_BYTES + 11 
STR_TSC_CLKS           = STR_UNITS_BYTES + 12
STR_CPU_CLKS           = STR_UNITS_BYTES + 13
STR_DATA_MOVE          = STR_UNITS_BYTES + 14
STR_CALC               = STR_UNITS_BYTES + 15
STR_NS                 = STR_UNITS_BYTES + 16
STR_MHZ                = STR_UNITS_BYTES + 17
STR_KERNEL_MODE        = STR_UNITS_BYTES + 18
STR_TRUE_CLOCK         = STR_UNITS_BYTES + 19
STR_TFMS               = STR_UNITS_BYTES + 20
STR_TSC                = STR_UNITS_BYTES + 21
STR_H                  = STR_UNITS_BYTES + 22
STR_SUPPORTED          = STR_UNITS_BYTES + 23
STR_NOT_SUPPORTED      = STR_UNITS_BYTES + 24 
STR_N_A                = STR_UNITS_BYTES + 25 
STR_NOT_SUPPORTED_CPU  = STR_UNITS_BYTES + 26
STR_NOT_SUPPORTED_OS   = STR_UNITS_BYTES + 27
;---------- Up strings for GUI tables -----------------------------------------;
STR_PARM_VALUE_HEX     = STR_NOT_SUPPORTED_OS + 1
STR_PARM_VALUE         = STR_PARM_VALUE_HEX + 1
STR_TOPOLOGY           = STR_PARM_VALUE_HEX + 2
STR_TOPOLOGY_SUMMARY   = STR_PARM_VALUE_HEX + 3
STR_TOPOLOGY_SUMM_EX   = STR_PARM_VALUE_HEX + 4
STR_NUMA               = STR_PARM_VALUE_HEX + 5 
STR_GROUPS             = STR_PARM_VALUE_HEX + 6 
STR_SMBIOS             = STR_PARM_VALUE_HEX + 7             
STR_ACPI_LIST          = STR_PARM_VALUE_HEX + 8
STR_ACPI_SUMMARY       = STR_PARM_VALUE_HEX + 9
STR_AFF_CPUID          = STR_PARM_VALUE_HEX + 10 
;---------- Strings for operating system information text ---------------------;
STR_OS_MEMORY_LOAD     = STR_AFF_CPUID + 1
STR_OS_TOTAL_PHYSICAL  = STR_OS_MEMORY_LOAD + 1
STR_OS_AVAIL_PHYSICAL  = STR_OS_MEMORY_LOAD + 2 
STR_OS_TOTAL_PAGE_FILE = STR_OS_MEMORY_LOAD + 3
STR_OS_AVAIL_PAGE_FILE = STR_OS_MEMORY_LOAD + 4
STR_OS_TOTAL_VIRTUAL   = STR_OS_MEMORY_LOAD + 5
STR_OS_AVAIL_VIRTUAL   = STR_OS_MEMORY_LOAD + 6
STR_OS_EXT_VIRTUAL     = STR_OS_MEMORY_LOAD + 7
STR_OS_MIN_ADDRESS     = STR_OS_MEMORY_LOAD + 8
STR_OS_MAX_ADDRESS     = STR_OS_MEMORY_LOAD + 9
STR_OS_PROC_MASK       = STR_OS_MEMORY_LOAD + 10
STR_OS_PROC_TYPE       = STR_OS_MEMORY_LOAD + 11
STR_OS_ALLOC_GRAN      = STR_OS_MEMORY_LOAD + 12
STR_OS_PROC_LEVEL      = STR_OS_MEMORY_LOAD + 13
STR_OS_PROC_REVISION   = STR_OS_MEMORY_LOAD + 14
STR_OS_PROC_CURRENT    = STR_OS_MEMORY_LOAD + 15
STR_OS_PROC_TOTAL      = STR_OS_MEMORY_LOAD + 16
STR_OS_PROC_GROUPS     = STR_OS_MEMORY_LOAD + 17
STR_OS_NUMA_DOMAINS    = STR_OS_MEMORY_LOAD + 18
STR_OS_NORMAL_PAGE     = STR_OS_MEMORY_LOAD + 19
STR_OS_LARGE_PAGE      = STR_OS_MEMORY_LOAD + 20
STR_OS_DISABLED        = STR_OS_MEMORY_LOAD + 21
STR_OS_ENABLED         = STR_OS_MEMORY_LOAD + 22
;---------- Strings for topology information text -----------------------------;
STR_TP_CPU_CORE        = STR_OS_ENABLED + 1
STR_TP_NUMA_NODE       = STR_TP_CPU_CORE + 1
STR_TP_L               = STR_TP_CPU_CORE + 2
STR_TP_CPU_PACKAGE     = STR_TP_CPU_CORE + 3
STR_TP_UNKNOWN_ID      = STR_TP_CPU_CORE + 4
STR_TP_UNIFIED         = STR_TP_CPU_CORE + 5
STR_TP_INSTRUCTION     = STR_TP_CPU_CORE + 6
STR_TP_DATA            = STR_TP_CPU_CORE + 7
STR_TP_TRACE           = STR_TP_CPU_CORE + 8
STR_TP_UNKNOWN_TYPE    = STR_TP_CPU_CORE + 9
STR_TP_POINTS          = STR_TP_CPU_CORE + 10
STR_TP_HT              = STR_TP_CPU_CORE + 11
STR_TP_NODE            = STR_TP_CPU_CORE + 12
STR_TP_CACHE_WAYS      = STR_TP_CPU_CORE + 13
STR_TP_CACHE_LINE      = STR_TP_CPU_CORE + 14
STR_TP_CACHE_SIZE      = STR_TP_CPU_CORE + 15
STR_TP_X               = STR_TP_CPU_CORE + 16
STR_TP_PLUS            = STR_TP_CPU_CORE + 17
;---------- Strings for extended topology information text --------------------;
STR_ET_GROUP           = STR_TP_PLUS + 1 
STR_ET_EFFICIENCY      = STR_ET_GROUP + 1
STR_ET_SMT             = STR_ET_GROUP + 2
;---------- Strings for SMBIOS information text -------------------------------;
STR_SMBIOS_UNKNOWN     = STR_ET_SMT + 1
STR_SMBIOS_VERSION     = STR_SMBIOS_UNKNOWN + 1
STR_SMBIOS_METHOD      = STR_SMBIOS_UNKNOWN + 2
STR_SMBIOS_DMI_REV     = STR_SMBIOS_UNKNOWN + 3
STR_SMBIOS_LENGTH      = STR_SMBIOS_UNKNOWN + 4
STR_SMBIOS_BYTES       = STR_SMBIOS_UNKNOWN + 5
;---------- Strings for ACPI information text ---------------------------------;
STR_ACPI_UNKNOWN       = STR_SMBIOS_BYTES + 1 
;---------- Strings for child screen = Memory and cache performance report ----;
STR_MR_FIRST           = STR_ACPI_UNKNOWN + 1  
STR_MR_APPLICATION     = STR_MR_FIRST + 1  
STR_MR_METHOD          = STR_MR_FIRST + 2
STR_MR_WIDTH           = STR_MR_FIRST + 3
STR_MR_THREADS         = STR_MR_FIRST + 4
STR_MR_HYPER_THR       = STR_MR_FIRST + 5
STR_MR_LARGE_PAGES     = STR_MR_FIRST + 6
STR_MR_NUMA            = STR_MR_FIRST + 7
STR_MR_P_GROUPS        = STR_MR_FIRST + 8
STR_MR_HYBRID          = STR_MR_FIRST + 9 
STR_MR_TARGET_OBJ      = STR_MR_FIRST + 10
STR_MR_PREF_DIST       = STR_MR_FIRST + 11
STR_MR_SIZE_TOTAL      = STR_MR_FIRST + 12
STR_MR_SIZE_PER_THR    = STR_MR_FIRST + 13
STR_MR_MEASURE_PROF    = STR_MR_FIRST + 14
STR_MR_MEASURE_REP     = STR_MR_FIRST + 15
STR_MR_MEMORY_ALLOC    = STR_MR_FIRST + 16
STR_MR_BLOCK_1         = STR_MR_FIRST + 17
STR_MR_BLOCK_2         = STR_MR_FIRST + 18
STR_MR_MEM_ALC_ALL     = STR_MR_FIRST + 19
STR_MR_MEM_ALC_THR     = STR_MR_FIRST + 20
STR_MR_MEAS_RESULTS    = STR_MR_FIRST + 21
STR_MR_DT_MS           = STR_MR_FIRST + 22
STR_MR_DTSC_SEC_MHZ    = STR_MR_FIRST + 23
STR_MR_DTSC_INS_CLK    = STR_MR_FIRST + 24
STR_MR_SPEED_MBPS      = STR_MR_FIRST + 25
STR_MR_LATENCY_NS      = STR_MR_FIRST + 26
STR_MR_APP_TYPE_32     = STR_MR_FIRST + 27   ; Application and OS type
STR_MR_APP_TYPE_64     = STR_MR_FIRST + 28
STR_MR_APP_TYPE_WOW    = STR_MR_FIRST + 29 
STR_MR_HT_00           = STR_MR_FIRST + 30   ; Hyper-threading option strings, for simple result window
STR_MR_HT_01           = STR_MR_FIRST + 31
STR_MR_HT_02           = STR_MR_FIRST + 32
STR_MR_LP_00           = STR_MR_FIRST + 33   ; Large pages, for simple result window
STR_MR_LP_01           = STR_MR_FIRST + 34
STR_MR_LP_02           = STR_MR_FIRST + 35
STR_MR_NUMA_00         = STR_MR_FIRST + 36   ; NUMA option strings
STR_MR_NUMA_01         = STR_MR_FIRST + 37
STR_MR_NUMA_02         = STR_MR_FIRST + 38
STR_MR_NUMA_03         = STR_MR_FIRST + 39
STR_MR_NUMA_04         = STR_MR_FIRST + 40
STR_MR_PG_00           = STR_MR_FIRST + 41   ; PG option strings
STR_MR_PG_01           = STR_MR_FIRST + 42
STR_MR_PG_02           = STR_MR_FIRST + 43  
STR_MR_HYBRID_00       = STR_MR_FIRST + 44   ; Hybrid topology option strings
STR_MR_HYBRID_01       = STR_MR_FIRST + 45
STR_MR_HYBRID_02       = STR_MR_FIRST + 46
STR_MR_HYBRID_03       = STR_MR_FIRST + 47
STR_MR_HYBRID_04       = STR_MR_FIRST + 48
STR_MR_PFDST_00        = STR_MR_FIRST + 49   ; Prefetch distance option strings          
STR_MR_PFDST_01        = STR_MR_FIRST + 50
STR_MR_PFDST_02        = STR_MR_FIRST + 51
STR_MR_PFDST_03        = STR_MR_FIRST + 52
STR_MR_MEASURE_00      = STR_MR_FIRST + 53   ; Measurement options strings
STR_MR_MEASURE_01      = STR_MR_FIRST + 54
STR_MR_MEASURE_02      = STR_MR_FIRST + 55
STR_MR_MEASURE_03      = STR_MR_FIRST + 56
STR_MR_PLUS_TIME       = STR_MR_FIRST + 57 
;---------- Strings for child screen = Memory and cache performance draw ------;
STR_MD_PD_NOT_USED    = STR_MR_PLUS_TIME + 1
STR_MD_PD_DEFAULT     = STR_MD_PD_NOT_USED + 1 
STR_MD_PD_MEDIUM      = STR_MD_PD_NOT_USED + 2
STR_MD_PD_LONG        = STR_MD_PD_NOT_USED + 3
STR_MD_PD_UNKNOWN     = STR_MD_PD_NOT_USED + 4 
STR_MD_REFRESH        = STR_MD_PD_NOT_USED + 5 
STR_MD_THREADS        = STR_MD_PD_NOT_USED + 6 
STR_MD_NO_HYBRID      = STR_MD_PD_NOT_USED + 7
STR_MD_HYBRID_BY_OS   = STR_MD_PD_NOT_USED + 8
STR_MD_HYBRID_P_E     = STR_MD_PD_NOT_USED + 9
STR_MD_HYBRID_P       = STR_MD_PD_NOT_USED + 10
STR_MD_HYBRID_E       = STR_MD_PD_NOT_USED + 11
STR_MD_LP_NOT_USED    = STR_MD_PD_NOT_USED + 12 
STR_MD_4K_PAGES       = STR_MD_PD_NOT_USED + 13
STR_MD_LARGE_PAGES    = STR_MD_PD_NOT_USED + 14
STR_MD_NUMA_OFF       = STR_MD_PD_NOT_USED + 15
STR_MD_NUMA_UNAWARE   = STR_MD_PD_NOT_USED + 16
STR_MD_NUMA_SINGLE    = STR_MD_PD_NOT_USED + 17
STR_MD_NUMA_LOCAL     = STR_MD_PD_NOT_USED + 18
STR_MD_NUMA_REMOTE    = STR_MD_PD_NOT_USED + 19
STR_MD_CPI            = STR_MD_PD_NOT_USED + 20
STR_MD_NSPI           = STR_MD_PD_NOT_USED + 21
STR_MD_MBPS           = STR_MD_PD_NOT_USED + 22
STR_MD_MINIMUM        = STR_MD_PD_NOT_USED + 23
STR_MD_MAXIMUM        = STR_MD_PD_NOT_USED + 24
STR_MD_AVERAGE        = STR_MD_PD_NOT_USED + 25
STR_MD_MEDIAN         = STR_MD_PD_NOT_USED + 26
STR_MD_TSC_CLOCK_MHZ  = STR_MD_PD_NOT_USED + 27
;---------- Strings for child screen = Vector brief performance report --------;
STR_VB_CPU_FEATURES    = STR_MD_TSC_CLOCK_MHZ + 1  
STR_VB_AVX_256         = STR_VB_CPU_FEATURES + 1
STR_VB_AVX2_256        = STR_VB_CPU_FEATURES + 2
STR_VB_AVX3_512        = STR_VB_CPU_FEATURES + 3
STR_VB_AVX512CD        = STR_VB_CPU_FEATURES + 4
STR_VB_AVX512PF        = STR_VB_CPU_FEATURES + 5
STR_VB_AVX512ER        = STR_VB_CPU_FEATURES + 6
STR_VB_AVX512VL        = STR_VB_CPU_FEATURES + 7
STR_VB_AVX512BW        = STR_VB_CPU_FEATURES + 8
STR_VB_AVX512DQ        = STR_VB_CPU_FEATURES + 9
STR_VB_AVX512_IFMA     = STR_VB_CPU_FEATURES + 10
STR_VB_AVX512_VBMI     = STR_VB_CPU_FEATURES + 11
STR_VB_AVX512_VBMI2    = STR_VB_CPU_FEATURES + 12
STR_VB_AVX512_BF16     = STR_VB_CPU_FEATURES + 13
STR_VB_AVX512_VAES     = STR_VB_CPU_FEATURES + 14
STR_VB_AVX512_GFNI     = STR_VB_CPU_FEATURES + 15
STR_VB_AVX512_VNNI     = STR_VB_CPU_FEATURES + 16
STR_VB_AVX512_BTALG    = STR_VB_CPU_FEATURES + 17
STR_VB_AVX512_VPOP     = STR_VB_CPU_FEATURES + 18
STR_VB_AVX512_VPCL     = STR_VB_CPU_FEATURES + 19
STR_VB_AVX512_VP2IN    = STR_VB_CPU_FEATURES + 20
STR_VB_AVX512_FP16     = STR_VB_CPU_FEATURES + 21
STR_VB_AVX512_4FMAP    = STR_VB_CPU_FEATURES + 22
STR_VB_AVX512_4VNNI    = STR_VB_CPU_FEATURES + 23
STR_VB_OS_CONTEXT      = STR_VB_CPU_FEATURES + 24
STR_VB_SSE128_XMM      = STR_VB_CPU_FEATURES + 25
STR_VB_AVX256_YMM      = STR_VB_CPU_FEATURES + 26
STR_VB_AVX512_ZMM_L    = STR_VB_CPU_FEATURES + 27
STR_VB_AVX512_ZMM_H    = STR_VB_CPU_FEATURES + 28
STR_VB_AVX512_K        = STR_VB_CPU_FEATURES + 29
STR_VB_TIMINGS         = STR_VB_CPU_FEATURES + 30
STR_VB_SSE128_READ     = STR_VB_CPU_FEATURES + 31             
STR_VB_SSE128_WRITE    = STR_VB_CPU_FEATURES + 32             
STR_VB_SSE128_COPY     = STR_VB_CPU_FEATURES + 33             
STR_VB_AVX256_READ     = STR_VB_CPU_FEATURES + 34             
STR_VB_AVX256_WRITE    = STR_VB_CPU_FEATURES + 35
STR_VB_AVX256_COPY     = STR_VB_CPU_FEATURES + 36             
STR_VB_AVX512_READ     = STR_VB_CPU_FEATURES + 37
STR_VB_AVX512_WRITE    = STR_VB_CPU_FEATURES + 38             
STR_VB_AVX512_COPY     = STR_VB_CPU_FEATURES + 39             
STR_VB_SQRTPD_XMM      = STR_VB_CPU_FEATURES + 40             
STR_VB_VSQRTPD_YMM     = STR_VB_CPU_FEATURES + 41             
STR_VB_VSQRTPD_ZMM     = STR_VB_CPU_FEATURES + 42
STR_VB_FCOS            = STR_VB_CPU_FEATURES + 43             
STR_VB_FSINCOS         = STR_VB_CPU_FEATURES + 44             
;---------- Strings for child screen = "About" window -------------------------;
STR_AB_PROGRAM_INFO    = STR_VB_FSINCOS + 1
STR_AB_GITHUB          = STR_AB_PROGRAM_INFO + 1
STR_AB_FASM            = STR_AB_PROGRAM_INFO + 2
STR_AB_GITHUB_LINK     = STR_AB_PROGRAM_INFO + 3
STR_AB_FASM_LINK       = STR_AB_PROGRAM_INFO + 4
STR_AB_SHELL_ERROR     = STR_AB_PROGRAM_INFO + 5 
;--- Equations for strings geometry and color with clickable sub-strings ------;
; This values must be updated if this string update at DATA.ASM module.
ABOUT_FULL_SIZE_1      = 15  ; all string size, chars
ABOUT_CLICK_START_1    = 8   ; start of 'GitHub' sub-string, 0-based
ABOUT_CLICK_SIZE_1     = 6   ; size of 'GitHub' sub-string
ABOUT_FULL_SIZE_2      = 30  ; all string size, chars 
ABOUT_CLICK_START_2    = 15  ; start of 'Flat Assembler' sub-string, 0-based 
ABOUT_CLICK_SIZE_2     = 14  ; size of 'Flat Assembler' sub-string
ABOUT_CLICK_COLOR      = 00FF0000h  ; Color for clicked, 00bbggrrh 
;---------- Strings for child screen = "Please wait" window -------------------;
STR_PW_CPU_BENCH       = STR_AB_SHELL_ERROR + 1  ; This 2 ID must be sequental
STR_PW_MEMORY_BENCH    = STR_PW_CPU_BENCH + 1
STR_PW_PLEASE_WAIT     = STR_PW_CPU_BENCH + 2
STR_PW_RUNNING         = STR_PW_CPU_BENCH + 3
;---------- Strings for fatal error messages, cannot run NCRB -----------------;
; This messages can be generated if resource DLL successfully loaded,
; see also message strings at executeble files NCRB32.ASM, NCRB64.ASM.
STR_ERROR_CPUID        = STR_PW_RUNNING + 1 
STR_ERROR_CPUID_F1     = STR_ERROR_CPUID + 1    
STR_ERROR_X87          = STR_ERROR_CPUID + 2
STR_ERROR_TSC          = STR_ERROR_CPUID + 3
STR_ERROR_TSC_FREQ     = STR_ERROR_CPUID + 4
STR_ERROR_MEMORY_API   = STR_ERROR_CPUID + 5
STR_ERROR_TOPOLOGY_API = STR_ERROR_CPUID + 6
;---------- Strings for non-fatal warning messages ----------------------------;
STR_WARNING_API        = STR_ERROR_TOPOLOGY_API + 1
STR_WARNING_WOW        = STR_WARNING_API + 1
;---------- Strings for runtime errors ----------------------------------------;
STR_RUNTIME_ALLOC      = STR_WARNING_WOW + 1  
STR_RUNTIME_RELEASE    = STR_RUNTIME_ALLOC + 1
STR_RUNTIME_TIMINGS    = STR_RUNTIME_ALLOC + 2
STR_RUNTIME_ADDRESS    = STR_RUNTIME_ALLOC + 3
;---------- Strings for input parsing errors and block size restrictions ------;
STR_PARSING_DECIMAL    = STR_RUNTIME_ADDRESS + 1
STR_PARSING_HEX        = STR_PARSING_DECIMAL + 1
STR_PARSING_PREVIOUS   = STR_PARSING_DECIMAL + 2
STR_CUSTOM_ERROR       = STR_PARSING_DECIMAL + 3
STR_CUSTOM_ERROR_MIN   = STR_PARSING_DECIMAL + 4
STR_CUSTOM_ERROR_MAX   = STR_PARSING_DECIMAL + 5
STR_CUSTOM_ERROR_ALIGN = STR_PARSING_DECIMAL + 6
;---------- Strings for support save text report ------------------------------;
STR_REPORT_FILE_NAME   = STR_CUSTOM_ERROR_ALIGN + 1 
STR_REPORT_FILTER_1    = STR_REPORT_FILE_NAME + 1
STR_REPORT_FILTER_2    = STR_REPORT_FILE_NAME + 2
STR_REPORT_FILTER_3    = STR_REPORT_FILE_NAME + 3
STR_REPORT_FAILED      = STR_REPORT_FILE_NAME + 4
STR_REPORT_EXISTS_1    = STR_REPORT_FILE_NAME + 5
STR_REPORT_EXISTS_2    = STR_REPORT_FILE_NAME + 6 
STR_REPORT_SAVED       = STR_REPORT_FILE_NAME + 7 
STR_REPORT_DEF_EXT     = STR_REPORT_FILE_NAME + 8 
;---------- Strings for Kernel Mode Driver and Service Control Program --------;
STR_NAME_KMD32         = STR_REPORT_DEF_EXT + 1  
STR_NAME_KMD64         = STR_NAME_KMD32 + 1
STR_DRV_NAME           = STR_NAME_KMD32 + 2
STR_DRV_DEVICE         = STR_NAME_KMD32 + 3
STR_DRV_ERROR_LOAD     = STR_NAME_KMD32 + 4
STR_DRV_ERROR_UNLOAD   = STR_NAME_KMD32 + 5
STR_DRV_ERROR_ACCESS   = STR_NAME_KMD32 + 6 
STR_DRE_GET_PATH       = STR_NAME_KMD32 + 7
STR_DRE_OPEN_SERVICE   = STR_NAME_KMD32 + 8
STR_DRE_CREATE_SERVICE = STR_NAME_KMD32 + 9
STR_DRE_GET_STATUS     = STR_NAME_KMD32 + 10
STR_DRE_START_SERVICE  = STR_NAME_KMD32 + 11
STR_DRE_CREATE_DEVICE  = STR_NAME_KMD32 + 12
;---------- Strings for CPU vendors detection ---------------------------------;
STR_SIGN_INTEL         = STR_DRE_CREATE_DEVICE + 1 
STR_SIGN_AMD           = STR_SIGN_INTEL + 1
;---------- Strings for Configuration INF file support and OS errors ----------;
STR_INF_TITLE          = STR_SIGN_AMD + 1 
STR_INF_UNKNOWN_OPTION = STR_INF_TITLE + 1 
STR_INF_BAD_OPTION     = STR_INF_TITLE + 2 
STR_INF_FILE_BIG       = STR_INF_TITLE + 3
STR_INF_FILE_BAD       = STR_INF_TITLE + 4
STR_OS_ERROR           = STR_INF_TITLE + 5   
;---------- Binders numbers for GUI scripts -----------------------------------;
; This binders for build GUI objects (widgets) by data from buffer (bindlist).
BINDER_BUILD_SYSINFO       = 0   
BINDER_BUILD_MEMORY        = 1
BINDER_BUILD_MEMORY_IA32   = 2 
BINDER_BUILD_MEMORY_X64    = 3
BINDER_BUILD_OS            = 4
BINDER_BUILD_NATIVE_OS     = 5
BINDER_BUILD_TOPOLOGY      = 6
BINDER_BUILD_TOPOLOGY_EX   = 7
BINDER_BUILD_NUMA          = 8
BINDER_BUILD_P_GROUPS      = 9
BINDER_BUILD_SMBIOS        = 10
BINDER_BUILD_ACPI          = 11
BINDER_BUILD_AFF_CPUID     = 12
BINDER_BUILD_MEMORY_RUN    = 13
BINDER_BUILD_MEMORY_MBPS   = 14
BINDER_BUILD_MEMORY_NS     = 15
BINDER_BUILD_MEMORY_DRAW   = 16 
BINDER_BUILD_VECTOR_BRIEF  = 17  
BINDER_BUILD_ABOUT         = 18
;---------- Binders numbers for GUI scripts: set widgets state ----------------;
; This binders for set GUI objects (widgets) state by data from buffer
; (bindlist), separate from build objects, because required set by
; system configuration detection results and by "Set defaults" button.
BINDER_SET_MEMORY          = 19
;---------- Binders numbers for GUI scripts: get widgets state ----------------;
; This binders for read data from GUI objects (widgets) to buffer (bindlist).
; Separate binder for editable strings custom parameters get,
; because errors messages generated. 
BINDER_GET_MEMORY          = 20
BINDER_GET_MEMORY_CUSTOM   = 21
;---------- Binders numbers for make widgets dependency -----------------------;
BINDER_WIDGET_NONTEMPORAL  = 22
BINDER_WIDGET_FORCE32X2    = 23
BINDER_WIDGET_PREFETCH     = 24
BINDER_WIDGET_CUSTOM_SIZE  = 25
;--- Binder for disable buttons when run benchmarks and enable after ----------;
BINDER_BUTTONS_ON_OFF      = 26 
;---------- List of pointers to strings used by GUI binders -------------------;
; This structures used as template for offsets in the bind buffer.
; For ia32/x64 unification, pointers always 64-bit, high dword = 0 for ia32. 
; CPU strings at system information screen
struct BINDCPU
vendor        dq  ?
tfms          db  15  dup ?
name          dq  ?    
tsc           db  32  dup ?
cpuBitmap     dq  ?
avx512bitmap  dq  ?
osBitmap      dq  ?
secondBitmap  dq  ?
secondVmm     dq  ?
ends
;---------- ACPI strings at system information screen -------------------------;
struct BINDACPI
oem           db  6  + 1  dup ?
manufact      db  8  + 1  dup ?
oemRev        db  25 + 1  dup ?
comment       db  50 + 1  dup ?
ends
;---------- Cache strings at system information screen ------------------------;
struct BINDCACHE
cacheBitmap   db  ?
l1c           db  40 + 1  dup ?
l1d           db  40 + 1  dup ?
l2u           db  40 + 1  dup ?
l3u           db  40 + 1  dup ?
l4u           db  40 + 1  dup ?
ends
;---------- Topology strings at system information screen ---------------------;
struct BINDTOPOLOGY
hybridBitmap  db  ?
threads       db  10 + 1  dup ?
cores         db  10 + 1  dup ?
sockets       db  10 + 1  dup ?
pCores        db  10 + 1  dup ?
eCores        db  10 + 1  dup ?
ends
;---------- OS/Memory/NUMA/Paging strings at system information screen --------;
struct BINDSYS
procTotal     db  10 + 1  dup ?
groups        db  10 + 1  dup ?
procCur       db  10 + 1  dup ?
numaNodes     db  10 + 1  dup ?
memPhys       db  20 + 1  dup ?
memAvail      db  20 + 1  dup ?
largePage     db  20 + 1  dup ?
largeEnable   db  20 + 1  dup ?   
masksList     db  80 + 1  dup ?    
ends

;---------- GUI control list format definition --------------------------------;
struct BINDLIST
;---------- Main system information screen ------------------------------------;
; set widgets from bindlist buffer
bindCpu       BINDCPU       ?
acpiEnable    db            ?    ; D0 = ACPI, D1 = MADT, D2 = SRAT
bindMadt      BINDACPI      ?
bindSrat      BINDACPI      ?
bindCache     BINDCACHE     ?
bindTopology  BINDTOPOLOGY  ?
bindSys       BINDSYS       ?
;---------- Memory and cache benchmarks screen, part 1 of 3 -------------------;
; get widgets state to bindlist buffer
; 1 bit per switch: enable or disable
getMemMethod   dd  ?
getMemObject   dw  ?
getMemOption   db  ?
getMemHybrid   db  ?
getMemSmp      db  ?
getMemNuma     db  ?
getMemAccess   db  ?
getMemLpages   db  ?
getMemPref     db  ?
getMemMeas     db  ?
getMemPix      db  ?
getMemSilent   db  ?
getBlkCustom   dq  ?
getBlkMmf      dq  ?
getBlkGpu      dq  ?
getBlkStart    dq  ?
getBlkStop     dq  ?
getBlkStep     dd  ?
getBlkDist     dd  ?
;---------- Memory and cache benchmarks screen, part 2 of 3 -------------------; 
; set widgets state by system information in the bindlist buffer
; also used by vector brief benchmark scenario
; 2 bits per switch: enable-disable and checked-unchecked
setMemMethod   dd  ? , ?
setMemObject   dw  ? , ?
setMemOption   db  ? , ?
setMemHybrid   db  ? , ?
setMemSmp      db  ? , ?
setMemNuma     db  ? , ?
setMemAccess   db  ? , ?
setMemLpages   db  ? , ?
setMemPref     db  ? , ?
setMemMeas     db  ? , ?
setMemPix      db  ? , ?
setMemSilent   db  ? , ?
setBlkCustom   dq  ?
setBlkMmf      dq  ?
setBlkGpu      dq  ?
setBlkStart    dq  ?
setBlkStop     dq  ?
setBlkStep     dd  ?
setBlkDist     dd  ?
;---------- Memory and cache benchmarks screen, part 3 of 3 -------------------;
; This fields used for individual widgets control for make widgets dependency
; when switched.
; 1) "Nontemporal" option depend on assembler method and object (cache/DRAM).
; 2) "Force 32x2"  option depend on assembler method.
; 3) "Prefetch"    option depend on assembler method and object (cache/DRAM).
; 4) "Custom block size" numeric field depend on custom block size selected.
scratchPad     db  8 dup (?)
;---------- Other screens - text blocks viewers -------------------------------;
; set widgets from bindlist buffer
TEXT_SCREEN_COUNT = 12
; this 12 pointers MUST BE SEQUENTAL, because accessed in the cycle
viewOs         dq  ?
viewNative     dq  ?
viewTp         dq  ?
viewTpSum      dq  ?
viewEt         dq  ?
viewEtSum      dq  ?
viewNuma       dq  ?
viewGroup      dq  ?
viewSmbios     dq  ?
viewAcpi       dq  ?
viewAcpiSum    dq  ?
viewAffCpu     dq  ?
; end of sequental group of pointers
;---------- Vector brief benchmark results screen support ---------------------;
VB_REPORT_COUNT  =  14  
VB_REPORT_STEP   =  41
vbSse128read   db  40 + 1  dup ?  ; Benchmark results for SSE128 Read
vbSse128write  db  40 + 1  dup ?  ; Benchmark results for SSE128 Write
vbSse128copy   db  40 + 1  dup ?  ; Benchmark results for SSE128 Copy 
vbAvx256read   db  40 + 1  dup ?  ; Benchmark results for AVX256 Read
vbAvx256write  db  40 + 1  dup ?  ; Benchmark results for AVX256 Write
vbAvx256copy   db  40 + 1  dup ?  ; Benchmark results for AVX256 Copy 
vbAvx512read   db  40 + 1  dup ?  ; Benchmark results for AVX512 Read
vbAvx512write  db  40 + 1  dup ?  ; Benchmark results for AVX512 Write
vbAvx512copy   db  40 + 1  dup ?  ; Benchmark results for AVX512 Copy 
vbSse128sqrt   db  40 + 1  dup ?  ; Benchmark results for SSE128 Square Root
vbAvx256sqrt   db  40 + 1  dup ?  ; Benchmark results for AVX256 Square Root
vbAvx512sqrt   db  40 + 1  dup ?  ; Benchmark results for AVX512 Square Root
vbX87cos       db  40 + 1  dup ?  ; Benchmark results for x87 Cosine (FCOS)
vbX87sincos    db  40 + 1  dup ?  ; Benchmark results for x87 Sine+Cosine (FSINCOS) 
;---------- Memory and cache simple benchmark results screen support ----------;
; TODO. Can share BINDLIST space, for example mr and vb.
;       This can be solution for alignment.
; But BINDLIST can be used for reports, include saved text strings.
mrApplication  db  40 + 1  dup ?
mrMethod       db  80 + 1  dup ?
mrWidth        db  40 + 1  dup ?
mrThreads      db  40 + 1  dup ?
mrHyperT       db  40 + 1  dup ?
mrLargeP       db  40 + 1  dup ?
mrNuma         db  40 + 1  dup ?
mrGroups       db  40 + 1  dup ?
mrHybrid       db  40 + 1  dup ?  
mrTarget       db  40 + 1  dup ?
mrDistance     db  40 + 1  dup ?
mrSizeTotal    db  40 + 1  dup ?
mrSizeThread   db  40 + 1  dup ?
mrMeasProf     db  40 + 1  dup ?
mrMeasRep      db  40 + 1  dup ?
mrBlock1       db  40 + 1  dup ?
mrBlock2       db  40 + 1  dup ?
mrAllocAll     db  40 + 1  dup ?
mrAllocThr     db  40 + 1  dup ?
mrDtMs         db  80 + 1  dup ?
mrDtscSec      db  40 + 1  dup ?
mrDtscIns      db  40 + 1  dup ?
mrLastValue    db  40 + 1  dup ? 
;---------- Memory and cache benchmark drawings window screen support ---------;
mdAsm          db  80 + 1  dup ?
mdPrefetch     db  40 + 1  dup ?
mdObject       db  40 + 1  dup ?
mdThreads      db  40 + 1  dup ?
mdRepeats      db  40 + 1  dup ?
mdPages        db  40 + 1  dup ?
mdNuma         db  40 + 1  dup ?
ends

;---------- Macro definitions for binders scripts instructions ----------------;
macro BIND_STOP
{ DB  0 }
MACRO SET_STRING srcid, dstid
{ DD 01h + (srcid) SHL 6 + (dstid) SHL 19 }
MACRO SET_INFO srcid, dstid
{ DD 02h + (srcid) SHL 6 + (dstid) SHL 19 }
MACRO SET_PTR srcid, dstid
{ DD 03h + (srcid) SHL 6 + (dstid) SHL 19 }
MACRO SET_BOOL srcid, srcbit, dstid
{ DD 04h + (srcid) SHL 9 + (srcbit) SHL 6 + (dstid) SHL 19 }
MACRO SET_SWITCH srcid, srcbit, dstid
{ DD 05h + (srcid) SHL 9 + (srcbit) SHL 6 + (dstid) SHL 19 }
MACRO SET_DEC32 srcid, dstid
{ DD 06h + (srcid) SHL 6 + (dstid) SHL 19 }
MACRO SET_HEX32 srcid, dstid
{ DD 07h + (srcid) SHL 6 + (dstid) SHL 19 } 
MACRO SET_HEX64 srcid, dstid
{ DD 08h + (srcid) SHL 6 + (dstid) SHL 19 } 
COMBO_STOP_ON  = 0
COMBO_STOP_OFF = 1
COMBO_CURRENT  = 2
COMBO_ADD      = 3
COMBO_INACTIVE = 4
MACRO SET_COMBO srcid, dstid
{ DD 09h + (srcid) SHL 6 + (dstid) SHL 19 }
MACRO SET_FONT srcid, dstid
{ DD 0Ah + (srcid) SHL 6 + (dstid) SHL 19 }
MACRO GET_SWITCH srcid, dstid, dstbit                             
{ DD 0Bh + (dstid) SHL 9 + (dstbit) SHL 6 + (srcid) SHL 19 }
MACRO GET_DEC32 srcid, dstid                             
{ DD 0Ch + (dstid) SHL 6 + (srcid) SHL 19 } 
MACRO GET_HEX32 srcid, dstid                             
{ DD 0Dh + (dstid) SHL 6 + (srcid) SHL 19 } 
MACRO GET_HEX64 srcid, dstid                             
{ DD 0Eh + (dstid) SHL 6 + (srcid) SHL 19 } 

;---------- Macro definitions for CPU/OS features bitmaps builders scripts ----;
R_EAX = 0
R_EBX = 1
R_ECX = 2
R_EDX = 3
MACRO ENTRY_STOP
{ DB 00h }
MACRO ENTRY_CPUID function, register, bit 
{
DB 01h + ( register SHL 6 )
DD function
DB bit
}
MACRO ENTRY_CPUID_S function, subfunction, register, bit 
{
DB 02h + ( register SHL 6 )
DD function, subfunction
DB bit
}
MACRO PREFIX_CPUID function, register, bit 
{
DB 03h + ( register SHL 6 )
DD function
DB bit
}
MACRO PREFIX_CPUID_S function, subfunction, register, bit 
{
DB 04h + ( register SHL 6 )
DD function, subfunction
DB bit
}
MACRO ENTRY_XCR0 bit
{ DB  05h , bit }
MACRO PREFIX_XCR0 bit
{ DB  06h , bit }
;---------- List of INTEL-specific cache descriptors for caches detection -----;
; Macro for decoding Intel cache detection data structures (descriptors) 
; This actual for CPUs not support function 
; CPUID 00000004h = Deterministic Cache Info, 
; Note if runs under Windows XP 32-bit, cache information cannot be detected
; by Win API GetLogicalProcessorInformation, means required use:
; CPUID 00000002h = Get Cache Descriptors, with decoding by this data
MACRO L1T x1, x2
{           ; 1 = ID for Trace Cache decode 
DB 1, x1    ; x2 = Cache Descriptor by Intel
DW x2       ; x2 = Cache Size, KuOps for trace cache 
}
MACRO L1I x1, x2
{           ; 1 = ID for L1 Instruction Cache decode
DB 2, x1    ; x1 = Cache Descriptor by Intel
DW x2       ; x2 = Cache Size, kilobytes 
}
MACRO L1D x1, x2
{           ; 2 = ID for L1 Data Cache decode
DB 3, x1    ; x1 = Cache Descriptor by Intel
DW x2       ; x2 = Cache Size, kilobytes 
}
MACRO L2U x1, x2
{           ; 3 = ID for L2 Unified Cache decode
DB 4, x1    ; x1 = Cache Descriptor by Intel
DW x2       ; x2 = Cache Size, kilobytes 
}
MACRO L3U x1, x2
{           ; 4 = ID for L3 Unified Cache decode
DB 5, x1    ; x1 = Cache Descriptor by Intel
DW x2       ; x2 = Cache Size, kilobytes 
}
MACRO END_CACHE
{           ; List terminator
DB 0        ; 0 = End of list
}

;---------- GUI common definitions --------------------------------------------; 
CLEARTYPE_QUALITY = 5
ID_FONT_1         = 0
ID_FONT_2         = 1
;--- Geometry parameters for area with drawings Speed = F( Block Size ) -------;
; Geometry X parameters
SUBWINX    = 773    ; Plot sub-window X size, pixels
GRIDX      = 11     ; Number of vertical lines in the coordinate X-grid
GRIDBLANKX = 70     ; Coordinate X blank offset for point X=0
GRIDSTEPX  = 64     ; Coordinate addend for X-grid step
SHIFTX     = 0      ; X-shift plot sub window in the dialogue window, pixels
; Geometry Y parameters 
SUBWINY    = 502    ; Plot sub-window Y size, pixels
GRIDY      = 10     ; Number of horizontal lines in the coordinate Y-grid
GRIDBLANKY = 25     ; Coordinate Y blank offset for point Y=0
GRIDSTEPY  = 48     ; Coordinate addend for Y-grid step
SHIFTY     = 0      ; Y-shift plot sub window in the dialogue window, pixels
;--- Color parameters of Sub-Window with drawings Speed = F( Block Size ) -----; 
; Brush color values = 00bbggrrh, bb=blue, gg=green, rr=red, 1 byte per color
BRUSH_GRID        = 00C8C8C8h         ; Grid with horizontal and vertical lines 
BRUSH_LINE        = 000101F0h         ; Draw Line Speed = F (Block Size), redundant because pen
BRUSH_BACKGROUND  = 00F9F9F9h         ; Draw window background
BRUSH_STATISTICS  = 00E82121h         ; Statistics table lines 
PEN_LINE          = BRUSH_LINE        ; Draw Line Speed = F (Block Size)
COLOR_TEXT_VALUES = 00414141h         ; Text front color, grid numeric values print
COLOR_TEXT_UNITS  = BRUSH_STATISTICS  ; Text front color, units print
COLOR_TEXT_BACK   = BRUSH_BACKGROUND  ; Text back color
COLOR_TEXT_INFO   = COLOR_TEXT_UNITS  ; Text front color, system info print
COLOR_TEXT_DUMP   = 0001B001h         ; Text front color, instruction dump print
;---------- WinAPI equations for DIB ( Device Independent Bitmap ) ------------;
DIB_RGB_COLORS    = 00h   ; Means mode: color table contain RGB values
DIB_PAL_COLORS    = 01h   ; Means mode: color tab. is indexes in the pal. table
DIB_PAL_INDICES   = 02h   ; Means mode: no color table exist, use default
;---------- Equations for build clks/mbps table texts -------------------------;
DRAW_TABLE_FIRST_TEXT = STR_MD_CPI
DRAW_TABLE_UP_COUNT   = 3
DRAW_TABLE_LEFT_COUNT = 4
DRAW_TABLE_COUNT      = DRAW_TABLE_UP_COUNT + DRAW_TABLE_LEFT_COUNT
;---------- Equations for build units texts -----------------------------------;
UNITS_FIRST_TEXT      = STR_UNITS_BYTES 
UNITS_COUNT           = 7
;---------- Equations for write TSC frequency at drawings window --------------;
DRAW_TSC_STRING_SIZE  = 32
TSC_VALUE_OFFSET      = 19
;---------- GUI geometry definitions for "About" window -----------------------;
ABOUT_YBASE1     =  45    ; Strings 1-3 vertical start position
ABOUT_YBASE2     =  106   ; Strings 4-5 vertical start position
ABOUT_YADD1      =  17    ; Strings 1-3 vertical step 
ABOUT_YADD2      =  17    ; Strings 4-5 vertical step 
ABOUT_YBOTTOM    =  45    ; Bottom field with "OK" button Y-size
ABOUT_ICONX      =  104   ; Icon base X-coordinate, upper-left corner
ABOUT_ICONY      =  5     ; Icon base Y-coordinate, upper-left corner
ABOUT_ICONDX     =  32    ; Icon X-size
ABOUT_ICONDY     =  32    ; Icon Y-size
;---------- GUI geometry and control definitions for "Wait" window ------------;
WAIT_YBASE1      =  23    ; Benchmark name string vertical start position
WAIT_YBASE2      =  85    ; "Please wait" string vertical start position
WAIT_YADD        =  18    ; Strings vertical step, here for rectangle size
WAIT_ICONX       =  69    ; Icon base X-coordinate, upper-left corner
WAIT_ICONY       =  48    ; Icon base Y-coordinate, upper-left corner
WAIT_MODE_CPU    =  0     ; "Please wait" for CPU benchmark
WAIT_MODE_MEMORY =  1     ; "Please wait" for Memory benchmark

;---------- Definitions for parsing configuration INF file, options keys ------;
OPTION_SCALE_MBPS_L1     = 0
OPTION_SCALE_MBPS_L2     = 1
OPTION_SCALE_MBPS_L3     = 2
OPTION_SCALE_MBPS_L4     = 3
OPTION_SCALE_MBPS_DRAM   = 4
OPTION_SCALE_MBPS_CUSTOM = 5
OPTION_SCALE_NS_L1       = 6
OPTION_SCALE_NS_L2       = 7
OPTION_SCALE_NS_L3       = 8
OPTION_SCALE_NS_L4       = 9
OPTION_SCALE_NS_DRAM     = 10
OPTION_SCALE_NS_CUSTOM   = 11

;---------- Definitions for parsing configuration INF file, structures --------;
; Definitions for configuration file (NCRB.INF) support,
; options descriptors, types of descriptors 
XEND    EQU  0
XKEY    EQU  1 
XDEC    EQU  2
XHEX    EQU  3
XSIZE   EQU  4
XSTR    EQU  5
XLAST   EQU  5
; Offsets of descriptors fields
X0      EQU  0
X1      EQU  1
X2      EQU  5
X3      EQU  9
X4      EQU  13
; Addends for addressing descriptors sequence
XBIG    EQU  17
XSMALL  EQU  13
XDELTA  EQU  XBIG - XSMALL
; ID = 0 = Terminator for list of options descriptors
macro OPTION_END
{
DB  XEND         ; ID = 0 = Terminator for list of options descriptors
}
; ID = 1 = means option is list of keywords
macro OPTION_KEYS  x1, x2, x3, x4
{
DB  XKEY           ; ID = 1 = means option is list of keywords
DD  x1 - OpDesc    ; Pointer to option long name string, 0-terminated
DD  x2 ; - OpDesc  ; Pointer to option value = byte 
DD  x3 - OpDesc    ; Pointer to option single word short name string, for detection
DD  x4 - OpDesc    ; Pointer to list of 0-terminated keywords, 0,0 means end of list 
}
; ID = 2 = means 32-bit unsigned value, interpreted as decimal
macro OPTION_DECIMAL_32  x1, x2, x3
{
DB  XDEC           ; ID = 2 = means 32-bit unsigned value, interpreted as decimal
DD  x1 - OpDesc    ; Pointer to option long name string, 0-terminated
DD  x2 ; - OpDesc  ; Pointer to option value = dword
DD  x3 - OpDesc    ; Pointer to option single word short name string, for detection 
}
; ID = 3 = means 64-bit unsigned value, interpreted as hex
macro OPTION_HEX_64  x1, x2, x3
{
DB  XHEX           ; ID = 3 = means 64-bit unsigned value, interpreted as hex
DD  x1 - OpDesc    ; Pointer to option long name string, 0-terminated
DD  x2 ; - OpDesc  ; Pointer to option value = qword
DD  x3 - OpDesc    ; Pointer to option single word short name string, for detection 
}
; ID = 3 = means 64-bit unsigned value, interpreted as hex
macro OPTION_SIZE_64  x1, x2, x3
{
DB  XSIZE          ; ID = 3 = means 64-bit unsigned value, interpreted as hex
DD  x1 - OpDesc    ; Pointer to option long name string, 0-terminated
DD  x2 ; - OpDesc  ; Pointer to option value = qword
DD  x3 - OpDesc    ; Pointer to option single word short name string, for detection 
}
; ID = 5 = means pointer to pointer to string
macro OPTION_STRING  x1, x2, x3
{
DB  XSTR           ; ID = 5 = means pointer to pointer to string
DD  x1 - OpDesc    ; Pointer to option long name string, 0-terminated
DD  x2 ; - OpDesc  ; Pointer to option value = pointer to string, 0-terminated
DD  x3 - OpDesc    ; Pointer to option single word short name string, for detection 
}

;--- Structure with configuration variables, located at executable file -------;
CONFIG_VALUES_COUNT_SCALE = 12
CONFIG_VALUES_COUNT_ALL = 13
struct CONFIGVALUES
optionMbpsL1       dd  ?  ; Use this offset for 6 bandwidth values, f(object)
optionMbpsL2       dd  ?
optionMbpsL3       dd  ?
optionMbpsL4       dd  ?
optionMbpsDram     dd  ?
optionMbpsCustom   dd  ?
optionNsL1         dd  ?  ; Use this offset for 6 latency values, f(object)
optionNsL2         dd  ?
optionNsL3         dd  ?
optionNsL4         dd  ?
optionNsDram       dd  ?
optionNsCustom     dd  ?
optionLoadKmd      dd  ?  ; Enable try load kernel mode driver
ends

;---------- Common definitions for parse configuration INF file routines ------;
PATH_BUFFER_SIZE   EQU  256   ; Limit for buffers with paths, include last 0

