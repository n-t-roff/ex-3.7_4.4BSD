#	@(#)Makefile	8.1 (Berkeley) 6/8/93

TBL=	tbl -g # Heirloom
ROFF=	troff -ms
DPDF=	dpost | ps2pdf - # AT&T troff

all: paper.pdf summary.pdf

paper.pdf: ex.rm
	${ROFF} ex.rm | ${DPDF} $@

summary.pdf: ex.summary
	${TBL} ex.summary | ${ROFF} | ${DPDF} $@

clean:
	rm -f *.pdf
