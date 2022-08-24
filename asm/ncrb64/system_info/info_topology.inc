;---------- Get Topology information ------------------------------------------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoTopology:
push 0
mov r13,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
call HelperAlignment
mov rbx,[DYNA_IMPORT._GetLogicalProcessorInformation]
test rbx,rbx
jz .exit
mov dword [r13],0            ; dword [r13]  = Buffer size variable, stack
mov rdx,r13                  ; RDX = Parm#2 = Pointer to buffer size variable 
mov rcx,rbp                  ; RCX = Parm#1 = Pointer to buffer 
call rbx
test rax,rax
jnz .error                   ; Go if unexpected error: zero-sized buffer
call [GetLastError]
cmp rax,ERROR_BUFFER_LIMIT
jne .error                   ; Go if other errors detected
mov r14d,[r13]
cmp r14d,VALUE_BUFFER_LIMIT
ja .error                    ; Go if required buffer size too big 
mov rdx,r13                  ; RDX = Parm#2 = Pointer to buffer size variable 
mov rcx,rbp                  ; RCX = Parm#1 = Pointer to buffer 
call rbx
test rax,rax
jz .error                    ; Go if unexpected error: zero-sized buffer
mov [DYNA_PTR.listTopology.objectStart],rbp
add rbp,r14
mov [DYNA_PTR.listTopology.objectStop],rbp
clc
.exit:
mov rsp,r13
pop rcx     ; This POP for remove stack variable
ret
.error:
mov al,MSG_ERROR_TOPOLOGY_API
stc
jmp .exit 

;--- Build Platform topology information for text report and GUI widgets ------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;

TEMP_THREADS   EQU  dword [rsp + 00 + 08]
TEMP_CORES     EQU  dword [rsp + 04 + 08]
TEMP_SOCKETS   EQU  dword [rsp + 08 + 08]
TEMP_RESERVED  EQU  dword [rsp + 12 + 08]   

TextTopology:
;---------- Build text block for topology screen, first block = objects list --;
mov [DYNA_PTR.textTopology1.objectStart],rbp
mov rsi,[DYNA_PTR.listTopology.objectStart]
mov rcx,[DYNA_PTR.listTopology.objectStop]
sub rcx,rsi
test cl,00011111b
jnz .error
shr ecx,5
jz .exit          ; jz .error
push rsi rcx 0 0  ; Include local variables
mov rdi,rbp
.scanRelations:
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
mov eax,[rsi + 08]
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
cmp byte [rsi + 12],1
jne @f
inc TEMP_THREADS
@@:
mov ax,STR_TP_CPU_CORE
call HelperRelationName 
call HelperAffinity
mov eax,( STR_TP_HT SHL 16 ) + 2910h
call HelperParameter8
jmp .doneRelation
.relationNuma: 
mov ax,STR_TP_NUMA_NODE
call HelperRelationName 
call HelperAffinity
mov eax,( STR_TP_NODE SHL 16 ) + 2910h
call HelperParameter32
jmp .doneRelation
.relationCache:
mov ax,STR_TP_L
call HelperRelationName
mov al,[rsi + 16]
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
mov eax,[rsi + 24]
cmp eax,3
ja .typeBad
add ax,STR_TP_UNIFIED
jmp .typeStore
.typeBad:
mov ax,STR_TP_UNKNOWN_TYPE
.typeStore:
lea rdi,[rdx + 04] 
call RelationNameEntry
call HelperAffinity
mov eax,( STR_TP_CACHE_WAYS SHL 16 ) + 2911h
call HelperParameter8
mov eax,( STR_TP_CACHE_LINE SHL 16 ) + 3212h
call HelperParameter16
lea rdi,[rdx + 3Fh]
push rcx rsi
mov eax,[rsi + 14h]
push rax
mov ax,STR_TP_CACHE_SIZE
call PoolStringWrite
mov bl,0
pop rax
call DecimalPrint32
pop rsi rcx
jmp .doneRelation 
.relationPackage:
inc TEMP_SOCKETS
mov ax,STR_TP_CPU_PACKAGE
call HelperRelationName 
call HelperAffinity
.doneRelation:
add rsi,32
pop rdi
dec ecx
jnz .scanRelations
pop r8 r9 rcx rsi
mov al,0
stosb
mov rbp,rdi
mov [DYNA_PTR.textTopology1.objectStop],rbp
;---------- Support screen 0 threads, cores, sockets --------------------------;
lea rax,[SYS_PARMS.summaryTopology]
mov qword [rax + SUMMARYTOPOLOGY.threads],r8
mov [rax + SUMMARYTOPOLOGY.sockets],r9d
;---------- Build text block for topology screen, second block = summary ------;
mov [DYNA_PTR.textTopology2.objectStart],rbp
mov rdi,rbp
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
mov rbp,rdi
.exit:
mov [DYNA_PTR.textTopology2.objectStop],rbp
clc
ret
.error:
mov al,MSG_ERROR_TOPOLOGY_API
stc
ret

