#!/bin/sh

LDFLAGS='-pthread -shared -lrt'
CFLAGS="-D_GNU_SOURCE  -fexceptions -fPIC -fno-omit-frame-pointer -pthread"

#OK octave
nasm -DISUNIX=1 -f elf64 addsig2vol_3_64.asm -o./addsig2vol_3_64.o
mkoctfile --mex -DMATLAB_MEX_FILE -v -I. -I/usr/include/octave-4.0.3/octave/ addsig2vol_3.c as2v_array.c -o ./addsig2vol_3_mex ./addsig2vol_3_64.o

