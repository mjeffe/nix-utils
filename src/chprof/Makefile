# Note:
#   hash.c, hash.h, crc.c, crc.h, parsecsv.c, and parsecsv.h
#   are part of James Lemley's jamestools

TARGETS = chprof ascii_chars

#SUPPORT = ../../contrib/support
SUPPORT = /home/mjeffe/src/acx/sawmill/jamestools/support
LINUX_LARGE_FILE_SUPPORT = -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
MAKEFLAGS = -i
#LIB_DEBUG = -L/home/mjeffe/src/electric-fence-2.1.14.1 -lefence
#CFLAGS = -g3 -L. -L$(SUPPORT) -I. -I$(SUPPORT) $(LINUX_LARGE_FILE_SUPPORT)
CFLAGS = -O2 -L. -L$(SUPPORT) -I. -I$(SUPPORT) $(LINUX_LARGE_FILE_SUPPORT)


all: $(TARGETS)

zfopen.o: $(SUPPORT)/zfopen.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(SUPPORT)/zfopen.c

parsecsv.o: $(SUPPORT)/parsecsv.c $(SUPPORT)/parsecsv.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(SUPPORT)/parsecsv.c

chprof.o: chprof.c $(SUPPORT)/parsecsv.h

chprof: chprof.o parsecsv.o zfopen.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o chprof chprof.o parsecsv.o zfopen.o $(LIB_DEBUG)

ascii_chars: ascii_chars.c

install:
	cp $(TARGETS) $(HOME)/bin/


force:
	true

tarball: force
	make clean
	rm -f chprof_src.tar.gz
	mkdir chprof
	cp \
	   chprof.c ascii_chars.c \
	   $(SUPPORT)/zfopen.c \
	   $(SUPPORT)/parsecsv.c \
	   $(SUPPORT)/parsecsv.h \
	   chprof/
	sed 's/SUPPORT..*/SUPPORT=./'< Makefile > chprof/Makefile
	tar -czvf chprof_src.tar.gz chprof
	rm -fr chprof


clean:
	rm -f *.o
	rm -f $(TARGETS)
	rm -f chprof_src.tar.gz

