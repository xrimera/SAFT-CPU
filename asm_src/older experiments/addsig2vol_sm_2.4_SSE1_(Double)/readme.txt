USCT Addsig2VOL 2 Core Routine

history:
- addsig2vol_Mul allows the use of an multiplicative voxelmap
- difference to 1.xx version : internal geometry computation with single instead of double
- uses SSE1 & FPU

BUILD Notes:
WIN32:
-compile with nasm:
'nasmw -f win32 ..\addsig2vol_fpu_united.asm'
-link with matlab lcc:
'mex addsig2vol_fpu.c addsig2vol_fpu_united.obj'
-should be compatible matlab 6-7, if you don't use 'inline' flag on linker

LINUX:
-sames as win32 build
-for linux build use nasm assembler and remove leading underscores in program jum in labels (example : "_as2v:" -> "as2v:") 


FZK 2006.04.21. 
Michael Zapf; FZK IPE 
