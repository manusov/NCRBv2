;---------- Get Extended Topology information ---------------------------------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
; Separate calls for 5 entries types (0-4) required for ordering
; text strings in the report.
InfoExtendedTopology:
push 0
mov r13,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
call HelperAlignment
mov rbx,[DYNA_IMPORT._GetLogicalProcessorInformationEx]
test rbx,rbx
jz .exit
xor r14d,r14d
mov al,0                  ; 0 = RelationProcessorCore
call HelperTopologyEx
jc .errorTopologyEx
mov al,2                  ; 2 = RelationCache 
call HelperTopologyEx
jc .errorTopologyEx
mov al,3                  ; 3 = RelationProcessorPackage
call HelperTopologyEx
jc .errorTopologyEx
mov al,1                  ; 1 = RelationNumaNode
call HelperTopologyEx
jc .errorTopologyEx
mov al,4                  ; 4 = RelationGroup
call HelperTopologyEx
.errorTopologyEx:
jc .error
mov [DYNA_PTR.listTopologyEx.objectStart],rbp
add rbp,r14
mov [DYNA_PTR.listTopologyEx.objectStop],rbp
clc
.exit:
mov rsp,r13
pop rcx
ret
.error:
mov al,MSG_ERROR_TOPOLOGY_API
stc
jmp .exit

;--- Build Platform ext. topology inform. for text report and GUI widgets -----;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;

; This temporary variables required for parameters, previously detected by
; standard topology (InfoTopology, TextTopology subroutines), because standard
; topology parameters must be not changed  if extended topology
; (InfoExtendedTopology, TextExtendedTopology subroutines) not supported. 
TEMP_THREADS   EQU  dword [rsp + 00 + 08]
TEMP_CORES     EQU  dword [rsp + 04 + 08]
TEMP_SOCKETS   EQU  dword [rsp + 08 + 08]
TEMP_RESERVED  EQU  dword [rsp + 12 + 08]   

