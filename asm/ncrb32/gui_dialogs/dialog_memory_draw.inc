;---------- Callback dialogue procedures for operations child windows ---------;
;           Handler for "Draw" button at "Memory" screen                       ; 
;                                                                              ;
; INPUT:   [esp + 04] = Parm#1 = HWND = Dialog box handle                      ; 
;          [esp + 08] = Parm#2 = UINT = Message                                ; 
;          [esp + 12] = Parm#3 = WPARAM, message-specific                      ;
;          [esp + 16] = Parm#4 = LPARAM, message-specific                      ;
;                                                                              ;
; OUTPUT:  EAX = status, TRUE = message recognized and processed               ;
;                        FALSE = not recognized, must be processed by OS,      ;
;                        see MSDN for status exceptions and details.           ;  
;                                                                              ;
;------------------------------------------------------------------------------;
DialogProcChildMemoryDraw:
push ebp
mov ebp,esp
push ebx esi edi
mov ebx,PARM_HWNDDLG           ; EBX = Window handle
;---------- Detect message type -----------------------------------------------;
mov eax,PARM_MSG               ; Use EAX for compact CMP, TODO. Optimize width.
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
jmp .finish
.processed:
mov eax,1
.finish:
pop edi esi ebx ebp
ret 16

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
mov edi,APP_DATA
push [edi + APPDATA.createdIcons + MEMORY_ICON_NUMBER * 4]
push ICON_SMALL
push WM_SETICON 
push ebx              ; push PARM_HWNDDLG
call [SendMessage]
mov esi,[edi + APPDATA.lockedStrings]
mov ax,STR_CHILD_MEM_DRAW
call IndexString
push esi
push ebx              ; push PARM_HWNDDLG
call [SetWindowText]
;---------- Set drawings window coordinates if previously saved ---------------;
mov edx,[SAVE_DRAW_POS.top]
test edx,edx
js @f
xor eax,eax
push SWP_SHOWWINDOW + SWP_NOSIZE + SWP_NOZORDER
push eax
push eax
push edx
push [SAVE_DRAW_POS.left]
push eax
push ebx
call [SetWindowPos]
@@: 
;---------- Initialize graphics context ---------------------------------------;
push 0                           ; Parm#1 = Handle to exist DC, 0 = Application current screen
call [CreateCompatibleDC]        ; Create compatible Device Context for buffered video output 
mov [GC_PARMS.handleMemDC],eax            ; Store handle
mov esi,eax                      ; ESI = Store handle for later use
xor eax,eax                      ; EAX = 0 for compact push 0
push eax                         ; Parm#6 = Offset in the file-mapping object, not used here
push eax                         ; Parm#5 = Handle to file-mapping object, not used here
push [GC_PARMS.bitmapPointer]    ; Parm#4 = Pointer to Bitmap Pointer variable
push DIB_RGB_COLORS              ; Parm#3 = Type of color data
push [APP_DATA.lockedBitmapInfo] ; Parm#2 = Pointer to Bitmap Descriptor
push esi                         ; Parm#1 = Handle to a Device Context
call [CreateDIBSection]
mov [GC_PARMS.handleBitmap],eax              
push eax                         ; Parm#2 = Handle to a Bitmap
push esi                         ; Parm#1 = Handle to a Device Context
call [SelectObject]
;---------- Create pen(s) -----------------------------------------------------;
mov esi,[APP_DATA.lockedPensList]  ; ESI = Pointer to color data
push dword [esi]                   ; Parm#3 = Pen color
push 0                             ; Parm#2 = Pen width, 0 means 1 pixel
push PS_SOLID                      ; Parm#1 = Pen style
call [CreatePen]
mov [GC_PARMS.handlePen],eax
;---------- Create brushes ----------------------------------------------------;
cld
mov esi,[APP_DATA.lockedBrushesList] ; ESI = Pointer to color data
mov edi,GC_PARMS.handlesBrushes      ; EDI = Pointer for store brushes handles
mov ebp,4                            ; EBP = Number of brushes
@@:
lodsd                            ; EAX = Color, ESI+4, select next color 
push eax                         ; Parm#1 = Color
call [CreateSolidBrush]
stosd                            ; Store brush handle, EDI+4
dec ebp
jnz @b                           ; Cycle for all brushes
;---------- Create font for texts in the drawings window ----------------------;
xor eax,eax                      ; EAX = 0 for compact push 0
push eax                         ; Parm#14 = Pointer to font typename string, here not used
push VARIABLE_PITCH              ; Parm#13 = Font pitch and family
push CLEARTYPE_QUALITY           ; Parm#12 = Output quality
push CLIP_DEFAULT_PRECIS         ; Parm#11 = Clip precision
push OUT_OUTLINE_PRECIS          ; Parm#10 = Output precision
push DEFAULT_CHARSET             ; Parm#9  = Charset
push eax                         ; Parm#8  = Strike, here=0=none
push eax                         ; Parm#7  = Underline, here=0=none
push eax                         ; Parm#6  = Italic, here=0=none
push FW_DONTCARE                 ; Parm#5  = Weight of the font
push eax                         ; Parm#4 = Orientation
push eax                         ; Parm#3 = Escapment
push eax                         ; Parm#2 = Width
push 16                          ; Parm#1 = Height
call [CreateFont]
mov [GC_PARMS.handleFont],eax 
;--- Create and set timer, for benchmarks progress ( but not for measure ) ----;
push 0                      ; Parm#4 = Notify function pointer, not used here
push TIMER_TICK_SHOW        ; Parm#3 = Timer ticks period, ms
push 0                      ; Parm#2 = Event ID
push ebx                    ; Parm#1 = Parent window handle
call [SetTimer]
;---------- Wait thread started ( 1 of 2 waits ) ------------------------------;
; Wait for measurement thread run, this event at thread entry point
; Note status after [WaitForSingleObject] ignored. TODO. Check it.
push THREAD_TIME_LIMIT                ; Parm#2 = Time limit
push [DRAW_THREAD_PARMS.eventStart]   ; Parm#1 = Handle for start event
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
push SAVE_DRAW_POS
push ebx
call [GetWindowRect]
;---------- Stop measurement thread -------------------------------------------;
mov [DRAW_THREAD_PARMS.measureBreak],1
;---------- Wait thread terminated ( 2 of 2 waits ) ---------------------------; 
; Wait for measurement thread done, this event at thread exit point
push THREAD_TIME_LIMIT                    ; Parm#2 = Time limit
push [DRAW_THREAD_PARMS.eventDone]   ; Parm#1 = Handle for done event
call [WaitForSingleObject]
;---------- Wait thread terminated by thread handle ---------------------------;
; Note status after [WaitForSingleObject] ignored. TODO. Check it.
push THREAD_TIME_LIMIT                 ; Parm#2 = Time limit
push [DRAW_THREAD_PARMS.threadHandle]  ; Parm#1 = Thread handle
call [WaitForSingleObject]
;---------- De-Initialize graphics context ------------------------------------;
push [GC_PARMS.handleMemDC]
call [DeleteDC]
push [GC_PARMS.handleBitmap]
call [DeleteObject]
;---------- Delete pen(s) -----------------------------------------------------;
push [GC_PARMS.handlePen]              ; Parm#1 = Pen handle 
call [DeleteObject]
;---------- Delete brushes ----------------------------------------------------;
cld
mov esi,GC_PARMS.handlesBrushes   ; ESI = Pointer for store brushes handles
mov ebp,4                  ; EBP = Number of brushes
@@:
lodsd                      ; EAX = Brush handle
push eax                   ; Parm#1 = Handle
call [DeleteObject]
dec ebp
jnz @b
;---------- Delete font -------------------------------------------------------;
push [GC_PARMS.handleFont]
call [DeleteObject]
;---------- Delete timer ------------------------------------------------------;
push 0                     ; Parm#2 = Set same as for SetTimer function
push ebx                   ; Parm#1 = Parent window handle
call [KillTimer]
;---------- Destroy window, disable draw counter ------------------------------;
push 1                           ; Parm#2 = Result for return
push ebx                         ; Parm#1 = Window handle
call [EndDialog]
jmp .processed

