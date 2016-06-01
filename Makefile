#	@(#)Makefile	8.1 (Berkeley) 6/18/93

# Things that can be turned off to save space:
#	LISPCODE (-l flag, showmatch and lisp options)
#	UCVISUAL (visual \ nonsense on upper case only terminals)
#	CHDIR (the undocumented chdir command)
#	CRYPT includes code to edit encrypted files (the -x option, like ed.)
#	VMUNIX makes ex considerably larger, raising many limits and
#		improving speed and simplicity of maintenance.
#
# If your system expands tabs to 4 spaces you should -DTABS=4 below.
#
VERSION=3.7
PREFIX=	${DESTDIR}/usr/local
BINDIR=	${PREFIX}/bin
LIBEXECDIR=${PREFIX}/libexec
PRESERVEDIR=/var/preserve
PROG=	ex
SUBDIR=	ex3.7recover ex3.7preserve
_CFLAGS=-g -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls \
	-fsanitize=address \
	-fsanitize=undefined \
	-fsanitize=integer
_CPPFLAGS=-DTABS=8 -DLISPCODE -DCHDIR -DUCVISUAL -DVMUNIX -DFASTTAG -DUSG3TTY
SRCS=	ex.c ex_addr.c ex_cmds.c ex_cmds2.c ex_cmdsub.c ex_data.c ex_extern.c \
	ex_get.c ex_io.c ex_put.c ex_re.c ex_set.c ex_subr.c ex_tagio.c \
	ex_temp.c ex_tty.c ex_unix.c ex_v.c ex_vadj.c ex_vget.c ex_vmain.c \
	ex_voper.c ex_vops.c ex_vops2.c ex_vops3.c ex_vput.c ex_vwind.c \
	printf.c
OBJS=	${SRCS:.c=.o}
LDADD=	-ltinfo

# There's a boostrap problem here -- ex is necessary to build ex_vars.h.
# The file ex_vars.h.init can be used to build a version of ex to start
# with.
CLEANFILES=ex_vars.h

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${_CFLAGS} ${LDFLAGS} -o $@ ${OBJS} ${LDADD}

install: ${BINDIR} ${LIBEXECDIR} ${PRESERVEDIR}
	install ${PROG} ${BINDIR}/
	for i in vi view edit; do \
		ln -sf ${BINDIR}/${PROG} ${BINDIR}/$$i; \
	done

uninstall:
	for i in ${PROG} vi view edit; do \
		rm -f ${BINDIR}/$$i; \
	done

${BINDIR} ${LIBDIR}:
	mkdir -p $@

${PRESERVEDIR}:
	mkdir -p $@
	chmod 1777 $@

clean:
	rm -f ${OBJS} ${PROG}

ex_vars.h: ex_data.c
	csh -f makeoptions ex_data.c ${CPP} ${_CPPFLAGS}

.c.o:
	${CC} ${CFLAGS} ${_CFLAGS} ${_CPPFLAGS} -c $<