TextExtendedTopology:
;--- Build text block for ext. topology screen, first block = objects list ----;
mov [DYNA_PTR.textTopologyEx1.objectStart],rbp
mov rsi,[DYNA_PTR.listTopologyEx.objectStart]
mov rcx,[DYNA_PTR.listTopologyEx.objectStop]
test rcx,rcx
jz .noTopologyEx
push rsi rcx 0 0
mov rdi,rbp
.scanRelationsEx:
mov rdx,rdi
push rcx
cld
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
pop rcx
push rdi
mov eax,[rsi + 00]
cmp eax,4
ja .relationUnknownEx
je .relationGroupEx
cmp al,3
je .relationPackageEx
cmp al,2
je .relationCacheEx
cmp al,1
je .relationNumaEx
cmp al,0
je .relationCoreEx
;---------- Extended topology entry type: UNKNOWN -----------------------------;
.relationUnknownEx:
mov ax,STR_TP_UNKNOWN_ID
call HelperRelationName 
jmp .doneRelationEx
;---------- Extended topology entry type: CORE --------------------------------;
.relationCoreEx:
inc TEMP_CORES                         ; Support hybrid unaware
inc TEMP_THREADS
cmp byte [rsi + 08],1
jne @f
inc TEMP_THREADS
@@:
lea rax,[SYS_PARMS.perfCoreTopology]   ; Support hybrid cores
lea rdi,[SYS_PARMS.perfCoreAffinity]
cmp byte [rsi + 09],1
je @f
lea rax,[SYS_PARMS.effCoreTopology]
lea rdi,[SYS_PARMS.effCoreAffinity]
@@:
inc [rax + SUMMARYTOPOLOGY.cores]
inc [rax + SUMMARYTOPOLOGY.threads]
cmp byte [rsi + 08],1
jne @f
inc [rax + SUMMARYTOPOLOGY.threads]
@@:
mov rax,[rsi + 32]
or [rdi],rax                           ; Store affinity mask
mov ax,STR_TP_CPU_CORE                 ; Write text about core
call HelperRelationName 
mov al,32
call HelperAffinityGroup
mov eax,( STR_ET_SMT SHL 16 ) + 2908h
call HelperParameter8
mov eax,( STR_ET_EFFICIENCY SHL 16 ) + 3209h
call HelperParameter8
jmp .doneRelationEx
;---------- Extended topology entry type: NUMA NODE ---------------------------;
.relationNumaEx: 
mov ax,STR_TP_NUMA_NODE
call HelperRelationName 
mov al,32
call HelperAffinityGroup
mov eax,( STR_TP_NODE SHL 16 ) + 2908h
call HelperParameter32
jmp .doneRelationEx
;---------- Extended topology entry type: CACHE -------------------------------;
.relationCacheEx:
mov ax,STR_TP_L
call HelperRelationName
mov al,[rsi + 08]
cmp al,1
jb .levelBadEx
cmp al,4
ja .levelBadEx
or al,30h
jmp .levelStoreEx
.levelBadEx:
mov al,'?'
.levelStoreEx:
mov ah,' '
stosw
mov eax,[rsi + 16]
cmp eax,3
ja .typeBadEx
add ax,STR_TP_UNIFIED
jmp .typeStoreEx
.typeBadEx:
mov ax,STR_TP_UNKNOWN_TYPE
.typeStoreEx:
lea rdi,[rdx + 04] 
call RelationNameEntry
mov al,40
call HelperAffinityGroup
mov eax,( STR_TP_CACHE_WAYS SHL 16 ) + 2909h
call HelperParameter8
mov eax,( STR_TP_CACHE_LINE SHL 16 ) + 320Ah
call HelperParameter16
lea rdi,[rdx + 3Fh]
push rcx rsi
mov eax,[rsi + 0Ch]
push rax
mov ax,STR_TP_CACHE_SIZE
call PoolStringWrite
mov bl,0
pop rax
call DecimalPrint32
pop rsi rcx
jmp .doneRelationEx 
;---------- Extended topology entry type: PACKAGE (CPU SOCKET) ----------------;
.relationPackageEx:
inc TEMP_SOCKETS
mov ax,STR_TP_CPU_PACKAGE
call HelperRelationName 
mov al,32
call HelperAffinityGroup
mov eax,( STR_ET_SMT SHL 16 ) + 2908h
call HelperParameter8
mov eax,( STR_ET_EFFICIENCY SHL 16 ) + 3209h
call HelperParameter8
jmp .doneRelationEx
;---------- Extended topology entry type: PROCESSOR GROUP ---------------------;
.relationGroupEx: 
mov ax,STR_ET_GROUP
call HelperRelationName 
mov al,32
call HelperAffinityMask
;---------- Extended topology entry detection done ----------------------------;
.doneRelationEx:
pop rdi
mov eax,[rsi + 04]
add rsi,rax
cmp rsi,rcx
jb .scanRelationsEx
pop r8 r9 rcx rsi
mov al,0
stosb
mov rbp,rdi
mov [DYNA_PTR.textTopologyEx1.objectStop],rbp
;---------- Support screen 0 threads, cores, sockets ( variables ) ------------;
lea rax,[SYS_PARMS.summaryTopology]
test r8d,r8d
jz @f
mov [rax + SUMMARYTOPOLOGY.threads],r8d
@@:
shr r8,32
jz @f
mov [rax + SUMMARYTOPOLOGY.cores],r8d
@@:
test r9d,r9d
jz @f
mov [rax + SUMMARYTOPOLOGY.sockets],r9d
@@:
.noTopologyEx:
;---------- Detect hybrid platform --------------------------------------------;
cmp [SYS_PARMS.perfCoreTopology.cores],0
je .rejectHybrid
cmp [SYS_PARMS.effCoreTopology.cores],0
je .rejectHybrid
mov [SYS_PARMS.hybridMode],1
jmp .doneHybrid
.rejectHybrid:
xor eax,eax
mov [SYS_PARMS.perfCoreTopology.cores],eax
mov [SYS_PARMS.perfCoreTopology.threads],eax
mov [SYS_PARMS.effCoreTopology.cores],eax
mov [SYS_PARMS.effCoreTopology.threads],eax
.doneHybrid:
;--- Build text block for extended topology screen, second block = summary ----;
mov [DYNA_PTR.textTopologyEx2.objectStart],rbp
mov rdi,rbp
mov ax,0101h                 ; Instruction cache , L1
call HelperCacheSummaryEx 
mov ax,0201h                 ; Data cache , L1
call HelperCacheSummaryEx 
mov ax,0002h                 ; Unified cache , L2
call HelperCacheSummaryEx 
mov ax,0003h                 ; Unified cache , L3
call HelperCacheSummaryEx 
mov ax,0004h                 ; Unified cache , L4
call HelperCacheSummaryEx 
mov al,0
stosb
mov rbp,rdi
mov [DYNA_PTR.textTopologyEx2.objectStop],rbp
ret

