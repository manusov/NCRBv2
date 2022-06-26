;---------- Run "Save report" dialogue and save text report file --------------;
;                                                                              ;
; INPUT  : None at registers, use GUI data structures.                         ;
;                                                                              ;
; OUTPUT : None at registers                                                   ;
;                                                                              ;
;------------------------------------------------------------------------------; 

; TODO. Check disk full.
; TODO. Results for system information, vector brief, simple run, 
;       table for drawings data.

ScenarioSaveReport:
pushad
cld
;---------- Initializing parameters and run dialogue for save report ----------;
.restartDialogue:
mov ax,STR_REPORT_FILE_NAME
mov edi,FILE_PATH_BUFFER
mov edx,edi                    ; EDX = Pointer to file name string
call PoolStringWrite
mov al,0                       ; AL = 0 used later also
stosb
mov esi,[APP_DATA.lockedStrings]
mov ax,STR_REPORT_FILTER_1
call IndexString               ; ESI = Pointer to file name filter string
mov edi,OPEN_FILE_NAME         ; EDI = Pointer to structure for blank
mov ebx,edi                    ; EBX = Pointer to structure 
mov ecx,sizeof.OPENFILENAME    ; ECX = Size of structure for blank, AL = 0 
rep stosb
mov [ebx + OPENFILENAME.lStructSize],sizeof.OPENFILENAME
mov [ebx + OPENFILENAME.lpstrFilter],esi
mov [ebx + OPENFILENAME.lpstrFile],edx
mov [ebx + OPENFILENAME.nMaxFile],FILE_PATH_MAXIMUM
mov esi,[APP_DATA.lockedStrings]
mov ax,STR_REPORT_DEF_EXT
call IndexString             ; ESI = Pointer to file default extension string
mov [ebx + OPENFILENAME.lpstrDefExt],esi
push ebx                     ; Parm#1 = Pointer to structure
call [GetSaveFileName]
test eax,eax
jz .noSelections             ; Go skip if operation cancelled by user or error
;---------- Save report operation selected by user, create report file --------; 
mov edx,CREATE_NEW
.createReport:
mov ecx,[ebx + OPENFILENAME.lpstrFile]
test ecx,ecx
jz .reportError 
xor eax,eax                  ; EAX = 0 for compact push
push eax                     ; Parm #7 = Template file, not used
push FILE_ATTRIBUTE_NORMAL   ; Parm #6 = File attributes
push edx                     ; Parm #5 = Creation disposition
push eax                     ; Parm #4 = Security attributes, not used
push eax                     ; Parm #3 = Share mode, not used
push GENERIC_WRITE           ; Parm #2 = Desired access
push ecx                     ; Parm #1 = Pointer to file name
call [CreateFileA]
cmp eax,INVALID_HANDLE_VALUE
je .createFailed 
test eax,eax
jnz .createOk
.createFailed:
call [GetLastError]
cmp eax,ERROR_FILE_EXISTS
jne .reportError 
;---------- Support situation File exists -------------------------------------;
push MB_YESNOCANCEL + MB_ICONWARNING   ; Parm#4 = Message type
push PROGRAM_NAME                ; Parm#3 = Pointer to caption string
mov edi,FILE_WORK_BUFFER
push edi                         ; Parm#2 = Pointer to message string
mov ax,STR_REPORT_EXISTS_1
call PoolStringWrite
mov esi,FILE_PATH_BUFFER
call StringWrite
mov ax,STR_REPORT_EXISTS_2
call PoolStringWrite
mov al,0
stosb
push 0                 ; Parm#1 = Owner window handle, 0 means no owner 
call [MessageBox]  
mov edx,CREATE_ALWAYS  ; EDX = Creation disposition for create file operation 
cmp eax,IDYES
je .createReport       ; Go save report if "Yes" 
cmp eax,IDNO
je .restartDialogue    ; Go restart dialogue if "No"
jmp .noSelections      ; Otherwise, go exit dialogue by "Cancel" (IDCANCEL) 
;---------- Report file create OK, save report file header --------------------;
.createOk:
xchg ebx,eax           ; EBX = Report file handle
mov esi,ABOUT_NAME
call HelperFileStringWrite 
jz .closeAfterError 
;---------- Save system information as text data ------------------------------;
; TODO.
;---------- Save vector brief test results as text data -----------------------;
; TODO.
;---------- Save simple run benchmarks results as text data -------------------; 
; TODO.
;---------- Save drawings benchmark results as text data ----------------------; 
; TODO.
;---------- Cycle for save differrent parts of report: text blocks ------------; 
mov ebp,REPORT_TEXT_COUNT             ; EBP = Number of tables in the report
mov edi,DYNA_PTR.textOs               ; EDI = Pointer to text tables list
mov edx,[APP_DATA.lockedReportInfo]   ; EDX = Pointer to report script
.textBlocks:
mov esi,[edi]               ; ESI = Pointer to text block
add edi,sizeof.ALLOCATOR
test esi,esi
jz .textSkip                ; Go skip if this text block is empty
push esi edi edx
mov edi,FILE_WORK_BUFFER
push edi
mov ax,0A0Dh
stosw
stosw
cmp ebp,REPORT_TEXT_COUNT
je @f
stosw
@@:
mov al,' '
stosb
mov ax,[edx + 0]
call PoolStringWrite
mov al,0
stosb
pop esi
call HelperFileStringWrite  ; Write name of screen, associated with table
jz @f
call HelperLineWrite
call HelperFileStringWrite  ; Write horizontal line
jz @f
pop edx
push edx
mov edi,FILE_WORK_BUFFER
push edi
mov ax,[edx + 2]
call PoolStringWrite
mov al,0
stosb
pop esi
call HelperFileStringWrite  ; Write table up string with columns names
jz @f
call HelperLineWrite
call HelperFileStringWrite  ; Write horizontal line
@@:
pop edx edi esi
jz .closeAfterError
push edx
call HelperFileStringWrite  ; Write table content
pop edx 
jz .closeAfterError
.textSkip:
add edx,4
dec ebp
jnz .textBlocks             ; Cycle for all screens with tables
jmp .closeOk
;---------- Close report file -------------------------------------------------;
.closeAfterError:
push ebx                     ; Parm #1 = File handle         
call [CloseHandle]
jmp .reportError
.closeOk:
push ebx                     ; Parm #1 = File handle         
call [CloseHandle]
test eax,eax
jz .reportError 
;---------- Build message about report saved OK -------------------------------;
push MB_OK              ; Parm#4 = Message type
push PROGRAM_NAME       ; Parm#3 = Pointer to caption string 
mov esi,[APP_DATA.lockedStrings]
mov edi,FILE_WORK_BUFFER
push edi                ; Parm#2 = Pointer to message string
mov ax,STR_REPORT_SAVED
call PoolStringWrite
mov esi,FILE_PATH_BUFFER
call StringWrite
mov ax,0000h + '.'
stosw
jmp .reportStatus
;---------- Build message about report failed ---------------------------------;
.reportError:
push MB_ICONERROR    ; Parm#4 = Message type 
push 0               ; Parm#3 = Pointer to caption string, 0 = "error"    
mov esi,[APP_DATA.lockedStrings]
mov ax,STR_REPORT_FAILED
call IndexString
push esi             ; Parm#2 = Pointer to message string 
;---------- Message output ----------------------------------------------------;
.reportStatus:
push 0               ; Parm#1 = Owner window handle, 0 means no owner 
call [MessageBox]  
;---------- Restore registers, exit -------------------------------------------;
.noSelections:
popad
ret

