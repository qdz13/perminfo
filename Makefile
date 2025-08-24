PREFIX = /usr/local
CC = cc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =
RM = rm -f

all: perminfo

perminfo:
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ perminfo.c

install: perminfo
	mkdir -p ${DESTDIR}${PREFIX}/bin
	install -Dm755 perminfo ${DESTDIR}${PREFIX}/bin/perminfo

uninstall:
	${RM} ${DESTDIR}${PREFIX}/bin/perminfo

clean:
	${RM} perminfo

.PHONY: all install uninstall clean