;------------------------------------------------------------------------------;
;                                                                              ;
;                           WM_COMMAND handler:                                ;
;                        interpreting user input.                              ; 
;                                                                              ;
;------------------------------------------------------------------------------;
.wmcommand:
mov eax,PARM_WPARAM
cmp eax,IDB_MD_CANCEL   ; Detect click "Exit" button in the draw child window
je .wmclose             ; Go close window if match
cmp eax,IDB_MD_REFRESH  ; Detect click "Refresh" button in the draw child window
jne .processed          ; Go skip if mismatch  
;---------- Handling "Refresh" button -----------------------------------------;
mov al,WAIT_MODE_MEMORY
call OpenPleaseWait                 ; Open "Please wait" window
; Set thread break flag, cause measurement thread termination 
mov [DRAW_THREAD_PARMS.measureBreak],1
; Wait for measurement thread done, this event at thread exit point
push THREAD_TIME_LIMIT              ; Parm#2 = Time limit
push [DRAW_THREAD_PARMS.eventDone]  ; Parm#1 = Handle for done event
call [WaitForSingleObject]
; Wait measurement thread terminated by thread handle
; Note status after [WaitForSingleObject] ignored. TODO. Check it.
push THREAD_TIME_LIMIT                 ; Parm#2 = Time limit
push [DRAW_THREAD_PARMS.threadHandle]  ; Parm#1 = Thread handle
call [WaitForSingleObject]
; Remove thread and associated handles, note can corrupt pointers
call DrawSessionStop
; Restart
mov [DRAW_PARMS.showStatic],1
call DrawSessionStart                ; note can corrupt pointers
jc .runtimeError                     ; Go error message box if status bad
; Wait for measurement thread run, this event at thread entry point
; Note status after [WaitForSingleObject] ignored. TODO. Check it.
push THREAD_TIME_LIMIT               ; Parm#2 = Time limit
push [DRAW_THREAD_PARMS.eventStart]  ; Parm#1 = Handle for start event
call [WaitForSingleObject]
;--- Conditionally wait measurements for all points if Silent mode ------------; 
test [BIND_LIST.getMemSilent],0001b  ; Check Silent mode
jz .noSilent                         ; Go skip if non silent mode
.waitSilent:
push 100                             ; Units = milliseconds
call [Sleep]
cmp [DRAW_THREAD_PARMS.measureCounter],DRAW_POINTS_COUNT
jb .waitSilent
.noSilent:
call ClosePleaseWait             ; Close "Please wait" window
jmp .processed
;---------- Handler for runtime errors ----------------------------------------;
.runtimeError:
push eax
call ClosePleaseWait                 ; Close "Please wait" window
pop eax
mov esi,[APP_DATA.lockedStrings]
movzx eax,al
add eax,STR_RUNTIME_ALLOC
call IndexString       ; Return ESI = Selected string address 
push MB_ICONERROR      ; Parm#4 = Attributes
push PROGRAM_NAME      ; Parm#3 = Pointer to title (caption) string
push esi               ; Parm#2 = Pointer to string: error name 
push ebx               ; Parm#1 = Parent Window = NULL
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
push 0                 ; Parm#3 = Background erase flag, 0=Background unchanged
push 0                 ; Parm#2 = Pointer to invalidated region, 0=All window
push ebx               ; Parm#1 = Window handle
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
mov esi,DRAW_THREAD_PARMS
mov ebp,[esi + DTHP.measureCounter]

