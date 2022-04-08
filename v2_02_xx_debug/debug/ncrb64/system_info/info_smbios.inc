;---------- Get SMBIOS information --------------------------------------------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoSmbios:
mov r13,rsp
and rsp,0FFFFFFFFFFFFFFF0h
sub rsp,32
;---------- Check required WinAPI support -------------------------------------;
lea rsi,[DYNA_IMPORT]
xor eax,eax
cmp [rsi + DYNAIMPORT._EnumSystemFirmwareTables],rax
je .skipSmbiosList
cmp [rsi + DYNAIMPORT._GetSystemFirmwareTable],rax
je .skipSmbiosList
;---------- Check SMBIOS information support ----------------------------------;
call HelperAlignment
mov r8d,VALUE_BUFFER_LIMIT  ; R8  = Parm#3 = Buffer size limit
mov rdx,rbp                 ; RDX = Parm#2 = Pointer to buffer
mov ecx,'BMSR'              ; RCX = Parm#1 = Signature for data type = SMBIOS
call [rsi + DYNAIMPORT._EnumSystemFirmwareTables]
test rax,rax
jz .skipSmbiosList
cmp rax,VALUE_BUFFER_LIMIT
ja .skipSmbiosList
test al,00000011b
jnz .skipSmbiosList 
;---------- Get SMBIOS data ( header + objects ) as single block --------------;
mov r9d,VALUE_BUFFER_LIMIT  ; R9  = Parm#4 = limit BufferSize 
mov r8,rbp                  ; R8  = Parm#3 = pointer pFirmwareTableBuffer
xor edx,edx                 ; RDX = Parm#2 = FirmwareTableID
mov ecx,'BMSR'              ; RCX = Parm#1 = FirmwareTableProviderSignature
call [rsi + DYNAIMPORT._GetSystemFirmwareTable]
test rax,rax
jz .skipSmbiosList
cmp rax,VALUE_BUFFER_LIMIT
ja .skipSmbiosList 
mov [DYNA_PTR.listSmbios.objectStart],rbp
add rbp,rax
mov [DYNA_PTR.listSmbios.objectStop],rbp
.skipSmbiosList:
clc                ; No error reporting for this information type, silent exit
mov rsp,r13
ret