;---------- Get topological information ---------------------------------------;
;                                                                              ;
; INPUT:   RBX = WinAPI function address                                       ;
;          RBP = Pointer to buffer for topology information                    ;
;          R14 = Buffer used size accumulator                                  ;
;          R13 = Pointer to temporary DWORD variable, located at stack frame   ;
;          AL  = Relation type by WinAPI encoding                              ;
;                                                                              ;
; OUTPUT:  R14 = Updated buffer size accumulator                               ;
;                                                                              ;
;------------------------------------------------------------------------------;             
HelperTopologyEx:
push rsi rdi
mov rsi,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
movzx edi,al
mov dword [r13],0            ; dword [r13]  = Buffer size variable, stack
mov r8,r13                   ; R8  = Parm#3 = Pointer to size
lea rdx,[rbp + r14]          ; RDX = Parm#2 = Pointer to buffer
mov ecx,edi                  ; RCX = Parm#1 = Relation type
call rbx
test rax,rax              ; check RAX plus clear CF, required if return
jnz .exit  ;  jnz .error  ; this variant means no data return, but no errors
call [GetLastError]
cmp rax,ERROR_BUFFER_LIMIT
jne .error                   ; Go if other errors detected
mov r12d,[r13]
cmp r12d,VALUE_BUFFER_LIMIT
ja .error                    ; Go if required buffer size too big 
mov r8,r13                   ; R8  = Parm#3 = Pointer to size
lea rdx,[rbp + r14]          ; RDX = Parm#2 = Pointer to buffer
mov ecx,edi                  ; RCX = Parm#1 = Relation type
add r14d,r12d
cmp r14d,VALUE_BUFFER_LIMIT
ja .error                    ; Go if required buffer size too big 
call rbx
test rax,rax
jz .error
.exit:
mov rsp,rsi
pop rdi rsi
ret
.error:
stc
jmp .exit

;--- Write affinity mask string for text generation by topology parsing -------;
;                                                                              ;
; INPUT:   RDX = Destination base address                                      ;
;          RSI = Pointer to parsed topology entry: affinity mask               ;
;          AL  = Field offset in the parsed structure                          ; 
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperAffinityGroup:
push rsi
movzx eax,al
add rsi,rax
lea rdi,[rdx + 18]   ; RDI = Destination text pointer
lea r8,[rdi + 16]    ; R8  = Mask text format limit
mov ax,[rsi + 08]    ; AX  = Processor group for affinity mask
call HexPrint16
mov al,'.'
stosb
mov rax,[rsi + 00]   ; RAX = Affinity mask
call EntryAffinity
pop rsi
ret

;--- Write proc. group string for text generation by topology parsing ---------;
;                                                                              ;
; INPUT:   RDX = Destination base address                                      ;
;          RSI = Pointer to parsed topology entry: affinity mask               ;
;          AL  = Field offset in the parsed structure                          ; 
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperAffinityMask:
push rsi
movzx eax,al
add rsi,rax
lea rdi,[rdx + 18]
mov ax,[rsi + 08]
call HexPrint16
pop rsi
ret