;---------- Skip statistics if yet no results ---------------------------------;
cmp [esi + DTHP.visualCounter],ebp
je .skipStatistics                         ; Go skip if no measurements added
cmp ebp,2
jb .skipStatistics                         ; Go skip if number of points < 2
mov [esi + DTHP.visualCounter],ebp

;---------- Start calculations ------------------------------------------------;
finit
fld [MEM_OPB.tscPeriodNs]
fldz
lea edi,[esi + DTHP.measureArray]
mov edx,DRAW_POINTS_COUNT * 8
push 0 0 ebp                                              ; + 3 dwords to stack
; Copy CPI (Clock Per Instruction) array and sum for averaging CPI
.cpiCopySum:
fadd qword [edi]
mov eax,[edi + 0]
mov ecx,[edi + 4]
mov [edi + edx + 0],eax
mov [edi + edx + 4],ecx
add edi,8
dec ebp
jnz .cpiCopySum
fidiv dword [esp]
; Sorting CPI for median,
; note can use integer 64-bit compare for floating point values 
.cpiReSort:
lea edi,[esi + DTHP.calculateArray]
mov ebp,[esi + DTHP.visualCounter]
dec ebp
xor eax,eax
.cpiSort:
mov ecx,[edi + 0 + 4]
cmp ecx,[edi + 8 + 4]
jb .cpiSorted
ja .cpiSortRequired
mov ecx,[edi + 0 + 0]
cmp ecx,[edi + 8 + 0]
jbe .cpiSorted 
.cpiSortRequired:
mov ecx,[edi + 0 + 0]
xchg ecx,[edi + 8 + 0]
mov [edi + 0 + 0],ecx
mov ecx,[edi + 0 + 4]
xchg ecx,[edi + 8 + 4]
mov [edi + 0 + 4],ecx
mov eax,1
.cpiSorted:
add edi,8
dec ebp
jnz .cpiSort
test eax,eax
jnz .cpiReSort 
; Get minimum and maximum CPI
mov eax,[esi + DTHP.visualCounter]
mov edx,eax
dec eax
fld [esi + DTHP.calculateArray + 0]
fld [esi + DTHP.calculateArray + eax * 8]
; Calculate median CPI, note assume DRAW_POINTS_COUNT even 
shr edx,1
jc .cpiMedianOdd
fld [esi + DTHP.calculateArray + edx * 8 - 8]
fadd [esi + DTHP.calculateArray + edx * 8 - 0]
fld1
fadd st0,st0
fdivp st1,st0
jmp .cpiMedianDone
.cpiMedianOdd:
fld [esi + DTHP.calculateArray + edx * 8 - 0]
.cpiMedianDone:
; Write statistics to thread parameters variables, 
; CPI (Clocks Per Instruction) and nsPI (Nanoseconds Per Instruction)
fst [esi + DTHP.statCpiMedian]
fmul st0,st4
fstp [esi + DTHP.statNspiMedian]
fst [esi + DTHP.statCpiMax]
fmul st0,st3
fstp [esi + DTHP.statNspiMax]
fst [esi + DTHP.statCpiMin]
fmul st0,st2
fstp [esi + DTHP.statNspiMin]
fst [esi + DTHP.statCpiAverage]
fmulp st1,st0
fstp [esi + DTHP.statNspiAverage]
; Calculate constant for convert CPI array to MBPS array
mov eax,[MEM_IPB.operandWidth]   ; EAX = bits per instruction
shr eax,3                        ; EAX = bytes per instruction 
mov edx,[MEM_IPB.updatedThreads]
mul edx
mov ecx,NSPI_TO_MBPS             ; This constant for decimal megabytes and ns.
mul ecx
push edx eax
fild qword [esp]
fdiv [MEM_OPB.tscPeriodNs]
pop edx eax
fst qword [esp + 4]              ; Save value for convert CPI to MBPS
lea edi,[esi + DTHP.calculateArray]
mov ebp,[esi + DTHP.visualCounter]
fldz
; Convert CPI array to MBPS array, calculate MBPS sum
.mbpsCalcSum:
fld st1
fdiv qword [edi]
fadd st1,st0
fstp qword [edi]
add edi,8
dec ebp
jnz .mbpsCalcSum
fidiv dword [esp]
; Get minimum and maximum MBPS, reverse from CPI
mov eax,[esi + DTHP.visualCounter]
mov edx,eax
dec eax
fld [esi + DTHP.calculateArray + eax * 8]
fld [esi + DTHP.calculateArray + 0]
; Calculate median MBPS 
shr edx,1
jc .mbpsMedianOdd
fld [esi + DTHP.calculateArray + edx * 8 - 8]
fadd [esi + DTHP.calculateArray + edx * 8 - 0]
fld1
fadd st0,st0
fdivp st1,st0
jmp .mbpsMedianDone
.mbpsMedianOdd:
fld [esi + DTHP.calculateArray + edx * 8 - 0]
.mbpsMedianDone:
; Write statistics to thread parameters variables, 
; MBPS (Megabytes Per Second) 
fstp [esi + DTHP.statMbpsMedian]
fstp [esi + DTHP.statMbpsMax]
fstp [esi + DTHP.statMbpsMin]
fstp [esi + DTHP.statMbpsAverage]
ffree st0
fincstp
; Select Y-scale by statistics
; Note FPU rounding mode temporary changed for this operation
cmp [DRAW_PARMS.selectMode],0
jne .nanoseconds 
fld [esi + DTHP.statMbpsMax]  ; Get maximum MBPS if Bandwidth mode
jmp .unitsSelected
.nanoseconds:
fld [esi + DTHP.statNspiMax]  ; Get maximum nsPI if Latency mode
.unitsSelected:
push 0 9             ; Divisor = coordinate grid count = 9
fidiv dword [esp]
fstcw [esp + 4]      ; Store FPU Control Word
mov ax,[esp + 4]
and ax,0F3FFh        ; Mask RC (Rounding Control), bits [11,10]
or ax,00800h         ; Set RC = 10b = Round Up
mov [esp + 6],ax
fldcw [esp + 6]      ; Load changed FPU Control Word
fistp dword [esp]    ; Store result (valueGridY) at Round Up mode
fldcw [esp + 4]      ; Restore original FPU Control Word
pop eax edx          ; EAX = calculated valueGridY, POP EDX for stack release 
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
push eax -GRIDSTEPY                                       ; + 2 dwords to stack
fild dword [esp]        ; ST0 = - grid step, pixels
fidiv dword [esp + 4]   ; ST0 = ( - grid step, pixels ) / ( Y units per grid )      
fstp [DRAW_PARMS.yMultiplier]
; Calculate data array for polyline Performance = f( Block size )
push LINE_BASE_Y
fild dword [esp]
pop eax
lea edi,[esi + DTHP.measureArray]
lea edx,[esi + DTHP.polylineArray]
mov ebp,[esi + DTHP.visualCounter]
mov eax,LINE_BASE_X
cmp [DRAW_PARMS.selectMode],0
jne .branchLatency 
; Line for bandwidth
fld qword [esp + 4 * 3]           ; Load value for convert CPI to MBPS
fmul [DRAW_PARMS.yMultiplier]
; First extra element required because N points means N-1 Polyline intervals
fld st0
fdiv qword [edi]
fadd st0,st2
mov [edx + 0],eax           ; Store POINT.x
fistp dword [edx + 4]       ; Store POINT.y 
inc eax
add edx,8
; Cycle for other elements
.lineWriteBandwidth:
fld st0
fdiv qword [edi]
fadd st0,st2
mov [edx + 0],eax           ; Store POINT.x
fistp dword [edx + 4]       ; Store POINT.y 
inc eax
add edi,8
add edx,8
dec ebp
jnz .lineWriteBandwidth
jmp .linesDone
; Line for latency  
.branchLatency:
fld [MEM_OPB.tscPeriodNs] 
fmul [DRAW_PARMS.yMultiplier]
; First extra element required because N points means N-1 Polyline intervals
fld qword [edi]
fmul st0,st1
fadd st0,st2 
mov [edx + 0],eax           ; Store POINT.x
fistp dword [edx + 4]       ; Store POINT.y
inc eax
add edx,8
; Cycle for other elements
.lineWriteLatency:
fld qword [edi]
fmul st0,st1
fadd st0,st2 
mov [edx + 0],eax           ; Store POINT.x
fistp dword [edx + 4]       ; Store POINT.y
inc eax
add edi,8
add edx,8
dec ebp
jnz .lineWriteLatency
; Done for bandwidth and latency
.linesDone:
ffree st0
fincstp
ffree st0
fincstp
add esp,4 * 5          ; Release stack

