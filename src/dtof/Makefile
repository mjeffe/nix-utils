# ------------------------------------------------------------------------
# $Id: Makefile 344 2020-01-11 16:28:21Z higuito $
#
# Note:
#   hash.c, hash.h, crc.c, crc.h, parsecsv.c, and parsecsv.h
#   are part of James Lemley's jamestools
#
# ------------------------------------------------------------------------

MRJ         = ../../
ACX         = ../../../acx
ACX_SUPPORT = $(ACX)/sawmill/jamestools/support
TARGETS     = dtof
MAKEFLAGS   = -i

# libcsv NOTE:  You have to install libcsv before you can compile. See notes in $(MRJ)/support/Makefile

# AIX Settings - Be sure to export OBJECT_MODE=64 or =32 before running make.
# Note: DataMatch and AA may not be 64-bit objects, and 32-bit and 64-bit objects
# may not be linked together.
CC_AIX = xlc
# IBM recommended optimization flags
AIX_OPTIMIZATIONS = -O3 -qstrict -qarch=pwr5 -qtune=pwr5
CFLAGS_AIX = $(AIX_OPTIMIZATIONS) -qcpluscmt -D_UNIX_ -D_LARGE_FILES
#LDFLAGS_AIX = -qmisalign -bmaxdata:1073741824

# TRU64 settings
CC_TRU64 = cc
CFLAGS_TRU64 = -O2 -D_UNIX_ #-taso -fPIC
#LDFLAGS_TRU64 = 

# HPUX settings
#CC_HPUX = /opt/aCC/bin/cc
CC_HPUX = cc
CFLAGS_HPUX = +DD64 -D_UNIX_ -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_OPEN_SOURCE=500 -D_XOPEN_SOURCE_EXTENDED
#LDFLAGS_HPUX = +DD64

# LINUX Settings
CC_LINUX = gcc
CFLAGS_LINUX = -Wall -O2
#CFLAGS_LINUX = -Wall -g -pg
CFLAGS_LINUX += -D_UNIX_ -D_LINUX_
#CFLAGS_LINUX += -D_GNU_SOURCE
CFLAGS_LINUX += -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64

# Set up variables depending on machine type
CC=$(CC_$(MAKE_MACHINE))
CFLAGS=$(CFLAGS_$(MAKE_MACHINE)) -I. -I$(ACX_SUPPORT) -I$(MRJ)/support -I$(MRJ)/support/contrib -I$(MRJ)/include
#LDFLAGS2=$(LDFLAGS_$(MAKE_MACHINE)) -lm -lpthread
LDFLAGS2=$(LDFLAGS_$(MAKE_MACHINE)) -L$(MRJ)/support -L$(MRJ)/lib




all: $(TARGETS)

zfopen.o: $(ACX_SUPPORT)/zfopen.c
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS2) -c $(ACX_SUPPORT)/zfopen.c

parsecsv.o: $(ACX_SUPPORT)/parsecsv.c $(ACX_SUPPORT)/parsecsv.h
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS2) -c $(ACX_SUPPORT)/parsecsv.c

dmax.o:
	cd $(MRJ)/support && make dmax.o

dtof.o: dtof.c $(MRJ)/support/dmax.h $(ACX_SUPPORT)/parsecsv.h
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS2) -c dtof.c

dtof: dtof.o $(MRJ)/support/dmax.o parsecsv.o zfopen.o
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS2) -o dtof dtof.o $(MRJ)/support/dmax.o parsecsv.o zfopen.o -lcsv

ptest.o: ptest.c $(MRJ)/support/dmax.h $(ACX_SUPPORT)/parsecsv.h

ptest: ptest.o parsecsv.o zfopen.o
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS2) -lpthread -o ptest ptest.o parsecsv.o zfopen.o

mdtof.o: mdtof.c mdtof.h $(MRJ)/support/dmax.h $(ACX_SUPPORT)/parsecsv.h
	$(CC) $(CFLAGS) -c mdtof.c -pthread

mdtof: mdtof.o parsecsv.o zfopen.o
	$(CHECK)
	$(CC) $(LDFLAGS2) -lpthread -o mdtof mdtof.o parsecsv.o zfopen.o


malloctest.o: malloctest.c 

malloctest: malloctest.o
	$(CHECK)
	$(CC) $(CFLAGS) $(LDFLAGS2) -o malloctest malloctest.o

tarball:
	make clean
	rm dtof_src.tar.gz
	mkdir dtof;
	cp \
	dtof.c \
	$(ACX_SUPPORT)/zfopen.c \
	$(ACX_SUPPORT)/zfopen.h \
	$(ACX_SUPPORT)/parsecsv.c \
	$(ACX_SUPPORT)/parsecsv.h \
	$(MRJ)/support/dmax.c \
	$(MRJ)/support/dmax.h \
	dtof/
	sed 's/MRJ = ..*/MRJ=./'< Makefile | \
        sed 's/ACX_SUPPORT = ..*/ACX_SUPPORT=./' > dtof/Makefile
	tar -czvf dtof_src.tar.gz dtof
	rm -fr dtof


foo.o: foo.c

foo: foo.o
	$(CC) $(CFLAGS) $(LDFLAGS2) -o foo foo.o

install:
	cp dtof $(HOME)/bin/

clean:
	rm -f *.o
	rm -f $(TARGETS)
	rm -f dtof_src.tar.gz
	cd $(MRJ)/support && $(MAKE) clean


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

