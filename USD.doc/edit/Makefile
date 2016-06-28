#	@(#)Makefile	8.1 (Berkeley) 6/8/93

SRCS=	edittut.ms
ROFF=	troff -ms
DPDF=	dpost | ps2pdf - # UNIX troff

paper.pdf: ${SRCS}
	${ROFF} ${SRCS} | ${DPDF} $@

# index for versatec is different from the one in edit.tut
# because the fonts are different and entries reference page
# rather than section numbers.  if you have a typesetter
# you should just use the index in edit.tut, and ignore editvindex.

editvindex:
	${TROFF} ${MACROS} -n22 edit.vindex

clean:
	rm -f *.pdf