.skipStatistics:
;---------- End of conditionally run section for calculate statistics ---------;

;---------- Setup paint context -----------------------------------------------;
push ebx
push PAINT_STRUCT     ; Parm#2 = Pointer to PAINTSTRUCT structure
push ebx              ; Parm#1 = Parent window handle
call [BeginPaint]
mov [GC_PARMS.handleDC],eax
mov esi,[GC_PARMS.handleMemDC]  ; ESI = Graphics transit buffer handle
mov edi,GRAPH_RECT              ; EDI = Pointer to rectangle structure
;---------- Setup font and color ----------------------------------------------;
push [GC_PARMS.handleFont]   ; Parm#2 = Handle to Selected Object
push esi                     ; Parm#1 = Handle to a Device Context
call [SelectObject]
push COLOR_TEXT_VALUES       ; Parm#2 = Text front color value = 00bbggrrh
push esi                     ; Parm#1 = Handle to a Device Context
call [SetTextColor]
push COLOR_TEXT_BACK         ; Parm#2 = Text background color value = 00bbggrrh
push esi                     ; Parm#1 = Handle to a Device Context
call [SetBkColor]

;---------- Skip static GUI objects if not a first pass -----------------------;
xor eax,eax
xchg eax,[DRAW_PARMS.showStatic]
test eax,eax
jz .skipStatic

