;---------- Callback dialogue procedures for operations child windows ---------;
;           Handler for "Draw" button at "Memory" screen                       ; 
;                                                                              ;
; INPUT:   RCX = Parm#1 = HWND = Dialog box handle                             ; 
;          RDX = Parm#2 = UINT = Message                                       ; 
;          R8  = Parm#3 = WPARAM, message-specific                             ;
;          R9  = Parm#4 = LPARAM, message-specific                             ;
;                                                                              ;
; OUTPUT:  RAX = status, TRUE = message recognized and processed               ;
;                        FALSE = not recognized, must be processed by OS,      ;
;                        see MSDN for status exceptions and details.           ;  
;                                                                              ;
;------------------------------------------------------------------------------;
DialogProcChildMemoryDraw:
cld
push rbx rsi rdi rbp r12 r13 r14 r15
mov rbp,rsp
and rsp,0FFFFFFFFFFFFFFF0h     ; Stack alignment
push rbp
sub rsp,32 + 8  ; Make parameters shadow for WinAPI calls and +8 for alignment
mov rbx,rcx     ; RBX = Window handle
;---------- Detect message type -----------------------------------------------;
cmp rdx,0000FFFFh
jae .skip
xchg eax,edx                   ; Use EAX for compact CMP, TODO. Optimize width.
cmp eax,WM_INITDIALOG
je .wminitdialog               ; Go if dialogue initialization message 
cmp eax,WM_CLOSE
je .wmclose                    ; Go if window close message
cmp eax,WM_COMMAND
je .wmcommand                  ; Go if command message
cmp eax,WM_TIMER
je .wmtimer                    ; Go if timer tick message
cmp eax,WM_PAINT
je .wmpaint                    ; Go if graphics paint message
;---------- Exit points -------------------------------------------------------;
.skip:
xor eax,eax
jmp .finish                    ; Go exit if unknown event
.processed:
mov eax,1
.finish:
add rsp,32 + 8
pop rbp
mov rsp,rbp
pop r15 r14 r13 r12 rbp rdi rsi rbx
ret

