;---------- NUMA nodes scan and build list of nodes ---------------------------;
;                                                                              ;
; INPUT:   ECX = Pointer to nodes list, destination for this subroutine        ;
;          EDX = Processor Groups option, used if value = PG_OPTIMAL (2)       ;
;          EAX = NUMA option, used for get nodes list scenario select          ;
;                                                                              ;
; OUTPUT:  CF = Error flag: 0(NC) = no errors, 1(C) = error                    ;
;          EAX = Number of allocated nodes, valid if CF=0(NC)                  ;
;          empty nodes with affinity mask = 0 rejected when count EAX          ; 
;          NUMA enumeration list updated                                       ;
;                                                                              ;
; This subroutine not allocate memory, required additional NUMA allocator.     ;
; Can destroy registers, volatile by Microsoft ia32 calling convention.        ; 
;------------------------------------------------------------------------------;
BuildNumaNodesList:
cld
push ebx esi edi ebp ecx 0 ; Note  push ecx 0  is create local varsiables
mov ebx,ecx                ; EBX = Base address of build nodes list
                           ; dword [esp+4] = Number of processor groups
;---------- Select scenario = f( NUMA option ) --------------------------------;
cmp eax,NUMA_NO_CONTROL
jbe .withoutNuma
;---------- Get maximum node number -------------------------------------------;
mov eax,esp              ; EAX = Pointer to variable 
push eax                 ; Parm #1 = Pointer to variable
call [DYNA_IMPORT._GetNumaHighestNodeNumber]   ; Return EAX=0 if error, EAX>0 if no errors
test eax,eax
jz .error                ; Go if function status = error
mov eax,[esp]            ; Get variable updated by OS API = highest node number
cmp eax,65536
jae .error               ; Go error if potential nodes count >= 65536 
;---------- Parameters for build list of nodes, EBX = Pointer to nodes list ---; 
lea edi,[eax + 1]                  ; EDI = Number of NUMA nodes, make 1-based
xor esi,esi                        ; ESI = Start node number, 0-based
lea ebp,[ebx + NUMA_LIST_SIZE]     ; EBP = Limit for nodes list
;---------- Cycle for build list of nodes -------------------------------------;
.numaNodesScan:
mov [ebx + NUMACTRL.nodeID],esi        ; Store node number
lea eax,[ebx + NUMACTRL.nodeAffinity]  ; EDX = Pointer Processor Mask
push eax                               ; Parm #2 = Pointer Processor Mask
push esi                               ; Parm #1 = Node number
cmp dword [esp + 8],PG_OPTIMAL         ; dword [esp+8] = PG_OPTIMAL if  
jne .getSmallMask                      ; Processor Groups: supported and enabled              
.getBigMask:
call [DYNA_IMPORT._GetNumaNodeProcessorMaskEx]  ; Write qword to [x], [x+8], mask+group
jmp .afterGetMask
.getSmallMask:
call [DYNA_IMPORT._GetNumaNodeProcessorMask]    ; Write qword to [rdx], affinity mask
mov [ebx + NUMACTRL.nodeGroup],0    ; Processor Group always 0 for this branch  
.afterGetMask:
test  eax,eax           ; EAX = OS API status after get mask function
jz .skipThisNode        ; Go skip validation if error returned
;---------- This for count ACTIVE NODES ONLY, when affinity mask non zero -----;
cmp dword [ebx + NUMACTRL.nodeAffinity],0
je .skipThisNode        ; Go skip validation if affinity mask = 0
add ebx,NUMACTRL_SIZE   ; Shift to next entry, means validate stored
.skipThisNode:
inc esi                 ; Select next node number
dec edi                 ; Node counter - 1
jz .nodesDone           ; Go exit if node counter done
cmp ebx,ebp             ; Check size limit, maximum 64 used entries
jb .numaNodesScan       ; Go cycle if entries available yet 
;---------- End of cycle for build list of nodes ------------------------------;
.nodesDone:
sub ebx,[esp + 4]       ; EBX = Number of bytes stored, now pointer - start p.
xchg eax,ebx            ; EAX = Number of bytes stored 
xor edx,edx
mov ebx,NUMACTRL_SIZE   ; EBX = One entry size
div ebx                 ; EAX = Total size / Entry size = entries stored,
                        ; note must count ACTIVE NODES if affinity mask non zero 
;---------- Exit points -------------------------------------------------------;
; EAX = detected nodes count, after reject nodes with mask = 0 
.done:
clc                       ; CF=0 means no errors
.exit:
pop ecx ecx ebp edi esi ebx   ; first,  pop ecx ecx  because local variable
ret
.error:
stc                       ; CF=1 means error detected
jmp .exit
;---------- Branch without NUMA support ---------------------------------------;
; for platform without NUMA or rejected by NUMA option settings
; Note about NUMA rejected, but Processor Groups supported 
.withoutNuma:
mov edi,ebx                     ; EDI = RBX = Base address of build nodes list
mov ecx,NUMACTRL_SIZE / 4       ; ECX = One NUMA entry size, units = dwords
xor eax,eax                     ; EAX = write data = 0
rep stosd                       ; Blank entry
mov eax,[OS_DATA.systemInfo.dwActiveProcessorMask]  ; EAX = mask 
mov [ebx + NUMACTRL.nodeAffinity],eax  ; set affinity mask = all system mask
mov eax,1
jmp .done
