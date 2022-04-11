;---------- Memory performance pattern ----------------------------------------;
; Memory access methods = " Latency (LCM and RDRAND) " , phase = walk list.    ;
;                                                                              ;
; INPUT:   RSI = Block #1 pointer (64-bit flat) ,                              ; 
;                here used to load walk list entry point                       ;
;          RDI = Block #2 pointer (64-bit flat) , not used for memory walker   ;
;          R8  = Block #3 pointer (64-bit flat) , not used for memory walker   ; 
;                For Read, Write, Modify use RSI as Source or Destination      ;
;                For Copy use RSI = Source , RDI = Destination                 ;
;          RCX = Block length, units = instructions                            ;
;          RBP = Number of measurement repeats                                 ;
;                                                                              ;
; OUTPUT:  None                                                                ;
;          Registers corrupted, but must save R14, R15                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
Pattern_Latency_Walk:
mov rcx,[rsi]          ; qword [rsi] = walk list entry point, Block #1
WalkRepeat:
mov rax,rcx
DumpStart_Latency_Walk:
.WalkLinked:           ; Walk linked list, use fetched data as next address
mov rax,[rax]
test rax,rax
jnz .WalkLinked        ; This cycle for memory walk: measure latency
DumpStop_Latency_Walk:
dec rbp
jnz WalkRepeat         ; This cycle for measurement iterations
ret