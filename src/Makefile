# ---------------------------------------------------------------------
# $Id: Makefile 297 2016-10-24 15:51:08Z u35616872 $
#
# Compile and install utilities
# ---------------------------------------------------------------------

TARGETS = install_all
#SUPPORT = ../contrib/support
SUPPORT = /home/mjeffe/src/acx/sawmill/jamestools/support

# utilities that are executable scripts (don't need to be compiled)
UTILS = \
	addseq \
	adhocexcel \
	bigfiles \
	cleandir.pl \
	dmcrypt_setup.sh \
	ematt \
	findmod.pl \
       	ftpit \
	iolog \
	iota \
	lastlog.pl \
	layout_to_ora.sh \
	ll \
	lsdate \
	lt \
	memu \
	mk_flux_menu.sh \
	mkload.pl \
	myterms.sh \
	reformat.pl \
	rr \
	start_dropbox.sh \
	svnsdiff \
	txt2printf \
	unmunge \
	webalize_pics.pl

# utilities that need to be compiled
C_UTILS = \
	addcrc \
	break_file \
	fixwav \
	rrin \
	rrout \
	sizes \
	spin \
	rrvalue \
	splitval

# utilities that live in their own subdirectory
SUBDIR_UTILS = \
	chprof \
	dtof

# -----------------------------------------
# Set up C comiler settings for each OS.
#
# NOTE: Be sure to export MAKE_MACHINE for the OS you are running this on.
# For example, on Linux:
#
# 	export MAKE_MACHINE=LINUX
# 	make
# -----------------------------------------

# 
# AIX Settings - you may have to run export OBJECT_MODE=64 or =32 before running make.
# (It may be possible to use the CFLAGS -q64 or -q32 and the LDFLAGS -b64 or -b32 instead)
# NOTE: splitval uses getopts which I can't find on AIX
CC_AIX = xlc
# IBM recommended optimization flags
AIX_OPTIMIZATIONS = -O3 -qstrict -qarch=pwr3 -qtune=pwr3
CFLAGS_AIX = $(AIX_OPTIMIZATIONS) -I. -D_LARGE_FILES -qcpluscmt -D_AIX_ 
#LDFLAGS_AIX = -b32 -bnoquiet
#TARGETS_AIX = rrout rrvalue

# HPUX settings
CC_HPUX = /opt/aCC/bin/cc
#CC_HPUX = cc
CFLAGS_HPUX    = -O2 +DD64
CFLAGS_HPUX   += -D_UNIX_ -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_OPEN_SOURCE=500 -D_XOPEN_SOURCE_EXTENDED
LDFLAGS_HPUX   = +DD64
#TARGETS_HPUX   = rrout rrvalue

# LINUX Settings
CC_LINUX = cc
LINUX_LARGE_FILE_SUPPORT = -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
#CFLAGS_LINUX = -O2 -I. -I$(SUPPORT) $(LINUX_LARGE_FILE_SUPPORT)
CFLAGS_LINUX = -g -I. -I$(SUPPORT) $(LINUX_LARGE_FILE_SUPPORT)
LDFLAGS_LINUX = 
#TARGETS_LINUX = $(C_UTILS)

# Now, set up generic variables depending on machine type
CC      = $(CC_$(MAKE_MACHINE))
CFLAGS  = $(CFLAGS_$(MAKE_MACHINE))
LDFLAGS = $(LDFLAGS_$(MAKE_MACHINE)) -L$(SUPPORT)
#TARGETS = $(TARGETS_$(MAKE_MACHINE))
# -----------------------------------------

all: $(TARGETS)

#
# C tools that need compiling
#
rrin: rrin.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o rrin rrin.c

rrout: rrout.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o rrout rrout.c

spin: spin.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o spin spin.c

fixwav: fixwav.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o fixwav fixwav.c

sizes: sizes.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o sizes sizes.c

break_file: break_file.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o break_file break_file.c

#
# tools with dependencies
#
rrvalue: rrvalue.o zfopen.o parsecsv.o crc.o
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o rrvalue rrvalue.c zfopen.o parsecsv.o crc.o

splitval: splitval.o zfopen.o parsecsv.o
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o splitval splitval.c zfopen.o parsecsv.o

addcrc: addcrc.o bintohex.o crc.o
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o addcrc addcrc.c bintohex.o crc.o


#
# tools in their own subdirectory
#
chprof: force
	$(CHECK)
	make -C chprof all
	make -C chprof install

dtof: force
	$(CHECK)
	make -C dtof all
	make -C dtof install

#
# support libs (not my code)
#
zfopen.o: $(SUPPORT)/zfopen.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(SUPPORT)/zfopen.c

crc.o: $(SUPPORT)/crc.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(SUPPORT)/crc.c

parsecsv.o: $(SUPPORT)/parsecsv.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(SUPPORT)/parsecsv.c

bintohex.o: $(SUPPORT)/bintohex.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(SUPPORT)/bintohex.c

#
# helper targets
#
tarball:
	make clean
	rm -f mrjutils_src.tar.gz
	rm -fr mrjutils
	mkdir mrjutils
	cp $(UTILS) ${C_UTILS:=.c} mrjutils/
	make -C addcrc tarball
	cp addcrc/addcrc_src.tar.gz mrjutils/
	make -C chprof tarball
	cp chprof/chprof_src.tar.gz mrjutils/
	make -C rrout tarball
	cp rrout/rrout_src.tar.gz mrjutils/
	make -C rrin tarball
	cp rrin/rrin_src.tar.gz mrjutils/
	make -C dtof tarball
	cp dtof/dtof_src.tar.gz mrjutils/
	sed 's/SUPPORT..*/SUPPORT=./'< Makefile > mrjutils/Makefile
	tar -czvf mrjutils_src.tar.gz mrjutils
	rm -fr mrjutils

install: $(UTILS)
	if test ! -d $(HOME)/bin; then \
	   mkdir $(HOME)/bin; \
	fi
	cp $(UTILS) $(HOME)/bin/

install_all: $(UTILS) $(C_UTILS) $(SUBDIR_UTILS)
	if test ! -d $(HOME)/bin; then \
	   mkdir $(HOME)/bin; \
	fi
	cp $(UTILS) $(C_UTILS) $(HOME)/bin/

clean:
	rm -f *.o $(C_UTILS) *.tar.gz
	make -C chprof clean
	make -C dtof clean

force:
	true

CHECK= \
  @if [ "$(MAKE_MACHINE)" = "" ]; \
  then \
     echo "MAKE_MACHINE not set: This must be set before running make."; \
     exit 1; \
  elif [ "$(MAKE_MACHINE)" = "AIX" -a "$(OBJECT_MODE)" = "" ]; \
  then \
     echo "Compiling on AIX and OBJECT_MODE is not set: This must be set before running make."; \
     exit 1; \
  fi

