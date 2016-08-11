USCT Addsig2VoL Ellipsoide backprojection/Synthetic apertue focssing technique Imaging Core 

Karlsruhe Institute of Technology, michael Zapf, 2016

About:
-------
Makes an SAFT image reconstruction. Fast.
And without seat belts, so beware of wrong parameter handover!

license:
--------
lgplv2.1
(contact me if this is a problem)


Features:
---------
- SSE instruction usage, effective register usage, data locality -> fast
- multithreading capable (PThreads needed)
- portable to Windows/Linux
- ia32/ia64 support

- Performance for P4-2GHz: 42 MVoxel/s, one order faster then c-code, 40 times faster then Matlab code. 


Requirements:
-------------
- CPU with SSE1 support (since P3 Katmai)
- Matlab 6.x up to 7.3 tested (Octave not tested, but should work)
- supported environments: Win32, Win64, Linux32 & Linux64
 

Usage: 
------
-	addsig2vol; 
	-> gives you parameter help

Build Notes:
-------------

WIN32/Linux32:
- assembling of addsig2vol_X_32.asm with nasm:
	for windows 'nasmw -f win32 ..\addsig2vol_3_32.asm'
	for linux 'nasm -f elf32 addsig2vol_3_32.asm'
         -> addsig2vol_3_32.obj
- link with matlab lcc (or other compiler):
	'mex addsig2vol_3.c addsig2vol_3_32.obj'
        -> addsig2vol_3.mexw32 for matlab >7.x
	-> addsig2vol_3.dll for matlab 6.x

- for multithreading add pthreads.lib & pthreads.dll/.SO and define pthreads in addsig2vol_x.c 
	build and link with MS VC 32bit
	mex addsig2vol_3.c  addsig2vol_3_32.obj portableFPU.c -L".\pthreads2.8" -I".\pthreads2.8" -lpthreadVC2

	build and link with MS VC 64bit
	'mex addsig2vol_3.c addsig2vol_3_64.obj portableFPU.c -L"." -I".\pthreads2.8" -l"\pthreads2.8\pthreads2.8 64bit Visualstudio patch\pthreadVC2_x64"'
	build and link with GCC 32bit
	'mex addsig2vol_3.c  addsig2vol_3_32.obj portableFPU.c -L".\pthreads" -I".\pthreads" -lpthreadGC2'

win64/LINUX64:
- assembling of addsig2vol_X_64.asm with Nasm assembler >2.0x
	for windows 'nasmw -f win64 ..\addsig2vol_3_64.asm'
	for linux 'nasm -f elf64 addsig2vol_3_64.asm'
	-> addsig2vol_3_32.obj

- link with matlab and 64 bit compiler:
	'mex addsig2vol_3.c addsig2vol_3_64.obj'
	-> addsig2vol_3.mexw64 for matlab >7.x

 linux 64
	2011-03 'mex -v LDFLAGS='-pthread -shared -lrt' CFLAGS="-D_GNU_SOURCE  -fexceptions -fPIC -fno-omit-frame-pointer -pthread" addsig2vol_3.c addsig2vol_3_64.o'

windows32
 2011-03  mex -v addsig2vol_3.c portableFPU.c addsig2vol_3_32.obj  -L".\pthreads2.8" -I".\pthreads2.8" -l"pthreadVC2"

windows64
 2011-10  mex -v addsig2vol_3.c portableFPU.c addsig2vol_3_64.obj  -L".\pthreads2.8" -I".\pthreads2.8" -l"pthreadVC2_64"


TODO:
http://www.osronline.com/ddkx/kmarch/64bitamd_6wmf.htm


Revisison history:
------------------

2016-08 Finally open sourced....

2011-03 new 64bit built for linux, portable FPU flags and realtime measurement, usage of "pstdint.h" from Paul Hsieh

2008.08.24 3.0
- added C-referenz reconstruction core
- added 64-bit assembler core (portable to linux64 & win64 calling conventions!), added second pixel pipline & used the new registers (r8-r15 & xmm8-xmm15) -> quadpixel computation, approx. 20% speedup
- added multithreading via PThreads


2006 2.x 
- difference to 1.xx version : internal geometry computation with single instead of double
- uses SSE1 & FPU, parallel computing of 2 pixel
- portable to linux & windows, should be compatible matlab 6-7, if you don't use 'inline' flag on linker
- for linux build use nasm assembler and remove leading underscores in program jum in labels (example : "_as2v:" -> "as2v") 

2005 1.0 
- first versions with FPU & SSE2


FZK 2008.08.26. 
Michael Zapf; FZK-IPE 

-----------------------------------------------------------------------------------------------

Additional Notes:


Linux
-----

The basic build commands under linux are the same as under windows:

compile .asm to .o
$ nasm -DISUNIX -f elf $<


remove all // comments from the .c file
$ sed 's,//.*,,' addsig2vol_2.c >addsig2vol_2.tmp.c

compile the modified .c file with the asm object file to a
$ mex -v addsig2vol_2.tmp.c addsig2vol_2.o


