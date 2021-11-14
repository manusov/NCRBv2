;---------- Get ACPI information ----------------------------------------------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  CF  = Error flag, 0(NC) = OK, 1(C) = Error                          ;
;          AL  = Error code, valid if CF = 1                                   ;
;          EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
InfoAcpi:
lea edx,[ACPI_DATA.tempMadt]
mov bl,2
.writeNotAvl:
mov ax,STR_N_A
lea edi,[edx + BINDACPI.oem]
push eax
call PoolStringWrite
pop eax
lea edi,[edx + BINDACPI.manufact]
push eax
call PoolStringWrite
pop eax
lea edi,[edx + BINDACPI.oemRev]
push eax
call PoolStringWrite
pop eax
lea edi,[edx + BINDACPI.comment]
call PoolStringWrite
add edx,sizeof.BINDACPI
dec bl
jnz .writeNotAvl
xor eax,eax
cmp [DYNA_IMPORT._EnumSystemFirmwareTables],eax
je .skipAcpiList
cmp [DYNA_IMPORT._GetSystemFirmwareTable],eax
je .skipAcpiList
call HelperAlignment
push VALUE_BUFFER_LIMIT      ; Parm#3 = Buffer size limit
lea eax,[ebp + 16]
push eax                     ; Parm#2 = Pointer to buffer
push 'IPCA'                  ; Parm#1 = Signature, select data type = ACPI
call [DYNA_IMPORT._EnumSystemFirmwareTables]
test eax,eax
jz .error
cmp eax,VALUE_BUFFER_LIMIT
ja .error
mov ebx,ebp
lea edi,[ebp + 16 + eax]     ; EDI = Pointer to 36-byte entries list
mov ecx,edi                  ; ECX = Limit of 4-byte entries  
mov [ebx],edi
add ebx,16                   ; EBX = Pointer to 4-byte tables signatures list
imul eax,eax,9
lea edx,[edi + eax]          ; EDX = Pointer to buffer for read table
test dl,00001111b
jz .scanAcpiList
and dl,11110000b
add edx,10h 
.scanAcpiList:
push ecx edx
push VALUE_BUFFER_LIMIT      ; Parm#4 = Buffer size limit  
push edx                     ; Parm#3 = Buffer pointer
push dword [ebx]             ; Parm#2 = Table signature
push 'IPCA'                  ; Parm#1 = Table provider signature
call [DYNA_IMPORT._GetSystemFirmwareTable]
pop edx ecx
test eax,eax
jz .skipOneTable
cmp eax,VALUE_BUFFER_LIMIT
ja .skipOneTable 
push ecx
cld
mov ecx,9
mov esi,edx
rep movsd
;--- start of addition for show ACPI tables MADT and SRAT at screen 0 ---------;
push esi edi ebx edx ebp
lea ebx,[ACPI_DATA.tempAcpiEnable]
or byte [ebx],00000001b
mov ebp,[edx]
cmp ebp,'APIC'
je .madtFound
cmp ebp,'SRAT'
je .sratFound
jmp .skipTable
.madtFound:
or byte [ebx],00000010b
lea ebx,[ACPI_DATA.tempMadt]
jmp .copyAcpi
.sratFound:
or byte [ebx],00000100b
lea ebx,[ACPI_DATA.tempSrat]
;---------- Unified ACPI table header parse -----------------------------------;
.copyAcpi:
lea esi,[edx + 10]
lea edi,[ebx + BINDACPI.oem]
mov ecx,6
call HelperCopyAcpi
lea esi,[edx + 16]
lea edi,[ebx + BINDACPI.manufact]
mov ecx,8
call HelperCopyAcpi
lea edi,[ebx + BINDACPI.oemRev]
mov ax,STR_OEM_REV
call PoolStringWrite
mov eax,[edx + 24]
call HexPrint32
mov ax,0000 + 'h'
stosw 
cmp ebp,'SRAT'
je .sratParse
;---------- MADT ( Multiple APIC description Table ) parse --------------------;
.madtParse:        ; Interpreting ACPI MADT table, get number of APICs
lea esi,[edx + 44] ; ESI = Pointer to APIC list in the MADT
mov ecx,[edx + 4]
add ecx,edx        ; ECX = Table limit
xor ebp,ebp        ; EBP = Counter of Local APICs (MADT), start clear EBP = 0
xor edi,edi        ; EDI = Counter of I/O APICs (MADT), start clear EDI = 0
.madtScanApics:    ; This code for parse ACPI MADT, signature = "APIC"
mov al,[esi]
cmp al,0           ; 0 = Local APIC
jne @f             ; Go skip if not a Local APIC
test byte [esi + 4],00000001b
jz @f              ; Go skip if Local APIC disabled
inc ebp            ; counter+1 if local APIC found
@@:
cmp al,1			     ; 1 = I/O APIC
jne @f				     ; Go skip if not a I/O APIC
inc edi            ; +1 if found
@@:
movzx eax,byte [esi + 1] ; RAX = Size of structure, for skip it and access next
add esi,eax
cmp esi,ecx
jb .madtScanApics  ; Cycle for scan structures in the MADT
;--- Write text results of MADT parse, string at system information screen ----;
push ebx edi
lea edi,[ebx + BINDACPI.comment]
mov ax,STR_LOCAL_APICS
call PoolStringWrite 
mov bl,0
xchg eax,ebp
call DecimalPrint32
mov ax,', '
stosw
mov ax,STR_IO_APICS
call PoolStringWrite 
pop eax
call DecimalPrint32
mov al,0
stosb
pop ebx
jmp .skipTable
;---------- SRAT ( System/Static Resource Affinity Table Table ) parse --------;
.sratParse:
push ebx
lea esi,[edx + 44]   ; ESI = Pointer to APIC list in the MADT
mov ebx,[edx + 04]
add ebx,edx          ; EBX = Table limit
;--- Push local variables, qword [esp] = port R8, dword [esp + 8] = port R9D --;
; port R8 = domains bitmap, port R9D = wrong domain number flag
push 0 0 0
;---------- Pre-blank registers used as counters ------------------------------;
xor ecx,ecx                   ; ECX = Counter of CPUs (SRAT)
xor edx,edx                   ; EDX = Counter of RAMs (SRAT)
;---------- Cycle for scan nodes ----------------------------------------------;
.sratScanNodes:
mov al,[esi]
mov dword [esp + 08],-1          ; Error marker, set
cmp al,2
ja .exitSrat
mov dword [esp + 08],0           ; Error marker, clear
;---------- Detect processor affinity structure -------------------------------;
cmp al,0                         ; 0 = Processor affinity structure
jne @f                           ; Go skip if not a CPU
test byte [esi + 04],00000001b   ; Check Flags.Enable
jz @f                            ; Go skip if entry disabled
inc ecx                          ; CPUs +1
mov ebp,[esi + 09]
and ebp,00FFFFFFh
shl ebp,24                       ; This component, bits[31-08] only for CPUs
@@:
;---------- Detect memory affinity structure ----------------------------------; 
cmp al,1                         ; 1 = Memory affinity structure
jne @f
test byte [esi + 28],00000001b   ; Check Flags.Enable
jz @f                            ; Go skip if entry disabled
inc edx                          ; RAMs +1
xor ebp,ebp                      ; This component, bits[31-08]=0 for RAMs
@@:
mov al,[esi + 02]                ; AL = NUMA Proximity Domain number
and eax,00011111b                ; Select bits for 32-bit half-mask
push ecx
xor ecx,ecx
bts ecx,eax
test byte [esi + 02],00100000b 
jnz .labelHigh
or [esp + 00 + 04 + 00],ecx
jmp .labelDone
.labelHigh:
or [esp + 00 + 04 + 04],ecx
.labelDone:
pop ecx
cmp byte [esi + 02],63
jbe @f
mov dword [esp + 08], -1   ; Mark output as invalid if Domain Number > 63
@@:
;---------- Make of cycle for scan nodes ---------------------------------------;
movzx eax,byte [esi + 01]  ; EAX = Size of structure, for skip it and access next
mov dword [esp + 08],-1    ; Error marker, set
cmp al,0
je .exitSrat
add esi,eax
cmp esi,ebx
jb .sratScanNodes          ; Cycle for scan structures
;---------- End of cycle for scan nodes ---------------------------------------;
xor eax,eax              ; Number of NUMA nodes
mov bl,64
mov esi,[esp + 00]       ; EBP:ESI = domains bitmap
mov ebp,[esp + 04]
.scanOnes:               ; This cycle for count "1" bits = Number of NUMA nodes
shrd esi,ebp,1           ; CF=1(C) if current bit =1
jnc @f
inc eax                  ; Counter +1 if "1" detected
@@:
shr ebp,1
dec bl
jnz .scanOnes
.exitSrat:
or eax,[esp + 08]        ; This is checks for invalid number
;---------- Remove stack frame ------------------------------------------------;
add esp,12
pop ebx
;--- Write text results of SRAT parse, string at system information screen ----;
push edx ecx eax                   ; EDX = RAMs, ECX = CPUs, EAX = NUMA domains
lea edi,[ebx + BINDACPI.comment]
mov ax,STR_DOMAINS
call PoolStringWrite 
mov bl,0
pop eax
cmp eax,-1
je .badDomains
call DecimalPrint32
jmp .doneDomains
.badDomains:
mov ax,'? '
stosw
.doneDomains:
mov ax,', '
stosw
mov ax,STR_CPUS
call PoolStringWrite 
pop eax
call DecimalPrint32
mov ax,', '
stosw
mov ax,STR_RAMS
call PoolStringWrite 
pop eax
call DecimalPrint32
mov al,0
stosb
.skipTable:
;--- end of addition for show ACPI tables MADT and SRAT at screen 0 -----------;
pop ebp edx ebx edi esi ecx
.skipOneTable:
add ebx,4
cmp ebx,ecx
jb .scanAcpiList 
.doneAcpiList:
mov [DYNA_PTR.listAcpi.objectStart],ebp
mov [DYNA_PTR.listAcpi.objectStop],edi
mov ebp,edi
.skipAcpiList:
mov edx,[DYNA_PTR.listAcpi.objectStart]
mov ecx,[DYNA_PTR.listAcpi.objectStop]
test ecx,ecx
jz .noAcpi
mov ecx,[edx]
mov ebx,ecx
sub ecx,edx
sub ecx,16
shr ecx,2
jecxz .noAcpi
mov [DYNA_PTR.textAcpi1.objectStart],ebp
push esi
mov esi,edx
.acpiScan:
mov edi,ebp
push ecx
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
mov eax,00040100h
call HelperCopyString
mov eax,0006080Ah
call HelperCopyString 
mov eax,00081110h
call HelperCopyString 
mov eax,0004201Ch
call HelperCopyString 
mov ax,2D18h
call HelperHexString
mov ax,3920h
call HelperHexString 
mov ax,4708h
call HelperDecimalString
add ebx,36
add ebp,82
pop ecx
loop .acpiScan 
.acpiDone:
pop esi
mov al,0
stosb
mov ebp,edi
mov [DYNA_PTR.textAcpi1.objectStop],ebp
.noAcpi: 
ret
.error:
stc
ret

