/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 */

#if 0
static char sccsid[] = "@(#)ex_get.c	8.1 (Berkeley) 6/9/93";
#endif /* not lint */

#include "ex.h"
#include "ex_tty.h"
#include "ex_vis.h"

/*
 * Input routines for command mode.
 * Since we translate the end of reads into the implied ^D's
 * we have different flavors of routines which do/don't return such.
 */
static	bool junkbs;
short	lastc = '\n';

static int getach(void);
static int smunch(int, char *);
static void checkjunk(int);

void
ignchar(void)
{
	ignore(ex_getchar());
}

int
ex_getchar(void)
{
	register int c;

	do
		c = getcd();
	while (!globp && c == CTRL('d'));
	return (c);
}

int
getcd(void)
{
	register int c;

again:
	c = getach();
	if (c == EOF)
		return (c);
	c &= TRIM;
	if (!inopen) {
		if (!globp && c == CTRL('d'))
			setlastchar('\n');
		else if (junk(c)) {
			checkjunk(c);
			goto again;
		}
	}
	return (c);
}

int
peekchar(void)
{

	if (peekc == 0)
		peekc = ex_getchar();
	return (peekc);
}

int
peekcd(void)
{
	if (peekc == 0)
		peekc = getcd();
	return (peekc);
}

static int
getach(void)
{
	register int c;
	static char inputline[BUFSIZ];

	c = peekc;
	if (c != 0) {
		peekc = 0;
		return (c);
	}
	if (globp) {
		if (*globp)
			return (*globp++);
		globp = 0;
		return (lastc = EOF);
	}
top:
	if (input) {
		if ((c = *input++)) {
			if (c &= TRIM)
				return (lastc = c);
			goto top;
		}
		input = 0;
	}
	flush();
	if (intty) {
		c = read(0, inputline, sizeof inputline - 4);
		if (c < 0)
			return (lastc = EOF);
		if (c == 0 || inputline[c-1] != '\n')
			inputline[c++] = CTRL('d');
		if (inputline[c-1] == '\n')
			noteinp();
		inputline[c] = 0;
		for (c--; c >= 0; c--)
			if (inputline[c] == 0)
#ifdef BIT8
				inputline[c] = ' ';
#else
				inputline[c] = QUOTE;
#endif
		input = inputline;
		goto top;
	}
	c = read(0, inputline, sizeof inputline - 1);
	if(c <= 0)
		return(lastc = EOF);
	inputline[c] = '\0';
	input = inputline;
	goto top;
}

/*
 * Input routine for insert/append/change in command mode.
 * Most work here is in handling autoindent.
 */
static	short	lastin;

int
gettty(void)
{
	register int c = 0;
	register char *cp = genbuf;
	char hadup = 0;
	int offset = Pline == numbline ? 8 : 0;
	int ch;

	if (intty && !inglobal) {
		if (offset) {
			holdcm = 1;
			ex_printf("  %4d  ", lineDOT() + 1);
			flush();
			holdcm = 0;
		}
		if (value(AUTOINDENT) ^ aiflag) {
			holdcm = 1;
#ifdef LISPCODE
			if (value(LISP))
				lastin = lindent(dot + 1);
#endif
			ex_tab(lastin + offset);
			while ((c = getcd()) == CTRL('d')) {
				if (lastin == 0 && isatty(0) == -1) {
					holdcm = 0;
					return (EOF);
				}
				lastin = backtab(lastin);
				ex_tab(lastin + offset);
			}
			switch (c) {

			case '^':
			case '0':
				ch = getcd();
				if (ch == CTRL('d')) {
					if (c == '0')
						lastin = 0;
					if (!OS) {
						ex_putchar('\b' | QUOTE);
						ex_putchar(' ' | QUOTE);
						ex_putchar('\b' | QUOTE);
					}
					ex_tab(offset);
					hadup = 1;
					c = ex_getchar();
				} else
					ungetchar(ch);
				break;

			case '.':
				if (peekchar() == '\n') {
					ignchar();
					noteinp();
					holdcm = 0;
					return (EOF);
				}
				break;

			case '\n':
				hadup = 1;
				break;
			}
		}
		flush();
		holdcm = 0;
	}
	if (c == 0)
		c = ex_getchar();
	while (c != EOF && c != '\n') {
		if (cp > &genbuf[LBSIZE - 2])
			error("Input line too long");
		*cp++ = c;
		c = ex_getchar();
	}
	if (c == EOF) {
		if (inglobal)
			ungetchar(EOF);
		return (EOF);
	}
	*cp = 0;
	cp = linebuf;
	if ((value(AUTOINDENT) ^ aiflag) && hadup == 0 && intty && !inglobal) {
		lastin = c = smunch(lastin, genbuf);
		for (c = lastin; c >= value(TABSTOP); c -= value(TABSTOP))
			*cp++ = '\t';
		for (; c > 0; c--)
			*cp++ = ' ';
	}
	CP(cp, genbuf);
	if (linebuf[0] == '.' && linebuf[1] == 0)
		return (EOF);
	return (0);
}

/*
 * Crunch the indent.
 * Hard thing here is that in command mode some of the indent
 * is only implicit, so we must seed the column counter.
 * This should really be done differently so as to use the whitecnt routine
 * and also to hack indenting for LISP.
 */
static int
smunch(int col, char *ocp)
{
	register char *cp;

	cp = ocp;
	for (;;)
		switch (*cp++) {

		case ' ':
			col++;
			continue;

		case '\t':
			col += value(TABSTOP) - (col % value(TABSTOP));
			continue;

		default:
			cp--;
			CP(ocp, cp);
			return (col);
		}
}

char	*cntrlhm =	"^H discarded\n";

static void
checkjunk(int c)
{

	if (junkbs == 0 && c == '\b') {
		write(2, cntrlhm, 13);
		junkbs = 1;
	}
}

void
setin(line *addr)
{

	if (addr == zero)
		lastin = 0;
	else
		ex_getline(*addr), lastin = smunch(0, linebuf);
}