;------------------------------------------------------------------------------;
;                                                                              ;
;                           WM_INITDIALOG handler:                             ;
;                 create child window: memory performance draw.                ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wminitdialog:
mov [DRAW_PARMS.showStatic],1
mov ax,BINDER_BUILD_MEMORY_DRAW
call Binder 
lea r15,[APP_DATA]
mov r9,[r15 + APPDATA.createdIcons + MEMORY_ICON_NUMBER * 8] 
mov r8d,ICON_SMALL 
mov edx,WM_SETICON 
mov rcx,rbx
call [SendMessage]
mov rsi,[r15 + APPDATA.lockedStrings]
mov ax,STR_CHILD_MEM_DRAW
call IndexString
mov rdx,rsi
mov rcx,rbx
call [SetWindowText]
;---------- Set drawings window coordinates if previously saved ---------------;
mov r9d,[SAVE_DRAW_POS.top]
test r9d,r9d
js @f
xor eax,eax
push rax
push SWP_SHOWWINDOW + SWP_NOSIZE + SWP_NOZORDER
push rax
push rax
mov r8d,[SAVE_DRAW_POS.left]
xor edx,edx
mov rcx,rbx
sub rsp,32
call [SetWindowPos]
add rsp,32 + 32 
@@:
;---------- Initialize graphics context ---------------------------------------;
xor ecx,ecx                        ; Parm#1 = RCX = Handle to exist DC, 0=Application current screen
call [CreateCompatibleDC]          ; Create compatible Device Context for buffered video output 
mov [GC_PARMS.handleMemDC],rax     ; Store handle
mov rsi,rax                        ; RSI = Store handle for later use
xchg rcx,rax                       ; Parm#1 = RCX = Handle to a Device Context
mov rdx,[APP_DATA.lockedBitmapInfo]  ; Parm#2 = RDX = Pointer to Bitmap Descriptor
mov r8d,DIB_RGB_COLORS             ; Parm#3 = R8  = Type of color data
lea r9,[GC_PARMS.bitmapPointer]    ; Parm#4 = R9  = Pointer to Bitmap Pointer variable
xor eax,eax                        ; RAX = 0 for compact push 0
push rax                           ; Parm#6 = Offset in the file-mapping object, not used here
push rax                           ; Parm#5 = Handle to file-mapping object, not used here
sub rsp,32                         ; Create parameters shadow
call [CreateDIBSection]
add rsp,32+16                      ; Remove parameters shadow and 2 parameters
mov [GC_PARMS.handleBitmap],rax
mov rcx,rsi                        ; Parm#1 = RCX = Handle to a Device Context
xchg rdx,rax                       ; Parm#2 = RDX = Handle to a Bitmap
call [SelectObject]
;---------- Create pen(s) -----------------------------------------------------;
mov rsi,[APP_DATA.lockedPensList]  ; RSI = Pointer to color data
mov ecx,PS_SOLID                   ; Parm#1 = RCX = Pen style
xor edx,edx                        ; Parm#2 = RDX = Pen width, 0 means 1 pixel 
mov r8d,[rsi]                      ; Parm#3 = R8  = Pen color
call [CreatePen]
mov [GC_PARMS.handlePen],rax
;---------- Create brushes ----------------------------------------------------;
cld
mov rsi,[APP_DATA.lockedBrushesList] ; RSI = Pointer to color data
lea rdi,[GC_PARMS.handlesBrushes]    ; RDI = Pointer for store brushes handles
mov ebp,4                            ; EBP = Number of brushes
@@:
lodsd                              ; EAX = Color, RSI+4, select next color 
xchg ecx,eax                       ; Parm#1 = RCX = Color
call [CreateSolidBrush]
stosq                              ; Store brush handle, RDI+8
dec ebp
jnz @b                             ; Cycle for all brushes
;---------- Create font for texts in the drawings window ----------------------;
mov ecx,16                         ; Parm#1 = RCX = Height
xor edx,edx                        ; Parm#2 = RDX = Width
xor r8d,r8d                        ; Parm#3 = R8  = Escapment
xor r9d,r9d                        ; Parm#4 = R9  = Orientation
xor eax,eax                        ; RAX = 0 for compact push 0
push rax                           ; Parm#14 = Pointer to font typename string, here not used
push VARIABLE_PITCH                ; Parm#13 = Font pitch and family
push CLEARTYPE_QUALITY             ; Parm#12 = Output quality
push CLIP_DEFAULT_PRECIS           ; Parm#11 = Clip precision
push OUT_OUTLINE_PRECIS            ; Parm#10 = Output precision
push DEFAULT_CHARSET               ; Parm#9  = Charset
push rax                           ; Parm#8  = Strike, here=0=none
push rax                           ; Parm#7  = Underline, here=0=none
push rax                           ; Parm#6  = Italic, here=0=none
push FW_DONTCARE                   ; Parm#5  = Weight of the font
sub rsp,32                         ; Create parameters shadow
call [CreateFont]
add rsp,32+80                      ; Remove parameters shadow and 10 parameters
mov [GC_PARMS.handleFont],rax 
;--- Create and set timer, for benchmarks progress ( but not for measure ) ----;
mov rcx,rbx                        ; Parm#1 = RCX = Parent window handle
xor edx,edx                        ; Parm#2 = RDX = Event ID
mov r8d,TIMER_TICK_SHOW            ; Parm#3 = R8 = Timer ticks period, ms
xor r9d,r9d                        ; Parm#4 = R9 = Notify function pointer, not used here
call [SetTimer]
;---------- Wait thread started ( 1 of 2 waits ) ------------------------------;
; Wait for measurement thread run, this event at thread entry point
; Note status after [WaitForSingleObject] ignored. TODO. check it.
; End of initialization, enable draw counter
mov rcx,[DRAW_THREAD_PARMS.eventStart]  ; RCX = Parm#1 = Handle for start event
mov edx,THREAD_TIME_LIMIT               ; RDX = Parm#2 = Time limit
call [WaitForSingleObject]
jmp .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                            WM_CLOSE handler:                                 ; 
;                  close window, include for "Cancel" button.                  ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmclose:
;---------- Save draw window coordinates for next open ------------------------;
lea rdx,[SAVE_DRAW_POS]
mov rcx,rbx
call [GetWindowRect]
;---------- Stop measurement thread -------------------------------------------;
mov [DRAW_THREAD_PARMS.measureBreak],1
;---------- Wait thread terminated ( 2 of 2 waits ) ---------------------------;
; Wait for measurement thread done, this event at thread exit point
mov rcx,[DRAW_THREAD_PARMS.eventDone]   ; RCX = Parm#1 = Handle for done event
mov edx,THREAD_TIME_LIMIT               ; RDX = Parm#2 = Time limit
call [WaitForSingleObject]
;---------- Wait thread terminated by thread handle ---------------------------;
; Note status after [WaitForSingleObject] ignored, better check it
; De-Initialize graphics context
mov rcx,[DRAW_THREAD_PARMS.threadHandle]  ; RCX = Parm#1 = Thread handle
mov edx,THREAD_TIME_LIMIT                 ; RDX = Parm#2 = Time limit
call [WaitForSingleObject]
;---------- Delete graphics controller context elements -----------------------;
mov rcx,[GC_PARMS.handleMemDC]
call [DeleteDC]
mov rcx,[GC_PARMS.handleBitmap]
call [DeleteObject]
;---------- Delete pen(s) -----------------------------------------------------;
mov rcx,[GC_PARMS.handlePen]           ; RCX = Parm#1 = Pen handle 
call [DeleteObject]
;---------- Delete brushes ----------------------------------------------------;
cld
lea rsi,[GC_PARMS.handlesBrushes]      ; RSI = Pointer for store brushes handles
mov ebp,4                              ; EBP = Number of brushes
@@:
lodsq                                  ; RAX = Brush handle
xchg rcx,rax                           ; Parm#1 = RCX = Handle
call [DeleteObject]
dec ebp
jnz @b
;---------- Delete font -------------------------------------------------------;
mov rcx,[GC_PARMS.handleFont]
call [DeleteObject]
;---------- Delete timer ------------------------------------------------------;
mov rcx,rbx                            ; Parm#1 = RCX = Parent window handle
xor rdx,rdx                            ; Set same as for SetTimer function
call [KillTimer]
;---------- Destroy window, disable draw counter ------------------------------;
mov edx,1                              ; RDX = Parm#2 = Result for return
mov rcx,rbx                            ; RCX = Parm#1 = Window handle
call [EndDialog]
jmp .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                           WM_COMMAND handler:                                ;
;                        interpreting user input.                              ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmcommand:
cmp r8,IDB_MD_CANCEL    ; Detect click "Exit" button in the draw child window
je .wmclose             ; Go close window if match
cmp r8,IDB_MD_REFRESH   ; Detect click "Refresh" button in the draw child window
jne .processed          ; Go skip if mismatch  
;---------- Handling "Refresh" button -----------------------------------------;
mov al,WAIT_MODE_MEMORY
call OpenPleaseWait                     ; Open "Please wait" window
; Set thread break flag, cause measurement thread termination
mov [DRAW_THREAD_PARMS.measureBreak],1
; Wait for measurement thread done, this event at thread exit point
mov rcx,[DRAW_THREAD_PARMS.eventDone]   ; RCX = Parm#1 = Handle for done event
mov edx,THREAD_TIME_LIMIT               ; RDX = Parm#2 = Time limit
call [WaitForSingleObject]
; Wait thread terminated by thread handle
; Note status after [WaitForSingleObject] ignored. TODO. Check it.
mov rcx,[DRAW_THREAD_PARMS.threadHandle]  ; RCX = Parm#1 = Thread handle
mov edx,THREAD_TIME_LIMIT                 ; RDX = Parm#2 = Time limit
call [WaitForSingleObject]
; Remove thread and associated handles, note can corrupt pointers
call DrawSessionStop
; Restart
mov [DRAW_PARMS.showStatic],1
call DrawSessionStart                   ; note can corrupt pointers
jc .runtimeError                        ; Go error message box if status bad
; Wait for measurement thread run, this event at thread entry point
; Note status after [WaitForSingleObject] ignored. TODO. check it.
mov rcx,[DRAW_THREAD_PARMS.eventStart]  ; RCX = Parm#1 = Handle for start event
mov edx,THREAD_TIME_LIMIT               ; RDX = Parm#2 = Time limit
call [WaitForSingleObject]
;--- Conditionally wait measurements for all points if Silent mode ------------; 
test [BIND_LIST.getMemSilent],0001b  ; Check Silent mode
jz .noSilent                         ; Go skip if non silent mode
.waitSilent:
mov ecx,100                          ; Units = milliseconds
sub rsp,32
call [Sleep]
add rsp,32
cmp [DRAW_THREAD_PARMS.measureCounter],DRAW_POINTS_COUNT
jb .waitSilent
.noSilent:
call ClosePleaseWait                ; Close "Please wait" window
jmp .processed
;---------- Handler for runtime errors ----------------------------------------;
.runtimeError:
push rax
call ClosePleaseWait                ; Close "Please wait" window
pop rax
mov rsi,[APP_DATA.lockedStrings]
movzx eax,al
add eax,STR_RUNTIME_ALLOC
call IndexString       ; Return ESI = Selected string address 
mov r9d,MB_ICONERROR   ; R9  = Parm#4 = Attributes
lea r8,[PROGRAM_NAME]  ; R8  = Parm#3 = Pointer to title (caption) string
mov rdx,rsi            ; RDX = Parm#2 = Pointer to string: error name 
mov rcx,rbx            ; RCX = Parm#1 = Parent Window - this window
call [MessageBox]  
jmp .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                             WM_TIMER handler:                                ;
;                 timer event handler for timer tick callback,                 ;
;           set revisualization request by screen data invalidation.           ;
;                                                                              ;
;------------------------------------------------------------------------------;
.wmtimer:
mov eax,[DRAW_THREAD_PARMS.measureCounter]
test eax,eax
jz @f                  ; Go skip visualization request if yet no results
cmp eax,[DRAW_THREAD_PARMS.visualCounter]
je @f                  ; Go skip visualization request if yet no new results
mov rcx,rbx            ; Parm#1 = RCX = Window handle 
xor edx,edx            ; Parm#2 = RDX = Pointer to invalidated region, 0=All window
xor r8d,r8d            ; Parm#3 = R8  = Background erase flag, 0=Background unchanged
call [InvalidateRect]  ; This cause callback WM_PAINT operation
@@:
jmp .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                             WM_PAINT handler:                                ;
;  draw text strings, coordinate axises, charts performance = f( block size ). ;                                  
;                                                                              ;
;------------------------------------------------------------------------------;

