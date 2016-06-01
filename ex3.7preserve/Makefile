#	@(#)Makefile	8.1 (Berkeley) 6/9/93

PROG=	ex3.7preserve
_CPPFLAGS=-I.. -DVMUNIX
${PROG}: ex3.7preserve.c
	${CC} ${CFLAGS} ${_CFLAGS} ${_CPPFLAGS} ${LDFLAGS} ${_LDFLAGS} -o $@ $?
clean:
	rm -f ${PROG}
