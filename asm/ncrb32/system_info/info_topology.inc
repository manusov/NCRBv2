;---------- Get Topology information ------------------------------------------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoTopology:
push eax                    ; for create variable
call HelperAlignment
mov ebx,[DYNA_IMPORT._GetLogicalProcessorInformation]
test ebx,ebx
jz .exit
mov dword [esp],0            ; dword [esp]  = Buffer size variable, stack
mov eax,esp
push eax                     ; Parm#2 = Pointer to buffer size variable 
push ebp                     ; Parm#1 = Pointer to buffer 
call ebx
test eax,eax
jnz .error                   ; Go if unexpected error: zero-sized buffer
call [GetLastError]
cmp eax,ERROR_BUFFER_LIMIT
jne .error                   ; Go if other errors detected
mov edi,[esp]
cmp edi,VALUE_BUFFER_LIMIT
ja .error                    ; Go if required buffer size too big
mov eax,esp 
push eax                     ; Parm#2 = Pointer to buffer size variable 
push ebp                     ; Parm#1 = Pointer to buffer 
call ebx
test eax,eax
jz .error                    ; Go if unexpected error: zero-sized buffer
mov [DYNA_PTR.listTopology.objectStart],ebp
add ebp,edi
mov [DYNA_PTR.listTopology.objectStop],ebp
.exit:
pop eax     ; This POP for remove stack variable
clc
ret
.error:
pop eax     ; This POP for remove stack variable
mov al,MSG_ERROR_TOPOLOGY_API
stc
ret

;--- Build Platform topology information for text report and GUI widgets ------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextTopology:
;
TEMP_THREADS   EQU  dword [esp + 00 + 04]
TEMP_CORES     EQU  dword [esp + 04 + 04]
TEMP_SOCKETS   EQU  dword [esp + 08 + 04]
;
mov [DYNA_PTR.textTopology1.objectStart],ebp
mov ebx,[DYNA_PTR.listTopology.objectStart]
mov eax,[DYNA_PTR.listTopology.objectStop]
sub eax,ebx
mov ecx,24
xor edx,edx
div ecx
test edx,edx
jnz .error
xchg ecx,eax
test ecx,ecx
jz .exit                 ; jz .error
mov edi,ebp
push ebx ecx esi 0 0 0   ; Include local variables
mov esi,ebx
.scanRelations:
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
mov eax,[esi + 04]
cmp eax,3
ja .relationUnknown
je .relationPackage
cmp al,2
je .relationCache
cmp al,1
je .relationNuma
cmp al,0
je .relationCore
.relationUnknown:
mov ax,STR_TP_UNKNOWN_ID
call HelperRelationName 
jmp .doneRelation
.relationCore:
inc TEMP_CORES
inc TEMP_THREADS
cmp byte [esi + 8],1
jne @f
inc TEMP_THREADS
@@:
mov ax,STR_TP_CPU_CORE
call HelperRelationName 
call HelperAffinity
mov eax,( STR_TP_HT SHL 16 ) + 2908h
call HelperParameter8
jmp .doneRelation
.relationNuma: 
mov ax,STR_TP_NUMA_NODE
call HelperRelationName 
call HelperAffinity
mov eax,( STR_TP_NODE SHL 16 ) + 2908h
call HelperParameter32
jmp .doneRelation
.relationCache:
mov ax,STR_TP_L
call HelperRelationName
mov al,[esi + 08]
cmp al,1
jb .levelBad
cmp al,4
ja .levelBad
or al,30h
jmp .levelStore
.levelBad:
mov al,'?'
.levelStore:
mov ah,' '
stosw
mov eax,[esi + 16]
cmp eax,3
ja .typeBad
add ax,STR_TP_UNIFIED
jmp .typeStore
.typeBad:
mov ax,STR_TP_UNKNOWN_TYPE
.typeStore:
lea edi,[edx + 04] 
call RelationNameEntry
call HelperAffinity
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
jmp .doneRelation 
.relationPackage:
inc TEMP_SOCKETS
mov ax,STR_TP_CPU_PACKAGE
call HelperRelationName 
call HelperAffinity
.doneRelation:
add esi,24
pop edi
dec ecx
jnz .scanRelations
;---------- Support screen 0 threads, cores, sockets --------------------------;
mov esi,[esp + 12]
lea ebx,[SYS_PARMS.summaryTopology]
pop dword [ebx + SUMMARYTOPOLOGY.threads]
pop dword [ebx + SUMMARYTOPOLOGY.cores]
pop dword [ebx + SUMMARYTOPOLOGY.sockets]
;---------- End of support screen 0 threads, cores, sockets -------------------;
pop esi ecx ebx
mov al,0
stosb
mov ebp,edi
mov [DYNA_PTR.textTopology1.objectStop],ebp
;---------- Build text block for topology screen, second block = summary ------;
mov [DYNA_PTR.textTopology2.objectStart],ebp
push esi
mov edi,ebp
mov esi,ebx
mov ax,0101h                 ; Instruction cache , L1
call HelperCacheSummary 
mov ax,0201h                 ; Data cache , L1
call HelperCacheSummary 
mov ax,0002h                 ; Unified cache , L2
call HelperCacheSummary 
mov ax,0003h                 ; Unified cache , L3
call HelperCacheSummary 
mov ax,0004h                 ; Unified cache , L4
call HelperCacheSummary 
mov al,0
stosb
mov ebp,edi
pop esi
.exit:
mov [DYNA_PTR.textTopology2.objectStop],ebp
clc
ret
.error:
mov al,MSG_ERROR_TOPOLOGY_API
stc
ret