;--- Build ACPI information for text report and GUI widgets -------------------;
;                                                                              ;
; INPUT:   EBP = Dynamically allocated memory pointer                          ;
;                                                                              ;
; OUTPUT:  EBP = Dynamically allocated memory pointer updated                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
TextAcpi:
mov edx,[APP_DATA.lockedDataAcpi]
test edx,edx
jz .noAcpiData 
mov eax,[DYNA_PTR.listAcpi.objectStart]
mov ecx,[DYNA_PTR.listAcpi.objectStop]
jecxz .noAcpiData
mov ecx,[eax]
sub ecx,eax
sub ecx,16
shr ecx,2
jecxz .noAcpiData
lea ebx,[eax + 16]
mov [DYNA_PTR.textAcpi2.objectStart],ebp
push esi
.acpiDataScan:
mov edi,ebp
push ecx edx
mov ecx,80
mov al,' '
rep stosb
mov ax,0A0Dh
stosw
mov eax,00040100h
call HelperCopyString
mov ax,', '
stosw
pop edx
mov esi,edx
mov ecx,[ebx]
.findAcpi:
cmp byte [esi],0
je .notFoundAcpi
cmp ecx,[esi]
je .foundAcpi
.skipAcpi:
lodsb
cmp al,0
jne .skipAcpi
jmp .findAcpi
.notFoundAcpi:
mov ax,STR_ACPI_UNKNOWN
call PoolStringWrite
jmp .doneAcpi
.foundAcpi:
add esi,4
call StringWrite
.doneAcpi:
add ebx,4
add ebp,82
pop ecx
loop .acpiDataScan
pop esi
.acpiDataDone:
mov al,0
stosb
mov ebp,edi
mov [DYNA_PTR.textAcpi2.objectStop],ebp
.noAcpiData:
ret

