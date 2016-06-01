#	@(#)Makefile	8.1 (Berkeley) 6/9/93

PROG=	ex3.7recover
SRCS=	ex3.7recover.c ../ex_extern.c
OBJS=	${SRCS:.c=.o}
_CPPFLAGS=-I.. -DVMUNIX -DUSG3TTY
${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${_CFLAGS} ${LDFLAGS} ${_LDFLAGS} -o $@ ${OBJS}
clean:
	rm -f ${OBJS} ${PROG}
.c.o:
	${CC} ${CFLAGS} ${_CFLAGS} ${_CPPFLAGS} -c $< -o $@