;--- Write cache size and count string for text gener. by topology parsing ----;
;                                                                              ;
; INPUT:   RSI = Pointer to topology information buffer                        ;
;          RDI = Destination text buffer                                       ;
;          RCX = Topology information buffer address limit                     ;  
;          AL  = Cache level                                                   ;  
;          AH  = Cache type                                                    ;    
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperCacheSummaryEx:
push rcx rsi
test ecx,ecx
jz .nodata
movzx ebx,ah
xchg edx,eax
xor r8d,r8d
xor r9d,r9d
;---------- Detect hybrid mode ------------------------------------------------;
cmp [SYS_PARMS.hybridMode],1
je .hybridMode
;--- Scan topology list for cache object, variant for non-hybrid CPU ----------;
.scanCaches:              ; This cycle for count number of cache units
cmp dword [rsi + 00],2
jne .done                 ; Go skip if descriptor type mismatch
cmp byte [rsi + 08],dl
jne .done                 ; Go skip if cache level mismatch
cmp dword [rsi + 16],ebx
jne .done                 ; Go skip if cache type mismatch
mov r8d,[rsi + 12]        ; R8D = cache size
inc r9d                   ; R9D, cache units count + 1
.done:
mov eax,[rsi + 04]
add rsi,rax
cmp rsi,rcx
jb .scanCaches            ; This cycle for count number cache units
mov dh,0                  ; Hybrid status = 0, none
call HelperCacheSummarySaveEx
jmp .nodata
;--- Scan topology list for cache object, variant for hybrid CPU --------------;
.hybridMode:
push rcx rsi
.scanPEcores:             ; This cycle for count number of cache units
cmp dword [rsi + 00],2
jne .donePEcores          ; Go skip if descriptor type mismatch
cmp byte [rsi + 08],dl
jne .donePEcores          ; Go skip if cache level mismatch
cmp dword [rsi + 16],ebx
jne .donePEcores          ; Go skip if cache type mismatch
mov rax,[rsi + 40]
test rax,[SYS_PARMS.perfCoreAffinity]
jz .donePEcores           ; Go skip if no P-Core affinity, here required both  
test rax,[SYS_PARMS.effCoreAffinity]
jz .donePEcores           ; Go skip if no E-Core affinity, here required both   
mov r8d,[rsi + 12]        ; R8D = cache size
inc r9d                   ; R9D, cache units count + 1
.donePEcores:
mov eax,[rsi + 04]
add rsi,rax
cmp rsi,rcx
jb .scanPEcores           ; This cycle for count number cache units
mov dh,0                  ; Hybrid status = 0, None
call HelperCacheSummarySaveEx
pop rsi rcx
xor r8d,r8d
xor r9d,r9d
push rcx rsi
.scanPcores:              ; This cycle for count number of cache units
cmp dword [rsi + 00],2
jne .donePcores           ; Go skip if descriptor type mismatch
cmp byte [rsi + 08],dl
jne .donePcores           ; Go skip if cache level mismatch
cmp dword [rsi + 16],ebx
jne .donePcores           ; Go skip if cache type mismatch
mov rax,[rsi + 40]
test rax,[SYS_PARMS.perfCoreAffinity]
jz .donePcores            ; Go skip if not a P-Core cache unit 
test rax,[SYS_PARMS.effCoreAffinity]
jnz .donePcores           ; Go skip if non-hybrid (for P+E both) cache unit 
mov r8d,[rsi + 12]        ; R8D = cache size
inc r9d                   ; R9D, cache units count + 1
.donePcores:
mov eax,[rsi + 04]
add rsi,rax
cmp rsi,rcx
jb .scanPcores            ; This cycle for count number cache units
mov dh,1                  ; Hybrid status = 1, P-Cores
call HelperCacheSummarySaveEx
pop rsi rcx
xor r8d,r8d
xor r9d,r9d
.scanEcores:              ; This cycle for count number of cache units
cmp dword [rsi + 00],2
jne .doneEcores           ; Go skip if descriptor type mismatch
cmp byte [rsi + 08],dl
jne .doneEcores           ; Go skip if cache level mismatch
cmp dword [rsi + 16],ebx
jne .doneEcores           ; Go skip if cache type mismatch
mov rax,[rsi + 40]
test rax,[SYS_PARMS.effCoreAffinity]
jz .doneEcores            ; Go skip if not a E-Core cache unit 
test rax,[SYS_PARMS.perfCoreAffinity]
jnz .doneEcores           ; Go skip if non-hybrid (for P+E both) cache unit 
mov r8d,[rsi + 12]        ; R8D = cache size
inc r9d                   ; R9D, cache units count + 1
.doneEcores:
mov eax,[rsi + 04]
add rsi,rax
cmp rsi,rcx
jb .scanEcores            ; This cycle for count number cache units
mov dh,2                  ; Hybrid status = 2, E-Cores
call HelperCacheSummarySaveEx
.nodata:
pop rsi rcx
ret

;--- Helper for add cache to text report and update size and count variables --; 
;                                                                              ; 
; INPUT:   RDI  = Destination text buffer                                      ;
;          DL   = Cache level by WinAPI                                        ;
;          BL   = Cache type code by WinAPI                                    ;
;          DH   = Hybrid status: 0 = none, 1 = P-Cores, 2 = E-Cores            ;
;          R8   = Cache size value, if 0, operation skipped                    ;
;          R9D  = Cache count value                                            ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperCacheSummarySaveEx:
push rbx rdx
test r8d,r8d
jz .nodata
movzx edx,dx
movzx ebx,bl     ; EBX[07-00] = BL = Cache type
shl edx,8
or ebx,edx       ; EBX[23-16] = Hybrid status, EBX[15-08] = BH = Cache level
mov rdx,rdi
cld
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
push rdi
lea rdi,[rdx + 01]
mov ax,STR_TP_L
call PoolStringWrite
mov al,bh
or al,30h
stosb
movzx ax,bl
add ax,STR_TP_UNIFIED
lea rdi,[rdx + 04] 
call RelationNameEntry
lea rdi,[rdx + 18]
mov bl,0FFh
mov eax,r8d                  ; R8D = Cache size for write size x count
call SizePrint64
lea rdi,[rdx + 41]
mov ax,STR_TP_X
call PoolStringWrite
mov eax,r9d                  ; R9D = Cache count for write size x count
mov bl,0
call DecimalPrint32
lea rdi,[rdx + 55]
shr ebx,16                   ; BL = Hybrid status, shift from EBX[23-16]
mov ax,STR_P_CORES
cmp bl,1
je @f
mov ax,STR_E_CORES
cmp bl,2
je @f
mov ax,STR_HYPHEN
@@:
call PoolStringWrite
pop rdi
;---------- additions for screen 0 --------------------------------------------;
mov al,[rsp + 00]
mov ah,[rsp + 08] 
dec bl
jz .perfCache
dec bl
jz .effCache
call HelperCacheSummaryBinary
jmp .nodata
.perfCache:
call HelperCachePerfCoreBinary
jmp .nodata
.effCache:
call HelperCacheEffCoreBinary
.nodata:
pop rdx rbx
ret