;---------- Helper for write null-terminated string to file -------------------;
;                                                                              ;
; INPUT  : EBX = File handle                                                   ;
;          ESI = Pointer to null-terminated text                               ; 
;                can be string or strings with CR, LF                          ; 
;                                                                              ;
; OUTPUT : EAX = OS status, 0 means error, otherwise no errors                 ;
;          Additionally, ZF flag set (Z) if EAX = 0 for fast detect errors     ;  
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperFileStringWrite:
cld
push esi edi ebp 0
mov ebp,esp
mov edi,esi
mov ecx,FILE_WORK_MAXIMUM
mov al,0
repne scasb
sub edi,esi
dec edi
.write:
push 0                 ; Parm#5 = Overlapped, not used
push ebp               ; Parm#4 = Pointer to output variable, count
push edi               ; Parm#3 = Number of chars ( length )
push esi               ; Parm#2 = Pointer to string ( buffer )
push ebx               ; Parm#1 = File handle
call [WriteFile]
mov ecx,[ebp]          ; ECX = Returned size
test eax,eax           ; EAX = status, 0 means error
jz .stop               ; Go exit if error
jecxz .stop            ; Go exit if returned size = 0
add esi,ecx            ; ESI = advance read pointer by returned size
sub edi,ecx            ; EDI = subtract current read size from size limit
ja .write              ; Repeat write if return size > 0 and limit not reached 
.stop:
test eax,eax           ; ZF flag = additional status for fast detect errors
pop ebp ebp edi esi
ret
;---------- Helper for write null-terminated line in the work buffer ----------;
;                                                                              ;
; INPUT  : None at registers                                                   ; 
;                                                                              ;
; OUTPUT : ESI = Setup pointer to work buffer start                            ;
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperLineWrite:
cld
mov edi,FILE_WORK_BUFFER
mov esi,edi
mov ax,0A0Dh
stosw
mov ecx,79
push eax
mov al,'-'
rep stosb
pop eax
stosw
mov al,0
stosb
ret