;---------- Align memory pointer, required for WinAPI calls -------------------;
;                                                                              ;
; INPUT:   RBP = Pointer                                                       ;
;                                                                              ;
; OUTPUT:  RBP = Aligned pointer, updated if required                          ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperAlignment:
test bpl,00001111b
jz @f
and bpl,0F0h
add rbp,010h
@@: 
ret

;--- Write relation name string for text generation by topology parsing -------;
;                                                                              ;
; INPUT:   RDX = Destination base address                                      ;
;          AX  = String ID at application strings pool                         ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperRelationName:
lea rdi,[rdx + 01]
RelationNameEntry:
push rcx rsi
call PoolStringWrite
pop rsi rcx
ret

;--- Write affinity mask string for text generation by topology parsing -------;
;                                                                              ;
; INPUT:   RDX = Destination base address                                      ;
;          RSI = Pointer to parsed topology entry: affinity mask               ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperAffinity:
lea rdi,[rdx + 18]   ; RDI = Destination text pointer
mov rax,[rsi + 00]   ; RAX = Affinity mask
lea r8,[rdi + 16]    ; R8  = Mask text format limit
EntryAffinity:
push rcx rdx rsi
bsf rcx,rax
bsr rdx,rax
cmp ecx,edx
je .modeSingle
push rcx rdx
.scanMask:
bt eax,ecx
jz .endMask
inc ecx
cmp ecx,edx
jb .scanMask
.endMask:
cmp ecx,edx
pop rdx rcx
je .modeInterval
.modeIndividual:
xor ecx,ecx
xor edx,edx
.cycleIndividual:
cmp rdi,r8
jae .overIndividual
shr rax,1
jnc .skipIndividual
push rax
test edx,edx
jz .firstIndividual
mov al,','
stosb
.firstIndividual:
inc edx
mov eax,ecx
mov bl,0
call DecimalPrint32
pop rax
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
pop rsi rdx rcx
ret

;--- Write number value string for text generation by topology parsing --------;
; Has 3 entry points depend on parameter width: 8, 16, 32 bits                 ;
;                                                                              ;
; INPUT:   RDX = Destination base address                                      ;
;          RSI = Pointer to parsed topology entry                              ;
;          AL          = Parameter offset in the topology entry structure      ; 
;          AH          = Text string position offset at destination            ;   
;          EAX.[31-16] = Parameter name String ID at application strings pool  ;  
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperParameter8:         ; Entry point for addressed 8-bit parameter
shld ebx,eax,16
movzx edi,ah
movzx eax,al
movzx eax,byte [rsi + rax]
ParameterEntry:
push rcx rsi
lea rdi,[rdx + rdi]
push rax
xchg eax,ebx
call PoolStringWrite
pop rax
mov bl,0
call DecimalPrint32
pop rsi rcx
ret
HelperParameter16:         ; Same entry point for addressed 16-bit parameter
shld ebx,eax,16
movzx edi,ah
movzx eax,al
movzx eax,word [rsi + rax]
jmp ParameterEntry
HelperParameter32:         ; Same entry point for addressed 32-bit parameter
shld ebx,eax,16
movzx edi,ah
movzx eax,al
mov eax,dword [rsi + rax]
jmp ParameterEntry

