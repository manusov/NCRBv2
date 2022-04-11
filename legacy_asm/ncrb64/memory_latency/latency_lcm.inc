;---------- Memory performance pattern ----------------------------------------;
; Memory access methods = " Latency (LCM) " , phase = build list.              ;
;                                                                              ;
; INPUT:   RSI = Block #1 pointer (64-bit flat) , here used as transit buffer  ;
;          RDI = Block #2 pointer (64-bit flat) , used for build walk list     ;
;          R8  = Block #3 pointer (64-bit flat) , reserved and not used yet    ;
;                For Read, Write, Modify use RSI as Source or Destination      ;
;                For Copy use RSI = Source , RDI = Destination                 ;
;          RCX = Block length, units = instructions                            ;
;          RBP = Number of measurement repeats                                 ;
;                                                                              ;
; OUTPUT:  None at registers                                                   ;
;          QWORD [Input RSI] = Pointer to walk list entry                      ;
;          Registers corrupted, but must save R14, R15                         ;
;                                                                              ;
;------------------------------------------------------------------------------;
Pattern_Latency_LCM:
push r14
;---------- Initializing parameters for build linear list ---------------------; 
mov rbx,rsi      ; RBX = Base address
mov rbp,rcx      ; RBP = Length, units = CPU instructions
mov r14d,8
xor eax,eax
;---------- Build linear list -------------------------------------------------;
.BuildLinear:    ; This cycle for build sequental offsets list
mov [rbx],rax
add rbx,r14
add rax,r14
dec rbp
jnz .BuildLinear
;---------- Initializing parameters for random unsorting linear list ----------; 
xor r10d,r10d
bsr rax,rcx
inc eax
bts r10,rax
dec r10
;---------- Address and size --------------------------------------------------;
mov rbx,rsi
mov rbp,rcx
;---------- Constants ---------------------------------------------------------;
RANDOM_SEED        EQU  3
RANDOM_MULTIPLIER  EQU  05DEECE66Dh  
RANDOM_ADDEND      EQU  0Bh   
;---------- Load constants ----------------------------------------------------;
mov r11,RANDOM_SEED
mov r12,RANDOM_MULTIPLIER
mov r13,RANDOM_ADDEND
;---------- Random unsorting linear list --------------------------------------;
.BuildRandom:    ; This cycle for build random offsets list
mov rax,r11
mul r12
add rax,r13
mov r11,rax
and rax,r10
cmp rax,rcx
jae .BuildRandom
shl rax,3
add rax,rsi
mov r8,[rbx]
mov r9,[rax]
mov [rax],r8
mov [rbx],r9
add rbx,r14
dec rbp
jnz .BuildRandom
;---------- Initializing parameters for build linked random offsets list ------;
xor eax,eax
mov r8,rsi
;---------- Build linked random offsets list ----------------------------------;
.BuildLinked:    ; This cycle for build linked random offsets list
mov rbx,[rsi]
add rbx,rdi
mov [rbx],rax
mov rax,rbx
add rsi,r14
dec rcx
jnz .BuildLinked 
;--- Store RAX = walk list entry point to Block#1 (original pointer RSI) ------;
mov [r8],rax
pop r14
ret
