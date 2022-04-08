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
push rbx rsi rdi rbp r12 r13 r14
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h   ; Stack alignment required because WinAPI used
sub rsp,32                   ; Parameters shadow required because WinAPI used
cld
;---------- Initializing parameters and run dialogue for save report ----------;
.restartDialogue:
mov ax,STR_REPORT_FILE_NAME
lea rdi,[FILE_PATH_BUFFER]
mov rdx,rdi                    ; RDX = Pointer to file name string
call PoolStringWrite
mov al,0                       ; AL = 0 used later also
stosb
mov rsi,[APP_DATA.lockedStrings]
mov ax,STR_REPORT_FILTER_1
call IndexString               ; RSI = Pointer to file name filter string
lea rdi,[OPEN_FILE_NAME]       ; RDI = Pointer to structure for blank
mov rbx,rdi                    ; RBX = Pointer to structure 
mov ecx,sizeof.OPENFILENAME    ; RCX = Size of structure for blank, AL = 0 
rep stosb
mov [rbx + OPENFILENAME.lStructSize],sizeof.OPENFILENAME
mov [rbx + OPENFILENAME.lpstrFilter],rsi
mov [rbx + OPENFILENAME.lpstrFile],rdx
mov [rbx + OPENFILENAME.nMaxFile],FILE_PATH_MAXIMUM
mov rsi,[APP_DATA.lockedStrings]
mov ax,STR_REPORT_DEF_EXT
call IndexString             ; RSI = Pointer to file default extension string
mov [rbx + OPENFILENAME.lpstrDefExt],rsi
mov rcx,rbx                  ; RCX = Parm#1 = Pointer to structure 
call [GetSaveFileName]
test rax,rax
jz .noSelections             ; Go skip if operation cancelled by user or error
;---------- Save report operation selected by user, create report file --------; 
mov edx,CREATE_NEW
.createReport:
mov rcx,[rbx + OPENFILENAME.lpstrFile]
test rcx,rcx
jz .reportError 
xor eax,eax                  ; RAX = 0 for compact push
push rax                     ; This push for alignment
push rax                     ; Parm #7 = Template file, not used
push FILE_ATTRIBUTE_NORMAL   ; Parm #6 = File attributes
push rdx                     ; Parm #5 = Creation disposition
xor r9d,r9d                  ; R9  = Parm #4 = Security attributes, not used
xor r8d,r8d                  ; R8  = Parm #3 = Share mode, not used
mov edx,GENERIC_WRITE        ; RDX = Parm #2 = Desired access
; rcx valid                  ; RCX = Parm #1 = Pointer to file name
sub rsp,32
call [CreateFileA]
add rsp,32
cmp rax,INVALID_HANDLE_VALUE
je .createFailed 
test rax,rax
jnz .createOk
.createFailed:
call [GetLastError]
cmp eax,ERROR_FILE_EXISTS
jne .reportError 
;---------- Support situation File exists -------------------------------------;
mov r9d,MB_YESNOCANCEL + MB_ICONWARNING   ; R9  = Parm#4 = Message type
lea r8,[PROGRAM_NAME]               ; R8  = Parm#3 = Pointer to caption string
lea rdi,[FILE_WORK_BUFFER]
mov rdx,rdi                         ; RDX = Parm#2 = Pointer to message string
mov ax,STR_REPORT_EXISTS_1
call PoolStringWrite
lea rsi,[FILE_PATH_BUFFER]
call StringWrite
mov ax,STR_REPORT_EXISTS_2
call PoolStringWrite
mov al,0
stosb
xor ecx,ecx            ; RCX = Parm#1 = Owner window handle, 0 means no owner 
call [MessageBox]  
mov edx,CREATE_ALWAYS  ; RDX = Creation disposition for create file operation 
cmp eax,IDYES
je .createReport       ; Go save report if "Yes" 
cmp eax,IDNO
je .restartDialogue    ; Go restart dialogue if "No"
jmp .noSelections      ; Otherwise, go exit dialogue by "Cancel" (IDCANCEL) 
;---------- Report file create OK, save report file header --------------------;
.createOk:
xchg rbx,rax                 ; RBX = Report file handle
lea rsi,[ABOUT_NAME]
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
mov r13d,REPORT_TEXT_COUNT             ; R13 = Number of tables in the report
lea r12,[DYNA_PTR.textOs]              ; R12 = Pointer to text tables list
mov r14,[APP_DATA.lockedReportInfo]    ; R14 = Pointer to report script
.textBlocks:
mov rsi,[r12]               ; RSI = Pointer to text block 
add r12,sizeof.ALLOCATOR
test rsi,rsi
jz .textSkip                ; Go skip if this text block is empty
push rsi
lea rdi,[FILE_WORK_BUFFER]
push rdi
mov ax,0A0Dh
stosw
stosw
cmp r13d,REPORT_TEXT_COUNT
je @f
stosw
@@:
mov al,' '
stosb
mov ax,[r14 + 0]
call PoolStringWrite
mov al,0
stosb
pop rsi
call HelperFileStringWrite  ; Write name of screen, associated with table
jz @f
call HelperLineWrite
call HelperFileStringWrite  ; Write horizontal line
jz @f
lea rdi,[FILE_WORK_BUFFER]
push rdi
mov ax,[r14 + 2]
call PoolStringWrite
mov al,0
stosb
pop rsi
call HelperFileStringWrite  ; Write table up string with columns names
jz @f
call HelperLineWrite
call HelperFileStringWrite  ; Write horizontal line
@@:
pop rsi
jz .closeAfterError
call HelperFileStringWrite  ; Write table content 
jz .closeAfterError
.textSkip:
add r14,4
dec r13d
jnz .textBlocks             ; Cycle for all screens with tables
jmp .closeOk
;---------- Close report file -------------------------------------------------;
.closeAfterError:
mov rcx,rbx                  ; RCX = Parm #1 = File handle         
call [CloseHandle]
jmp .reportError
.closeOk:
mov rcx,rbx                  ; RCX = Parm #1 = File handle         
call [CloseHandle]
test rax,rax
jz .reportError 
;---------- Build message about report saved OK -------------------------------;
mov r9d,MB_OK           ; R9  = Parm#4 = Message type
lea r8,[PROGRAM_NAME]   ; R8  = Parm#3 = Pointer to caption string 
mov rsi,[APP_DATA.lockedStrings]
lea rdi,[FILE_WORK_BUFFER]
mov rdx,rdi             ; RDX = Parm#2 = Pointer to message string
mov ax,STR_REPORT_SAVED
call PoolStringWrite
lea rsi,[FILE_PATH_BUFFER]
call StringWrite
mov ax,0000h + '.'
stosw
jmp .reportStatus
;---------- Build message about report failed ---------------------------------;
.reportError:
mov r9d,MB_ICONERROR ; R9  = Parm#4 = Message type 
xor r8d,r8d          ; R8  = Parm#3 = Pointer to caption string, 0 = "error"    
mov rsi,[APP_DATA.lockedStrings]
mov ax,STR_REPORT_FAILED
call IndexString
mov rdx,rsi          ; RDX = Parm#2 = Pointer to message string 
;---------- Message output ----------------------------------------------------;
.reportStatus:
xor ecx,ecx          ; RCX = Parm#1 = Owner window handle, 0 means no owner 
call [MessageBox]  
;---------- Restore RSP after stack alignment and parameters shadow, exit -----;
.noSelections:
mov rsp,rbp
pop r14 r13 r12 rbp rdi rsi rbx
ret

