USCT Addsig2VOL 2 Core Routine

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
-for linux build use nasm assembler and remove leading underscores in program jum in labels (example : "_as2v:" -> "as2v") 


FZK 2006.03.30. 
Michael Zapf; FZK IPE 



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
   spectial characters:
   $ ln -s "Reflection4.3.1.original/asm src/addsig2vol 2.2 SSE1 (Double)" reflection


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


Uwe Mayer 2006-07-28
