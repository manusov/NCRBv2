;---------- Get Extended Topology information ---------------------------------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
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
TextExtendedTopology:
;
TEMP_THREADS   EQU  dword [esp + 00 + 04]
TEMP_CORES     EQU  dword [esp + 04 + 04]
TEMP_SOCKETS   EQU  dword [esp + 08 + 04]
;
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
.relationUnknownEx:
mov ax,STR_TP_UNKNOWN_ID
call HelperRelationName 
jmp .doneRelationEx
.relationCoreEx:
inc TEMP_CORES
inc TEMP_THREADS
cmp byte [esi + 8],1
jne @f
inc TEMP_THREADS
@@:
mov ax,STR_TP_CPU_CORE
call HelperRelationName 
mov al,32
call HelperAffinityGroup
mov eax,( STR_ET_SMT SHL 16 ) + 2908h
call HelperParameter8
mov eax,( STR_ET_EFFICIENCY SHL 16 ) + 3209h
call HelperParameter8
jmp .doneRelationEx
.relationNumaEx: 
mov ax,STR_TP_NUMA_NODE
call HelperRelationName 
mov al,32
call HelperAffinityGroup
mov eax,( STR_TP_NODE SHL 16 ) + 2908h
call HelperParameter32
jmp .doneRelationEx
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
.relationGroupEx: 
mov ax,STR_ET_GROUP
call HelperRelationName 
mov al,32
call HelperAffinityMask
.doneRelationEx:
pop edi
add esi,[esi + 04]
cmp esi,ecx
jb .scanRelationsEx
;---------- Support screen 0 threads, cores, sockets ( variables ) ------------;
mov esi,[esp + 12]
lea ebx,[SYS_PARMS.summaryTopology]
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
; INPUT:   EBX = WinAPI function address                                       ;
;          EBP = Pointer to buffer for topology information                    ;
;          EDI = Buffer used size accumulator                                  ;
;          AL  = Relation type by WinAPI encoding                              ;
; OUTPUT:  EDI = Updated buffer size accumulator                               ;
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
test eax,eax
jnz .error
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
; INPUT:   EDX = Destination base address                                      ;
;          ESI = Pointer to parsed topology entry: affinity mask               ;
;          AL  = Field offset in the parsed structure                          ; 
; OUTPUT:  None                                                                ;
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
; INPUT:   EDX = Destination base address                                      ;
;          ESI = Pointer to parsed topology entry: affinity mask               ;
;          AL  = Field offset in the parsed structure                          ; 
; OUTPUT:  None                                                                ;
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
; Has 3 entry points depend on parameter width: 8, 16, 32 bits                 ;
; INPUT:   ESI = Pointer to topology information buffer                        ;
;          EDI = Destination text buffer                                       ;
;          ECX = Topology information buffer address limit                     ;  
;          AL  = Cache level                                                   ;  
;          AH  = Cache type                                                    ;    
; OUTPUT:  None                                                                ;
;------------------------------------------------------------------------------;
HelperCacheSummaryEx:
push ecx esi 0 0 ebp
mov ebp,esp
push eax
test ecx,ecx
jz .nodata
movzx ebx,ah
xchg edx,eax
.scanCaches:
cmp dword [esi + 00],2
jne .done
cmp byte [esi + 08],dl
jne .done
cmp dword [esi + 16],ebx
jne .done
mov eax,[esi + 12]
mov [ebp + 04],eax
inc dword [ebp + 08]
.done:
mov eax,[esi + 04]
add esi,eax
cmp esi,ecx
jb .scanCaches
cmp dword [ebp + 08],0
je .nodata
mov bh,dl
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
mov eax,[ebp + 04]
xor edx,edx
call SizePrint64
pop edx
lea edi,[edx + 41]
mov ax,STR_TP_X
call PoolStringWrite
mov eax,[ebp + 08]
mov bl,0
call DecimalPrint32
pop edi
;---------- additions for screen 0 --------------------------------------------;
pop eax
push eax
call HelperSummaryCache
.nodata:
pop eax ebp eax eax esi ecx
ret

