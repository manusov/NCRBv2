;---------- Get Extended Topology information ---------------------------------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
; Separate calls for 5 entries types (0-4) required for ordering
; text strings in the report.
InfoExtendedTopology:
call HelperAlignment
mov ebx,[DYNA_IMPORT._GetLogicalProcessorInformationEx]
test ebx,ebx
jz .exit
xor edi,edi
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
mov [DYNA_PTR.listTopologyEx.objectStart],ebp
add ebp,edi
mov [DYNA_PTR.listTopologyEx.objectStop],ebp
.exit:
clc
ret
.error:
mov al,MSG_ERROR_TOPOLOGY_API
stc
ret

;--- Build Platform ext. topology inform. for text report and GUI widgets -----;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;

; This temporary variables required for parameters, previously detected by
; standard topology (InfoTopology, TextTopology subroutines), because standard
; topology parameters must be not changed  if extended topology
; (InfoExtendedTopology, TextExtendedTopology subroutines) not supported. 
TEMP_THREADS   EQU  dword [esp + 00 + 04]
TEMP_CORES     EQU  dword [esp + 04 + 04]
TEMP_SOCKETS   EQU  dword [esp + 08 + 04]

TextExtendedTopology:
;--- Build text block for ext. topology screen, first block = objects list ----;
mov [DYNA_PTR.textTopologyEx1.objectStart],ebp
mov ebx,[DYNA_PTR.listTopologyEx.objectStart]
mov ecx,[DYNA_PTR.listTopologyEx.objectStop]
test ecx,ecx
jz .noTopologyEx
mov edi,ebp
push ebx ecx esi 0 0 0
mov esi,ebx
.scanRelationsEx:
mov edx,edi
push ecx
cld
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
pop ecx
push edi
mov eax,[esi + 00]
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
cmp byte [esi + 08],1
jne @f
inc TEMP_THREADS
@@:
mov eax,SYS_PARMS.perfCoreTopology     ; Support hybrid cores
mov edi,SYS_PARMS.perfCoreAffinity
cmp byte [esi + 09],1
je @f
mov eax,SYS_PARMS.effCoreTopology
mov edi,SYS_PARMS.effCoreAffinity
@@:
inc [eax + SUMMARYTOPOLOGY.cores]
inc [eax + SUMMARYTOPOLOGY.threads]
cmp byte [esi + 08],1
jne @f
inc [eax + SUMMARYTOPOLOGY.threads]
@@:
mov eax,[esi + 32]
or [edi],eax                           ; Store affinity mask
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
mov al,[esi + 08]
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
mov eax,[esi + 16]
cmp eax,3
ja .typeBadEx
add ax,STR_TP_UNIFIED
jmp .typeStoreEx
.typeBadEx:
mov ax,STR_TP_UNKNOWN_TYPE
.typeStoreEx:
lea edi,[edx + 04] 
call RelationNameEntry
mov al,40
call HelperAffinityGroup
mov eax,( STR_TP_CACHE_WAYS SHL 16 ) + 2909h
call HelperParameter8
mov eax,( STR_TP_CACHE_LINE SHL 16 ) + 320Ah
call HelperParameter16
lea edi,[edx + 3Fh]
push ecx esi
mov eax,[esi + 0Ch]
push eax
mov ax,STR_TP_CACHE_SIZE
call PoolStringWrite
mov bl,0
pop eax
call DecimalPrint32
pop esi ecx
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
pop edi
add esi,[esi + 04]
cmp esi,ecx
jb .scanRelationsEx
;---------- Support screen 0 threads, cores, sockets ( variables ) ------------;
mov esi,[esp + 12]
mov ebx,SYS_PARMS.summaryTopology
pop ecx
jecxz @f
mov [ebx + SUMMARYTOPOLOGY.threads],ecx
@@:
pop ecx
jecxz @f
mov [ebx + SUMMARYTOPOLOGY.cores],ecx
@@:
pop ecx
jecxz @f
mov [ebx + SUMMARYTOPOLOGY.sockets],ecx
@@:
;---------- End of support screen 0 threads, cores, sockets -------------------;
pop esi ecx ebx
mov al,0
stosb
mov ebp,edi
mov [DYNA_PTR.textTopologyEx1.objectStop],ebp
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
mov [DYNA_PTR.textTopologyEx2.objectStart],ebp
push esi
mov edi,ebp
mov esi,ebx
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
mov ebp,edi
pop esi
mov [DYNA_PTR.textTopologyEx2.objectStop],ebp
ret

