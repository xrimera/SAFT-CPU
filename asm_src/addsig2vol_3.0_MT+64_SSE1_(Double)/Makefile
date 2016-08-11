# Linux Makefile for building the reflection shared object file
#
# usage:
#   $ make clean all
#
# author: Uwe Mayer
# revision: $Rev$
# date: $Date$

# MATLAB_PATH = /usr/local/matlab/
# MATLAB_ARCH = glnx86
MATLAB_PATH = /usr/local/matlab
MATLAB_ARCH = glnxa64

#MEX_INCLUDE = -I/opt/matlab7.1/extern/include -I/home/ipefser1/sturuit1/usr/include
MEX_INCLUDE = -I$(MATLAB_PATH)/extern/include
MEX = $(MATLAB_PATH)/bin/mex
#MEX_LDFLAGS = -L/home/ipefser1/sturuit1/usr/lib -lgcc_s

CFLAGS    = $(GLOBAL_CFLAGS)  $(INCLUDES) -fPIC -O3
LDFLAGS   = $(GLOBAL_LDFLAGS) $(LIBDIRS) $(LIBS)
# MEXFLAGS  = -largeArrayDims -O -cxx -Dzapf_bug
MEXFLAGS  = -largeArrayDims -O -cxx 

OBJS32 = addsig2vol_3 addsig2vol_3_32.o32
OBJS64 = addsig2vol_3.o addsig2vol_3_64.o64 
OBJS64_ = addsig2vol_3.o addsig2vol_3_64_.o64

TARGET=addsig2vol_3

# this target is executed if make is called without
# additional targets

# builds the linux binary
all: ${OBJS64} ${TARGET}
# all: ${OBJS32} ${TARGET}


clean:
	$(RM) *.dll
	$(RM) *.obj *.o *.o64
	$(RM) *.mexglx *.mexa64 *.mexw*
	$(RM) *.tmp.c


# generic rule for compiling a single-file target, specified above
${TARGET}: ${TARGET}.o ${TARGET}.tmp.c
	$(MEX) ${MEX_FLAGS} ${MEX_INCLUDE} ${MEX_LDFLAGS} ${OBJS64}
# ${TARGET}: ${TARGET}.o ${TARGET}.tmp.c
# 	$(MEX) ${MEX_FLAGS} ${MEX_INCLUDE} ${MEX_LDFLAGS} $@.tmp.c $<
# 	mv $@.tmp.mexglx $@.mexglx


.c.o:
	$(CC) $(CFLAGS) $(MEX_INCLUDE) -c $< -o $@

# build elf-object files from .asm files
# on linux the -DISUNIX switch must be passed to create the
# global symbol "as2v_complex" instead of "_as2v_comples" as under
# windows
%.o32 : %.asm
	nasm -DISUNIX=1 -f elf $<
	
%.o64 : %.asm
	nasm -DISUNIX=1 -f elf64 $< -o $@

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

