# ---------------------------------------------------------------------
# $Id: Makefile 297 2016-10-24 15:51:08Z u35616872 $
#
# This helps to install these files in the right place
# ---------------------------------------------------------------------

TARGETS = install_all
#SUPPORT = ../contrib/support
SUPPORT = /home/mjeffe/src/acx/sawmill/jamestools/support

UTILS = fiex fif findmod.pl ftpit layout_to_ora.sh ll memu \
        rr.sh sortlex.sh unix2msdos unmunge.sh uppercase bigfiles x xe lt \
        alertlog iota svnsdiff txt2printf lsdate

C_UTILS = fixwav spin keepalive bf sizes
SUBDIR_UTILS = addcrc chprof rrobin dtof

# -----------------------------------------
# C comiler settings

# LINUX Settings
CC_LINUX = cc
LINUX_LARGE_FILE_SUPPORT = -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS_LINUX = -O2 -I. -I$(SUPPORT) $(LINUX_LARGE_FILE_SUPPORT)
LDFLAGS_LINUX = 
#TARGETS_LINUX = $(C_UTILS)

# Set up variables depending on machine type
CC      = $(CC_$(MAKE_MACHINE))
CFLAGS  = $(CFLAGS_$(MAKE_MACHINE))
LDFLAGS = $(LDFLAGS_$(MAKE_MACHINE))
#TARGETS = $(TARGETS_$(MAKE_MACHINE))
# -----------------------------------------


all: $(TARGETS)


fixwav: fixwav.o
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o fixwav fixwav.c

spin  : spin.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o spin spin.c

sizes : sizes.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o sizes sizes.c

keepalive  : keepalive.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o keepalive keepalive.c

bf : bf.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS) -o bf bf.c

addcrc : force
	$(CHECK)
	make -C addcrc all
	make -C addcrc install

chprof: force
	$(CHECK)
	make -C chprof all
	make -C chprof install

rrobin: force
	$(CHECK)
	make -C rrobin all
	make -C rrobin install

dtof : force
	$(CHECK)
	make -C dtof all
	make -C dtof install

force :
	true

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
	make -C rrobin tarball
	cp rrobin/rrobin_src.tar.gz mrjutils/
	make -C dtof tarball
	cp dtof/dtof_src.tar.gz mrjutils/
	sed 's/SUPPORT..*/SUPPORT=./'< Makefile > mrjutils/Makefile
	tar -czvf mrjutils_src.tar.gz mrjutils
	rm -fr mrjutils


install : $(UTILS)
	if test ! -d $(HOME)/bin; then \
	   mkdir $(HOME)/bin; \
	fi
	cp $(UTILS) $(HOME)/bin/

install_all : $(UTILS) $(C_UTILS) $(SUBDIR_UTILS)
	if test ! -d $(HOME)/bin; then \
	   mkdir $(HOME)/bin; \
	fi
	cp $(UTILS) $(C_UTILS) $(HOME)/bin/

clean:
	rm -f *.o $(C_UTILS) *.tar.gz
	make -C addcrc clean
	make -C chprof clean
	make -C rrobin clean
	make -C dtof clean



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

