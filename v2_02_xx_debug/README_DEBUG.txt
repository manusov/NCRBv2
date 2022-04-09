This is DEBUG SAMPLES directory.
v2.02.xx under construction.

v2.01.xx stable source at:
https://github.com/manusov/NCRBv2/tree/master/asm

v2.01.xx stable executable at:
https://github.com/manusov/NCRBv2/tree/master/exe


NCRB v2.02.xx items. All both for x64 and ia32 versions.
---------------------------------------------------------
1.  +  Update version numbers at EXE32, EXE64 and Resource DLL. Internal and versioninfo data.
2.  +  Replace "Draw 3D ..." option name to "Silent mode". Note "Draw 3D" can add later to Draw screen.
3.  +  Remove buttons "Resize" and "Silent" from Draw screen, remove it names and handlers.
4.  +  Add button "Refresh" to Draw screen: change "Resize" to "Refresh".
5.  +  Add Pen resource to DLL, start initialization and Draw window initialization.
6.  +  Make "Draw" button handling branches: for Realtime and Silent modes.
7.  +  Window procedure for Draw window: messages branches structure.
8.  +  Window procedure for Draw window: WM_PAINT branch.
9.  +  Bug with colors text at X and Y axies.
10. +  Support "Refresh" button at Draw window.
11. +  Remove redundant variables: drawPrevious, timerCount.
12. +  Performance optimization: draw window statical and dynamical objects:
        statical  = single draw at first pass,
        dynamical = draw for each measurement result.
13. +  Skip if measureCounter = visualCounter, means no points added. Check at WM_TIMER.
14. +  Instruction size optimization: "LEA" vs "MOV reg,offset" differentiate for x64 and ia32.
        replace "LEA reg,[addr]" to "MOV reg,addr" is optimal for ia32, not required for x64.
15. +  Scale values for MBPS (bandwidth) and ns (latency): approximation bug with valueGridY.
16. +  Pre-verify at Oracle VMBox virtual machines: WinXP 32/64 , Win7 32/64 , Win10 32/64.
17. +  Pre-verify all buttons and options, plus save report function.

Additional experiments.
------------------------
1.     Experiments with threads priority.
2.     Experiments with visualization timer refresh rate.
3.     Memory fence required for measurement and visualization threads shared variables.

Total inspect, comments, optimization, verification, performance optimization and monitoring.
----------------------------------------------------------------------------------------------
1.    Verify at Oracle VMBox virtual machines: WinXP 32/64 , Win7 32/64 , Win10 32/64.
2.    Verify error reporting for vector brief, simple run and drawings.
3.    Verify all options.
4.    Compare results for v2.01.xx and v2.02.xx. Y-values and X-points:
       point for performance change = cache size.
5.    Inspect for unused objects, remove it.
6.    Skip re-scaling if new maximum - old maximum < threshold.
7.    Inspect and work with "TODO" strings in the source text.
8.    Optimize statistics calculation for add one result case.
9.    Note about "Draw 3D..." button restore, probably at Draw window.
10.   Make alternative for FPU rounding mode temporary changed for Y scale select operation.


