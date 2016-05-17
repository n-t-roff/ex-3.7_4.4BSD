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
PROG=	ex
SUBDIR=	ex3.7recover ex3.7preserve
CFLAGS	=-DTABS=8 -DLISPCODE -DCHDIR -DUCVISUAL -DVMUNIX -DFASTTAG -DUSG3TTY
SRCS=	ex.c ex_addr.c ex_cmds.c ex_cmds2.c ex_cmdsub.c ex_data.c ex_extern.c \
	ex_get.c ex_io.c ex_put.c ex_re.c ex_set.c ex_subr.c ex_tagio.c \
	ex_temp.c ex_tty.c ex_unix.c ex_v.c ex_vadj.c ex_vget.c ex_vmain.c \
	ex_voper.c ex_vops.c ex_vops2.c ex_vops3.c ex_vput.c ex_vwind.c \
	printf.c
OBJS=	${SRCS:.c=.o}
BINMODE=1755
DPADD=	${LIBTERMCAP}
LDADD=	-ltinfo
MAN1=	ex.0 vi.0
MLINKS=	ex.1 edit.1 vi.1 view.1
LINKS=	${BINDIR}/ex ${BINDIR}/vi ${BINDIR}/ex ${BINDIR}/view \
	${BINDIR}/ex ${BINDIR}/edit

# There's a boostrap problem here -- ex is necessary to build ex_vars.h.
# The file ex_vars.h.init can be used to build a version of ex to start
# with.
CLEANFILES=ex_vars.h

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJS} ${LDADD}

clean:
	rm -f ${OBJS} ${PROG}

ex_vars.h: ex_data.c
	csh -f makeoptions ex_data.c ${CFLAGS:M-[ID]*}

.c.o:
	${CC} ${CFLAGS} -c $<
