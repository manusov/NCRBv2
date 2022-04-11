;---------- Memory performance pattern ----------------------------------------;
; Memory access methods = " Latency (LCM) " ,                                  ; 
; mode = 32, phase = build list.                                               ;
;                                                                              ;
; INPUT:   ESI = Block #1 pointer (32-bit flat) , here used as transit buffer  ;
;          EDI = Block #2 pointer (32-bit flat) , used for build walk list     ;
;          ECX = Block length, units = instructions                            ;
;          EBX:EBP = Number of measurement repeats, EBX=High32, EBP=Low32      ;
;                    not used for this pattern                                 ;
;                                                                              ;
; OUTPUT:  None at registers                                                   ;
;          DWORD [Input ESI] = Pointer to walk list entry                      ;
;          Registers corrupted                                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
Pattern_Latency_LCM_32:
;--- Initializing parameters for build linear list, build linear list ---------;  
push edi ecx esi 
xor eax,eax
mov ebp,4
.BuildLinear:     ; This cycle for build sequental offsets list
mov [esi+0],eax
add esi,ebp
add eax,ebp
dec ecx
jnz .BuildLinear
pop esi ecx
;---------- Constants ---------------------------------------------------------;
RANDOM_SEED        EQU  3
RANDOM_MULTIPLIER  EQU  00DEECE66Dh  ;  05DEECE66Dh  
RANDOM_ADDEND      EQU  0Bh   
;--- Initializing parameters for random unsorting linear list, build it -------;
push ecx esi
xor edi,edi
bsr eax,ecx
inc eax
bts edi,eax
dec edi               ; EDI = mask
mov ebp,ecx           ; EBP = limit
mov ebx,RANDOM_SEED   ; EBX = Value
.BuildRandom:
mov eax,ebx
mov edx,RANDOM_MULTIPLIER
mul edx
add eax,RANDOM_ADDEND
mov ebx,eax
and eax,edi
cmp eax,ebp
jae .BuildRandom
shl eax,2         
add eax,[esp+00]                 ; dword [esp+00] = unmodified base ESI
push dword [eax] dword [esi]
pop dword [eax] dword [esi]
add esi,4
dec ecx
jnz .BuildRandom
pop esi ecx edi
;--- Initializing parameters for build linked random offsets list, build it ---;
push esi
xor eax,eax        ; EAX = Value
mov ebp,4          ; EBP = Addend
.BuildLinked:      ; This cycle for build linked random offsets list
mov ebx,[esi]      ; EBX = Load offset from random offsets list
add ebx,edi        ; EBX = Block #2 base + offset = random position at Block #2 
mov [ebx],eax      ; Store address or terminator (0) value at this position 
mov eax,ebx        ; EAX = Just used address, for next link
add esi,ebp        ; ESI = Update pointer for random offsets list  
dec ecx            ; ECX = Cycle counter
jnz .BuildLinked 
pop esi
;--- Store EAX = walk list entry point to Block#1 (original pointer ESI) ------;
mov [esi],eax
ret
