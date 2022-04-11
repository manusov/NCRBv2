;---------- Get SMBIOS information --------------------------------------------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoSmbios:
;---------- Check required WinAPI support -------------------------------------;
mov esi,DYNA_IMPORT
xor eax,eax
cmp [esi + DYNAIMPORT._EnumSystemFirmwareTables],eax
je .skipSmbiosList
cmp [esi + DYNAIMPORT._GetSystemFirmwareTable],eax
je .skipSmbiosList
;---------- Check SMBIOS information support ----------------------------------;
call HelperAlignment
push VALUE_BUFFER_LIMIT  ; Parm#3 = Buffer size limit
push ebp                 ; Parm#2 = Pointer to buffer
push 'BMSR'              ; Parm#1 = Signature for data type = SMBIOS
call [esi + DYNAIMPORT._EnumSystemFirmwareTables]
test eax,eax
jz .skipSmbiosList
cmp eax,VALUE_BUFFER_LIMIT
ja .skipSmbiosList
test al,00000011b
jnz .skipSmbiosList 
;---------- Get SMBIOS data ( header + objects ) as single block --------------;
push VALUE_BUFFER_LIMIT  ; Parm#4 = limit BufferSize 
push ebp                 ; Parm#3 = pointer pFirmwareTableBuffer
push 0                   ; Parm#2 = FirmwareTableID
push 'BMSR'              ; Parm#1 = FirmwareTableProviderSignature
call [esi + DYNAIMPORT._GetSystemFirmwareTable]
test eax,eax
jz .skipSmbiosList
cmp eax,VALUE_BUFFER_LIMIT
ja .skipSmbiosList 
mov [DYNA_PTR.listSmbios.objectStart],ebp
add ebp,eax
mov [DYNA_PTR.listSmbios.objectStop],ebp
.skipSmbiosList:
clc                ; No error reporting for this information type, silent exit
ret

;--- Build SMBIOS information for text report and GUI widgets -----------------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextSmbios:
;---------- Build text block for SMBIOS screen --------------------------------;
mov esi,[DYNA_PTR.listSmbios.objectStart]   ; ESI = Pointer to SMBIOS data
mov ecx,[DYNA_PTR.listSmbios.objectStop]    ; ECX = End of buffer
test ecx,ecx
jz .noSmbios
mov ebx,[esi + DATA_HEADER.dataLength]
cmp ebx,sizeof.DATA_HEADER
jb .noSmbios
lea ebx,[esi + ebx + sizeof.DATA_HEADER]
cmp ebx,ecx
jbe @f
mov ebx,ecx                                 ; EBX = Limit for SMBIOS data
@@:
mov edi,ebp
mov [DYNA_PTR.textSmbios.objectStart],edi
;---------- SMBIOS header -----------------------------------------------------;
mov edx,edi
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
push ebx esi edi
lea edi,[edx + 01]
mov eax,esi
sub eax,[DYNA_PTR.listSmbios.objectStart]
call HexPrint32
mov byte [edx + 14],'-'
mov byte [edx + 21],'8'
lea edi,[edx + 29]
mov edx,esi
mov ax,STR_SMBIOS_VERSION 
call PoolStringWrite
movzx eax,byte [edx + DATA_HEADER.versionMajor]
mov bl,0
call DecimalPrint32
mov al,'.'
stosb 
movzx eax,byte [edx + DATA_HEADER.versionMinor]
call DecimalPrint32
mov ax,STR_SMBIOS_METHOD
call PoolStringWrite
movzx eax,byte [edx + DATA_HEADER.method]
call DecimalPrint32
mov ax,STR_SMBIOS_DMI_REV
call PoolStringWrite
movzx eax,byte [edx + DATA_HEADER.revisionDmi]
call DecimalPrint32
mov ax,STR_SMBIOS_LENGTH
call PoolStringWrite
mov eax,[edx + DATA_HEADER.dataLength]
call DecimalPrint32
mov ax,STR_SMBIOS_BYTES
call PoolStringWrite
pop edi esi ebx
;---------- Interval after header ---------------------------------------------;
add esi,8
mov ax,0A0Dh
stosw
;---------- SMBIOS structures -------------------------------------------------;
; TODO. Check code 127 terminator or not, use size limit only ?
.smbiosScan:
cmp byte [esi],127
je .smbiosDone 
push ebx
mov ebx,esi
push ebx
mov edx,edi
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
push edi
lea edi,[edx + 01]
mov eax,esi
sub eax,[DYNA_PTR.listSmbios.objectStart]
call HexPrint32
lea edi,[edx + 14]
movzx eax,byte [esi + 00]
mov bl,0
mov bh,al
call DecimalPrint32
lea edi,[edx + 21]
movzx eax,byte [esi + 01]
push eax
call DecimalPrint32
pop eax
add esi,eax
lea edi,[edx + 29]
movzx ax,bh
push esi
mov esi,[APP_DATA.lockedDataSmbios]
cmp ax,SMBIOS_UNKNOWN_TYPE
jbe @f
mov esi,[APP_DATA.lockedStrings]
mov ax,STR_SMBIOS_UNKNOWN
@@:
call IndexString
call StringWrite 
pop esi edi ebx
;---------- Inside SMBIOS structure: write raw strings from structure ---------;
.stringsWrite:
cmp byte [esi],0
jne .stringNonEmpty
inc esi
jmp .stringDone
.stringNonEmpty:
mov edx,edi
mov ecx,80
mov al,' '
cld
rep stosb
mov ax,0A0Dh
stosw
push edi
lea edi,[edx + 29]
call HelperCopySmbios
pop edi
.stringDone:
cmp byte [esi],0 
jne .stringsWrite 
mov ax,0A0Dh
stosw
;---------- Inside SMBIOS structure: write structure dump ---------------------;
.dumpWrite:
cmp ebx,esi
ja .endLine
mov edx,edi
mov ecx,80
mov al,' '
cld
rep stosb
mov ax,0A0Dh
stosw
push edi
lea edi,[edx + 29]
mov ecx,16
.dumpLine:
cmp ebx,esi
ja .endByte
mov al,[ebx]
inc ebx
call HexPrint8
mov al,' '
stosb
loop .dumpLine
stc
.endByte:
pop edi
.endLine:
jb .dumpWrite
mov ax,0A0Dh
stosw
;---------- Cycle for SMBIOS structures at single data block ------------------;
pop ebx
inc esi
cmp esi,ebx
jb .smbiosScan 
.smbiosDone:
dec edi
dec edi
mov al,0
stosb
mov ebp,edi
mov [DYNA_PTR.textSmbios.objectStop],ebp
.noSmbios:
ret

;--- Copy fixed size string without last 0, verify chars printability ---------;
;                                                                              ;
; INPUT:   ESI = Source string                                                 ;
;          EDI = Destination string                                            ;
;                                                                              ;
; OUTPUT:  ESI = Updated by copy                                               ;
;          EDI = Updated by copy                                               ;
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