;---------- Get topological information ---------------------------------------;
;                                                                              ;
; INPUT:   EBX = WinAPI function address                                       ;
;          EBP = Pointer to buffer for topology information                    ;
;          EDI = Buffer used size accumulator                                  ;
;          AL  = Relation type by WinAPI encoding                              ;
;                                                                              ;
; OUTPUT:  EDI = Updated buffer size accumulator                               ;
;                                                                              ;
;------------------------------------------------------------------------------;             
HelperTopologyEx:
push ebx esi eax 0
mov eax,esp
push eax                     ; Parm#3 = Pointer to size
lea eax,[ebp + edi]
push eax                     ; Parm#2 = Pointer to buffer
movzx eax,byte [esp + 12]
push eax                     ; Parm#1 = Relation type
call ebx
test eax,eax              ; check EAX plus clear CF, required if return
jnz .exit  ;  jnz .error  ; this variant means no data return, but no errors
call [GetLastError]
cmp eax,ERROR_BUFFER_LIMIT
jne .error                   ; Go if other errors detected
mov edx,[esp]
cmp edx,VALUE_BUFFER_LIMIT
ja .error                    ; Go if required buffer size too big 
lea ecx,[ebp + edi]
add edi,edx
cmp edi,VALUE_BUFFER_LIMIT
ja .error                    ; Go if required buffer size too big 
mov eax,esp
push eax                     ; Parm#3 = Pointer to size
push ecx                     ; Parm#2 = Pointer to buffer
movzx eax,byte [esp + 12]
push eax                     ; Parm#1 = Relation type
call ebx
test eax,eax
jz .error
.exit:
pop eax eax esi ebx
ret
.error:
stc
jmp .exit

;--- Write affinity mask string for text generation by topology parsing -------;
;                                                                              ;
; INPUT:   EDX = Destination base address                                      ;
;          ESI = Pointer to parsed topology entry: affinity mask               ;
;          AL  = Field offset in the parsed structure                          ; 
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperAffinityGroup:    ; TODO. Optimize by EntryAffinity
push esi ebp
movzx eax,al
add esi,eax
lea edi,[edx + 18]
lea ebp,[edi + 16]
mov ax,[esi + 04]
call HexPrint16
mov al,'.'
stosb
push ecx edx
mov eax,[esi + 00]
bsf ecx,eax
bsr edx,eax
cmp ecx,edx
je .modeSingle
push ecx edx
.scanMask:
bt eax,ecx
jz .endMask
inc ecx
cmp ecx,edx
jb .scanMask
.endMask:
cmp ecx,edx
pop edx ecx
je .modeInterval
.modeIndividual:
xor ecx,ecx
xor edx,edx
.cycleIndividual:
cmp edi,ebp
jae .overIndividual
shr eax,1
jnc .skipIndividual
push eax
test edx,edx
jz .firstIndividual
mov al,','
stosb
.firstIndividual:
inc edx
mov eax,ecx
mov bl,0
call DecimalPrint32
pop eax
.skipIndividual:
inc ecx
cmp cl,64
jb .cycleIndividual
jmp .done
.overIndividual:
mov ax,STR_TP_POINTS
call PoolStringWrite
jmp .done
.modeInterval:
xchg eax,ecx
mov bl,0
call DecimalPrint32 
mov al,'-'
stosb
.modeSingle:
xchg eax,edx
mov bl,0
call DecimalPrint32
.done:
pop edx ecx ebp esi
ret

