# Linux Makefile for building the reflection shared object file
#
# usage:
#   $ make clean all
#
# author: Uwe Mayer
# revision: $Rev$
# date: $Date$

#MEX_INCLUDE = -I/opt/matlab7.1/extern/include -I/home/ipefser1/sturuit1/usr/include
#MEX_LDFLAGS = -L/home/ipefser1/sturuit1/usr/lib -lgcc_s

TARGET=addsig2vol_mul

# this target is executed if make is called without
# additional targets
default: all

# builds the linux binary
all: ${TARGET}


clean:
	$(RM) *.dll
	$(RM) *.obj *.o
	$(RM) *.mexglx
	$(RM) *.tmp.c


# generic rule for compiling a single-file target, specified above
${TARGET}: ${TARGET}.o ${TARGET}.tmp.c
	mex -v ${MEX_INCLUDE} ${MEX_LDFLAGS} $@.tmp.c $< 
	mv $@.tmp.mexglx $@.mexglx


# build elf-object files from .asm files
# on linux the -DISUNIX switch must be passed to create the
# global symbol "as2v_complex" instead of "_as2v_comples" as under
# windows
%.o : %.asm
	nasm -DISUNIX=1 -f elf $<

# the .c code contains c++-style comments (// this is a comment)
# which are not supported by the Linux version of mex (Matlab v7.1)
# this automatically removes tailing double-slashes
%.tmp.c : %.c
	@echo
	@echo "  -- WARNING ------------------------------------------------------"
	@echo "  automatically removing tailing c++-style comments may lead to"
	@echo "  conflicts if double-slashes are contained in the middle of a"
	@echo "  c++ expression, i.e."
	@echo "    a = \"//\";"
	@echo "  -- WARNING ------------------------------------------------------"
	@echo
	sed 's,//.*,,' $< >$@