;---------- Copy text string with chars checks --------------------------------;
; INPUT:   EBX = Source base                                                   ;
;          EBP = Destination base                                              ;
;          AL  = Offset for source string                                      ;
;          AH  = Offset for destination string                                 ;
;          EAX.[31-16] = String size                                           ;    
; OUTPUT:  EDI = Last used updated pointer                                     ;
;          String at [EBP + AH] size EAX.[31-16] updated                       ;   
;------------------------------------------------------------------------------;
HelperCopyString:
movzx ecx,al
lea esi,[ebx + ecx]
movzx ecx,ah
lea edi,[ebp + ecx]
xor ecx,ecx
shld ecx,eax,16
.copy:
lodsb
cmp al,0
je .space
cmp al,' '
jb .change
cmp al,'z'
jbe .store 
.change:
mov al,'.'
jmp .store
.space:
mov al,' '
.store:
stosb
loop .copy
ret

;---------- Write decimal number text string, 8 bit value ---------------------;
; INPUT:   EBX = Source base                                                   ;
;          EBP = Destination base                                              ;
;          AL  = Offset for source data contain 8-bit value                    ;
;          AH  = Offset for destination string                                 ;
; OUTPUT:  EDI = Last used updated pointer                                     ;
;          String at [EBP + AH] updated                                        ;   
;------------------------------------------------------------------------------;
HelperDecimalString:
push ebx
movzx ecx,ah
lea edi,[ebp + ecx]
movzx ecx,al
movzx eax,byte [ebx + ecx]
mov bl,0
call DecimalPrint32 
pop ebx
ret