;---------- Blank work field, background color --------------------------------;
mov [edi + RECT.left],0
mov [edi + RECT.top],0
mov [edi + RECT.right],SUBWINX
mov [edi + RECT.bottom],SUBWINY
push [GC_PARMS.handlesBrushes + 8]  ; Parm#3 = Handle to a color brush
push edi                            ; Parm#2 = Pointer to RECT structure
push esi                            ; Parm#1 = Handle to a Device Context
call [FillRect]
;---------- Coordinate X-grid, vertical lines ---------------------------------;
mov ebx,GRIDBLANKX         ; EBX = Base point, X
mov ebp,GRIDX              ; EBP = Number of vertical lines
mov [edi + RECT.top],0
mov [edi + RECT.bottom],SUBWINY - GRIDBLANKY  ; Base point, Y
.xVgrid:
lea eax,[ebx + 1]          ; Line width = 1 pixel
mov [edi + RECT.left],ebx
mov [edi + RECT.right],eax
push [GC_PARMS.handlesBrushes + 00]  ; Parm#3 = Handle to a color brush
push edi                   ; Parm#2 = Pointer to RECT structure
push esi                   ; Parm#1 = Handle to a Device Context
call [FillRect]            ; Fill rectangle function
add ebx,GRIDSTEPX          ; Add X grid step
dec ebp                    ; Cycle for X-grid vertical lines
jnz .xVgrid
;---------- Coordinate Y-grid, horizontal lines -------------------------------;
mov ebx,SUBWINY - GRIDBLANKY  ; EBX = Base point, Y
mov ebp,GRIDY                 ; EBP = Number of horizontal lines
mov [edi + RECT.left],GRIDBLANKX
mov [edi + RECT.right],SUBWINX
.yHgrid:
lea eax,[ebx - 1]             ; Line width = 1 pixel
mov [edi + RECT.top],eax
mov [edi + RECT.bottom],ebx
push [GC_PARMS.handlesBrushes + 00]  ; Parm#3 = Handle to a color brush
push edi                   ; Parm#2 = Pointer to RECT structure
push esi                   ; Parm#1 = Handle to a Device Context
call [FillRect]            ; Fill rectangle function
sub ebx,GRIDSTEPY          ; Subtract Y grid step
dec ebp                    ; Cycle for Y-grid vertical lines
jnz .yHgrid
;---------- Initializing values for X grid text write cycle -------------------;
; Note. X text don't depends on measurements, Y text depends on measurements.
push GRIDX                 ; Dword [esp + 8] = Number of lines, X
push GRIDBLANKX            ; Dword [esp + 4] = Base point, X
push SUBWINY - 20          ; Dword [esp + 0] = Base point Y, 20 pixels up
xor ebp,ebp                ; EBP = 0 = Units value, X starts from 0
;---------- X grid text write cycle -------------------------------------------;
.xGrid:
;---------- Build ASCII string ------------------------------------------------;
push edi
mov edi,TEMP_BUFFER     ; EDI = Pointer for print decimal string as ASCII
mov edx,edi
mov bl,0                ; BL = Template control for print number, 0=No template
mov eax,ebp             ; EAX = Value for print, Bytes/KB/MB
call DecimalPrint32     ; Print number
mov ecx,edi
sub ecx,edx
mov al,0
stosb                   ; Store terminator byte = 0
pop edi
;---------- Set region for text string write ----------------------------------;
mov eax,[esp + 00 + 00]
add eax,18  
push eax                    ; Parm#5 = Y coordinate down/right corner
mov eax,[esp + 04 + 04]
add eax,65
push eax                    ; Parm#4 = X coordinate down/right corner
push dword [esp + 00 + 08]  ; Parm#3 = Y coordinate up/left corner
push dword [esp + 04 + 12]  ; Parm#2 = X coordinate up/left corner
push edi                    ; Parm#1 = Pointer to RECT structure to store parameters
call [SetRect]              ; Set region rectangle function 
;---------- Draw text ---------------------------------------------------------;
push DT_LEFT      ; Parm#5 = Method of formatting/alignment of text string
push edi          ; Parm#4 = Pointer to RECT structure, initialized by [SetRect] function
push -1           ; Parm#3 = String length, -1 means NULL-terminated string
push TEMP_BUFFER  ; Parm#2 = Pointer to text string
push esi          ; Parm#1 = Handle to a Device Context
call [DrawText]   ; Draw text function 
;---------- Modify parameters and make cycle ----------------------------------;
add ebp,[DRAW_PARMS.valueGridX]   ; Addend per X grid
add dword [esp + 04 + 00],GRIDSTEPX
dec dword [esp + 08 + 00]
jnz .xGrid 
;---------- X-units name (B/KB/MB), change color, select string, write --------;
push COLOR_TEXT_UNITS        ; Parm#2 = Text front color value = 00bbggrrh
push esi                     ; Parm#1 = Handle to a Device Context
call [SetTextColor]
mov eax,[DRAW_PARMS.selectUnits]
mov ebp,[UNITS_TEXTS.bytes]  ; EBP = Pointer to text string, Cache&RAM mode
cmp al,0
je @f
mov ebp,[UNITS_TEXTS.kb]
cmp al,1
je @f
mov ebp,[UNITS_TEXTS.mb]
@@:
mov eax,SUBWINX - 49      ; Transit parm#2 = X coordinate up/left corner
mov ebx,SUBWINY - 43      ; Transit parm#3 = Y coordinate up/left corner
mov ecx,SUBWINX - 13      ; Transit parm#4 = X coordinate down/right corner
mov edx,SUBWINY - 25      ; Transit parm#5 = Y coordinate down/right corner
call HelperTextConstDraw
;---------- Y-units name (MBPS/nanoseconds), use color from previous step -----;
mov ebp,[UNITS_TEXTS.mbps]   ; EBP = Pointer to text string , Mode = Cache&RAM
cmp [DRAW_PARMS.selectMode],0
je @f
mov ebp,[UNITS_TEXTS.nanoseconds]
@@:
mov eax,7                 ; Transit parm#2 = X coordinate up/left corner
mov ebx,5                 ; Transit parm#3 = Y coordinate up/left corner
mov ecx,40 + 7 + 20 + 24  ; Transit parm#4 = X coordinate down/right corner
mov edx,18 + 5            ; Transit parm#5 = Y coordinate down/right corner
call HelperTextConstDraw
;---------- Print statistics table, lines and texts ---------------------------;
; Horizontal sequence of strings:  CPI, nsPI, MBPS
mov dword [esp + 04],CLKS_MBPS_TEXTS      ; Dword [esp + 04] = Pointer to text strings pointers table
mov dword [esp + 00],DRAW_TABLE_UP_COUNT  ; Dword [esp + 00] = Strings count
mov eax,522               ; Transit parm#2 = X coordinate up/left corner
mov ecx,522 + 40          ; Transit parm#4 = X coordinate down/right corner
mov edx,27 + 17           ; Transit parm#5 = Y coordinate down/right corner
@@:
mov ebx,[esp + 04]        ; EBP = Pointer to text string
add dword [esp + 04],4    ; Update pointer to next offset
mov ebp,[ebx]             ; Can mov ebp,[ebp] but don't use stack segment
mov ebx,27                ; Transit parm#3 = Y coordinate up/left corner
call HelperTextConstDraw
add eax,64                ; Shift 64 points right, X + 64
add ecx,64                ; Shift 64 points right, X + 64
dec dword [esp + 00]
jnz @b                    ; Cycle for: CPI, nsPI, MBPS 
;---------- Vertical sequence of strings: maximum, minimum, average, median ---;
; dword [esp + 04] = Pointer to continue of text strings pointers table
mov dword [esp + 00],DRAW_TABLE_LEFT_COUNT  ; Dword [esp + 00] = Strings count
mov ebx,27 + 19           ; Transit parm#3 = Y coordinate up/left corner
mov ecx,458 + 60          ; Transit parm#4 = X coordinate down/right corner
mov edx,27 + 19 + 17      ; Transit parm#5 = Y coordinate down/right corner
@@:
mov eax,[esp + 04]        ; Get pointer to text string
add dword [esp + 04],4    ; Update pointer to next offset
mov ebp,[eax]             ; Can mov ebp,[ebp] but don't use stack segment
mov eax,458               ; Transit parm#2 = X coordinate up/left corner
call HelperTextConstDraw
add ebx,15                ; Shift 15 points down, Y + 15
add edx,15                ; Shift 15 points down, Y + 15
dec dword [esp + 00]
jnz @b                    ; Cycle for: maximum, minimum, average, median
;---------- Lines of table, up horizontal line and left vertical line ---------;
mov [edi + RECT.left],519
mov [edi + RECT.top],27 + 17
mov [edi + RECT.right],519 + 240
mov [edi + RECT.bottom],27 + 17 + 2
push [GC_PARMS.handlesBrushes + 12]  ; Parm#3 = Handle to a color brush
push edi                             ; Parm#2 = Pointer to RECT structure
push esi                             ; Parm#1 = Handle to a Device Context
call [FillRect]               ; Fill rectangle function, up horizontal line
mov [edi + RECT.left],518
mov [edi + RECT.top],27 + 19
mov [edi + RECT.right],518 + 2
mov [edi + RECT.bottom],27 + 19 + 17*4
push [GC_PARMS.handlesBrushes + 12]  ; Parm#3 = Handle to a color brush
push edi                             ; Parm#2 = Pointer to RECT structure
push esi                             ; Parm#1 = Handle to a Device Context
call [FillRect]               ; Fill rectangle function, left vertical line
;---------- Target method CPU instructions dump -------------------------------;
mov ebx,MEM_IPB
mov dword [esp + 08],0
mov eax,[ebx + MEMIPB.dumpMethodStart]
mov [esp + 04],eax
mov eax,[ebx + MEMIPB.dumpMethodLength]
mov [esp + 00],eax
test eax,eax
jz .skipAsmDump
cmp eax,128
jbe @f
mov dword [esp + 00],128
@@:
push COLOR_TEXT_DUMP      ; Parm#2 = Text front color value = 00bbggrrh
push esi                  ; Parm#1 = Handle to a Device Context
call [SetTextColor]
.asmDump:
mov ebp,TEMP_BUFFER
push edi
mov edi,ebp
mov eax,[esp + 04 + 04]
add eax,[esp + 08 + 04]
mov al,[eax]
call HexPrint8
mov al,0
stosb
pop edi
;---------- Set coordinates for dump ------------------------------------------;
mov eax,SUBWINX - 250 + 64   ; Transit parm#2 = X coordinate, up left corner
mov ebx,95 + 48              ; Transit parm#3 = Y coordinate up/left corner
mov ecx,SUBWINX - 005 + 64   ; Transit parm#4 = X coordinate down/right corner
mov edx,95 + 18 + 48         ; Transit parm#5 = Y coordinate down/right corner 
;---------- Adjust coordinates for dump, print dump ---------------------------;
push esi
mov esi,[esp + 08 + 04]       ; dword [esp + 00] = counter, ESI = counter temporary copy
and esi,07h                   ; Bits 0-2 used for X-positioning
imul esi,esi,21               ; X scale = 21 pixels per char
add eax,esi                   ; Add offset to X-start
add ecx,esi                   ; Add offset to X-end
mov esi,[esp + 08 + 04]       ; dword [esp + 00] = counter, ESI = counter temporary copy
shr esi,3
and esi,0Fh                   ; Bits 3-6 used for Y-positioning
imul esi,esi,18               ; Y scale = 18 pixels per char 
add ebx,esi                   ; Add offset to Y-start
add edx,esi                   ; Add offset to Y-end
pop esi
call HelperTextConstDraw
inc dword [esp + 08]
dec dword [esp + 00]
jnz .asmDump 
.skipAsmDump:
;---------- TSC clock frequency -----------------------------------------------;
push edi
mov edi,DRAW_TSC_VALUE
mov word [edi],0000h + '?'                   ; This char '?' used if error
mov eax,dword [MEM_OPB.tscFrequencyMHz + 0]  ; EDX:EAX = TSC Frequency, MHz
mov edx,dword [MEM_OPB.tscFrequencyMHz + 4]
mov ecx,eax                                  ; Frequency = 0 means error
or ecx,edx
jz @f                                  ; Go skip value write if error
mov bx,0100h                           ; BX=Template X.Y , 1 digit after '.'
call DoublePrint
mov al,0
stosb
@@:
pop edi
push COLOR_TEXT_INFO         ; Parm#2 = Text front color value = 00bbggrrh
push esi                     ; Parm#1 = Handle to a Device Context
call [SetTextColor]
mov eax,SUBWINX - 187        ; Transit Parm#2 = X coordinate up/left corner
mov ebx,6                    ; Transit Parm#3 = Y coordinate up/left corner
mov ecx,SUBWINX - 5          ; Transit Parm#4 = X coordinate down/right corner
mov edx,6+18                 ; Transit Parm#5 = Y coordinate down/right corner
mov ebp,DRAW_TSC             ; EBP = Pointer to text string
call HelperTextConstDraw

