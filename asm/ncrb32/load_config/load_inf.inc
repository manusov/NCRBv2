;---------- Read file ---------------------------------------------------------;
; Universal read file subroutine used for read configuration file NCRB.INF.    ;
;                                                                              ;
; INPUT: ECX = Pointer to file path and name,                                  ;
;              0-terminated ASCII string                                       ;
;        EDX = Pointer to file handle                                          ;
;        ESI = Pointer to loaded file base address variable,                   ; 
;              this variable is buffer base address for file read              ;
;        EDI = Pointer to file size variable,                                  ; 
;              this variable is size limit for this buffer                     ;   
;                                                                              ;
; OUTPUT: EAX = OS API last operation status code                              ;
;               0 = Error , non zero means no errors                           ;
;         Variable file handle at [input EDX] = updated by file open           ;
;         Variable file size at [input EDI] = Read size, 0 if error            ;
;------------------------------------------------------------------------------;
READ_SIZE   EQU  dword [ebp + 0]
READ_BASE   EQU  dword [ebp + 4]
SIZE_LIMIT  EQU  dword [ebp + 8] 

LoadInf:
push ebx ebp ebp ebp ebp     ; 3 last pushes for variables
mov ebp,esp
mov ebx,edx                  ; EBX = non volatile pointer to file handle
;---------- Open file, by input parameters: ECX = Pointer to file name --------; 
xor eax,eax                  ; EAX = 0 for store result = 0 if file name = 0
jecxz .error                 ; Skip operation if file name pointer = 0
push eax                     ; Parm #7 = Template file, not used
push FILE_ATTRIBUTE_NORMAL   ; Parm #6 = File attributes
push OPEN_EXISTING           ; Parm #5 = Creation/Open disposition
push eax                     ; Parm #4 = Security attributes, not used
push eax                     ; Parm #3 = Share mode, not used
push GENERIC_READ            ; Parm #2 = Desired access
push ecx                     ; Parm #1 = Pointer to file name
call [CreateFileA]
mov [ebx],eax                ; Save file handle
;---------- Initializing for read file ----------------------------------------;
mov READ_SIZE,0          ; READ_SIZE  = 0, clear read size counter
mov eax,[esi]
mov READ_BASE,eax        ; READ_BASE  = Base address of memory buffer
mov eax,[edi]
mov SIZE_LIMIT,eax       ; SIZE_LIMIT = Size limit of memory buffer
;---------- Read file ---------------------------------------------------------;
.read:
mov ecx,[ebx]            ; ECX = File handle
jecxz .error             ; Skip read and close if handle = 0 (if open error)
xor eax,eax
push eax                 ; This push = space for output variable = dword
mov edx,esp
push eax                 ; Parm #5 = Pointer to overlapped str., not used
push edx                 ; Parm #4 = Pointer to output size
push SIZE_LIMIT          ; R8  = Parm #3 = Buffer size limit
push READ_BASE           ; Parm #2 = Buffer base address for read
push ecx                 ; Parm #1 = File handle
call [ReadFile]
pop ecx                  ; ECX = Output size, EAX = Output status
;---------- Analusing read results --------------------------------------------;
test eax,eax
jz .error                ; Go error if OS status = 0
jecxz .result            ; Go normal read termination if returned size = 0
test ecx,ecx
js .error                ; Go error if size negative, note for 32-bit only
add READ_SIZE,ecx        ; Accumulate read size
add READ_BASE,ecx        ; Advance read pointer by returned size
sub SIZE_LIMIT,ecx       ; Subtract current read size from size limit 
ja .read                 ; Repeat read if return size > 0 and limit not reached 
jb .error   ; Error if read size > size limit, if SIZE_LIMIT = 0, means read OK
;---------- Write result size -------------------------------------------------;
.result:
mov eax,READ_SIZE
mov [edi],eax            ; Write size = file size if read OK 
jmp .close
.error:
mov dword [edi],0        ; Write size = 0 if read error
;---------- Close file --------------------------------------------------------;
.close:
mov ecx,[ebx]            ; ECX = File handle
jecxz .exit
push ecx                 ; Parm #1 = File handle
call [CloseHandle]
.exit:
pop ebp ebp ebp ebp ebx  ; 3 first pushes for clear stack
ret
