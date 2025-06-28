PREFIX = /usr/local
CC = cc
CFLAGS = -Wall -Wextra -O3

all: perminfo

perminfo:
	${CC} ${CFLAGS} -o $@ perminfo.c

install: perminfo
	mkdir -p ${DESTDIR}${PREFIX}/bin
	install -Dm755 perminfo ${DESTDIR}${PREFIX}/bin/perminfo

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/perminfo

clean:
	rm -f perminfo

.PHONY: all install uninstall clean
