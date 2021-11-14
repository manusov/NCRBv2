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
;                        see MSDN for status exceptions and details            ;  
;                                                                              ;
;------------------------------------------------------------------------------;
DialogProcChildMemoryDraw:
push ebp
mov ebp,esp
push ebx esi edi
mov ebx,PARM_HWNDDLG             ; EBX = Window handle
;---------- Detect message type -----------------------------------------------;
mov eax,PARM_MSG
cmp eax,WM_INITDIALOG
je .wminitdialog
cmp eax,WM_COMMAND
je .wmcommand
cmp eax,WM_CLOSE
je .wmclose
cmp eax,WM_TIMER
je .wmtimer
cmp eax,WM_PAINT
je .wmpaint
;---------- Exit points -------------------------------------------------------;
.skip:
xor eax,eax
jmp .finish
.processed:
mov eax,1
.finish:
pop edi esi ebx ebp
ret 16
;--- WM_INITDIALOG handler: create child window: memory performance draw ------;
.wminitdialog:                   ; Build main window 
mov ax,BINDER_BUILD_MEMORY_DRAW
call Binder 
mov [GUI_PARMS.childWinHandle],ebx
lea edi,[APP_DATA]
push [edi + APPDATA.createdIcons + MEMORY_ICON_NUMBER * 4]
push ICON_SMALL
push WM_SETICON 
push PARM_HWNDDLG
call [SendMessage]
mov esi,[edi + APPDATA.lockedStrings]
mov ax,STR_CHILD_MEM_DRAW
call IndexString
push esi
push PARM_HWNDDLG
call [SetWindowText]
mov [GUI_PARMS.silentMode],0         ; Pre-disable silent mode
mov [DRAW_PARMS.drawPreviousY],0     ; Initializing drawing context
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
;---------- Create brushes ----------------------------------------------------;
cld
mov esi,[APP_DATA.lockedBrushesList] ; ESI = Pointer to color data
lea edi,[GC_PARMS.handlesBrushes]    ; EDI = Pointer for store brushes handles
mov ebp,4                            ; EBP = Number of brushes
@@:
lodsd                            ; EAX = Color, RSI+4, select next color 
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
;---------- End of initialization, enable draw counter ------------------------;
mov [GUI_PARMS.childWinRunning],1
jmp .processed
;---------- WM_CLOSE handler: close window ------------------------------------;
.wmclose:
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
;---------- Delete brushes ----------------------------------------------------;
cld
lea esi,[GC_PARMS.handlesBrushes]   ; ESI = Pointer for store brushes handles
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
mov byte [GUI_PARMS.childWinRunning],0
push 1                           ; Parm#2 = Result for return
push ebx                         ; Parm#1 = Window handle
call [EndDialog]
jmp .processed
;---------- WM_COMMAND handler: interpreting user input -----------------------; 
.wmcommand:
cmp PARM_WPARAM,IDB_MD_CANCEL  ; Detect click "Exit" button in the draw child window
je .wmclose
cmp PARM_WPARAM,IDB_MD_RESIZE  ; Detect click "Resize" button in the draw child window
je .drawResize
cmp PARM_WPARAM,IDB_MD_SILENT  ; Detect click "Silent" button in the draw child window
jne .processed
;------------------------ "Silent" button handler -----------------------------;
mov eax,TIMER_TICK_SILENT ; Period (ms) for screen refresh frequency = low 
mov cl,1                  ; Silent Mode = 1
jmp .resizeEntry
;------------------------ "Resize" button handler -----------------------------; 
.drawResize:
mov eax,TIMER_TICK_SHOW   ; Period (ms) for screen refresh frequency = high
mov cl,0                  ; Silent Mode = 0
;---------- Entry point for silent mode branch --------------------------------;
.resizeEntry:
push ecx
;---------- Start parameters frame here ---------------------------------------;
push 0                    ; Parm#4 = Notify function pointer, not used here
push eax                  ; Parm#3 = Timer ticks period, ms
push 0                    ; Parm#2 = Event ID
push ebx                  ; Parm#1 = Parent window handle
call [SetTimer]
;---------- Set thread break flag, cause measurement thread termination -------; 
mov [DRAW_THREAD_PARMS.measureBreak],1
;--- Wait for measurement thread done, this event at thread exit point --------;
push THREAD_TIME_LIMIT               ; Parm#2 = Time limit
push [DRAW_THREAD_PARMS.eventDone]   ; Parm#1 = Handle for done event
call [WaitForSingleObject]
;---------- Wait measurement thread terminated by thread handle ---------------;
; Note status after [WaitForSingleObject] ignored. TODO. Check it.
push THREAD_TIME_LIMIT                 ; Parm#2 = Time limit
push [DRAW_THREAD_PARMS.threadHandle]  ; Parm#1 = Thread handle
call [WaitForSingleObject]
;---------- When previous thread terminated, can set [SilentMode] -------------;
pop ecx
mov [GUI_PARMS.silentMode],cl 
;---------- Remove thread and associated handles, note can corrupt pointers ---;
call DrawSessionStop
;---------- Clear drawings variables ------------------------------------------; 
mov [DRAW_PARMS.drawPreviousY],0
mov [DRAW_PARMS.timerCount],0
;---------- Rescaling ---------------------------------------------------------;
lea edi,[DRAW_THREAD_PARMS]
finit
cmp [MEM_IPB.updatedAsm],LATENCY_MODE
jae .L40
fld [edi + DTHP.statMbpsMax]
push 90 10
jmp .L41
.L40:
fld [edi + DTHP.statNspiMax]
push 9 2
.L41:
fidiv dword [esp + 4]
fimul dword [esp]
fistp dword [esp + 4]
;---------- Restart -----------------------------------------------------------;
call DrawSessionStart   ; note can corrupt pointers
pop eax eax
test eax,eax
jnz @f
inc eax
@@:
mov [DRAW_PARMS.valueGridY],eax
;---------- Rescale, same operation at drawsessionstart.inc -------------------;
push eax -GRIDSTEPY
fild dword [esp]      ; ST0 = - grid step, pixels
fidiv dword [esp+4]   ; ST0 = ( - grid step, pixels ) / ( Y units per grid )      
fstp [DRAW_PARMS.yMultiplier]
pop eax eax
; Wait for measurement thread run, this event at thread entry point
; Note status after [WaitForSingleObject] ignored. TODO. Check it.
push THREAD_TIME_LIMIT               ; Parm#2 = Time limit
push [DRAW_THREAD_PARMS.eventStart]  ; Parm#1 = Handle for start event
call [WaitForSingleObject]
mov [GUI_PARMS.childWinRunning],1
jmp .processed
;---------- WM_TIMER handler --------------------------------------------------;
.wmtimer:
lea ecx,[GUI_PARMS.childWinRunning]
lea edx,[DRAW_PARMS.timerCount]    ; For two variables: COUNT, PREVIOUS Y
xor eax,eax
;--------- Check session running ----------------------------------------------;
cmp [ecx],al               ; 0=Stop, 1=First, 2=Progress
je .SkipTimer              ; Skip if halted
inc dword [edx]            ; Counter +1
;---------- Check session abort -----------------------------------------------;
cmp [DRAW_THREAD_PARMS.measureBreak],0
je .YetNotHalt 
mov [ecx],al               ; Running=0 means HALTED
;---------- Set revisualization request ---------------------------------------;
.YetNotHalt:
push 0         ; Parm#3 = Background erase flag, 0=Background unchanged
push 0         ; Parm#2 = Pointer to invalidated region, 0=All window
push ebx       ; Parm#1 = Window handle
call [InvalidateRect]      ; This cause callback WM_PAINT operation
.SkipTimer:
jmp .processed
;---------- WM_PAINT handler --------------------------------------------------;
.wmpaint:
push ebx
;---------- Make paint context ------------------------------------------------;
push PAINT_STRUCT     ; Parm#2 = Pointer to PAINTSTRUCT structure
push ebx              ; Parm#1 = Parent window handle
call [BeginPaint]
mov [GC_PARMS.handleDC],eax
;---------- Prepare non-volatile parms, select handler = F (execution phase) --; 
mov al,[GUI_PARMS.childWinRunning]
cmp al,0                   ; 0 = Halted
je .EndPaint
cmp al,1                   ; 1 = First drawings (grid)
je .FirstPaint             ; Go if this is first pass, draw axes
;---------- Continue draw y=f(x), not a first pass, setup drawings ------------;
lea ecx,[DRAW_THREAD_PARMS]            ; ECX = Pointer to draw parameters block
lea edi,[ecx + DTHP.measureArray]      ; EDI = Pointer to measure data array
mov ebp,[ecx + DTHP.measureCounter]    ; EBP = Measure counter
mov ebx,[ecx + DTHP.visualCounter]     ; EBX = Visual counter
lea esi,[ebx+1]                        ; ESI = Divisor for averaging 
mov eax,ebx
shl eax,3         ; 2^3 = 8 bytes per entry (QWORD)
add edi,eax       ; EDI = Positioning pointer to current entry
mov [ecx + DTHP.visualCounter],ebp
sub ebp,ebx       ; EBP = Required number of pixels for visual now
jbe .EndLine      ; Go if now present data already visualized
add ebx,GRIDBLANKX + 1 
;---------- This cycle for EBP points draw per one timer tick -----------------;
.DrawLine: 
; Calculate EAX = F(measurement result)
; Remember about can use integer compare for floation point operands,
; because shifted order. Instead FCOM / FSTSW AX / SAHF.
; Optimize registers usage, offsets relative R8 many times is not optimal.
; Better use 2 indexes: base and offset [ecx+edx] ?
; Better use helpers subroutines for comparision and conditional update ?
; Use median ordering for min-max also.
; Remove finit from body of cycle ? 
; Get current measurement, must be before ordering
; at this point FPU x87 initialized
finit
fld qword [edi]       ; ST0 = Current measured Clocks per Instruction (CPI)
;---------- Calculate average CPI, nsPI, MBPS ---------------------------------;
push esi              ; ESI = divisor for averaging
fld st0               ; ST1 = ST0 = current measured value
fadd [DRAW_THREAD_PARMS.statCpiSum]
fst [DRAW_THREAD_PARMS.statCpiSum]
fidiv dword [esp]                        ; this access to R13 in the stack
fst [DRAW_THREAD_PARMS.statCpiAverage]   ; store average CPI
call HelperCPItoNSPI
fst [DRAW_THREAD_PARMS.statNspiAverage]  ; store average nsPI = f (average CPI)
call HelperNSPItoMBPS
fadd [DRAW_THREAD_PARMS.statMbpsSum]
fst [DRAW_THREAD_PARMS.statMbpsSum]  ; store average MBPS, averaging separately
fidiv dword [esp]                    ; this access to ESI in the stack
fstp [DRAW_THREAD_PARMS.statMbpsAverage]
pop esi
;--- Median ordering, also used for min/max, ordering by "bubble" method ------;
; Note floating point values can be ordered by integer compare,
; because shifted exponent used at FP format
push ebx edi ebp
.L23:
lea ecx,[esi - 1]
lea edx,[DRAW_THREAD_PARMS.measureArray]
mov ebx,edx                ; EBX  = Pointer to array first element, used later
lea edi,[edx + ecx*8]      ; EDI = Pointer to array last element, used later
jecxz .L21                 
xor eax,eax
.L20:
mov ebp,[edx + 0 + 4]
cmp ebp,[edx + 8 + 4]
jb .L22
ja .L24
mov ebp,[edx + 0 + 0]
cmp ebp,[edx + 8 + 0]
jbe .L22
.L24:
mov ebp,[edx + 0 + 0]
xchg ebp,[edx + 8 + 0]
mov [edx + 0 + 0],ebp
mov ebp,[edx + 0 + 4]
xchg ebp,[edx + 8 + 4]
mov [edx + 0 + 4],ebp
mov al,1
.L22:
add edx,8
dec ecx
jnz .L20        ; DEC/JNZ faster then LOOP
test eax,eax
jnz .L23 
.L21:
;---------- Get minimum and maximum after median ordering ---------------------;
; minimum time - maximum bandwidth
fld qword [ebx]                    ; ST0 = first element, minimum CPI
fst [DRAW_THREAD_PARMS.statCpiMin]
call HelperCPItoNSPI
fst [DRAW_THREAD_PARMS.statNspiMin]
call HelperNSPItoMBPS
fstp [DRAW_THREAD_PARMS.statMbpsMax]
;---------- Maximum time - minimum bandwidth ----------------------------------;
fld qword [edi]                   ; ST0 = last element, maximum CPI
fst [DRAW_THREAD_PARMS.statCpiMax]
call HelperCPItoNSPI
fst [DRAW_THREAD_PARMS.statNspiMax]
call HelperNSPItoMBPS
fstp [DRAW_THREAD_PARMS.statMbpsMin]
;--- Get median CPI, nsPI after median ordering, note FPU already initialized -; 
lea ecx,[esi - 1]
shr ecx,1                     ; This branch for median ODD-sized array
fld qword [ebx + ecx*8]       ; Get median
jnc .L30                      ; Go if median for ODD size
fadd qword [ebx + ecx*8 + 8]
fld1
fadd st0,st0
fdivp st1,st0
.L30:
fst [DRAW_THREAD_PARMS.statCpiMedian]
call HelperCPItoNSPI
fstp [DRAW_THREAD_PARMS.statNspiMedian]
;---------- Get median MBPS, recalculate because reciprocal restrictions ------;
lea ecx,[esi - 1]
shr ecx,1                      ; This branch for median ODD-sized array
fld qword [ebx + ecx*8]        ; Get median
pushf
call HelperCPItoNSPI
call HelperNSPItoMBPS
popf
jnc .L31                       ; Go if median for ODD size
fld qword [ebx + ecx*8 + 8]
call HelperCPItoNSPI
call HelperNSPItoMBPS
faddp st1,st0
fld1
fadd st0,st0
fdivp st1,st0
.L31:
fstp [DRAW_THREAD_PARMS.statMbpsMedian]
pop ebp edi ebx
;---------- Start calculations for drawings Y = f( CPI ) ----------------------;
; ST0 = current value, loaded before re-ordering
call HelperCPItoNSPI
cmp [MEM_IPB.updatedAsm],LATENCY_MODE
jae .L50
call HelperNSPItoMBPS
.L50:
fmul [DRAW_PARMS.yMultiplier]  ; ST0 = Pixel offset from BaseY=0, negative offset
push eax
fistp dword [esp]
pop eax
;---------- Setup coordinates for GUI -----------------------------------------;
; note at this point can set EAX=0 for draw horizontal line DEBUG.
add eax,SUBWINY-GRIDBLANKY - 1
lea edx,[ebx + 1]
lea ecx,[eax + 1]
;--- Work with Y-coordinate, adjust Y-coordinates for make solid line by Y ----;
push esi
mov esi,[DRAW_PARMS.drawPreviousY]   ; ESI = DrawPreviousY
test esi,esi
jz @f
mov ecx,esi
@@:
mov [DRAW_PARMS.drawPreviousY],eax   ; Set DrawPreviousY
jnz @f
inc ecx
inc eax
@@:
cmp eax,ecx
jbe @f
xchg eax,ecx      ; Must be Down > Top
@@:
jne @f
inc ecx           ; Make Down+1 if Top=Down
@@:
mov esi,ecx
sub esi,eax
cmp esi,1
jbe @f
inc ecx           ; Make down extra-pixel for vertical line  
@@:
pop esi
;---------- Store coordinates for rectangle as part of line Y=F(X) ------------;
; Add support approximation modes
push ebp edi
mov ebp,[MEM_IPB.updatedApprox]
test ebp,ebp
jz .NoApproxMode
mov edi,00001111b
cmp ebp,APPROX_X16
je .CheckApproxMode
mov edi,00011111b
.CheckApproxMode:
test esi,edi
jnz .NoApproxMode
dec ebx
dec eax
add edx,2  
add ecx,2
.NoApproxMode:
pop edi ebp
;---------- Build RECT structure with rectangle points coordinates ------------;
push edi
mov [GRAPH_RECT.left],ebx
mov [GRAPH_RECT.top],eax
mov [GRAPH_RECT.right],edx
mov [GRAPH_RECT.bottom],ecx
;---------- Draw current point Y=F(X) -----------------------------------------;
push [GC_PARMS.handlesBrushes + 04]   ; Parm#3 = Handle to a color brush
push GRAPH_RECT                       ; Parm#2 = Pointer to RECT strucnure
push [GC_PARMS.handleMemDC]           ; Parm#1 = Handle to a Device Context
call [FillRect]
pop edi
;---------- Cycle with update parameters --------------------------------------;
add edi,8      ; EDI = Pointer to measurement results array
inc esi        ; ESI = Divisor for averaging
inc ebx        ; EBX = X-coordinate offset by visualization position
dec ebp        ; EBP = Number of pixels per timer tick
jnz .DrawLine  ; This cycle for EBP points draw per one timer tick
.EndLine:
jmp .RedrawTexts
;--- This branch for first paint ----------------------------------------------;
; draw statical elements, not changed when draw
.FirstPaint:
mov [GUI_PARMS.childWinRunning],2
mov [DRAW_PARMS.timerCount],0
;---------- Draw --------------------------------------------------------------;
push [GC_PARMS.handleFont]     ; Parm#2 = Handle to Selected Object
push [GC_PARMS.handleMemDC]    ; Parm#1 = Handle to a Device Context
call [SelectObject]
push COLOR_TEXT_VALUES         ; Parm#2 = RDX = Text front color value = 00bbggrrh
push [GC_PARMS.handleMemDC]    ; Parm#1 = RCX = Handle to a Device Context
call [SetTextColor]
push COLOR_TEXT_BACK           ; Parm#2 = Text background color value = 00bbggrrh
push [GC_PARMS.handleMemDC]    ; Parm#1 = Handle to a Device Context
call [SetBkColor]
;---------- Blank work field --------------------------------------------------;
xor eax,eax
lea ebx,[GRAPH_RECT]
mov [ebx + RECT.left],eax
mov [ebx + RECT.top],eax
mov [ebx + RECT.right],SUBWINX
mov [ebx + RECT.bottom],SUBWINY
push [GC_PARMS.handlesBrushes + 8]   ; Parm#3 = Handle to a color brush
push ebx                             ; Parm#2 = Pointer to RECT strucnure
push [GC_PARMS.handleMemDC]          ; Parm#1 = Handle to a Device Context
call [FillRect]
;---------- Coordinate X-grid, vertical lines ---------------------------------;
mov esi,GRIDBLANKX          ; ESI = Base point, X
mov edi,GRIDX               ; EDI = Number of lines
.XVgrid:
lea eax,[esi + 1]           ; Line width = 1 pixel
mov [ebx + RECT.left],esi
mov [ebx + RECT.top],0
mov [ebx + RECT.right],eax
mov [ebx + RECT.bottom],SUBWINY-GRIDBLANKY
push [GC_PARMS.handlesBrushes + 00]  ; Parm#3 = Handle to a color brush
push ebx                             ; Parm#2 = Pointer to RECT strucnure
push [GC_PARMS.handleMemDC]          ; Parm#1 = Handle to a Device Context
call [FillRect]             ; Fill rectangle function
add esi,GRIDSTEPX           ; Add X grid step
dec edi                     ; Cycle for X-grid vertical lines
jnz .XVgrid
;---------- Coordinate Y-grid, horizontal lines -------------------------------;
mov esi,SUBWINY-GRIDBLANKY  ; ESI = Base point, Y
mov edi,GRIDY               ; EDI = Number of lines
.YHgrid:
lea eax,[esi - 1]
mov [ebx + RECT.left],GRIDBLANKX
mov [ebx + RECT.top],eax
mov [ebx + RECT.right],SUBWINX
mov [ebx + RECT.bottom],esi
push [GC_PARMS.handlesBrushes + 00]  ; Parm#3 = Handle to a color brush
push ebx                             ; Parm#2 = Pointer to RECT strucnure
push [GC_PARMS.handleMemDC]          ; Parm#1 = Handle to a Device Context
call [FillRect]             ; Fill rectangle function
sub esi,GRIDSTEPY           ; Subtract Y grid step
dec edi                     ; Cycle for Y-grid vertical lines
jnz .YHgrid
;---------- Initializing for grid texts write ---------------------------------;
; Initializing values for X-grid vertical lines and
; Y-grid horizontal lines texts
mov bx,200h + GRIDX     ; BH = Counter, Pass#1 = XGrid (BH=2), Pass#2 = YGrid (BH=1), BL = Number of lines, X
mov esi,GRIDBLANKX      ; ESI = Base point, X
mov edi,SUBWINY-20      ; EDI = Base point Y, 20 pixels up
xor ebp,ebp             ; EBP = Units value, X starts from 0
;---------- This cycle both for X and Y grids texts ---------------------------; 
; X/Y logic dynamically selected 
.BothGrids:
;---------- Build ASCII string ------------------------------------------------;
push ebx edi
lea edi,[TEMP_BUFFER]
mov edx,edi
mov bl,0             ; BL = Template control for print number, 0=No template
mov eax,ebp          ; EAX = Value for print, Bytes/KB/MB
call DecimalPrint32  ; Print number
mov ecx,edi
sub ecx,edx
mov al,0
stosb                ; Store terminator byte = 0
pop edi ebx
;---------- Calculate correction value for Y axis grid values print -----------;
mov edx,9
sub edx,ecx         ; R10 = chars positions count
jbe .RejectAddend   ; Go skip if correction not required 
imul edx,edx,7
;---------- Reject corrections if horizontal axis or overflow -----------------;
cmp bh,1            ; BH=1 means Y-mode , BH=2 means X-mode
je .DoneAddend      ; Go with EDX = correction if branch for vertical axis text 
.RejectAddend:
xor edx,edx
.DoneAddend:
;---------- Set region for text string write ----------------------------------;
lea eax,[edi + 18]    ; EAX = Y coordinate down/right corner  
push eax              ; Parm#5 = Y coordinate down/right corner
lea eax,[esi + edx + 65]
push eax              ; Parm#4 = X coordinate down/right corner
push edi              ; Parm#3 = Y coordinate up/left corner
lea eax,[esi + edx]   
push eax              ; Parm#2 = X coordinate up/left corner
push GRAPH_RECT       ; Parm#1 = Pointer to RECT structure to store parameters
call [SetRect]        ; Set region rectangle function 
; Draw text
push DT_LEFT          ; Parm#5 = Method of formatting/alignment of text string
push GRAPH_RECT       ; Parm#4 = Pointer to RECT structure, initialized by [SetRect] function
push -1               ; Parm#3 = String length, -1 means NULL-terminated string
push TEMP_BUFFER      ; Parm#2 = Pointer to text string
push [GC_PARMS.handleMemDC]    ; Parm#1 = Handle to a Device Context
call [DrawText]        ; Draw text function 
;---------- Cycle for lines of selected grid ----------------------------------;
lea ecx,[DRAW_PARMS]
cmp bh,1                          ; BH=1 means Y-mode , BH=2 means X-mode
je .YGrid
;---------- X-mode entry point ------------------------------------------------;
add ebp,[ecx + DRPM.valueGridX]   ; Addend per X grid
add esi,GRIDSTEPX
jmp .CycleGrids
;---------- Y-mode entry point ------------------------------------------------;
.YGrid:
add ebp,[ecx + DRPM.valueGridY]   ; Addend per Y grid
sub edi,GRIDSTEPY
.CycleGrids:
dec bl
jnz .BothGrids
;---------- Cycle for horizontal and vertical grids ---------------------------;
mov bl,GRIDY - 1                              ; BL = Number of lines, Y
mov esi,GRIDBLANKX - 68                       ; ESI = X coordinate
mov edi,SUBWINY-GRIDBLANKY - GRIDSTEPY - 15   ; EDI = Y coordinate
mov ebp,[ecx + DRPM.valueGridY]  ; EBP = Units value, Y starts with offset
dec bh             ; BH = Counter, Pass#1 = XGrid (BH=2), Pass#2 = YGrid (BH=1)
jnz .BothGrids
push COLOR_TEXT_UNITS     ; Parm#2 = Text front color value = 00bbggrrh
push [GC_PARMS.handleMemDC]        ; Parm#1 = Handle to a Device Context
call [SetTextColor]
mov eax,[DRAW_PARMS.selectUnits]
mov esi,[UNITS_TEXTS.bytes]   ; ESI = Pointer to text string, size units select
cmp al,0
je @f
mov esi,[UNITS_TEXTS.kb]
cmp al,1
je @f
mov esi,[UNITS_TEXTS.mb]
@@:
mov edx,SUBWINX-49        ; EDX = X coordinate up/left corner
mov ecx,SUBWINY-43        ; ECX = Y coordinate up/left corner
mov ebx,SUBWINX-13        ; EBX = X coordinate down/right corner
mov eax,SUBWINY-25        ; EAX = Y coordinate down/right corner
call HelperTextConstDraw
;---------- Prepare for Y-units, use color from previous step -----------------;
mov esi,[UNITS_TEXTS.mbps]   ; ESI = Pointer to text string, performance units 
cmp [DRAW_PARMS.selectMode],0
je @f
mov esi,[UNITS_TEXTS.nanoseconds]
@@:
mov edx,7                  ; EDX = X coordinate up/left corner
mov ecx,5                  ; ECX = Y coordinate up/left corner
mov ebx,40+7+20 + 24       ; EBX = X coordinate down/right corner
mov eax,18+5               ; EAX = Y coordinate down/right corner
call HelperTextConstDraw
;---------- Print statistics table, lines and texts ---------------------------;
;---------- Horizontal sequence of strings:  CPI, nsPI, MBPS ------------------;
lea edi,[CLKS_MBPS_TEXTS]      ; EDI = Pointer to text strings pointers table
mov ebp,DRAW_TABLE_UP_COUNT    ; EBP = Strings count
mov edx,522                    ; EDX = X coordinate up/left corner
mov ecx,27                     ; ECX = Y coordinate up/left corner
mov ebx,522 + 40               ; EBX = X coordinate down/right corner
mov eax,27 + 17                ; EAX = Y coordinate down/right corner
@@:
mov esi,[edi]                  ; ESI = Pointer to text string
add edi,4                      ; Update pointer to next offset
call HelperTextConstDraw
add edx,64                     ; Shift 64 points right, X+64
add ebx,64                     ; Shift 64 points right, X+64
dec ebp
jnz @b                         ; cycle for:  CPI, nsPI, MBPS 
;---------- Vertical sequence of strings: maximum, minimum, average, median ---;
; EDI = Pointer to continue of text strings pointers table
mov ebp,DRAW_TABLE_LEFT_COUNT  ; EBP = Strings count
mov edx,458                    ; EDX = X coordinate up/left corner
mov ecx,27 + 19                ; ECX = Y coordinate up/left corner
mov ebx,458 + 60               ; EBX = X coordinate down/right corner
mov eax,27 + 19 + 17           ; EAX = Y coordinate down/right corner
@@:
mov esi,[edi]                  ; ESI = Pointer to text string
add edi,4                      ; Update pointer to next offset
call HelperTextConstDraw
add ecx,15                    ; Shift 15 points down, Y+15
add eax,15                    ; Shift 15 points down, Y+15
dec ebp
jnz @b                        ; cycle for:  maximum, minimum, average, median
;---------- lines of table, up horizontal line and left vertical line ---------;
lea ebx,[GRAPH_RECT]
mov [ebx + RECT.left],519
mov [ebx + RECT.top],27 + 17
mov [ebx + RECT.right],519 + 240
mov [ebx + RECT.bottom],27 + 17 + 2
push [GC_PARMS.handlesBrushes + 12]    ; Parm#3 = Handle to a color brush
push ebx                               ; Parm#2 = Pointer to RECT strucnure
push [GC_PARMS.handleMemDC]            ; Parm#1 = Handle to a Device Context
call [FillRect]                 ; Fill rectangle function, up horizontal line
mov [ebx + RECT.left],518
mov [ebx + RECT.top],27 + 19
mov [ebx + RECT.right],518 + 2
mov [ebx + RECT.bottom],27 + 19 + 17*4
push [GC_PARMS.handlesBrushes + 12]    ; Parm#3 = Handle to a color brush
push ebx                               ; Parm#2 = Pointer to RECT strucnure
push [GC_PARMS.handleMemDC]            ; Parm#1 = Handle to a Device Context
call [FillRect]                 ; Fill rectangle function, left vertical line
;---------- Target method CPU instructions dump -------------------------------;
lea esi,[MEM_IPB]
mov ebp,[esi + MEMIPB.dumpMethodLength]
mov esi,[esi + MEMIPB.dumpMethodStart]
test ebp,ebp
jz .skipAsmDump
cmp ebp,128
jbe @f
mov ebp,128
@@:
push COLOR_TEXT_DUMP         ; Parm#2 = Text front color value = 00bbggrrh
push [GC_PARMS.handleMemDC]  ; Parm#1 = Handle to a Device Context
call [SetTextColor]
xor eax,eax
.AsmDump:
;---------- Write string for one hex byte -------------------------------------;
push eax
push eax
lea edi,[TEMP_BUFFER]
lodsb
call HexPrint8
mov al,0
stosb
pop eax
;---------- Set coordinates ---------------------------------------------------;
push esi
mov esi,eax                 ; EAX = counter, ESI = counter temporary copy #1
mov edx,SUBWINX - 250 + 64  ; EDX = X coordinate, up left corner
mov ecx,95 + 48             ; ECX = Y coordinate up/left corner
mov ebx,SUBWINX - 005 + 64  ; EBX = X coordinate down/right corner
mov eax,95 + 18 + 48        ; EAX = Y coordinate down/right corner 
;---------- Adjust coordinates ------------------------------------------------;
push esi
and esi,07h                 ; Bits 0-2 used for X-positioning
imul esi,esi,21             ; X scale = 21 pixels per char
add edx,esi                 ; Add offset to X-start
add ebx,esi                 ; Add offset to X-end
pop esi
shr esi,3
and esi,0Fh                 ; Bits 3-6 used for Y-positioning
imul esi,esi,18             ; Y scale = 18 pixels per char 
add ecx,esi                 ; Add offset to Y-start
add eax,esi                 ; Add offset to Y-end
;---------- Draw one hex byte -------------------------------------------------;
lea esi,[TEMP_BUFFER]
call HelperTextConstDraw
pop esi eax
inc eax
dec ebp
jnz .AsmDump 
.skipAsmDump:
;---------- TSC clock frequency -----------------------------------------------;
lea edi,[DRAW_TSC_VALUE]
mov word [edi],0000h + '?'                   ; This char '?' used if error
mov eax,dword [MEM_OPB.tscFrequencyMHz + 0]  ; EDX:EAX = TSC Frequency, MHz
mov edx,dword [MEM_OPB.tscFrequencyMHz + 4]
mov ecx,eax
or ecx,edx                  ; Frequency = 0 means error
jz @f                       ; Go skip value write if error
mov bx,0100h                ; BX=Template X.Y , 1 digit after '.'
call DoublePrint
mov al,0
stosb
@@:
push COLOR_TEXT_INFO        ; Parm#2 = Text front color value = 00bbggrrh
push [GC_PARMS.handleMemDC]          ; Parm#1 = Handle to a Device Context
call [SetTextColor]
mov edx,SUBWINX - 187       ; EDX = X coordinate up/left corner
mov ecx,6                   ; ECX = Y coordinate up/left corner
mov ebx,SUBWINX - 5         ; EBX = X coordinate down/right corner
mov eax,6+18                ; EAX = Y coordinate down/right corner
lea esi,[DRAW_TSC]          ; ESI = Pointer to text string
call HelperTextConstDraw
;---------- This text strings must be updated at each visualization tick ------;
.RedrawTexts:
;--- Initializing parameters for benchmark statistics, bandwidth and timings --;
mov ebp,0300h
;---------- Print timings statistics, CPI minimum, maximum, average, median ---;
; note CPI = Clocks Per Instruction
lea esi,[DRAW_THREAD_PARMS.statCpiMin]  ; ESI = variables group base
mov edx,SUBWINX - 250
mov ebx,SUBWINX - 190 
call HelperWriteStatisticsYconst
;---------- Print timings statistics, nsPI minimum, maximum, average, median --;
; note nsPI = nanoseconds per instruction
lea esi,[DRAW_THREAD_PARMS.statNspiMin]  ; ESI = variables group base
mov edx,SUBWINX - 250 + 64*1
mov ebx,SUBWINX - 190 + 64*1 
call HelperWriteStatisticsYconst
;---------- This number template for Megabytes per Second ---------------------;
mov ebp,0200h
;---------- Print timings statistics, MBPS minimum, maximum, average, median --;
; note MBPS = megabytes per second
; Note Min/Max here swapped, because min. time means max. bandwidth
lea esi,[DRAW_THREAD_PARMS.statMbpsMax]  ; ESI = variables group base
mov edx,SUBWINX - 250 + 64*2
mov ebx,SUBWINX - 150 + 64*2 
call HelperWriteStatisticsYconst
;---------- End of prepare, visual prepared objects ---------------------------;
.DrawVisual:
xor eax,eax          ; EAX = 0 for compact push 0
push SRCCOPY         ; Parm#9 = Raster operation code
push eax             ; Parm#8 = Y source
push eax             ; Parm#7 = X source
push [GC_PARMS.handleMemDC]   ; Parm#6 = Handle of source Device Context
push SUBWINY         ; Parm#5 = Height
push SUBWINX         ; Parm#4 = Width
push SHIFTY          ; Parm#3 = Y destination
push SHIFTX          ; Parm#2 = X destination
push [GC_PARMS.handleDC]      ; Parm#1 = Handle of destination Device Context
call [BitBlt]                       
;---------- Delete paint context ----------------------------------------------;
.EndPaint:
push PAINT_STRUCT       ; Parm#2 = Pointer to PAINTSTRUCT structure
push dword [esp + 4]    ; Parm#1 = Parent window handle
call [EndPaint]         ; This WinAPI returns BOOL status, but ignored
;---------- Paint callback handling done --------------------------------------;
pop ebx
jmp .processed

