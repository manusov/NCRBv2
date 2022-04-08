;---------- Memory performance pattern ----------------------------------------;
; Memory access methods = " Latency (RDRAND) " ,                               ; 
; mode = 32x2, phase = build list.                                             ;
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
Pattern_Latency_RDRAND_32x2:
;--- Initializing parameters for build linear list, build linear list ---------;  
push edi ecx esi 
xor eax,eax
xor edx,edx
mov ebp,8
.BuildLinear:      ; This cycle for build sequental offsets list
mov [esi+0],eax
mov [esi+4],edx
add esi,ebp
add eax,ebp
dec ecx
jnz .BuildLinear
pop esi ecx
;--- Initializing parameters for random unsorting linear list, build it -------;
push ecx esi
xor edi,edi
bsr eax,ecx
inc eax
bts edi,eax
dec edi               ; EDI = mask
mov ebp,ecx           ; EBP = limit
.BuildRandom:
rdrand eax
jnc .BuildRandom      ; Wait Hardware RNG ready 
and eax,edi
cmp eax,ebp
jae .BuildRandom
shl eax,3
add eax,[esp+00]                   ; dword [esp+00] = unmodified base ESI
push dword [eax] dword [esi]
pop dword [eax] dword [esi]
add esi,8
dec ecx
jnz .BuildRandom
pop esi ecx edi
;--- Initializing parameters for build linked random offsets list, build it ---;
push esi
xor eax,eax           ; EAX = Value
mov ebp,8             ; EBP = Addend
.BuildLinked:         ; This cycle for build linked random offsets list
mov ebx,[esi]
add ebx,edi
mov [ebx+0],eax
mov [ebx+4],eax       ; Duplicate at high dword
mov eax,ebx
add esi,ebp
dec ecx
jnz .BuildLinked 
pop esi
;--- Store EAX = walk list entry point to Block#1 (original pointer ESI) ------;
mov [esi],eax
ret