LINE_BASE_X = GRIDBLANKX + 1
LINE_BASE_Y = 476

.wmpaint:
;---------- Conditionally run section for calculate statistics ----------------;
; Calculate minimum, maximum, average, median for CPI, NSPI, MBPS.
; CPI = Clocks per Instruction, NSPI = Nanoseconds per Instruction,
; MBPS = Megabytes per Second. 
lea rsi,[DRAW_THREAD_PARMS]
mov ebp,[rsi + DTHP.measureCounter]

;---------- Skip statistics if yet no results ---------------------------------;
cmp [rsi + DTHP.visualCounter],ebp
je .skipStatistics                         ; Go skip if no measurements added
cmp ebp,2
jb .skipStatistics                         ; Go skip if number of points < 2
mov [rsi + DTHP.visualCounter],ebp

;---------- Start calculations ------------------------------------------------;
finit
fld [MEM_OPB.tscPeriodNs]
fldz
lea rdi,[rsi + DTHP.measureArray]
mov ecx,8
mov edx,DRAW_POINTS_COUNT * 8
; Copy CPI (Clock Per Instruction) array and sum for averaging CPI
push 0 rbp                                                ; + 2 qwords to stack
.cpiCopySum:
fadd qword [rdi]
mov rax,[rdi]
mov [rdi + rdx],rax
add rdi,rcx
dec ebp
jnz .cpiCopySum
fidiv dword [rsp]
; Sorting CPI for median,
; note can use integer 64-bit compare for floating point values 
.cpiReSort:
lea rdi,[rsi + DTHP.calculateArray]
mov ebp,[rsi + DTHP.visualCounter]
dec ebp
xor eax,eax
.cpiSort:
mov r8,[rdi]
cmp r8,[rdi + rcx]
jbe .cpiSorted
xchg r8,[rdi + rcx]
mov [rdi],r8
mov eax,1
.cpiSorted:
add rdi,rcx
dec ebp
jnz .cpiSort
test eax,eax
jnz .cpiReSort 
; Get minimum and maximum CPI
mov eax,[rsi + DTHP.visualCounter]
mov edx,eax
dec eax
fld [rsi + DTHP.calculateArray + 0]
fld [rsi + DTHP.calculateArray + rax * 8]
; Calculate median CPI, note assume DRAW_POINTS_COUNT even 
shr edx,1
jc .cpiMedianOdd
fld [rsi + DTHP.calculateArray + rdx * 8 - 8]
fadd [rsi + DTHP.calculateArray + rdx * 8 - 0]
fld1
fadd st0,st0
fdivp st1,st0
jmp .cpiMedianDone
.cpiMedianOdd:
fld [rsi + DTHP.calculateArray + rdx * 8 - 0]
.cpiMedianDone:
; Write statistics to thread parameters variables, 
; CPI (Clocks Per Instruction) and nsPI (Nanoseconds Per Instruction)
fst [rsi + DTHP.statCpiMedian]
fmul st0,st4
fstp [rsi + DTHP.statNspiMedian]
fst [rsi + DTHP.statCpiMax]
fmul st0,st3
fstp [rsi + DTHP.statNspiMax]
fst [rsi + DTHP.statCpiMin]
fmul st0,st2
fstp [rsi + DTHP.statNspiMin]
fst [rsi + DTHP.statCpiAverage]
fmulp st1,st0
fstp [rsi + DTHP.statNspiAverage]
; Calculate constant for convert CPI array to MBPS array
mov eax,[MEM_IPB.operandWidth]     ; RAX = bits per instruction
shr eax,3                          ; RAX = bytes per instruction 
mov edx,[MEM_IPB.updatedThreads]
mul rdx
imul rax,rax,NSPI_TO_MBPS          ; This constant for decimal megabytes and ns.
push rax
fild qword [rsp]
fdiv [MEM_OPB.tscPeriodNs]
pop rax
fst qword [rsp + 8]                ; Save value for convert CPI to MBPS
lea rdi,[rsi + DTHP.calculateArray]
mov ebp,[rsi + DTHP.visualCounter]
fldz
; Convert CPI array to MBPS array, calculate MBPS sum
.mbpsCalcSum:
fld st1
fdiv qword [rdi]
fadd st1,st0
fstp qword [rdi]
add rdi,rcx
dec ebp
jnz .mbpsCalcSum
fidiv dword [rsp]
; Get minimum and maximum MBPS, reverse from CPI
mov eax,[rsi + DTHP.visualCounter]
mov edx,eax
dec eax
fld [rsi + DTHP.calculateArray + rax * 8]
fld [rsi + DTHP.calculateArray + 0]
; Calculate median MBPS 
shr edx,1
jc .mbpsMedianOdd
fld [rsi + DTHP.calculateArray + rdx * 8 - 8]
fadd [rsi + DTHP.calculateArray + rdx * 8 - 0]
fld1
fadd st0,st0
fdivp st1,st0
jmp .mbpsMedianDone
.mbpsMedianOdd:
fld [rsi + DTHP.calculateArray + rdx * 8 - 0]
.mbpsMedianDone:
; Write statistics to thread parameters variables, 
; MBPS (Megabytes Per Second) 
fstp [rsi + DTHP.statMbpsMedian]
fstp [rsi + DTHP.statMbpsMax]
fstp [rsi + DTHP.statMbpsMin]
fstp [rsi + DTHP.statMbpsAverage]
ffree st0
fincstp
; Select Y-scale by statistics
; Note FPU rounding mode temporary changed for this operation
cmp [DRAW_PARMS.selectMode],0
jne .nanoseconds 
fld [rsi + DTHP.statMbpsMax]  ; Get maximum MBPS if Bandwidth mode
jmp .unitsSelected
.nanoseconds:
fld [rsi + DTHP.statNspiMax]  ; Get maximum nsPI if Latency mode
.unitsSelected:
push 9               ; Divisor = coordinate grid count = 9
fidiv dword [rsp]
fstcw [rsp + 4]      ; Store FPU Control Word
mov ax,[rsp + 4]
and ax,0F3FFh        ; Mask RC (Rounding Control), bits [11,10]
or ax,00800h         ; Set RC = 10b = Round Up
mov [rsp + 6],ax
fldcw [rsp + 6]      ; Load changed FPU Control Word
fistp dword [rsp]    ; Store result (valueGridY) at Round Up mode
fldcw [rsp + 4]      ; Restore original FPU Control Word
pop rax              ; RAX = calculated valueGridY
; Check for underflow (units per grid = 0) and correct to units per grid = 1 
test eax,eax
jnz @f
inc eax
@@:
; Override Y scale by INF configuration file. For measurement condition match,
; auto-scale calculations must be executed even if fixed scale mode.
cmp [DRAW_PARMS.fixedYscale],0
je @f
mov eax,[DRAW_PARMS.valueGridY]
@@:
; Set selected Y-scale and calculate multiplier for pixel coordinates
cmp [DRAW_PARMS.valueGridY],eax
je @f
mov [DRAW_PARMS.showStatic],1
@@:
mov [DRAW_PARMS.valueGridY],eax
push rax -GRIDSTEPY                                       ; + 2 qwords to stack
fild dword [rsp]        ; ST0 = - grid step, pixels
fidiv dword [rsp + 8]   ; ST0 = ( - grid step, pixels ) / ( Y units per grid )      
fstp [DRAW_PARMS.yMultiplier]
; Calculate data array for polyline Performance = f( Block size )
push LINE_BASE_Y
fild dword [rsp]
pop rax
lea rdi,[rsi + DTHP.measureArray]
lea rdx,[rsi + DTHP.polylineArray] 
mov ebp,[rsi + DTHP.visualCounter]
mov eax,LINE_BASE_X
cmp [DRAW_PARMS.selectMode],0
jne .branchLatency 
; Line for bandwidth
fld qword [rsp + 8 * 3]         ; Load value for convert CPI to MBPS
fmul [DRAW_PARMS.yMultiplier]
; First extra element required because N points means N-1 Polyline intervals
fld st0
fdiv qword [rdi]
fadd st0,st2
mov [rdx + 0],eax         ; Store POINT.x
fistp dword [rdx + 4]     ; Store POINT.y 
inc eax
add rdx,rcx
; Cycle for other elements
.lineWriteBandwidth:
fld st0
fdiv qword [rdi]
fadd st0,st2
mov [rdx + 0],eax         ; Store POINT.x
fistp dword [rdx + 4]     ; Store POINT.y 
inc eax
add rdi,rcx
add rdx,rcx
dec ebp
jnz .lineWriteBandwidth
jmp .linesDone
; Line for latency  
.branchLatency:
fld [MEM_OPB.tscPeriodNs] 
fmul [DRAW_PARMS.yMultiplier]
; First extra element required because N points means N-1 Polyline intervals
fld qword [rdi]
fmul st0,st1
fadd st0,st2 
mov [rdx + 0],eax         ; Store POINT.x
fistp dword [rdx + 4]     ; Store POINT.y
inc eax
add rdx,rcx
; Cycle for other elements
.lineWriteLatency:
fld qword [rdi]
fmul st0,st1
fadd st0,st2 
mov [rdx + 0],eax         ; Store POINT.x
fistp dword [rdx + 4]     ; Store POINT.y
inc eax
add rdi,rcx
add rdx,rcx
dec ebp
jnz .lineWriteLatency