;---------- Point for skip static GUI objects if not a first pass -------------;
add esp,12
.skipStatic:
sub esp,12

;---------- Skip results visualization if yet no results ----------------------;
cmp [DRAW_THREAD_PARMS.visualCounter],2
jb .skipDynamic                             ; Go skip if no measurements added

;---------- Conditionally run section for visual measurement results ----------;
; Draw Y-axis text depend on re-scale, write statistics and line Y=F(X). 
;---------- Setup font and color ----------------------------------------------;
push [GC_PARMS.handleFont]   ; Parm#2 = Handle to Selected Object
push esi                     ; Parm#1 = Handle to a Device Context
call [SelectObject]
push COLOR_TEXT_VALUES       ; Parm#2 = Text front color value = 00bbggrrh
push esi                     ; Parm#1 = Handle to a Device Context
call [SetTextColor]
push COLOR_TEXT_BACK         ; Parm#2 = Text background color value = 00bbggrrh
push esi                     ; Parm#1 = Handle to a Device Context
call [SetBkColor]
;---------- Initializing values for Y grid text write cycle -------------------;
; Note. X text don't depends on measurements, Y text depends on measurements.
mov dword [esp + 08],GRIDY - 1                              ; Dword [esp + 8] = Number of lines, Y
mov dword [esp + 04],GRIDBLANKX - 68                        ; Dword [esp + 4] = X coordinate
mov dword [esp + 00],SUBWINY - GRIDBLANKY - GRIDSTEPY - 15  ; Dword [esp + 0] = Y coordinate
mov ebp,[DRAW_PARMS.valueGridY]     ; EBP = Units value, Y starts with offset
;---------- Y grid text write cycle -------------------------------------------;
.yGrid:
;---------- Build ASCII string ------------------------------------------------;
push edi
mov edi,TEMP_BUFFER     ; EDI = Pointer for print decimal string as ASCII
mov edx,edi
mov bl,0                ; BL = Template control for print number, 0=No template
mov eax,ebp             ; EAX = Value for print, Bytes/KB/MB
call DecimalPrint32     ; Print number
mov ecx,edi
sub ecx,edx
mov al,0
stosb                   ; Store terminator byte = 0
pop edi
;---------- Calculate correction value for Y axis grid values print -----------;
mov edx,9
sub edx,ecx             ; EDX = chars positions count
jbe .rejectAddend       ; Go skip if correction not required 
imul edx,edx,7
jmp .doneAddend 
.rejectAddend:
xor edx,edx
.doneAddend:
;---------- Set region for text string write ----------------------------------;
mov eax,[esp + 00 + 00]
add eax,18
push eax                    ; Parm#5 = Y coordinate down/right corner
mov eax,[esp + 04 + 04]
lea eax,[eax + edx + 65]
push eax                    ; Parm#4 = X coordinate down/right corner
push dword [esp + 00 + 08]  ; Parm#3 = Y coordinate up/left corner
mov eax,[esp + 04 + 12]
add eax,edx
push eax                    ; Parm#2 = X coordinate up/left corner
push edi                    ; Parm#1 = Pointer to RECT structure to store parameters
call [SetRect]              ; Set region rectangle function 
;---------- Draw text ---------------------------------------------------------;
push DT_LEFT      ; Parm#5 = Method of formatting/alignment of text string
push edi          ; Parm#4 = Pointer to RECT structure, initialized by [SetRect] function
push -1           ; Parm#3 = String length, -1 means NULL-terminated string
push TEMP_BUFFER  ; Parm#2 = Pointer to text string
push esi          ; Parm#1 = Handle to a Device Context
call [DrawText]   ; Draw text function 
;---------- Modify parameters and make cycle ----------------------------------;
add ebp,[DRAW_PARMS.valueGridY]   ; Addend per Y grid
sub dword [esp + 00 + 00],GRIDSTEPY
dec dword [esp + 08 + 00]
jnz .yGrid 
;---------- Visual statistics -------------------------------------------------;     
; Set color for text
push COLOR_TEXT_INFO     ; Parm#2 = Text front color value = 00bbggrrh
push esi                 ; Parm#1 = Handle to a Device Context
call [SetTextColor]
; This text strings must be updated at each visualization tick
; Initializing parameters for benchmark statistics, bandwidth and timings
mov ebp,0300h
;---------- Print timings statistics, CPI minimum, maximum, average, median ---;
; note CPI = Clocks Per Instruction
mov ebx,DRAW_THREAD_PARMS.statCpiMin   ; EBX = variables group base
mov eax,SUBWINX - 250
mov ecx,SUBWINX - 190 
call HelperWriteStatistics
;---------- Print timings statistics, nsPI minimum, maximum, average, median --;
; note nsPI = nanoseconds per instruction
mov ebx,DRAW_THREAD_PARMS.statNspiMin  ; EBX = variables group base
mov eax,SUBWINX - 250 + 64*1
mov ecx,SUBWINX - 190 + 64*1 
call HelperWriteStatistics
;---------- This number template for Megabytes per Second ---------------------;
mov ebp,0200h
;---------- Print timings statistics, MBPS minimum, maximum, average, median --;
; note MBPS = megabytes per second
; note Min/Max here swapped, because min. time means max. bandwidth
mov ebx,DRAW_THREAD_PARMS.statMbpsMax  ; EBX = variables group base
mov eax,SUBWINX - 250 + 64*2
mov ecx,SUBWINX - 150 + 64*2 
call HelperWriteStatistics
;--- Draw PolyLine by measurement results Performance = F ( Block Size ) ------;
push [GC_PARMS.handlePen]              ; Parm#2 = Pen handle
push esi                               ; Parm#1 = Handle to a Device Context
call [SelectObject]
mov edi,eax
mov eax,[DRAW_THREAD_PARMS.visualCounter]
inc eax                                ; +1, because N points means N-1 Polyline intervals
push eax                               ; Parm#3 = Number of points in the array
push DRAW_THREAD_PARMS.polylineArray   ; Parm#2 = Pointer to POINT structures array
push esi                               ; Parm#1 = Handle to a Device Context
call [Polyline]
push edi                               ; Parm#2 = Pen handle, restore original
push esi                               ; Parm#1 = Handle to a Device Context
call [SelectObject]