rename the .tmp.mexglx to .mexglx
$ mv addsig2vol_2.tmp.mexglx addsig2vol_2.mexglx


link addsig2vol_2.mexglx to the top source directory:
$ cd "Reflection 4.3.1"
$ ln -s asm\ src/addsig2vol\ 2.2\ SSE1\ \(Double\)/addsig2vol_2.mexglx

Done



Testing the .mexglx
-------------------
- change into the directory of addsig2vol_2.mexglx
- startup matlab and run:

  >>> addsig2vol_2
  
addSig2Vol_2 SSE1 Assembler Optimized v2.2

 Calculate the ellip. backprojection.
Uses SSE.

#define out       plhs[0] (Single(1:end))
#define out2      plhs[1] DEBUG (Double(1:end))
#define AScan     prhs[0] (Single(1:3000))
#define pix_vect  prhs[1] (Single(1:3))
#define rec_pos   prhs[2] (Single(1:3))
#define send_pos  prhs[3] (Single(1:3))
#define speed     prhs[4] (Single)
#define res       prhs[5] (Single)
#define timeint   prhs[6] (Single)
#define IMAGE_XYZ prhs[7] (UINT32(1:3))
#define IMAGE_SUM prhs[8] (Double(1:end))

	ï¿½ 2006.04.21. M.Zapf FZK-IPE


Troubleshooting
---------------

Q: compilation error: File-Not-Found
    mex -v   addsig2vol_2.tmp.c addsig2vol_2.o
    /opt/matlab7.1/bin/mex: line 914: cd: /home/ipefser1/sturuit1/Reflection4.3.1.original/asm: Datei oder Verzeichnis nicht gefunden
A: dynamically loaded library
   The mex version for Linux does not like spaces in path names. Create a symbolic
   link to the "addsig2vol 2.2 SSE1 (Double)" directory which does not contain
   special characters:
   $ ln -s "Reflection4.3.1.original/asm src/addsig2vol 2.2 SSE1 (Double)" reflection


Q: C++ Style comments are complaint about as Errors ?!
A: remove the CFLAGS="-ANSI ..." entry from the compiler parameters.

Q: Compile Problem with pstdint.h ?!
A: Linux again was able to break compatiblity... change uint64_t definiton from "long long" to "long" .

Q: undefined symbol error while executing "addsig2vol_2" in matlab:
   ??? Invalid MEX-file '/home/ipefser1/sturuit1/Reflection4.3.1.original/asm src/addsig2vol 2.2 SSE1 (Double)/addsig2vol_2.mexglx': 
   /home/ipefser1/sturuit1/Reflection4.3.1.original/asm src/addsig2vol 2.2 SSE1 (Double)/addsig2vol_2.mexglx: undefined symbol: as2v_complex.
A: The .asm file was not compiled with -DISUNIX flag passed to nasm, thus Windows-style
   labels have been created: "_as2v_complex", instead of "as2v_complex"

   a. try using the Makefile
   b. rename the _as2v_complex labels in the .asm file to as2v_complex


Q: Invalid MEX-File error involving libgcc_s.so.1,GCC 3.3 and usr/lib/libstdc++.so.6
A: Matlab comes with its own set of libraries and there is a conflict between libgcc_s.so 3.3
   and the current libstdc++.so.6; You need to prevent linking against libstdc++.so.6:
   
   a. run:
      $ mex -setup

      and choose (2): Template Options file for building gcc MEX-files
      Then edit $HOME/.matlab/R14/mexopts.sh and remove all occurences of
      -lstdc++, as in:
            CLIBS="$RPATH $MLIBS -lm -lstdc++"

      Then make sure that the configuration variables outputed by mex -v during
      compilation do not include -lstdc++.
      You may also check the dependency list of addsig2vol_2.mexglx, which sould not
      include libstdc++.so.6:

      $ ldd -v addsig2vol_2.mexglx |grep stdc
	  libstdc++.so.5 => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5 (0x403f9000)
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
	  /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5:
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
	
      as opposed to:

      $ ldd -v addsig2vol_2.mexglx |grep stdc
        ./addsig2vol_2.mexglx: /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libgcc_s.so.1: version `GCC_3.3' not found (required by /usr/lib/libstdc++.so.6)
        libstdc++.so.6 => /usr/lib/libstdc++.so.6 (0x400b4000)
        libstdc++.so.5 => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5 (0x404d5000)
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
        /usr/lib/libstdc++.so.6:
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
        /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5:
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (GLIBCPP_3.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5
                libstdc++.so.5 (CXXABI_1.2) => /opt/matlab7.1/bin/glnx86/../../sys/os/glnx86/libstdc++.so.5

   b. rename libgcc_s.so.1 in .../matlab7.1/sys/os/glnx86 to
      old.libgcc_s.so.1

      In this case libstcd++.so.6 may be presend in the output to "ldd -v"


MZ 2011-03
Uwe Mayer 2006-07-28