; Done for bandwidth and latency
.linesDone:
ffree st0
fincstp
ffree st0
fincstp
add rsp,4 * 8                                         ; Release stack, 4 qwords

.skipStatistics:
;---------- End of conditionally run section for calculate statistics ---------;

;---------- Setup paint context -----------------------------------------------;
mov rcx,rbx                ; Parm#1 = RCX = Parent window handle
lea rdx,[PAINT_STRUCT]     ; Parm#2 = RDX = Pointer to PAINTSTRUCT structure
call [BeginPaint]
mov [GC_PARMS.handleDC],rax
mov rsi,[GC_PARMS.handleMemDC]  ; RSI = Graphics transit buffer handle
lea rdi,[GRAPH_RECT]            ; RDI = Pointer to rectangle structure
xor ebp,ebp                     ; RBP = 0, used for compact encoding
;---------- Setup font and color ----------------------------------------------;
mov rdx,[GC_PARMS.handleFont]   ; Parm#2 = RDX = Handle to Selected Object
mov rcx,rsi                     ; Parm#1 = RCX = Handle to a Device Context
call [SelectObject]
mov edx,COLOR_TEXT_VALUES       ; Parm#2 = RDX = Text front color value = 00bbggrrh
mov rcx,rsi                     ; Parm#1 = RCX = Handle to a Device Context
call [SetTextColor]
mov edx,COLOR_TEXT_BACK         ; Parm#2 = RDX = Text background color value = 00bbggrrh
mov rcx,rsi                     ; Parm#1 = RCX = Handle to a Device Context
call [SetBkColor]