;------------------------ Helpers for calculations ----------------------------;

;--- Convert CPI (Clocks per Instruction) to nsPI (nanosec. per instr.) -------;
;                                                                              ;
; INPUT:  ST0 = CPI as floating point double                                   ;
;                                                                              ;
; OUTPUT: ST0 = nsPI as floating point double                                  ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperCPItoNSPI:
fmul [MEM_OPB.tscPeriodNs]
ret
;--- Convert nsPI (nanosec. per instr.) to MBPS (Megabytes per Second) --------;
;                                                                              ;
; INPUT:  ST0 = nsPI as floating point double                                  ;
;                                                                              ;
; OUTPUT: ST0 = MBPS  as floating point double                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;
HelperNSPItoMBPS:
push NSPI_TO_MBPS               ; This constant for decimal megabytes and ns.
mov eax,[MEM_IPB.operandWidth]  ; EAX = bits per instruction
shr eax,3                       ; EAX = bytes per instruction 
push eax
fild dword [esp + 4]  ; ST0 = 1000, f(decimal megabytes, ns), ST1 = nsPI 
fxch st1              ; ST0 = nsPI , ST1 = 1000 
fdivp st1,st0         ; ST0 = 1000 / nsPI = decimal megainstructions per second
fimul dword [esp + 0] ; ST0 = decimal megabytes per second
fimul [MEM_IPB.updatedThreads]  ; Multiply by number of threads
pop eax eax
ret

