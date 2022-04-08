This is DEBUG SAMPLES directory.
v2.02.xx under construction.

v2.01.xx stable source at:
https://github.com/manusov/NCRBv2/tree/master/asm

v2.01.xx stable executable at:
https://github.com/manusov/NCRBv2/tree/master/exe


NCRB v2.02.00 items. All both for x64 and ia32 versions.
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
15.    Experiments with threads priority.
16.    Experiments with visualization timer refresh rate.
17.    Memory fence required for measurement and visualization threads shared variables.


Total inspect, comments, optimization, verification, performance optimization and monitoring.
----------------------------------------------------------------------------------------------

1.    Verify at Oracle VMBox virtual machines: WinXP 32/64 , Win7 32/64 , Win10 32/64.
2.    Compare results for v2.01.xx and v2.02.xx. Y-values and X-points:
       point for performance change = cache size.
3.    Inspect for unused objects, remove it.
4.    Skip re-scaling if new maximum - old maximum < threshold.
5.    Inspect and work with "TODO" strings in the source text.
6.    Optimize statistics calculation for add one result case.
7.    Note about "Draw 3D..." button restore, probably at Draw window.
  