;--- Write proc. group string for text generation by topology parsing ---------;
;                                                                              ;
; INPUT:   EDX = Destination base address                                      ;
;          ESI = Pointer to parsed topology entry: affinity mask               ;
;          AL  = Field offset in the parsed structure                          ; 
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperAffinityMask:
push esi
movzx eax,al
add esi,eax
lea edi,[edx + 18]
mov ax,[esi + 08]
call HexPrint16
pop esi
ret

;--- Write cache size and count string for text gener. by topology parsing ----;
;                                                                              ;
; INPUT:   ESI = Pointer to topology information buffer                        ;
;          EDI = Destination text buffer                                       ;
;          ECX = Topology information buffer address limit                     ;  
;          AL  = Cache level                                                   ;  
;          AH  = Cache type                                                    ;    
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperCacheSummaryEx:
push ecx esi 0 0 ebp
mov ebp,esp
test ecx,ecx
jz .nodata
movzx ebx,ah
xchg edx,eax
;---------- Detect hybrid mode ------------------------------------------------;
cmp [SYS_PARMS.hybridMode],1
je .hybridMode
;--- Scan topology list for cache object, variant for non-hybrid CPU ----------;
.scanCaches:              ; This cycle for count number of cache units
cmp dword [esi + 00],2
jne .done                 ; Go skip if descriptor type mismatch
cmp byte [esi + 08],dl
jne .done                 ; Go skip if cache level mismatch
cmp dword [esi + 16],ebx
jne .done                 ; Go skip if cache type mismatch
mov eax,[esi + 12]        ; EAX = cache size
mov [ebp + 04],eax
inc dword [ebp + 08]      ; Cache units count + 1
.done:
mov eax,[esi + 04]
add esi,eax
cmp esi,ecx
jb .scanCaches            ; This cycle for count number cache units
mov dh,0                  ; Hybrid status = 0, none
call HelperCacheSummarySaveEx
jmp .nodata
;--- Scan topology list for cache object, variant for hybrid CPU --------------;
.hybridMode:
push ecx esi
.scanPEcores:             ; This cycle for count number of cache units
cmp dword [esi + 00],2
jne .donePEcores          ; Go skip if descriptor type mismatch
cmp byte [esi + 08],dl
jne .donePEcores          ; Go skip if cache level mismatch
cmp dword [esi + 16],ebx
jne .donePEcores          ; Go skip if cache type mismatch
mov eax,[esi + 40]
test eax,[SYS_PARMS.perfCoreAffinity]
jz .donePEcores           ; Go skip if no P-Core affinity, here required both 
test eax,[SYS_PARMS.effCoreAffinity]
jz .donePEcores           ; Go skip if no E-Core affinity, here required both  
mov eax,[esi + 12]        ; EAX = cache size
mov [ebp + 04],eax
inc dword [ebp + 08]      ; Cache units count + 1
.donePEcores:
mov eax,[esi + 04]
add esi,eax
cmp esi,ecx
jb .scanPEcores           ; This cycle for count number cache units
mov dh,0                  ; Hybrid status = 0, None
call HelperCacheSummarySaveEx
pop esi ecx
mov dword [ebp + 04],0
mov dword [ebp + 08],0
push ecx esi
.scanPcores:              ; This cycle for count number of cache units
cmp dword [esi + 00],2
jne .donePcores           ; Go skip if descriptor type mismatch
cmp byte [esi + 08],dl
jne .donePcores           ; Go skip if cache level mismatch
cmp dword [esi + 16],ebx
jne .donePcores           ; Go skip if cache type mismatch
mov eax,[esi + 40]
test eax,[SYS_PARMS.perfCoreAffinity]
jz .donePcores            ; Go skip if not a P-Core cache unit 
test eax,[SYS_PARMS.effCoreAffinity]
jnz .donePcores           ; Go skip if non-hybrid (for P+E both) cache unit 
mov eax,[esi + 12]        ; EAX = cache size
mov [ebp + 04],eax
inc dword [ebp + 08]      ; Cache units count + 1
.donePcores:
mov eax,[esi + 04]
add esi,eax
cmp esi,ecx
jb .scanPcores            ; This cycle for count number cache units
mov dh,1                  ; Hybrid status = 1, P-Cores
call HelperCacheSummarySaveEx
pop esi ecx
mov dword [ebp + 04],0
mov dword [ebp + 08],0
.scanEcores:              ; This cycle for count number of cache units
cmp dword [esi + 00],2
jne .doneEcores           ; Go skip if descriptor type mismatch
cmp byte [esi + 08],dl
jne .doneEcores           ; Go skip if cache level mismatch
cmp dword [esi + 16],ebx
jne .doneEcores           ; Go skip if cache type mismatch
mov eax,[esi + 40]
test eax,[SYS_PARMS.effCoreAffinity]
jz .doneEcores            ; Go skip if not a P-Core cache unit 
test eax,[SYS_PARMS.perfCoreAffinity]
jnz .doneEcores           ; Go skip if non-hybrid (for P+E both) cache unit 
mov eax,[esi + 12]        ; EAX = cache size
mov [ebp + 04],eax
inc dword [ebp + 08]      ; Cache units count + 1
.doneEcores:
mov eax,[esi + 04]
add esi,eax
cmp esi,ecx
jb .scanEcores            ; This cycle for count number cache units
mov dh,2                  ; Hybrid status = 2, E-Cores
call HelperCacheSummarySaveEx
.nodata:
pop ebp eax eax esi ecx
ret