;---------- Write hex number text string, 32 bit value ------------------------;
; INPUT:   RBX = Source base                                                   ;
;          RBP = Destination base                                              ;
;          AL  = Offset for source data contain 8-bit value                    ;
;          AH  = Offset for destination string                                 ;
; OUTPUT:  EDI = Last used updated pointer                                     ;
;          String at [RBP + AH] updated                                        ;   
;------------------------------------------------------------------------------;
HelperHexString:
call HelperHexStringNh
mov al,'h'
stosb 
ret
HelperHexStringNh:    ; This entry point for write without "h"
movzx ecx,ah
lea edi,[ebp + ecx]
movzx ecx,al
mov eax,[ebx + ecx]
jmp HexPrint32

;--- Copy fixed size string and write last 0, verify chars printability -------;
;                                                                              ;
; INPUT:   RSI = Source string                                                 ;
;          RDI = Destination string                                            ;
;          RCX = String size                                                   ;
;                                                                              ;
; OUTPUT:  RSI = Updated by copy                                               ;
;          RDI = Updated by copy                                               ;
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperCopyAcpi:
lodsb
cmp al,0
je .space
cmp al,' '
jb .change
cmp al,'z'
jbe .store
.change:
mov al,'.'
jmp .store
.space:
mov al,' '
.store:
stosb
loop HelperCopyAcpi 
mov al,0
stosb
ret