;---------- Skip static GUI objects if not a first pass -----------------------;
xor eax,eax
xchg eax,[DRAW_PARMS.showStatic]
test eax,eax
jz .skipStatic

;---------- Blank work field, background color --------------------------------;
mov [rdi + RECT.left],ebp
mov [rdi + RECT.top],ebp
mov [rdi + RECT.right],SUBWINX
mov [rdi + RECT.bottom],SUBWINY
mov r8,[GC_PARMS.handlesBrushes + 16]  ; Parm#3 = R8 = Handle to a color brush
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT structure
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
call [FillRect]
;---------- Coordinate X-grid, vertical lines ---------------------------------;
mov r12d,GRIDBLANKX          ; R12D = Base point, X
mov r13d,GRIDX               ; R13D = Number of vertical lines
mov [rdi + RECT.top],ebp
mov [rdi + RECT.bottom],SUBWINY - GRIDBLANKY  ; Base point, Y
.xVgrid:
lea rax,[r12 + 1]            ; Line width = 1 pixel
mov [rdi + RECT.left],r12d
mov [rdi + RECT.right],eax
mov r8,[GC_PARMS.handlesBrushes + 00]  ; Parm#3 = R8 = Handle to a color brush
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT structure
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
call [FillRect]              ; Fill rectangle function
add r12d,GRIDSTEPX           ; Add X grid step
dec r13d                     ; Cycle for X-grid vertical lines
jnz .xVgrid
;---------- Coordinate Y-grid, horizontal lines -------------------------------;
mov r12d,SUBWINY - GRIDBLANKY  ; R12D = Base point, Y
mov r13d,GRIDY                 ; R13D = Number of horizontal lines
mov [rdi + RECT.left],GRIDBLANKX
mov [rdi + RECT.right],SUBWINX
.yHgrid:
lea rax,[r12 - 1]            ; Line width = 1 pixel
mov [rdi + RECT.top],eax
mov [rdi + RECT.bottom],r12d
mov r8,[GC_PARMS.handlesBrushes + 00]  ; Parm#3 = R8 = Handle to a color brush
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT structure
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
call [FillRect]              ; Fill rectangle function
sub r12d,GRIDSTEPY           ; Subtract Y grid step
dec r13d                     ; Cycle for Y-grid vertical lines
jnz .yHgrid
;---------- Initializing values for X grid text write cycle -------------------;
; Note. X text don't depends on measurements, Y text depends on measurements.
mov r12d,GRIDX          ; R12D = Number of lines, X
mov r13d,GRIDBLANKX     ; R13D = Base point, X
mov r14d,SUBWINY - 20   ; R14D = Base point Y, 20 pixels up
; Here EBP = 0 = Units value, X starts from 0
;---------- X grid text write cycle -------------------------------------------;
.xGrid:
;---------- Build ASCII string ------------------------------------------------;
push rbx rdi
lea rdi,[TEMP_BUFFER]   ; RDI = Pointer for print decimal string as ASCII
mov rdx,rdi
mov bl,0                ; BL = Template control for print number, 0=No template
mov eax,ebp             ; EAX = Value for print, Bytes/KB/MB
call DecimalPrint32     ; Print number
mov rcx,rdi
sub rcx,rdx
mov al,0
stosb                   ; Store terminator byte = 0
pop rdi rbx
;---------- Set region for text string write ----------------------------------;
mov rcx,rdi             ; Parm#1 = RCX = Pointer to RECT structure to store parameters
mov edx,r13d            ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,r14d            ; Parm#3 = R8  = Y coordinate up/left corner
lea r9,[r13 + 65]       ; Parm#4 = R9  = X coordinate down/right corner
lea rax,[r14 + 18]      ; RAX = Y coordinate down/right corner  
push 0                  ; This for stack alignment
push rax                ; Parm#5 = Y coordinate down/right corner
sub rsp,32              ; Create parameters shadow
call [SetRect]          ; Set region rectangle function 
add rsp,32 + 16         ; Remove parameters shadow, 1 parameter, stack alignment
;---------- Draw text ---------------------------------------------------------;
mov rcx,rsi             ; Parm#1 = RCX = Handle to a Device Context
lea rdx,[TEMP_BUFFER]   ; Parm#2 = RDX = Pointer to text string 
mov r8,-1               ; Parm#3 = R8 = String length, -1 means NULL-terminated string
mov r9,rdi              ; Parm#4 = R9 = Pointer to RECT structure, initialized by [SetRect] function
push 0                  ; This for stack alignment
push DT_LEFT            ; Parm#5 = Method of formatting/alignment of text string
sub rsp,32              ; Create parameters shadow
call [DrawText]         ; Draw text function 
add rsp,32 + 16         ; Remove parameters shadow, 1 parameter, stack alignment
;---------- Modify parameters and make cycle ----------------------------------;
add ebp,[DRAW_PARMS.valueGridX]   ; Addend per X grid
add r13d,GRIDSTEPX
dec r12d
jnz .xGrid 
;---------- X-units name (B/KB/MB), change color, select string, write --------;
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_UNITS     ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
mov eax,[DRAW_PARMS.selectUnits]
mov r12,[UNITS_TEXTS.bytes]       ; R12 = Pointer to text string, Cache&RAM mode
cmp al,0
je @f
mov r12,[UNITS_TEXTS.kb]
cmp al,1
je @f
mov r12,[UNITS_TEXTS.mb]
@@:
mov edx,SUBWINX - 49      ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,SUBWINY - 43      ; Parm#3 = R8  = Y coordinate up/left corner
mov r9d,SUBWINX - 13      ; Parm#4 = R9  = X coordinate down/right corner
mov eax,SUBWINY - 25      ; RAX = Transit for Parm#5 = Y coordinate down/right corner
call HelperTextConstDraw
;---------- Y-units name (MBPS/nanoseconds), use color from previous step -----;
mov r12,[UNITS_TEXTS.mbps]   ; R12 = Pointer to text string , Mode = Cache&RAM
cmp [DRAW_PARMS.selectMode],0
je @f
mov r12,[UNITS_TEXTS.nanoseconds]
@@:
mov edx,7                 ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,5                 ; Parm#3 = R8 = Y coordinate up/left corner
mov r9d,40 + 7 + 20 + 24  ; Parm#4 = R9 = X coordinate down/right corner
mov eax,18 + 5            ; RAX = Transit for Parm#5 = Y coordinate down/right corner
call HelperTextConstDraw
;---------- Print statistics table, lines and texts ---------------------------;
; Horizontal sequence of strings:  CPI, nsPI, MBPS
lea r14,[CLKS_MBPS_TEXTS]      ; R14 = Pointer to text strings pointers table
mov r13d,DRAW_TABLE_UP_COUNT   ; R13 = Strings count
mov edx,522                    ; RDX = X coordinate up/left corner
mov r8d,27                     ; R8 = Y coordinate up/left corner
mov r9d,522 + 40               ; R9 = X coordinate down/right corner
mov eax,27 + 17                ; RAX = Y coordinate down/right corner
@@:
mov r12,[r14]                  ; R12 = Pointer to text string
add r14,8                      ; Update pointer to next offset
call HelperTextConstDraw
add edx,64                     ; Shift 64 points right, X+64
add r9d,64                     ; Shift 64 points right, X+64
dec r13d
jnz @b                         ; cycle for: CPI, nsPI, MBPS 
;---------- Vertical sequence of strings: maximum, minimum, average, median ---;
; R14 = Pointer to continue of text strings pointers table
mov r13d,DRAW_TABLE_LEFT_COUNT ; R13 = Strings count
mov edx,458                    ; RDX = X coordinate up/left corner
mov r8d,27 + 19                ; R8  = Y coordinate up/left corner
mov r9d,458 + 60               ; R9  = X coordinate down/right corner
mov eax,27 + 19 + 17           ; RAX = Y coordinate down/right corner
@@:
mov r12,[r14]                  ; Get pointer to text string
add r14,8                      ; Update pointer to next offset
call HelperTextConstDraw
add r8d,15                     ; Shift 15 points down, Y+15
add eax,15                     ; Shift 15 points down, Y+15
dec r13d
jnz @b                         ; cycle for: maximum, minimum, average, median
;---------- Lines of table, up horizontal line and left vertical line ---------;
mov [rdi + RECT.left],519
mov [rdi + RECT.top],27 + 17
mov [rdi + RECT.right],519 + 240
mov [rdi + RECT.bottom],27 + 17 + 2
mov rdx,rdi                   ; Parm#2 = RDX = Pointer to RECT structure
mov rcx,rsi                   ; Parm#1 = RCX = Handle to a Device Context
mov r8,[GC_PARMS.handlesBrushes + 24]  ; Parm#3 = R8 = Handle to a color brush
call [FillRect]               ; Fill rectangle function, up horizontal line
mov [rdi + RECT.left],518
mov [rdi + RECT.top],27 + 19
mov [rdi + RECT.right],518 + 2
mov [rdi + RECT.bottom],27 + 19 + 17*4
mov rdx,rdi                   ; Parm#2 = RDX = Pointer to RECT structure
mov rcx,rsi                   ; Parm#1 = RCX = Handle to a Device Context
mov r8,[GC_PARMS.handlesBrushes + 24]  ; Parm#3 = R8 = Handle to a color brush
call [FillRect]               ; Fill rectangle function, left vertical line
;---------- Target method CPU instructions dump -------------------------------;
lea r12,[MEM_IPB]
xor r15,r15
mov r14,[r12 + MEMIPB.dumpMethodStart]
mov r13d,[r12 + MEMIPB.dumpMethodLength]
test r13d,r13d
jz .skipAsmDump
cmp r13d,128
jbe @f
mov r13d,128
@@:
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_DUMP      ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
.asmDump:
lea r12,[TEMP_BUFFER]
push rdi
mov rdi,r12
mov al,[r14 + r15]
call HexPrint8
mov al,0
stosb
pop rdi
;---------- Set coordinates for dump ------------------------------------------;
mov edx,SUBWINX - 250 + 64   ; RDX = X coordinate, up left corner
mov r8d,95 + 48              ; R8  = Y coordinate up/left corner
mov r9d,SUBWINX - 005 + 64   ; R9  = X coordinate down/right corner
mov eax,95 + 18 + 48         ; RAX = Y coordinate down/right corner 
;---------- Adjust coordinates for dump, print dump ---------------------------;
mov ecx,r15d                 ; R15 = counter, ECX = counter temporary copy #1
mov r10d,r15d                ; R10 = counter temporary copy #2  
and ecx,07h                  ; Bits 0-2 used for X-positioning
imul ecx,ecx,21              ; X scale = 21 pixels per char
add edx,ecx                  ; Add offset to X-start
add r9d,ecx                  ; Add offset to X-end
shr r10d,3
and r10d,0Fh                 ; Bits 3-6 used for Y-positioning
imul r10d,r10d,18            ; Y scale = 18 pixels per char 
add r8d,r10d                 ; Add offset to Y-start
add eax,r10d                 ; Add offset to Y-end
call HelperTextConstDraw
inc r15
dec r13d
jnz .asmDump 
.skipAsmDump:
;---------- TSC clock frequency -----------------------------------------------;
push rbx rdi
lea rdi,[DRAW_TSC_VALUE]
mov word [rdi],0000h + '?'         ; This char '?' used if error
mov rax,[MEM_OPB.tscFrequencyMHz]  ; RAX = TSC Frequency, MHz
test rax,rax                       ; Frequency = 0 means error
jz @f                              ; Go skip value write if error
mov bx,0100h                       ; BX=Template X.Y , 1 digit after '.'
call DoublePrint
mov al,0
stosb
@@:
pop rdi rbx
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_INFO      ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
mov edx,SUBWINX - 187        ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,6                    ; Parm#3 = R8 = Y coordinate up/left corner
mov r9d,SUBWINX - 5          ; Parm#4 = R9 = X coordinate down/right corner
mov eax,6+18                 ; RAX = Transit for Parm#5 = Y coordinate down/right corner
lea r12,[DRAW_TSC]           ; R12 = Pointer to text string
call HelperTextConstDraw

