USCT Addsig2VOL Core Routine

compile with nasm:
'nasmw -f win32 ..\addsig2vol_fpu_united.asm'


link with matlab lcc:
'mex addsig2vol_fpu.c addsig2vol_fpu_united.obj'

should be compatible matlab 6-7, if you don't use 'inline' flag on linker

FZK 2006.02.20. 
Michael zapf 
