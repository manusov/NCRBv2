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
;                        see MSDN for status exceptions and details            ;  
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
cmp eax,WM_COMMAND
je .wmcommand                  ; Go if command message
cmp eax,WM_CLOSE
je .wmclose                    ; Go if window close message
cmp eax,WM_TIMER
je .wmtimer
cmp eax,WM_PAINT
je .wmpaint
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
;--- WM_INITDIALOG handler: create child window: memory performance draw ------; 
.wminitdialog:
mov ax,BINDER_BUILD_MEMORY_DRAW
call Binder 
mov [GUI_PARMS.childWinHandle],rbx
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
mov [GUI_PARMS.silentMode],0       ; Pre-disable silent mode
mov [DRAW_PARMS.drawPreviousY],0   ; Initializing drawing context
;---------- Initialize graphics context ---------------------------------------æ
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
mov [GUI_PARMS.childWinRunning],1
jmp .processed
;---------- WM_CLOSE handler: close window, include for "Cancel" button  ------;
.wmclose:
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
mov byte [GUI_PARMS.childWinRunning],0
mov edx,1                              ; RDX = Parm#2 = Result for return
mov rcx,rbx                            ; RCX = Parm#1 = Window handle
call [EndDialog]
jmp .processed
;---------- WM_COMMAND handler: interpreting user input -----------------------; 
.wmcommand:
cmp r8,IDB_MD_CANCEL    ; Detect click "Exit" button in the draw child window
je .wmclose
cmp r8,IDB_MD_RESIZE    ; Detect click "Resize" button in the draw child window
je .drawResize
cmp r8,IDB_MD_SILENT    ; Detect click "Silent" button in the draw child window
jne .processed
;------------------------ "Silent" button handler -----------------------------;
mov r8d,TIMER_TICK_SILENT   ; Timer period (ms), screen refresh low frequency
mov edi,1                   ; Silent Mode = 1
jmp .resizeEntry
;------------------------ "Resize" button handler -----------------------------; 
.drawResize:
mov r8d,TIMER_TICK_SHOW     ; Timer period (ms), screen refresh high frequency
xor edi,edi                 ; Silent Mode = 0
;---------- Entry point for silent mode branch --------------------------------;
.resizeEntry:     ; R8  = Parm#3 = Timer ticks period, ms
mov rcx,rbx       ; RCX = Parm#1 = Parent window handle
xor edx,edx       ; RDX = Parm#2 = Event ID
xor r9d,r9d       ; R9  = Parm#4 = Notify function pointer, not used here
call [SetTimer]
;---------- Set thread break flag ---------------------------------------------;
mov [DRAW_THREAD_PARMS.measureBreak],1
;--- Wait for measurement thread done, this event at thread exit point --------;
mov rcx,[DRAW_THREAD_PARMS.eventDone]   ; RCX = Parm#1 = Handle for done event
mov edx,THREAD_TIME_LIMIT               ; RDX = Parm#2 = Time limit
call [WaitForSingleObject]
;---------- Wait thread terminated by thread handle ---------------------------;
; Note status after [WaitForSingleObject] ignored. TODO. Check it.
mov rcx,[DRAW_THREAD_PARMS.threadHandle]  ; RCX = Parm#1 = Thread handle
mov edx,THREAD_TIME_LIMIT                 ; RDX = Parm#2 = Time limit
call [WaitForSingleObject]
;---------- When previous thread terminated, can set [SilentMode] -------------;
mov [GUI_PARMS.silentMode],dil
;---------- Remove thread and associated handles, note can corrupt pointers ---;
call DrawSessionStop
;---------- Clear drawings variables ------------------------------------------;
mov [DRAW_PARMS.drawPreviousY],0
mov [DRAW_PARMS.timerCount],0
;---------- Rescaling ---------------------------------------------------------;
lea rdi,[DRAW_THREAD_PARMS]
finit
cmp [MEM_IPB.updatedAsm],LATENCY_MODE
jae .L40
fld [rdi + DTHP.statMbpsMax]
push 90 10
jmp .L41
.L40:
fld [rdi + DTHP.statNspiMax]
push 9 2
.L41:
fidiv dword [rsp + 8]
fimul dword [rsp]
fistp dword [rsp + 8]
;---------- Restart -----------------------------------------------------------;
call DrawSessionStart   ; note can corrupt pointers
pop rax rax
test eax,eax
jnz @f
inc eax
@@:
mov [DRAW_PARMS.valueGridY],eax
;---------- Rescale, same operation at drawsessionstart.inc -------------------;
push rax -GRIDSTEPY
fild dword [rsp]        ; ST0 = - grid step, pixels
fidiv dword [rsp + 8]   ; ST0 = ( - grid step, pixels ) / ( Y units per grid )      
fstp [DRAW_PARMS.yMultiplier]
pop rax rax
;--- Wait for measurement thread run, this event at thread entry point --------;
; Note status after [WaitForSingleObject] ignored. TODO. check it.
mov rcx,[DRAW_THREAD_PARMS.eventStart]  ; RCX = Parm#1 = Handle for start event
mov edx,THREAD_TIME_LIMIT               ; RDX = Parm#2 = Time limit
call [WaitForSingleObject]
mov [GUI_PARMS.childWinRunning],1
jmp .processed
;---------- WM_TIMER handler ( timer event: timer tick callback ) -------------;
.wmtimer:
lea rcx,[GUI_PARMS.childWinRunning]
lea rdx,[DRAW_PARMS.timerCount]     ; For two variables: COUNT, PREVIOUS Y
xor eax,eax                         ; This clear entire RAX
;---------- Check session running ---------------------------------------------;
cmp [rcx],al           ; 0=Stop, 1=First, 2=Progress
je .SkipTimer          ; Skip if halted
inc dword [rdx]        ; Counter +1
;---------- Check session abort -----------------------------------------------;
cmp [DRAW_THREAD_PARMS.measureBreak],0
je .YetNotHalt 
mov [rcx],al           ; Running=0 means HALTED
;---------- Set revisualization request ---------------------------------------;
.YetNotHalt:
mov rcx,rbx            ; Parm#1 = RCX = Window handle 
xor edx,edx            ; Parm#2 = RDX = Pointer to invalidated region, 0=All window
xor r8d,r8d            ; Parm#3 = R8  = Background erase flag, 0=Background unchanged
call [InvalidateRect]  ; This cause callback WM_PAINT operation
.SkipTimer:
jmp .processed
;---------- WM_PAINT handler --------------------------------------------------;
.wmpaint:
;---------- Make paint context ------------------------------------------------;
mov rcx,rbx                ; Parm#1 = RCX = Parent window handle
lea rdx,[PAINT_STRUCT]     ; Parm#2 = RDX = Pointer to PAINTSTRUCT structure
call [BeginPaint]
mov [GC_PARMS.handleDC],rax
;---------- Prepare non-volatile parms, select handler = F (execution phase) --; 
mov rsi,[GC_PARMS.handleMemDC]      ; Get to non-volatile register
lea rdi,[GRAPH_RECT]
xor ebp,ebp                ; RBP = 0, used for compact encoding
lea rcx,[GUI_PARMS.childWinRunning]
mov al,[rcx]
cmp al,0                   ; 0 = Halted
je .EndPaint
cmp al,1                   ; 1 = First drawings (grid)
je .FirstPaint             ; Go if this is first pass, draw axes
;---------- Continue draw y=f(x), not a first pass, setup drawings ------------;
lea rcx,[DRAW_THREAD_PARMS]
lea r14,[rcx + DTHP.measureArray]
mov ebp,[rcx + DTHP.measureCounter]
mov r12d,[rcx + DTHP.visualCounter]
lea r13,[r12 + 1]
mov rax,r12
shl rax,3       ; 2^3 = 8 bytes per entry (QWORD)
add r14,rax
mov [rcx + DTHP.visualCounter],ebp
sub ebp,r12d
jbe .EndLine
add r12,GRIDBLANKX + 1 
;---------- This cycle for EBP points draw per one timer tick -----------------;
.DrawLine: 
;---------- Calculate EAX = F( measurement result ) ---------------------------;
; Remember about can use integer compare for floation point operands,
; because shifted order. Instead FCOM / FSTSW AX / SAHF.
; Optimize registers usage, offsets relative R8 many times is not optimal.
; Better use 2 indexes: base and offset [rcx+rdx] ?
; Better use helpers subroutines for comparision and conditional update ?
; Use median ordering for min-max also.
; Remove finit from body of cycle ? 
; Load pointer, used for all statistics calculation
lea r11,[DRAW_THREAD_PARMS]
;---------- Get current measurement, must be before ordering ------------------;
; at this point FPU x87 initialized
finit
fld qword [r14]
;---------- Calculate average CPI ---------------------------------------------;
push r13                          ; R13 = divisor for averaging
fld st0                           ; ST1 = ST0 = current measured value
fadd [r11 + DTHP.statCpiSum]
fst [r11 + DTHP.statCpiSum]
fidiv dword [rsp]                 ; this access to R13 in the stack
fst [r11 + DTHP.statCpiAverage]   ; store average CPI
call HelperCPItoNSPI
fst [r11 + DTHP.statNspiAverage]  ; store average nsPI = f (average CPI)
call HelperNSPItoMBPS
fadd [r11 + DTHP.statMbpsSum]
fst [r11 + DTHP.statMbpsSum]      ; store average MBPS, averaging separately
fidiv dword [rsp]                 ; this access to R13 in the stack
fstp [r11 + DTHP.statMbpsAverage]
pop r13
;--- Median ordering, also used for min/max, ordering by "bubble" method ------;
; Note floating point values can be ordered by integer compare,
; because shifted exponent used at FP format
.L23:
lea rcx,[r13-1]
lea rdx,[r11 + DTHP.measureArray]
mov r9,rdx                            ; R9  = Pointer to array first element
lea r10,[rdx + rcx*8]                 ; R10 = Pointer to array last element
jrcxz .L21
xor eax,eax
.L20:
mov r8,[rdx + 0]
cmp r8,[rdx + 8]
jbe .L22
xchg r8,[rdx + 8]
mov [rdx + 0],r8
mov al,1
.L22:
add rdx,8
dec ecx
jnz .L20        ; DEC/JNZ faster then LOOP
test eax,eax
jnz .L23 
.L21:
;---------- Get minimum and maximum after median ordering ---------------------;
; Minimum time - maximum bandwidth
fld qword [r9]                    ; ST0 = first element, minimum CPI
fst [r11 + DTHP.statCpiMin]
call HelperCPItoNSPI
fst [r11 + DTHP.statNspiMin]
call HelperNSPItoMBPS
fstp [r11 + DTHP.statMbpsMax]
;---------- Maximum time - minimum bandwidth ----------------------------------;
fld qword [r10]                   ; ST0 = last element, maximum CPI
fst [r11 + DTHP.statCpiMax]
call HelperCPItoNSPI
fst [r11 + DTHP.statNspiMax]
call HelperNSPItoMBPS
fstp [r11 + DTHP.statMbpsMin]
;--- Get median CPI, nsPI after median ordering, note FPU already initialized -; 
lea rcx,[r13-1]
shr ecx,1                      ; This branch for median ODD-sized array
fld qword [r9 + rcx*8]         ; Get median
jnc .L30                       ; Go if median for ODD size
fadd qword [r9 + rcx*8 + 8]
fld1
fadd st0,st0
fdivp st1,st0
.L30:
fst [r11 + DTHP.statCpiMedian]
call HelperCPItoNSPI
fstp [r11 + DTHP.statNspiMedian]
;---------- Get median MBPS, recalculate because reciprocal restrictions ------;
lea rcx,[r13-1]
shr ecx,1                      ; This branch for median ODD-sized array
fld qword [r9 + rcx*8]         ; Get median
pushf
call HelperCPItoNSPI
call HelperNSPItoMBPS
popf
jnc .L31                       ; Go if median for ODD size
fld qword [r9 + rcx*8 + 8]
call HelperCPItoNSPI
call HelperNSPItoMBPS
faddp st1,st0
fld1
fadd st0,st0
fdivp st1,st0
.L31:
fstp [r11 + DTHP.statMbpsMedian]
;---------- Start calculations for drawings Y = f( CPI ) ----------------------;
; ST0 = current value, loaded before re-ordering
call HelperCPItoNSPI
cmp [MEM_IPB.updatedAsm],LATENCY_MODE
jae .L50
call HelperNSPItoMBPS
.L50:
fmul [DRAW_PARMS.yMultiplier]  ; ST0 = Pixel offset from BaseY=0, negative offset
push rax
fistp dword [rsp]
pop rax
;---------- Setup coordinates for GUI -----------------------------------------;
; note at this point can set EAX=0 for draw horizontal line DEBUG.
add eax,SUBWINY-GRIDBLANKY - 1
lea rdx,[r12+1]
lea ecx,[eax+1]
;--- Work with Y-coordinate, adjust Y-coordinates for make solid line by Y ----;
mov r8d,[DRAW_PARMS.drawPreviousY]   ; R8D = DrawPreviousY
test r8d,r8d
jz @f
mov ecx,r8d
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
mov r8d,ecx
sub r8d,eax
cmp r8d,1
jbe @f
inc ecx           ; Make down extra-pixel for vertical line  
@@:
;---------- Store coordinates for rectangle as part of line Y=F(X) ------------;
; Add support approximation modes
push r12
mov r8d,[MEM_IPB.updatedApprox]
test r8d,r8d
jz .NoApproxMode
mov r9d,00001111b
cmp r8d,APPROX_X16
je .CheckApproxMode
mov r9d,00011111b
.CheckApproxMode:
test r13d,r9d
jnz .NoApproxMode
dec r12d
dec eax
add edx,2  
add ecx,2
.NoApproxMode:
mov [rdi+RECT.left],r12d
mov [rdi+RECT.top],eax
mov [rdi+RECT.right],edx
mov [rdi+RECT.bottom],ecx
pop r12
;---------- Draw current point Y=F(X) -----------------------------------------;
mov rcx,rsi                    ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                    ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[GC_PARMS.handlesBrushes + 08]   ; Parm#3 = R8 = Handle to a color brush
call [FillRect]
;---------- Cycle with update parameters --------------------------------------;
add r14,8      ; R14 = Pointer to measurement results array
inc r13d       ; R13D = Divisor for averaging
inc r12d       ; R12D = X-coordinate offset by visualization position
dec ebp        ; EBP = Number of pixels per timer tick
jnz .DrawLine  ; This cycle for EBP points draw per one timer tick
.EndLine:
jmp .RedrawTexts
;---------- This branch for first paint ---------------------------------------;  
; Draw statical elements, not changed when draw
.FirstPaint:
mov byte [rcx],2               ; [rcx] = [Win1_Running]
mov [DRAW_PARMS.timerCount],ebp
;---------- Draw --------------------------------------------------------------;
mov rcx,rsi                    ; Parm#1 = RCX = Handle to a Device Context
mov rdx,[GC_PARMS.handleFont]  ; Parm#2 = RDX = Handle to Selected Object
call [SelectObject]
mov rcx,rsi                    ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_VALUES      ; Parm#2 = RDX = Text front color value = 00bbggrrh
call [SetTextColor]
mov rcx,rsi                    ; Parm#1 = RCX = Handle to a Device Context
mov edx,COLOR_TEXT_BACK        ; Parm#2 = RDX = Text background color value = 00bbggrrh
call [SetBkColor]
;---------- Blank work field --------------------------------------------------;
mov [rdi + RECT.left],ebp
mov [rdi + RECT.top],ebp
mov [rdi + RECT.right],SUBWINX
mov [rdi + RECT.bottom],SUBWINY
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[GC_PARMS.handlesBrushes + 16]  ; Parm#3 = R8 = Handle to a color brush
call [FillRect]
;---------- Coordinate X-grid, vertical lines ---------------------------------;
mov r12d,GRIDBLANKX          ; R12D = Base point, X
mov r13d,SUBWINY-GRIDBLANKY  ; R13D = Base point, Y
mov r14d,GRIDX               ; R14D = Number of lines
.XVgrid:
lea rax,[r12 + 1]            ; Line width = 1 pixel
mov [rdi+RECT.left],r12d
mov [rdi+RECT.top],ebp
mov [rdi+RECT.right],eax
mov [rdi+RECT.bottom],r13d
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                  ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[GC_PARMS.handlesBrushes + 00]  ; Parm#3 = R8 = Handle to a color brush
call [FillRect]              ; Fill rectangle function
add r12d,GRIDSTEPX           ; Add X grid step
dec r14d                     ; Cycle for X-grid vertical lines
jnz .XVgrid
;---------- Coordinate Y-grid, horizontal lines -------------------------------;
mov r12d,SUBWINY-GRIDBLANKY  ; R12D = Base point, Y
mov r13d,GRIDY               ; R13D = Number of lines
.YHgrid:
lea rax,[r12 - 1]
mov [rdi + RECT.left],GRIDBLANKX
mov [rdi + RECT.top],eax
mov [rdi + RECT.right],SUBWINX
mov [rdi + RECT.bottom],r12d
mov rcx,rsi                   ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                   ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[GC_PARMS.handlesBrushes + 00]  ; Parm#3 = R8 = Handle to a color brush
call [FillRect]               ; Fill rectangle function
sub r12d,GRIDSTEPY            ; Subtract Y grid step
dec r13d                      ; Cycle for Y-grid vertical lines
jnz .YHgrid
;---------- Initializing for grid texts write ---------------------------------;
; Initializing values for X-grid vertical lines and
; Y-grid horizontal lines texts
push rbx                ; Save RBX=Handle, this also important for stack alignment: 5 parms. functions
mov bh,2                ; BH = Counter, Pass#1 = XGrid (BH=2), Pass#2 = YGrid (BH=1)
mov r14d,GRIDX          ; R14D = Number of lines, X
mov r12d,GRIDBLANKX     ; R12D = Base point, X
mov r13d,SUBWINY-20     ; R13D = Base point Y, 20 pixels up
xor ebp,ebp             ; EBP = Units value, X starts from 0
;--- This cycle both for X and Y grids texts, X/Y logic dynamically selected --; 
.BothGrids:
;---------- Build ASCII string ------------------------------------------------;
push rdi
lea rdi,[TEMP_BUFFER]   ; RDI = Pointer for print decimal string as ASCII
mov rdx,rdi
mov bl,0                ; BL = Template control for print number, 0=No template
mov eax,ebp             ; EAX = Value for print, Bytes/KB/MB
call DecimalPrint32     ; Print number
mov rcx,rdi
sub rcx,rdx
mov al,0
stosb                        ; Store terminator byte = 0
pop rdi
;---------- Calculate correction value for Y axis grid values print -----------;
mov r10d,9
sub r10,rcx         ; R10 = chars positions count
jbe .RejectAddend   ; Go skip if correction not required 
imul r10,r10,7
;---------- Reject corrections if horizontal axis or overflow -----------------;
cmp bh,1            ; BH=1 means Y-mode , BH=2 means X-mode
je .DoneAddend      ; Go with R10 = correction if branch for vertical axis text 
.RejectAddend:
xor r10,r10
.DoneAddend:
;---------- Set region for text string write ----------------------------------;
; Here don't align stack because PUSH RBX
mov rcx,rdi               ; Parm#1 = RCX = Pointer to RECT structure to store parameters
lea rdx,[r12 + r10]       ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,r13d              ; Parm#3 = R8  = Y coordinate up/left corner
lea r9,[r12 + r10 + 65]   ; Parm#4 = R9  = X coordinate down/right corner
lea rax,[r13 + 18]        ; Parm#5 = RAX = Y coordinate down/right corner  
push rax                  ; Parm#5 = Y coordinate down/right corner
sub rsp,32                ; Create parameters shadow
call [SetRect]            ; Set region rectangle function 
add rsp,32+8              ; Remove parameters shadow and one parameter
lea rdx,[TEMP_BUFFER]
;---------- Draw text ---------------------------------------------------------;
; Here don't align stack because PUSH RBX , Parm#2 = RDX = Pointer to text string
mov rcx,rsi                  ; Parm#1 = RCX = Handle to a Device Context, Parm#2 = RDX set by previous step
mov r8,-1                    ; Parm#3 = R8 = String length, -1 means NULL-terminated string
mov r9,rdi                   ; Parm#4 = R9 = Pointer to RECT structure, initialized by [SetRect] function
push DT_LEFT                 ; Parm#5 = Method of formatting/alignment of text string
sub rsp,32                   ; Create parameters shadow
call [DrawText]              ; Draw text function 
add rsp,32+8                 ; Remove parameters shadow and 1 parameter
;---------- Cycle for lines of selected grid ----------------------------------;
lea rcx,[DRAW_PARMS]
cmp bh,1                     ; BH=1 means Y-mode , BH=2 means X-mode
je .YGrid
;---------- X-mode entry point ------------------------------------------------;
add ebp,[rcx + DRPM.valueGridX]   ; Addend per X grid
add r12d,GRIDSTEPX
jmp .CycleGrids
;---------- Y-mode entry point ------------------------------------------------;
.YGrid:
add ebp,[rcx + DRPM.valueGridY]   ; Addend per Y grid
sub r13d,GRIDSTEPY
.CycleGrids:
dec r14d
jnz .BothGrids
;---------- Cycle for horizontal and vertical grids ---------------------------;
mov r14d,GRIDY - 1                             ; R14D = Number of lines, Y
mov r12d,GRIDBLANKX - 68                       ; R12D = X coordinate
mov r13d,SUBWINY-GRIDBLANKY - GRIDSTEPY - 15   ; R13D = Y coordinate
mov ebp,[rcx + DRPM.valueGridY]  ; EBP = Units value, Y starts with offset
dec bh             ; BH = Counter, Pass#1 = XGrid (BH=2), Pass#2 = YGrid (BH=1)
jnz .BothGrids
;---------- Restore RBX = Handle ----------------------------------------------; 
; This also important for stack alignment: 5 parms. functions inside fragment
pop rbx
;---------- Prepare for X-units, here change color ----------------------------;
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
mov edx,SUBWINX-49   ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,SUBWINY-43   ; Parm#3 = R8  = Y coordinate up/left corner
mov r9d,SUBWINX-13   ; Parm#4 = R9  = X coordinate down/right corner
mov eax,SUBWINY-25   ; RAX = Transit for Parm#5 = Y coordinate down/right corner
call HelperTextConstDraw
;---------- Prepare for Y-units, use color from previous step -----------------;
mov r12,[UNITS_TEXTS.mbps]   ; R12 = Pointer to text string , Mode = Cache&RAM
cmp [DRAW_PARMS.selectMode],0
je @f
mov r12,[UNITS_TEXTS.nanoseconds]
@@:
mov edx,7                    ; Parm#2 = RDX = X coordinate up/left corner
mov r8d,5                    ; Parm#3 = R8 = Y coordinate up/left corner
mov r9d,40+7+20 + 24         ; Parm#4 = R9 = X coordinate down/right corner
mov eax,18+5                 ; RAX = Transit for Parm#5 = Y coordinate down/right corner
call HelperTextConstDraw
;---------- Print statistics table, lines and texts ---------------------------;
;---------- Horizontal sequence of strings:  CPI, nsPI, MBPS ------------------;
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
mov r8d,27 + 19                ; R8 = Y coordinate up/left corner
mov r9d,458 + 60               ; R9 = X coordinate down/right corner
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
mov rcx,rsi                   ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                   ; Parm#2 = RDX = Pointer to RECT strucnure
mov r8,[GC_PARMS.handlesBrushes + 24]  ; Parm#3 = R8 = Handle to a color brush
call [FillRect]               ; Fill rectangle function, up horizontal line
mov [rdi + RECT.left],518
mov [rdi + RECT.top],27 + 19
mov [rdi + RECT.right],518 + 2
mov [rdi + RECT.bottom],27 + 19 + 17*4
mov rcx,rsi                   ; Parm#1 = RCX = Handle to a Device Context
mov rdx,rdi                   ; Parm#2 = RDX = Pointer to RECT strucnure
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
.AsmDump:
lea r12,[TEMP_BUFFER]
push rdi
mov rdi,r12
mov al,[r14 + r15]
call HexPrint8
mov al,0
stosb
pop rdi
;---------- Set coordinates ---------------------------------------------------;
mov edx,SUBWINX - 250 + 64  ; RDX = X coordinate, up left corner
mov r8d,95 + 48             ; R8  = Y coordinate up/left corner
mov r9d,SUBWINX - 005 + 64  ; R9  = X coordinate down/right corner
mov eax,95 + 18 + 48        ; RAX = Y coordinate down/right corner 
;---------- Adjust coordinates ------------------------------------------------;
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
jnz .AsmDump 
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
;---------- This text strings must be updated at each visualization tick ------;
.RedrawTexts:
;--- Initializing parameters for benchmark statistics, bandwidth and timings --;
mov r13d,0300h
;---------- Print timings statistics, CPI minimum, maximum, average, median ---;
; note CPI = Clocks Per Instruction
lea r14,[DRAW_THREAD_PARMS.statCpiMin]  ; R14 = variables group base
mov edx,SUBWINX - 250
mov r9d,SUBWINX - 190 
call HelperWriteStatisticsYconst
;---------- Print timings statistics, nsPI minimum, maximum, average, median --;
; note nsPI = nanoseconds per instruction
lea r14,[DRAW_THREAD_PARMS.statNspiMin]  ; R14 = variables group base
mov edx,SUBWINX - 250 + 64*1
mov r9d,SUBWINX - 190 + 64*1 
call HelperWriteStatisticsYconst
;---------- This number template for Megabytes per Second ---------------------;
mov r13d,0200h
;---------- Print timings statistics, MBPS minimum, maximum, average, median --;
; note MBPS = megabytes per second
; note Min/Max here swapped, because min. time means max. bandwidth
lea r14,[DRAW_THREAD_PARMS.statMbpsMax]  ; R14 = variables group base
mov edx,SUBWINX - 250 + 64*2
mov r9d,SUBWINX - 150 + 64*2 
call HelperWriteStatisticsYconst
;---------- End of prepare, visual prepared objects ---------------------------;
.DrawVisual:
mov rcx,[GC_PARMS.handleDC] ; Parm#1 = RCX = Handle of destination Device Context
mov edx,SHIFTX          ; Parm#2 = RDX = X destination
mov r8d,SHIFTY          ; Parm#3 = R8  = Y destination
mov r9d,SUBWINX         ; Parm#4 = R9  = Width
xor eax,eax             ; RAX = 0 for compact push 0
push rax                ; This push for align stack, not a parameter
push SRCCOPY            ; Parm#9 = Raster operation code
push rax                ; Parm#8 = Y source
push rax                ; Parm#7 = X source
push [GC_PARMS.handleMemDC]      ; Parm#6 = Handle of source Device Context
push SUBWINY            ; Parm#5 = Height
sub rsp,32              ; Create parameters shadow
call [BitBlt]                       
add rsp,32+40+8         ; Remove parameters shadow, 5 parameters, 1 qword for align
;---------- Delete paint context ----------------------------------------------;
.EndPaint:
mov rcx,rbx             ; Parm#1 = RCX = Parent window handle
lea rdx,[PAINT_STRUCT]  ; Parm#2 = RDX = Pointer to PAINTSTRUCT structure
call [EndPaint]         ; This WinAPI returns BOOL status, but ignored
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
mov eax,[MEM_IPB.operandWidth]  ; RAX = bits per instruction
shr eax,3                       ; RAX = bytes per instruction 
push rax
fild dword [rsp+8]   ; ST0 = 1000, f(decimal megabytes, ns), ST1 = nsPI 
fxch st1             ; ST0 = nsPI , ST1 = 1000 
fdivp st1,st0        ; ST0 = 1000 / nsPI = decimal megainstructions per second
fimul dword [rsp+0]  ; ST0 = decimal megabytes per second
fimul [MEM_IPB.updatedThreads]  ; Multiply by number of threads
pop rax rax
ret
;-------------------------- Helpers for drawings ------------------------------;
; Note about stack re-alignment when subroutines call

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
add rsp,32+8          ; Remove parameters shadow, 1 parameter, align
mov rcx,rsi           ; Parm#1 = RCX = Handle of a Device Context
mov rdx,r12           ; Parm#2 = RDX = Pointer to text string
mov r8,-1             ; Parm#3 = R8 = String length, -1 means NULL-terminated string
mov r9,rdi            ; Parm#4 = R9 = Pointer to RECT structure, initialized by [SetRect] function
push DT_LEFT          ; Parm#5 = Method of formatting/alignment of text string
sub rsp,32            ; Create parameters shadow
call [DrawText]       ; Set region rectangle function 
add rsp,32+8          ; Remove parameters shadow, 1 parameter, align
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
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          RAX, RBX, RCX, RDX, RSI, RDI, RBP, R8, R9, R14                      ;  
;          is non volatile for this subroutine                                 ;
;                                                                              ;
;------------------------------------------------------------------------------;      
HelperTextVarDouble:
push rax rbx rcx rdx rdi
push rax rbx rdi       ; Note about stack re-alignment when subroutine call
lea rdi,[TEMP_BUFFER]
mov r12,rdi
mov ecx,11-3         ; 11 (not 9) for blank if variable X-size font 
mov ax,0000h + '?'   ; Blank field with "?" means yet not determined
stosw
mov al,' '
rep stosb            ; Blank entire field, if next value shorter then previous
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
;          R8D = Y up left position, for common entry point only               ; 
;          R9D = X down right position                                         ;
;          EAX = Y down right position, for common entry point only            ;
;          R13 = numeric template for floating point print                     ;
;          R14 = pointer to variables group                                    ;
;          RSI  = Handle of a Device Context                                   ;
;                                                                              ;
; OUTPUT:  None in the CPU registers                                           ;
;          RSI, R13 is non volatile for this subroutine                        ;
;                                                                              ;
;------------------------------------------------------------------------------;      
HelperWriteStatisticsYconst:  ; Entry point for Y fixed (R8D, EAX pre-defined)
mov r8d,24 + 23
mov eax,42 + 23
HelperWriteStatistics:     ; Common entry point
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