.skipDynamic:
;---------- End of conditionally run section for visual measurement results ---;

add esp,12                 ; Release stack
;---------- Copy graphics objects from transit buffer to graphics memory ------;
xor eax,eax                ; EAX = 0 for compact push 0
push SRCCOPY               ; Parm#9 = Raster operation code
push eax                   ; Parm#8 = Y source
push eax                   ; Parm#7 = X source
push esi                   ; Parm#6 = Handle of source Device Context
push SUBWINY               ; Parm#5 = Height
push SUBWINX               ; Parm#4 = Width
push SHIFTY                ; Parm#3 = Y destination
push SHIFTX                ; Parm#2 = X destination
push [GC_PARMS.handleDC]   ; Parm#1 = Handle of destination Device Context
call [BitBlt]                       
;---------- Delete paint context ----------------------------------------------;
pop ebx
push PAINT_STRUCT       ; Parm#2 = Pointer to PAINTSTRUCT structure
push ebx                ; Parm#1 = Parent window handle
call [EndPaint]         ; This WinAPI returns BOOL status, but ignored
jmp .processed

;------------------------- Helpers for drawings -------------------------------;

;--- Print text string --------------------------------------------------------;
;                                                                              ;
; INPUT:   ESI = Handle of a Device Context                                    ;
;          EDI = Pointer to RECT structure for draw region                     ;
;          EBP = Pointer to text string                                        ;
;          EAX = xLeft                                                         ;
;          EBX = yTop                                                          ;
;          ECX = xRight                                                        ;
;          EDX = yBottom                                                       ;
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          All registers is non volatile for this subroutine                   ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperTextConstDraw:
pushad
push edx              ; Parm#5 = yBottom coordinate, down/right corner
push ecx              ; Parm#4 = xRight coordinate, down/right corner
push ebx              ; Parm#3 = yTop coordinate, up/left corner
push eax              ; Parm#2 = xLeft coordinate, up/left corner
push edi              ; Parm#1 = Pointer to RECT structure to store parameters
call [SetRect]        ; Set region rectangle function 
push DT_LEFT          ; Parm#5 = Method of formatting/alignment of text string
push edi              ; Parm#4 = Pointer to RECT structure, initialized by [SetRect] function
push -1               ; Parm#3 = String length, -1 means NULL-terminated string
push ebp              ; Parm#2 = Pointer to text string
push esi              ; Parm#1 = Handle of a Device Context
call [DrawText]       ; Set region rectangle function 
popad
ret
;--- Print floating point value, double ---------------------------------------;
;                                                                              ;
; INPUT:   ESI = Handle of a Device Context                                    ;
;          EDI = Pointer to RECT structure for draw region                     ;
;          EBP = Numeric template for floating point print                     ;
;          EAX = xLeft                                                         ;
;          EBX = yTop                                                          ;
;          ECX = xRight                                                        ;
;          EDX = yBottom                                                       ;
;          Qword [ESP + 04] = Floating point value, double, 64-bit,            ;
;                             after caller push qword and call store EIP       ;
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          All registers is non volatile for this subroutine                   ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperTextVarDouble:
pushad
pushad
mov edi,TEMP_BUFFER
push edi
mov ecx,11 - 3        ; 11 (not 9) for blank if variable X-size font 
mov ax,0000h + '?'    ; Blank field with "?" means yet not determined
stosw
mov al,' '
rep stosb             ; Blank entire field, if next value shorter then previous
mov al,0
stosb
pop edi
mov ebx,ebp
mov eax,[esp + 04 + 32*2]
mov edx,[esp + 08 + 32*2]
mov ecx,eax
or ecx,edx
jz @f
call DoublePrint
@@:
popad
mov ebp,TEMP_BUFFER
call HelperTextConstDraw 
popad
ret
;--- Print cycle for Min, Max, Average, Median --------------------------------;
;                                                                              ;
; INPUT:   ESI = Handle of a Device Context                                    ;
;          EDI = Pointer to RECT structure for draw region                     ;
;          EBP = Numeric template for floating point print                     ;
;          EBX = Pointer to variables group                                    ;
;          EAX = X up left position                                            ;
;          ECX = X down right position                                         ;
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          All registers is non volatile for this subroutine                   ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperWriteStatistics:
pushad
push ebx 0 0 0
mov ebx,24 + 23              ; Y up left position
mov edx,42 + 23              ; Y down right position
mov dword [esp + 08],4
@@:
push eax edx
mov eax,[esp + 12 + 08]
add dword [esp + 12 + 08],8  ; Sequental access to Min, Max, Average, Median
mov edx,[eax + 04]
mov eax,[eax + 00]
mov [esp + 00 + 08],eax
mov [esp + 04 + 08],edx
pop edx eax
call HelperTextVarDouble
add ebx,15
add edx,15
dec dword [esp + 08]
jnz @b
add esp,16
popad
ret