;---------- Point for skip static GUI objects if not a first pass -------------;
.skipStatic:

;---------- Skip results visualization if yet no results ----------------------;
cmp [DRAW_THREAD_PARMS.visualCounter],2
jb .skipDynamic                             ; Go skip if no measurements added

;---------- Conditionally run section for visual measurement results ----------;
; Draw Y-axis text depend on re-scale, write statistics and line Y=F(X).
;---------- Setup font and color ----------------------------------------------;
mov rdx,[GC_PARMS.handleFont]   ; Parm#2 = RDX = Handle to Selected Object
mov rcx,rsi                     ; Parm#1 = RCX = Handle to a Device Context
call [SelectObject]
mov edx,COLOR_TEXT_VALUES       ; Parm#2 = RDX = Text front color value = 00bbggrrh
mov rcx,rsi                     ; Parm#1 = RCX = Handle to a Device Context
call [SetTextColor]
mov edx,COLOR_TEXT_BACK         ; Parm#2 = RDX = Text background color value = 00bbggrrh
mov rcx,rsi                     ; Parm#1 = RCX = Handle to a Device Context
call [SetBkColor]
;---------- Initializing values for Y grid text write cycle -------------------;
; Note. X text don't depends on measurements, Y text depends on measurements.
mov r12d,GRIDY - 1                               ; R12D = Number of lines, Y
mov r13d,GRIDBLANKX - 68                         ; R13D = X coordinate
mov r14d,SUBWINY - GRIDBLANKY - GRIDSTEPY - 15   ; R14D = Y coordinate
mov ebp,[DRAW_PARMS.valueGridY]     ; EBP = Units value, Y starts with offset
;---------- Y grid text write cycle -------------------------------------------;
.yGrid:
;---------- Build ASCII string ------------------------------------------------;
push rbx rdi
lea rdi,[TEMP_BUFFER]   ; RDI = Pointer for print decimal string as ASCII
mov rdx,rdi
mov bl,0                ; BL = Template control for print number, 0=No template
mov eax,ebp             ; EAX = Value for print, Bytes/KB/MB
call DecimalPrint32     ; Print number
mov rcx,rdi
sub rcx,rdx
mov al,0
stosb                   ; Store terminator byte = 0
pop rdi rbx
;---------- Calculate correction value for Y axis grid values print -----------;
mov r10d,9
sub r10,rcx             ; R10 = chars positions count
jbe .rejectAddend       ; Go skip if correction not required 
imul r10,r10,7
jmp .doneAddend 
.rejectAddend:
xor r10,r10
.doneAddend:
;---------- Set region for text string write ----------------------------------;
mov rcx,rdi             ; Parm#1 = RCX = Pointer to RECT structure to store parameters
lea rdx,[r13 + r10]     ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,r14d            ; Parm#3 = R8  = Y coordinate up/left corner
lea r9,[r13 + r10 + 65] ; Parm#4 = R9  = X coordinate down/right corner
lea rax,[r14 + 18]      ; RAX = Y coordinate down/right corner  
push 0                  ; This for stack alignment
push rax                ; Parm#5 = Y coordinate down/right corner
sub rsp,32              ; Create parameters shadow
call [SetRect]          ; Set region rectangle function 
add rsp,32 + 16         ; Remove parameters shadow, 1 parameter, stack alignment
;---------- Draw text ---------------------------------------------------------;
mov rcx,rsi             ; Parm#1 = RCX = Handle to a Device Context
lea rdx,[TEMP_BUFFER]   ; Parm#2 = RDX = Pointer to text string 
mov r8,-1               ; Parm#3 = R8 = String length, -1 means NULL-terminated string
mov r9,rdi              ; Parm#4 = R9 = Pointer to RECT structure, initialized by [SetRect] function
push 0                  ; This for stack alignment
push DT_LEFT            ; Parm#5 = Method of formatting/alignment of text string
sub rsp,32              ; Create parameters shadow
call [DrawText]         ; Draw text function 
add rsp,32 + 16         ; Remove parameters shadow, 1 parameter, stack alignment
;---------- Modify parameters and make cycle ----------------------------------;
add ebp,[DRAW_PARMS.valueGridY]   ; Addend per Y grid
sub r14d,GRIDSTEPY
dec r12d
jnz .yGrid 
;---------- Visual statistics -------------------------------------------------;     
; Set color for text
mov rcx,rsi                 ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_INFO     ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
; This text strings must be updated at each visualization tick
; Initializing parameters for benchmark statistics, bandwidth and timings
mov r13d,0300h
;---------- Print timings statistics, CPI minimum, maximum, average, median ---;
; note CPI = Clocks Per Instruction
lea r14,[DRAW_THREAD_PARMS.statCpiMin]    ; R14 = variables group base
mov edx,SUBWINX - 250
mov r9d,SUBWINX - 190 
call HelperWriteStatistics
;---------- Print timings statistics, nsPI minimum, maximum, average, median --;
; note nsPI = nanoseconds per instruction
lea r14,[DRAW_THREAD_PARMS.statNspiMin]   ; R14 = variables group base
mov edx,SUBWINX - 250 + 64*1
mov r9d,SUBWINX - 190 + 64*1 
call HelperWriteStatistics
;---------- This number template for Megabytes per Second ---------------------;
mov r13d,0200h
;---------- Print timings statistics, MBPS minimum, maximum, average, median --;
; note MBPS = megabytes per second
; note Min/Max here swapped, because min. time means max. bandwidth
lea r14,[DRAW_THREAD_PARMS.statMbpsMax]   ; R14 = variables group base
mov edx,SUBWINX - 250 + 64*2
mov r9d,SUBWINX - 150 + 64*2 
call HelperWriteStatistics
;--- Draw PolyLine by measurement results Performance = F ( Block Size ) ------;
mov rdx,[GC_PARMS.handlePen]               ; Parm#2 = RDX = Pen handle
mov rcx,rsi                                ; Parm#1 = RCX = Handle to a Device Context
call [SelectObject]
mov rdi,rax
mov r8d,[DRAW_THREAD_PARMS.visualCounter]  ; Parm#3 = R8  = Number of points in the array
inc r8d                                    ; +1, because N points means N-1 Polyline intervals
lea rdx,[DRAW_THREAD_PARMS.polylineArray]  ; Parm#2 = RDX = Pointer to POINT structures array
mov rcx,rsi                                ; Parm#1 = RCX = Handle to a Device Context
call [Polyline]
mov rdx,rdi                                ; Parm#2 = RDX = Pen handle, restore original
mov rcx,rsi                                ; Parm#1 = RCX = Handle to a Device Context
call [SelectObject]