;--- Write cache size and count string for text gener. by topology parsing ----;
;                                                                              ;
; INPUT:   RSI = Pointer to topology information buffer                        ;
;          RDI = Destination text buffer                                       ;
;          ECX = Entries count at topology information buffer                  ;  
;          AL  = Cache level                                                   ;  
;          AH  = Cache type                                                    ;    
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperCacheSummary:
push rcx rsi
jecxz .nodata             ; Go skip if list empty
movzx ebx,ah
xchg edx,eax
xor r8d,r8d         ; R8D  = Cache size
xor r9d,r9d         ; R9D  = Cache units count
xor r10d,r10d       ; R10D = First variant of cache size for detect hybrid CPU
;--- Scan topology list for cache object, this is first pass for hybrid CPU ---;
push rcx rsi
.scanCaches:              ; This cycle for count number cache units
cmp dword [rsi + 08],2
jne .done                 ; Go skip if descriptor type mismatch
cmp byte [rsi + 16],dl
jne .done                 ; Go skip if cache level mismatch
cmp dword [rsi + 24],ebx
jne .done                 ; Go skip if cache type mismatch
test r10d,r10d
jz .store                 ; Go assign this cache size if yet not assigned
cmp [rsi + 20],r10d
jne .done                 ; Skip this cache size if differrent size (hybrid)
.store:
mov r8d,[rsi + 20]        ; R8D = cache size
inc r9d                   ; R9D, cache units count + 1
mov r10d,r8d
.done:
add rsi,32
loop .scanCaches              ; This cycle for count number cache units
call HelperCacheSummarySave
pop rsi rcx
;--- Scan topology list for cache object, this is second pass for hybrid CPU --;
xor r8d,r8d         ; R8D  = Cache size
xor r9d,r9d         ; R9D  = Cache units count
.scanCachesHybrid:        ; This cycle for count number cache units
cmp dword [rsi + 08],2
jne .doneHybrid           ; Go skip if descriptor type mismatch
cmp byte [rsi + 16],dl
jne .doneHybrid           ; Go skip if cache level mismatch
cmp dword [rsi + 24],ebx
jne .doneHybrid           ; Go skip if cache type mismatch
cmp [rsi + 20],r10d
je .doneHybrid            ; Skip this cache size if previous same size (hybrid)
mov r8d,[rsi + 20]        ; R8D = cache size
inc r9d                   ; R9D, cache units count + 1
.doneHybrid:
add rsi,32
loop .scanCachesHybrid        ; This cycle for count number cache units
call HelperCacheSummarySave
.nodata:
pop rsi rcx
ret

;--- Helper for add cache to text report and update size and count variables --; 
;                                                                              ; 
; INPUT:   RDI  = Destination text buffer                                      ;
;          DL   = Cache level                                                  ;
;          BL   = Cache type code                                              ;
;          R8   = Cache size value, if 0, operation skipped                    ;
;          R9D  = Cache count value                                            ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperCacheSummarySave:
push rbx rdx r10
test r8d,r8d
jz .nodata
mov bh,dl
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
pop rdi
mov al,[rsp + 08]
mov ah,[rsp + 16] 
call HelperCacheSummaryBinary
.nodata:
pop r10 rdx rbx
ret

;--- Helper for write cache size and count variables by topology WinAPI -------;
; 3 entry points: non-hybrid CPU, hybrid CPU P-Cores, hybrid CPU E-Cores       ;
;                                                                              ; 
; INPUT:   AX = Cache level and type code                                      ;
;          R8  = Cache size value                                              ;
;          R9D = Cache count value                                             ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperCachePerfCoreBinary:             ; Entry point for hybrid P-Cores info
lea rbx,[SYS_PARMS.perfCoreCache]
jmp HelperCacheBinaryEntry
HelperCacheEffCoreBinary:              ; Entry point for hybrid E-Cores info
lea rbx,[SYS_PARMS.effCoreCache]
jmp HelperCacheBinaryEntry
HelperCacheSummaryBinary:              ; Entry point for non-hybrid
lea rbx,[SYS_PARMS.summaryCache]
HelperCacheBinaryEntry:
lea rsi,[rbx + SUMMARYCACHE.sizeL1C]
cmp ax,0101h
je .detected 
lea rsi,[rbx + SUMMARYCACHE.sizeL1D]
cmp ax,0201h
je .detected 
lea rsi,[rbx + SUMMARYCACHE.sizeL2U]
cmp ax,0002h
je .detected 
lea rsi,[rbx + SUMMARYCACHE.sizeL3U]
cmp ax,0003h
je .detected 
lea rsi,[rbx + SUMMARYCACHE.sizeL4U]
cmp ax,0004h
je .detected 
jmp .nodata
.detected:
mov [rsi + 0],r8
mov [rsi + 8],r9d
.nodata:
ret
