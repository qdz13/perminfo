PREFIX = /usr/local
CC = cc

all: perminfo

perminfo:
	${CC} -Wall -Wextra -O3 -o $@ perminfo.c
	
install: perminfo
	mkdir -p ${DESTDIR}${PREFIX}/bin
	install -Dm755 perminfo ${DESTDIR}${PREFIX}/bin/perminfo

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/perminfo

clean:
	rm -f perminfo

.PHONY: all install uninstall clean