.skipDynamic:
;---------- End of conditionally run section for visual measurement results ---;

;---------- Copy graphics objects from transit buffer to graphics memory ------;
xor eax,eax           ; RAX = 0 for compact push 0
push rax              ; This push for align stack, not a parameter
push SRCCOPY          ; Parm#9 = Raster operation code
push rax              ; Parm#8 = Y source
push rax              ; Parm#7 = X source
push rsi              ; Parm#6 = Handle of source Device Context
push SUBWINY          ; Parm#5 = Height
mov r9d,SUBWINX       ; Parm#4 = R9  = Width
mov r8d,SHIFTY        ; Parm#3 = R8  = Y destination
mov edx,SHIFTX        ; Parm#2 = RDX = X destination
mov rcx,[GC_PARMS.handleDC]  ; Parm#1 = RCX = Handle of destination Device Context
sub rsp,32            ; Create parameters shadow
call [BitBlt]                       
add rsp,32 + 40 + 8     ; Remove parameters shadow, 5 parameters, 1 qword for align
;---------- Delete paint context ----------------------------------------------;
mov rcx,rbx             ; Parm#1 = RCX = Parent window handle
lea rdx,[PAINT_STRUCT]  ; Parm#2 = RDX = Pointer to PAINTSTRUCT structure
call [EndPaint]         ; This WinAPI returns BOOL status, but ignored
jmp .processed