;-------------------------- Helpers for drawings ------------------------------;
; Note about stack re-alignment when subroutines call

;--- Print text string --------------------------------------------------------;
;                                                                              ;
; INPUT:   ESI = Pointer to text string                                        ;   
;          EDX = xLeft                                                         ; 
;          ECX = yTop                                                          ;
;          EBX = xRight                                                        ;
;          EAX = yBottom                                                       ;
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          All registers (EAX, EBX, ECX, EDX, ESI, EDI, EBP)                   ; 
;          is  non volatile for this subroutine                                ;    
;                                                                              ;
;------------------------------------------------------------------------------;
HelperTextConstDraw:
push eax ecx edx
;---------- Set region rectangle ----------------------------------------------; 
push eax            ; Parm#5 = yBottom 
push ebx            ; Parm#4 = xRight
push ecx            ; Parm#3 = yTop   
push edx            ; Parm#2 = xLeft                      
push GRAPH_RECT     ; Parm#1 = Pointer to RECT structure to store parameters
call [SetRect]      ; Set region rectangle function 
;---------- Draw text ---------------------------------------------------------;
push DT_LEFT        ; Parm#5 = Method of formatting/alignment of text string
push GRAPH_RECT     ; Parm#4 = Pointer to RECT structure, initialized by [SetRect] function
push -1             ; Parm#3 = String length, -1 means NULL-terminated string
push esi            ; Parm#2 = Pointer to text string
push [GC_PARMS.handleMemDC]  ; Parm#1 = Handle of a Device Context
call [DrawText]     ; Draw text function 
pop edx ecx eax
ret
;--- Print floating point value, double ---------------------------------------;
;                                                                              ;
; INPUT:   EDX = X up left position                                            ;
;          ECX = Y up left position                                            ; 
;          EBX = X down right position                                         ;
;          EAX = Y down right position                                         ;
;          EBP = numeric template for floating point print                     ;
;          EDI:ESI = floating point value, double, 64-bit                      ;
;                    EDI = High32 , ESI = Low32                                ;
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          All registers (EAX, EBX, ECX, EDX, ESI, EDI, EBP)                   ; 
;          is  non volatile for this subroutine                                ;    
;                                                                              ;
;------------------------------------------------------------------------------;      
HelperTextVarDouble:
push eax ecx edx
;---------- Build text string -------------------------------------------------;
push eax ebx ecx edx edi
lea edi,[TEMP_BUFFER]
mov ecx,11-3         ; 11 (not 9) for blank if variable X-size font 
mov ax,0000h + '?'   ; Blank field with "?" means yet not determined
stosw
mov al,' '
rep stosb            ; Blank entire field, if next value shorter then previous
mov al,0
stosb
pop edi
mov ebx,ebp          ; BH, BL = template
mov eax,esi          ; EDX:EAX = value, double
mov edx,edi
test eax,eax
jnz .L1
test edx,edx
jz .L0
.L1:
push edi
lea edi,[TEMP_BUFFER]
call DoublePrint
pop edi
.L0:
pop edx ecx ebx eax
;---------- Draw text string --------------------------------------------------;
push esi
lea esi,[TEMP_BUFFER]
call HelperTextConstDraw
pop esi 
pop edx ecx eax
ret
;--- Print cycle for Min, Max, Average, Median --------------------------------;
;                                                                              ;
; INPUT:   EDX = X up left position                                            ;
;          ECX = Y up left position, for common entry point only               ; 
;          EBX = X down right position                                         ;
;          EAX = Y down right position, for common entry point only            ;
;          EBP = numeric template for floating point print                     ;
;          ESI = pointer to variables group                                    ;
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          RSI, EBP is non volatile for this subroutine                        ;
;                                                                              ;
;------------------------------------------------------------------------------;      
HelperWriteStatisticsYconst:  ; Entry point for Y fixed (ECX, EAX pre-defined)
mov ecx,24 + 23
mov eax,42 + 23
HelperWriteStatistics:        ; Common entry point
mov edi,4
@@:
push esi edi
mov edi,[esi+4]
mov esi,[esi+0]
call HelperTextVarDouble
add ecx,15
add eax,15
pop edi esi
add esi,8                  ; Sequental access to Min, Max, Average, Median
dec edi
jnz @b
ret
