exec_prefix = /usr/local
BINDIR = ${exec_prefix}/sbin
MANDIR = ${prefix}/man

CC =		gcc
CCOPT = -O0
DEFS =		 -DHAVE__PROGNAME=1 -DHAVE_FCNTL_H=1 -DHAVE_GRP_H=1 -DHAVE_MEMORY_H=1 -DHAVE_PATHS_H=1 -DHAVE_SYS_EVENT_H=1 -DHAVE_OSRELDATE_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_DIRENT_H=1 -DHAVE_LIBCRYPT=1 -DHAVE_STRERROR=1 -DHAVE_WAITPID=1 -DHAVE_VSNPRINTF=1 -DHAVE_DAEMON=1 -DHAVE_SETSID=1 -DHAVE_SETLOGIN=1 -DHAVE_GETADDRINFO=1 -DHAVE_GETNAMEINFO=1 -DHAVE_GAI_STRERROR=1 -DHAVE_UNISTD_H=1 -DHAVE_GETPAGESIZE=1 -DHAVE_MMAP=1 -DHAVE_SELECT=1 -DHAVE_TM_GMTOFF=1 -DHAVE_INT64T=1 -DHAVE_SOCKLENT=1 
INCLS =		-I. -I/usr/local/include/mysql
CFLAGS =	$(CCOPT) $(DEFS) $(INCLS)
LDFLAGS =	-L/usr/local/lib -L/usr/local/lib/mysql/
LIBS =		-lmysqlclient -lc_r -lcrypt -static -lm -lz

.c.o:
	@rm -f $@
	$(CC) $(CFLAGS) -c $*.c

SRC	=	OpenRelayScanner.c fdwatch.c utils.c
OBJ	=	$(SRC:.c=.o)

ALL =		OpenRelayScanner

OpenRelayScanner:	$(OBJ)
	@rm -f $@
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(B_OBJ) $(LIBS) $(NETLIBS)
	@rm -f $(OBJ)

OpenRelayScanner.o:	OpenRelayScanner.h config.h fdwatch.h
fdwatch.o:	fdwatch.h
utils.o:    OpenRelayScanner.h