;------------------------- Helpers for drawings -------------------------------;
; Note about stack re-alignment when subroutines call
; see Microsoft x64 calling convention.

;--- Print text string --------------------------------------------------------;
;                                                                              ;
; INPUT:   RDI = Pointer to RECT structure for draw region                     ;
;          RDX  = xLeft                                                        ;
;          R8D  = yTop                                                         ;
;          R9D  = xRight                                                       ;
;          EAX  = yBottom                                                      ;
;          RSI  = Handle of a Device Context                                   ;
;          R12  = Pointer to text string                                       ;
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          RAX, RDX, R8, R9  non volatile for this subroutine                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperTextConstDraw:
push rax rdx r8 r9
mov rcx,rdi           ; Parm#1 = RCX = Pointer to RECT structure to store parameters
push rax              ; Parm#5 = Y coordinate down/right corner
sub rsp,32            ; Create parameters shadow
call [SetRect]        ; Set region rectangle function 
add rsp,32 + 8        ; Remove parameters shadow, 1 parameter, align
mov rcx,rsi           ; Parm#1 = RCX = Handle of a Device Context
mov rdx,r12           ; Parm#2 = RDX = Pointer to text string
mov r8,-1             ; Parm#3 = R8 = String length, -1 means NULL-terminated string
mov r9,rdi            ; Parm#4 = R9 = Pointer to RECT structure, initialized by [SetRect] function
push DT_LEFT          ; Parm#5 = Method of formatting/alignment of text string
sub rsp,32            ; Create parameters shadow
call [DrawText]       ; Set region rectangle function 
add rsp,32 + 8        ; Remove parameters shadow, 1 parameter, align
pop r9 r8 rdx rax
ret
;--- Print floating point value, double ---------------------------------------;
;                                                                              ;
; INPUT:   EDX = X up left position                                            ;
;          R8D = Y up left position                                            ;
;          R9D = X down right position                                         ;
;          EAX = Y down right position                                         ;
;          R11 = floating point value, double, 64-bit                          ;
;          R13 = numeric template for floating point print                     ;
;          RSI  = Handle of a Device Context                                   ;
;          RDI = Pointer to RECT structure for draw region                     ;
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          RAX, RBX, RCX, RDX, RSI, RDI, RBP, R8, R9, R14                      ;
;          is non volatile for this subroutine                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperTextVarDouble:
push rax rbx rcx rdx rdi
push rax rbx rdi      ; Note about stack re-alignment when subroutine call
lea rdi,[TEMP_BUFFER]
mov r12,rdi
mov ecx,11 - 3        ; 11 (not 9) for blank if variable X-size font 
mov ax,0000h + '?'    ; Blank field with "?" means yet not determined
stosw
mov al,' '
rep stosb             ; Blank entire field, if next value shorter then previous
mov al,0
stosb
mov rdi,r12
mov ebx,r13d
mov rax,r11
test rax,rax
jz @f
call DoublePrint
@@:
pop rdi rbx rax
call HelperTextConstDraw  ; Note this use WinAPI inside, stack must be aligned 
pop rdi rdx rcx rbx rax
ret
;--- Print cycle for Min, Max, Average, Median --------------------------------;
;                                                                              ;
; INPUT:   EDX = X up left position                                            ;
;          R9D = X down right position                                         ;
;          R13 = numeric template for floating point print                     ;
;          R14 = pointer to variables group                                    ;
;          RSI  = Handle of a Device Context                                   ;
;          RDI = Pointer to RECT structure for draw region                     ;
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          RSI, R13 is non volatile for this subroutine                        ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperWriteStatistics:
mov r8d,24 + 23            ; Y up left position
mov eax,42 + 23            ; Y down right position
push r13                   ; Note about stack re-alignment for calls
mov ecx,4
@@:
mov r11,[r14]
add r14,8                  ; Sequental access to Min, Max, Average, Median
call HelperTextVarDouble
add r8d,15
add eax,15
loop @b
pop r13
ret
