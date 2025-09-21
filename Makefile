PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
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
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	install -Dm644 perminfo.1 ${DESTDIR}${MANPREFIX}/man1/perminfo.1

uninstall:
	${RM} ${DESTDIR}${PREFIX}/bin/perminfo
	${RM} ${DESTDIR}${MANPREFIX}/man1/perminfo.1

clean:
	${RM} perminfo

.PHONY: all install uninstall clean
