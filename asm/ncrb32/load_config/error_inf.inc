;---------- Show details about detected error and wait key press --------------;
;                                                                              ;
; INPUT:   ECX = Pointer to error description first string, 0 means skip       ;
;          EDX = Pointer to error description second string, 0 means skip      ;
;          EAX = Windows style error code for decoding by WinAPI and           ;
;                show string "<Error name> (code)", 0 means skip               ;
;          EDI = Pointer to work ( transit ) buffer for prepare text data      ;
;                no limits provided, caller must associate buffer size and     ;
;                text output size, typically additional space available        ;
;                                                                              ;
; OUTPUT:  Buffer at [input EDI]                                               ;
;                                                                              ;
;------------------------------------------------------------------------------;
ErrorInf:
cld
push ebx esi edi ebp 
xchg ebp,eax            ; EBP = OS error code
jecxz @f
mov esi,ecx
call StringWrite
mov ax,0A0Dh
stosw
@@:
test edx,edx
jz @f
mov esi,edx
call StringWrite
mov ax,0A0Dh
stosw
@@:
test ebp,ebp            ; R8D = WinAPI error code
jz @f
mov esi,[APP_DATA.lockedStrings]  ; RSI = Pointer to strings pool
mov ax,STR_OS_ERROR
call IndexString        ; RSI = Pointer to string
call StringWrite        ; Write "OS error " to buffer
mov al,' '
stosb
mov bl,0                ; BL  = numeric template code
mov eax,ebp             ; EAX = WinAPI error code
call DecimalPrint32     ; Write error code decimal number to buffer
mov ax,' ='
stosw
stosb
xchg eax,ebp            ; EAX = WinAPI error code
call DecodeError        ; Write OS error description string to buffer
@@:
mov al,0
stosb                   ; Termination for strings group
pop ebp edi esi ebx 
ret
;---------- Translation error code to error name string -----------------------;
;                                                                              ;
; INPUT:   EAX = Error code for translation                                    ;
;          EDI = Destination address for build text string                     ;
;                                                                              ;
; OUTPUT:  EDI = Modified by string write                                      ;
;          Memory at [Input EDI] = output string                               ;
;                                  not 0-terminated                            ;
;------------------------------------------------------------------------------;
DecodeError:
push esi
;---------- Get text string from OS -------------------------------------------;
xor ecx,ecx
push ecx             ; Pointer to dynamically allocated buffer
mov edx,esp
push ecx             ; Parm #7 = Arguments, parameter ignored
push ecx             ; Parm #6 = Size, parameter ignored
push edx             ; Parm #5 = Pointer to pointer to allocated buffer
push LANG_NEUTRAL    ; Parm #4 = Language ID
push eax             ; Parm #3 = Message ID, code for translation
push ecx             ; Parm #2 = Message source, ignored
push FORMAT_MESSAGE_ALLOCATE_BUFFER + FORMAT_MESSAGE_FROM_SYSTEM  ; Parm #1 = Flags
call [FormatMessage]
pop esi              ; ESI = Updated pointer to allocated buffer
;---------- End of get text string from OS, copy string -----------------------;
mov ecx,esi
jecxz .unknown    ; Skip string copy if buffer pointer = null 
test eax,eax
jz .unknown       ; Skip string copy if output size = 0 
call StringWrite
jmp .release
.unknown:
mov al,'?'
stosb             ; Write "?" if cannot get string
;---------- Release buffer ----------------------------------------------------;
.release:
jecxz .exit       ; Skip memory release if buffer pointer = null
push ecx          ; Parm#1 = Pointer to memory block for release 
call [LocalFree]
.exit:
pop esi
ret