;--- Build SMBIOS information for text report and GUI widgets -----------------;
;                                                                              ;
; INPUT:   RBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  RBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextSmbios:
;---------- Build text block for SMBIOS screen --------------------------------;
mov rsi,[DYNA_PTR.listSmbios.objectStart]   ; RSI = Pointer to SMBIOS data
mov rcx,[DYNA_PTR.listSmbios.objectStop]    ; RCX = End of buffer
test rcx,rcx
jz .noSmbios
mov r14d,[rsi + DATA_HEADER.dataLength]
cmp r14d,sizeof.DATA_HEADER
jb .noSmbios
lea r14,[rsi + r14 + sizeof.DATA_HEADER]
cmp r14,rcx
jbe @f
mov r14,rcx                                 ; R14 = Limit for SMBIOS data
@@:
mov rdi,rbp
mov [DYNA_PTR.textSmbios.objectStart],rdi
;---------- SMBIOS header -----------------------------------------------------;
mov rdx,rdi
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
push rsi rdi
lea rdi,[rdx + 01]
mov rax,rsi
sub rax,[DYNA_PTR.listSmbios.objectStart]
call HexPrint32
mov byte [rdx + 14],'-'
mov byte [rdx + 21],'8'
lea rdi,[rdx + 29]
mov rdx,rsi
mov ax,STR_SMBIOS_VERSION 
call PoolStringWrite
movzx eax,byte [rdx + DATA_HEADER.versionMajor]
mov bl,0
call DecimalPrint32
mov al,'.'
stosb 
movzx eax,byte [rdx + DATA_HEADER.versionMinor]
call DecimalPrint32
mov ax,STR_SMBIOS_METHOD
call PoolStringWrite
movzx eax,byte [rdx + DATA_HEADER.method]
call DecimalPrint32
mov ax,STR_SMBIOS_DMI_REV
call PoolStringWrite
movzx eax,byte [rdx + DATA_HEADER.revisionDmi]
call DecimalPrint32
mov ax,STR_SMBIOS_LENGTH
call PoolStringWrite
mov eax,[rdx + DATA_HEADER.dataLength]
call DecimalPrint32
mov ax,STR_SMBIOS_BYTES
call PoolStringWrite
pop rdi rsi
;---------- Interval after header ---------------------------------------------;
add rsi,8
mov ax,0A0Dh
stosw
;---------- SMBIOS structures -------------------------------------------------;
; TODO. Check code 127 terminator or not, use size limit only ?
.smbiosScan:
mov rbx,rsi
cmp byte [rsi],127
je .smbiosDone 
push rbx
mov rdx,rdi
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
push rdi
lea rdi,[rdx + 01]
mov rax,rsi
sub rax,[DYNA_PTR.listSmbios.objectStart]
call HexPrint32
lea rdi,[rdx + 14]
movzx eax,byte [rsi + 00]
mov bl,0
mov bh,al
call DecimalPrint32
lea rdi,[rdx + 21]
movzx eax,byte [rsi + 01]
push rax
call DecimalPrint32
pop rax
add rsi,rax
lea rdi,[rdx + 29]
movzx ax,bh
push rsi
mov rsi,[APP_DATA.lockedDataSmbios]
cmp ax,SMBIOS_UNKNOWN_TYPE
jbe @f
mov rsi,[APP_DATA.lockedStrings]
mov ax,STR_SMBIOS_UNKNOWN
@@:
call IndexString
call StringWrite 
pop rsi rdi rbx
;---------- Inside SMBIOS structure: write raw strings from structure ---------;
.stringsWrite:
cmp byte [rsi],0
jne .stringNonEmpty
inc rsi
jmp .stringDone
.stringNonEmpty:
mov rdx,rdi
mov ecx,80
mov al,' '
cld
rep stosb
mov ax,0A0Dh
stosw
push rdi
lea rdi,[rdx + 29]
call HelperCopySmbios
pop rdi
.stringDone:
cmp byte [rsi],0 
jne .stringsWrite 
mov ax,0A0Dh
stosw
;---------- Inside SMBIOS structure: write structure dump ---------------------;
.dumpWrite:
cmp rbx,rsi
ja .endLine
mov rdx,rdi
mov ecx,80
mov al,' '
cld
rep stosb
mov ax,0A0Dh
stosw
push rdi
lea rdi,[rdx + 29]
mov ecx,16
.dumpLine:
cmp rbx,rsi
ja .endByte
mov al,[rbx]
inc rbx
call HexPrint8
mov al,' '
stosb
loop .dumpLine
stc
.endByte:
pop rdi
.endLine:
jb .dumpWrite
mov ax,0A0Dh
stosw
;---------- Cycle for SMBIOS structures at single data block ------------------;
inc rsi
cmp rsi,r14
jb .smbiosScan
.smbiosDone:
dec rdi
dec rdi
mov al,0
stosb
mov rbp,rdi
mov [DYNA_PTR.textSmbios.objectStop],rbp
.noSmbios:
ret

;--- Copy fixed size string without last 0, verify chars printability ---------;
;                                                                              ;
; INPUT:   RSI = Source string                                                 ;
;          RDI = Destination string                                            ;
;                                                                              ;
; OUTPUT:  RSI = Updated by copy                                               ;
;          RDI = Updated by copy                                               ;
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperCopySmbios:
cld
.copy:
lodsb
cmp al,0
je .exit
cmp al,' '
jb .change
cmp al,'z'
jbe .store 
.change:
mov al,'.'
.store:
stosb
jmp .copy
.exit:
ret