;---------- Helper for write null-terminated string to file -------------------;
;                                                                              ;
; INPUT  : RBX = File handle                                                   ;
;          RSI = Pointer to null-terminated text                               ; 
;                can be string or strings with CR, LF                          ; 
;                                                                              ;
; OUTPUT : RAX = OS status, 0 means error, otherwise no errors                 ;
;          Additionally, ZF flag set (Z) if RAX = 0 for fast detect errors     ;  
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperFileStringWrite:
cld
push rsi rdi rbp 0
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h   ; Stack alignment required because WinAPI used
sub rsp,32                   ; Parameters shadow required because WinAPI used
mov rdi,rsi
mov ecx,FILE_WORK_MAXIMUM
mov al,0
repne scasb
sub rdi,rsi
dec rdi
.write:
xor eax,eax
push rax                ; This push for alignment
push rax                ; Parm#5 = Overlapped, not used
mov r9,rbp              ; Parm#4 = Pointer to output variable, count
mov r8,rdi              ; Parm#3 = Number of chars ( length )
mov rdx,rsi             ; Parm#2 = Pointer to string ( buffer )
mov rcx,rbx             ; Parm#1 = File handle
sub rsp,32
call [WriteFile]
add rsp,32
mov rcx,[rbp]           ; RCX = Returned size
test rax,rax            ; RAX = status, 0 means error
jz .stop                ; Go exit if error
jrcxz .stop             ; Go exit if returned size = 0
add rsi,rcx             ; RSI = advance read pointer by returned size
sub rdi,rcx             ; RDI = subtract current read size from size limit
ja .write               ; Repeat write if return size > 0 and limit not reached 
.stop:
test rax,rax            ; ZF flag = additional status for fast detect errors
mov rsp,rbp
pop rbp rbp rdi rsi
ret
;---------- Helper for write null-terminated line in the work buffer ----------;
;                                                                              ;
; INPUT  : None at registers                                                   ; 
;                                                                              ;
; OUTPUT : RSI = Setup pointer to work buffer start                            ;
;                                                                              ;
;------------------------------------------------------------------------------; 
HelperLineWrite:
cld
lea rdi,[FILE_WORK_BUFFER]
mov rsi,rdi
mov ax,0A0Dh
stosw
mov ecx,79
push rax
mov al,'-'
rep stosb
pop rax
stosw
mov al,0
stosb
ret
