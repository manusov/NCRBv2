;---------- Show details about detected error and wait key press --------------;
; Input / Output parameters and Volatile / Non volatile registers              ;
; compatible with Microsoft x64 calling convention                             ;
;                                                                              ;
; INPUT:   RCX = Pointer to error description first string, 0 means skip       ;
;          RDX = Pointer to error description second string, 0 means skip      ;
;          R8D = Windows style error code for decoding by WinAPI and           ;
;                show string "<Error name> (code)", 0 means skip               ;
;          R9  = Pointer to work (transit) buffer for prepare text data        ;
;                no limits provided, caller must associate buffer size and     ;
;                text output size, typically additional space available        ;
;                                                                              ;
; OUTPUT:  Buffer at [input R9]                                                ;
;                                                                              ;
;------------------------------------------------------------------------------;
ErrorInf:
cld
push rbx rsi rdi 
mov rdi,r9              ; RDI = Output buffer pointer
jrcxz @f
mov rsi,rcx
call StringWrite
mov ax,0A0Dh
stosw
@@:
test rdx,rdx
jz @f
mov rsi,rdx
call StringWrite
mov ax,0A0Dh
stosw
@@:
test r8d,r8d            ; R8D = WinAPI error code
jz @f
mov rsi,[APP_DATA.lockedStrings]  ; RSI = Pointer to strings pool
mov ax,STR_OS_ERROR
call IndexString        ; RSI = Pointer to string
call StringWrite        ; Write "OS error" to buffer
mov al,' '
stosb
mov bl,0                ; BL  = numeric template code
mov eax,r8d             ; EAX = WinAPI error code
call DecimalPrint32     ; Write error code decimal number to buffer
mov ax,' ='
stosw
stosb
xchg eax,r8d            ; EAX = WinAPI error code
call DecodeError        ; Write OS error description string to buffer
@@:
mov al,0
stosb                   ; Termination for strings group
pop rdi rsi rbx 
ret
;---------- Translation error code to error name string -----------------------;
;                                                                              ;
; INPUT:   RAX = Error code for translation                                    ;
;          RDI = Destination address for build text string                     ;
;                                                                              ;
; OUTPUT:  RDI = Modified by string write                                      ;
;          Memory at [Input RDI] = output string                               ;
;                                  not 0-terminated                            ;
;------------------------------------------------------------------------------;
DecodeError:
push rsi rbp
mov rbp,rsp                  ; RBP = storage for RSP and pointer to frame
and rsp,0FFFFFFFFFFFFFFF0h   ; Align stack
;---------- Get text string from OS -------------------------------------------;
xor ecx,ecx
push rcx                 ; Pointer to dynamically allocated buffer
mov rdx,rsp
push rcx                 ; Parm #7 = Arguments, parameter ignored
push rcx                 ; Parm #6 = Size, parameter ignored
push rdx                 ; Parm #5 = Pointer to pointer to allocated buffer
mov ecx,FORMAT_MESSAGE_ALLOCATE_BUFFER + FORMAT_MESSAGE_FROM_SYSTEM  ; Parm #1 = RCX = Flags
xor edx,edx              ; Parm #2 = RDX = Message source, ignored
mov r8,rax               ; Parm #3 = R8  = Message ID, code for translation  
mov r9d,LANG_NEUTRAL     ; Parm #4 = R9  = Language ID
sub rsp,32
call [FormatMessage]
add rsp,32+24
pop rsi                  ; RSI = Updated pointer to allocated buffer
;---------- End of get text string from OS, copy string -----------------------;
mov rcx,rsi
jrcxz .unknown           ; Skip string copy if buffer pointer = null 
test rax,rax
jz .unknown              ; Skip string copy if output size = 0 
call StringWrite
jmp .release
.unknown:
mov al,'?'
stosb                    ; Write "?" if cannot get string
;---------- Release buffer ----------------------------------------------------;
.release:
jrcxz .exit              ; Skip memory release if buffer pointer = null 
sub rsp,32
call [LocalFree]         ; RCX = Pointer to memory block for release
add rsp,32
.exit:
mov rsp,rbp
pop rbp rsi
ret
