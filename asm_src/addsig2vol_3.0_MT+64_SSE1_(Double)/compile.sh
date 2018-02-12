#!/bin/sh

LDFLAGS='-pthread -shared -lrt'
CFLAGS="-D_GNU_SOURCE  -fexceptions -fPIC -fno-omit-frame-pointer -pthread"

#OK octave / mex (assembler)
nasm -DISUNIX=1 -f elf64 addsig2vol_3_64.asm -o./addsig2vol_3_64.o
mkoctfile --mex -DBUILDMEX -DC_CODE -v -I. -I/usr/include/octave-4.0.3/octave/ -I./munit/ timestats.c addsig2vol_3_mex.c ./munit/munit.c addsig2vol_3.c as2v_array.c -o ./addsig2vol_3_mex ./addsig2vol_3_64.o



#OK normal c mit main
#gcc -g -c -I./munit ./munit/munit.c -o ./builds/munit.o
#gcc -g -c -I./* -I/usr/include/octave-4.0.3/octave/ -I/usr/include as2v_array.c  -o ./builds/as2v_array.o
#gcc -g -c -I./* -I/usr/include timestats.c  -o ./builds/timestats.o
#gcc -g -DC_CODE -c -I./* -I/usr/include/octave-4.0.3/octave/ -I/usr/include addsig2vol_3.c -o./builds/addsig2vol_3_c.o
#gcc -g -DC_CODE -c -I./* -I/usr/include/octave-4.0.3/octave/ -I/usr/include -I./muni addsig2vol_3_unittests.c -o./builds/addsig2vol_3_unittests.o
#gcc -o ./builds/addsig2vol_3_asm.o ./addsig2vol_3_64.o ./builds/addsig2vol_3_unittests.o ./builds/munit.o ./builds/as2v_array.o ./builds/timestats.o ./builds/addsig2vol_3_c.o  -lm -lpthread -o ./builds/linkedc



## anpassen wie unten / asm mit main
nasm -DISUNIX=1 -f elf64 addsig2vol_3_64.asm -o./addsig2vol_3_64.o
gcc -g -c -I./munit ./munit/munit.c -o ./builds/munit.o
gcc -g -c -I./* -I/usr/include/octave-4.0.3/octave/ -I/usr/include as2v_array.c  -o ./builds/as2v_array.o
gcc -g -c -I./* -I/usr/include timestats.c  -o ./builds/timestats.o
gcc -g -c -I./* -I/usr/include/octave-4.0.3/octave/ -I/usr/include addsig2vol_3.c -o./builds/addsig2vol_3_asm.o
gcc -g -c -I./* -I/usr/include/octave-4.0.3/octave/ -I/usr/include -I./muni addsig2vol_3_unittests.c -o./builds/addsig2vol_3_unittests.o
gcc -o ./builds/addsig2vol_3_asm.o ./addsig2vol_3_64.o ./builds/addsig2vol_3_unittests.o ./builds/munit.o ./builds/as2v_array.o ./builds/timestats.o ./builds/addsig2vol_3_asm.o  -lm -lpthread -o ./builds/linkedasm




#OK octave / mex (assembler), für original Code
nasm -DISUNIX=1 -f elf64 addsig2vol_3_64.asm -o./addsig2vol_3_64.o
mkoctfile --mex -DBUILDMEX -DC_CODE -v -I. -I/usr/include/octave-4.0.3/octave/ addsig2vol_3.c -o ./addsig2vol_3_mex ./addsig2vol_3_64.o