;--- Helper for add cache to text report and update size and count variables --; 
;                                                                              ; 
; INPUT:   EDI = Destination text buffer                                       ;
;          DL  = Cache level by WinAPI                                         ;
;          BL  = Cache type code by WinAPI                                     ;
;          DH  = Hybrid status: 0 = none, 1 = P-Cores, 2 = E-Cores             ;
;          EBP = Pointer to input parameters                                   ;
;          DWORD [EBP + 04] = Cache size value, if 0, operation skipped        ;
;          DWORD [EBP + 08] = Cache count value                                ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperCacheSummarySaveEx:
push ebx edx
cmp dword [ebp + 08],0
je .nodata
movzx edx,dx
movzx ebx,bl     ; EBX[07-00] = BL = Cache type
shl edx,8
or ebx,edx       ; EBX[23-16] = Hybrid status, EBX[15-08] = BH = Cache level
mov edx,edi
cld
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
push edi
lea edi,[edx + 01]
mov ax,STR_TP_L
call PoolStringWrite
mov al,bh
or al,30h
stosb
movzx ax,bl
add ax,STR_TP_UNIFIED
lea edi,[edx + 04] 
call RelationNameEntry
lea edi,[edx + 18]
mov bl,0FFh
push edx
mov eax,[ebp + 04]     ; dword [ebp + 04] = Cache size for write size x count
xor edx,edx
call SizePrint64
pop edx
lea edi,[edx + 41]
mov ax,STR_TP_X
call PoolStringWrite
mov eax,[ebp + 08]     ; dword [ebp + 08] = Cache count for write size x count
mov bl,0
call DecimalPrint32
lea edi,[edx + 55]
shr ebx,16             ; BL = Hybrid status, shift from EBX[23-16]
mov ax,STR_P_CORES
cmp bl,1
je @f
mov ax,STR_E_CORES
cmp bl,2
je @f
mov ax,STR_HYPHEN
@@:
call PoolStringWrite
pop edi
;---------- additions for screen 0 --------------------------------------------;
mov al,[esp + 00]
mov ah,[esp + 04] 
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
pop edx ebx
ret