;---------- Align memory pointer, required for WinAPI calls -------------------;
; INPUT:   EBP = Pointer                                                       ;
; OUTPUT:  EBP = Aligned pointer, updated if required                          ;
;------------------------------------------------------------------------------;
HelperAlignment:
test bp,00001111b
jz @f
and ebp,0FFFFFFF0h
add ebp,010h
@@: 
ret

;--- Write relation name string for text generation by topology parsing -------;
; INPUT:   RDX = Destination base address                                      ;
;          AX  = String ID at application strings pool                         ;
; OUTPUT:  None                                                                ;
;------------------------------------------------------------------------------;
HelperRelationName:
lea edi,[edx + 01]
RelationNameEntry:
push ecx esi
call PoolStringWrite
pop esi ecx
ret

;--- Write affinity mask string for text generation by topology parsing -------;
; INPUT:   EDX = Destination base address                                      ;
;          ESI = Pointer to parsed topology entry: affinity mask               ;
; OUTPUT:  None                                                                ;
;------------------------------------------------------------------------------;
HelperAffinity:
push ecx edx esi ebp
lea edi,[edx + 18]   ; EDI = Destination text pointer
mov eax,[esi + 00]   ; EAX = Affinity mask
lea ebp,[edi + 16]   ; EBP  = Mask text format limit
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
pop ebp esi edx ecx
ret

;--- Write number value string for text generation by topology parsing --------;
; Has 3 entry points depend on parameter width: 8, 16, 32 bits                 ;
; INPUT:   EDX = Destination base address                                      ;
;          ESI = Pointer to parsed topology entry                              ;
;          AL          = Parameter offset in the topology entry structure      ; 
;          AH          = Text string position offset at destination            ;   
;          EAX.[31-16] = Parameter name String ID at application strings pool  ;  
; OUTPUT:  None                                                                ;
;------------------------------------------------------------------------------;
HelperParameter8:         ; Entry point for addressed 8-bit parameter
shld ebx,eax,16
movzx edi,ah
movzx eax,al
movzx eax,byte [esi + eax]
ParameterEntry:
push ecx esi
lea edi,[edx + edi]
push eax
xchg eax,ebx
call PoolStringWrite
pop eax
mov bl,0
call DecimalPrint32
pop esi ecx
ret
HelperParameter16:         ; Same entry point for addressed 16-bit parameter
shld ebx,eax,16
movzx edi,ah
movzx eax,al
movzx eax,word [esi + eax]
jmp ParameterEntry
HelperParameter32:         ; Same entry point for addressed 32-bit parameter
shld ebx,eax,16
movzx edi,ah
movzx eax,al
mov eax,dword [esi + eax]
jmp ParameterEntry

;--- Write cache size and count string for text gener. by topology parsing ----;
; Has 3 entry points depend on parameter width: 8, 16, 32 bits                 ;
; INPUT:   ESI = Pointer to topology information buffer                        ;
;          EDI = Destination text buffer                                       ;
;          ECX = Entries count at topology information buffer                  ;  
;          AL  = Cache level                                                   ;  
;          AH  = Cache type                                                    ;    
; OUTPUT:  None                                                                ;
;------------------------------------------------------------------------------;
HelperCacheSummary:
push ecx esi 0 0 ebp
mov ebp,esp
push eax
test ecx,ecx
jz .nodata
movzx ebx,ah
xchg edx,eax
.scanCaches:
cmp dword [esi + 04],2
jne .done
cmp byte [esi + 08],dl
jne .done
cmp dword [esi + 16],ebx
jne .done
mov eax,[esi + 12]
mov [ebp + 04],eax
inc dword [ebp + 08]
.done:
add esi,24
loop .scanCaches
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
pop eax
push eax
call HelperSummaryCache
.nodata:
pop eax ebp eax eax esi ecx
ret

;--- Helper for write cache size and count variables by topology WinAPI -------;
;                                                                              ; 
; INPUT:   AX = Cache level and type code                                      ;
;          dword [EBP + 4] = Cache size value                                  ;
;          dword [EBP + 8] = Cache count value                                 ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperSummaryCache:
lea ebx,[SYS_PARMS.summaryCache]
lea esi,[ebx + SUMMARYCACHE.sizeL1C]
cmp ax,0101h
je .detected 
lea esi,[ebx + SUMMARYCACHE.sizeL1D]
cmp ax,0201h
je .detected 
lea esi,[ebx + SUMMARYCACHE.sizeL2U]
cmp ax,0002h
je .detected 
lea esi,[ebx + SUMMARYCACHE.sizeL3U]
cmp ax,0003h
je .detected 
lea esi,[ebx + SUMMARYCACHE.sizeL4U]
cmp ax,0004h
je .detected 
jmp .nodata
.detected:
mov eax,[ebp + 08]
mov [esi + 8],eax
mov eax,[ebp + 04]
xor edx,edx
mov [esi + 0],eax
mov [esi + 4],edx
mov bl,0FFh
mov eax,[ebp + 04]
.nodata:
ret
