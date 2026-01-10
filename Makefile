PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
ZSHCOMPPREFIX = ${PREFIX}/share/zsh/site-functions
CC = cc
CFLAGS = -Wall -Wextra -Wimplicit-fallthrough -O2 -std=c23
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
	mkdir -p ${DESTDIR}${ZSHCOMPPREFIX}
	install -Dm644 completions/zsh/_perminfo ${DESTDIR}${ZSHCOMPPREFIX}/_perminfo

uninstall:
	${RM} ${DESTDIR}${PREFIX}/bin/perminfo ${DESTDIR}${MANPREFIX}/man1/perminfo.1

clean:
	${RM} perminfo

.PHONY: all install uninstall clean
