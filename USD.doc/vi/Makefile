#	@(#)Makefile	8.1 (Berkeley) 8/14/93

SRCS=	vi.in vi.chars
TBL=	tbl -g # Heirloom
ROFF=	troff -ms
DPDF=	dpost | ps2pdf - # DWB, Heirloom

all: paper.pdf summary.pdf viapwh.pdf

paper.pdf: ${SRCS}
	${TBL} ${SRCS} | ${ROFF} | ${DPDF} $@

summary.pdf: vi.summary
	${TBL} vi.summary | ${ROFF} | ${DPDF} $@

viapwh.pdf: vi.apwh.ms
	${ROFF} vi.apwh.ms | ${DPDF} $@

clean:
	rm -f *.pdf
