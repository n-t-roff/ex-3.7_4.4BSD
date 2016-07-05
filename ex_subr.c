/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 */

#if 0
static char sccsid[] = "@(#)ex_subr.c	8.1 (Berkeley) 6/9/93";
#endif /* not lint */

#include "ex.h"
#include "ex_re.h"
#include "ex_tty.h"
#include "ex_vis.h"

static void qcount(int);
static void save(line *, line *);
static void merror1(char *);

/*
 * Random routines, in alphabetical order.
 */

int
any(int c, char *s)
{
	register int x;

	while ((x = *s++))
		if (x == c)
			return (1);
	return (0);
}

int
backtab(int i)
{
	register int j;

	j = i % value(SHIFTWIDTH);
	if (j == 0)
		j = value(SHIFTWIDTH);
	i -= j;
	if (i < 0)
		i = 0;
	return (i);
}

void
change(void)
{

	tchng++;
	chng = tchng;
}

/*
 * Column returns the number of
 * columns occupied by printing the
 * characters through position cp of the
 * current line.
 */
int
column(char *cp)
{

	if (cp == 0)
		cp = &linebuf[LBSIZE - 2];
	return (qcolumn(cp, (char *) 0));
}

/*
 * Ignore a comment to the end of the line.
 * This routine eats the trailing newline so don't call newline().
 */
void
comment(void)
{
	register int c;

	do {
		c = ex_getchar();
	} while (c != '\n' && c != EOF);
	if (c == EOF)
		ungetchar(c);
}

void
Copy(char *to, char *from, ssize_t size)
{

	while (size-- > 0)
		*to++ = *from++;
}

void
copyw(line *to, line *from, int size)
{
	while (size-- > 0)
		*to++ = *from++;
}

void
copywR(line *to, line *from, int size)
{

	while (--size >= 0)
		to[size] = from[size];
}

int
ctlof(int c)
{

	return (c == TRIM ? '?' : c | ('A' - 1));
}

void
dingdong(void)
{

	if (VB)
		putpad(VB);
	else if (value(ERRORBELLS))
		putch('\207');
}

int
fixindent(int indent)
{
	register int i;
	register char *cp;

	i = whitecnt(genbuf);
	cp = vpastwh(genbuf);
	if (*cp == 0 && i == indent && linebuf[0] == 0) {
		genbuf[0] = 0;
		return (i);
	}
	CP(genindent(i), cp);
	return (i);
}

void
filioerr(char *cp)
{
	register int oerrno = errno;

	lprintf("\"%s\"", cp);
	errno = oerrno;
	syserror();
}

char *
genindent(int indent)
{
	register char *cp;

	for (cp = genbuf; indent > 0 && indent >= value(TABSTOP); indent -= value(TABSTOP))
		*cp++ = '\t';
	for (; indent > 0; indent--)
		*cp++ = ' ';
	return (cp);
}

void
getDOT(void)
{

	ex_getline(*dot);
}

line *
getmark(int c)
{
	register line *addr;
	
	for (addr = one; addr <= dol; addr++)
		if (names[c - 'a'] == (*addr &~ 01)) {
			return (addr);
		}
	return (0);
}

int
getn(char *cp)
{
	register int i = 0;

	while (isdigit((int)*cp))
		i = i * 10 + *cp++ - '0';
	if (*cp)
		return (0);
	return (i);
}

void
ignnEOF(void)
{
	register int c = ex_getchar();

	if (c == EOF)
		ungetchar(c);
	else if (c=='"')
		comment();
}

int
iswhite(int c)
{

	return (c == ' ' || c == '\t');
}

int
junk(int c)
{

	if (c && !value(BEAUTIFY))
		return (0);
	if (c >= ' ' && c != TRIM)
		return (0);
	switch (c) {

	case '\t':
	case '\n':
	case '\f':
		return (0);

	default:
		return (1);
	}
}

void
killed(void)
{

	killcnt(addr2 - addr1 + 1);
}

void
killcnt(int cnt)
{

	if (inopen) {
		notecnt = cnt;
		notenam = notesgn = "";
		return;
	}
	if (!notable(cnt))
		return;
	ex_printf("%d lines", cnt);
	if (value(TERSE) == 0) {
		ex_printf(" %c%s", Command[0] | ' ', Command + 1);
		if (Command[strlen(Command) - 1] != 'e')
			ex_putchar('e');
		ex_putchar('d');
	}
	putNFL();
}

int
lineno(line *a)
{

	return (a - zero);
}

int
lineDOL(void)
{

	return (lineno(dol));
}

int
lineDOT(void)
{

	return (lineno(dot));
}

void
markDOT(void)
{

	markpr(dot);
}

void
markpr(line *which)
{

	if ((inglobal == 0 || inopen) && which <= endcore) {
		names['z'-'a'+1] = *which & ~01;
		if (inopen)
			ncols['z'-'a'+1] = cursor;
	}
}

int
markreg(int c)
{

	if (c == '\'' || c == '`')
		return ('z' + 1);
	if (c >= 'a' && c <= 'z')
		return (c);
	return (0);
}

/*
 * Mesg decodes the terse/verbose strings. Thus
 *	'xxx@yyy' -> 'xxx' if terse, else 'xxx yyy'
 *	'xxx|yyy' -> 'xxx' if terse, else 'yyy'
 * All others map to themselves.
 */
char *
mesg(char *str)
{
	register char *cp;

	str = strcpy(genbuf, str);
	for (cp = str; *cp; cp++)
		switch (*cp) {

		case '@':
			if (value(TERSE))
				*cp = 0;
			else
				*cp = ' ';
			break;

		case '|':
			if (value(TERSE) == 0)
				return (cp + 1);
			*cp = 0;
			break;
		}
	return (str);
}

void
merror(char *s) {
	imerror(s, 0);
}

/*VARARGS2*/
void
imerror(char *seekpt, int i)
{
	register char *cp = linebuf;

	if (seekpt == 0)
		return;
	merror1(seekpt);
	if (*cp == '\n')
		putnl(), cp++;
	if (inopen > 0 && CE)
		vclreol();
	if (SO && SE)
		putpad(SO);
	ex_printf(mesg(cp), i);
	if (SO && SE)
		putpad(SE);
}

static void
merror1(char *seekpt)
{

	strcpy(linebuf, seekpt);
}

int
morelines(void)
{
#ifdef UNIX_SBRK
	if (sbrk(1024 * sizeof (line)) == (void *)-1)
		return (-1);
	endcore += 1024;
#else
	ssize_t d;
	line *prev = fendcore;
	value(LINELIMIT) += 2048;
	fendcore = realloc(fendcore, value(LINELIMIT) * sizeof(line *));
	endcore = fendcore + value(LINELIMIT) - 1;
	if ((d = fendcore - prev)) {
		extern line *tad1;
		extern line *llimit;
		addr1   += d;
		addr2   += d;
		dol     += d;
		dot     += d;
		one     += d;
		truedol += d;
		unddol  += d;
		zero    += d;
		unddel  += d;
		undap1  += d;
		undap2  += d;
		undadot += d;
		tad1    += d;
		llimit  += d;
		if (wdot)    wdot    += d;
		if (vUNDdot) vUNDdot += d;
	}
#endif
	return (0);
}

void
nonzero(void)
{

	if (addr1 == zero) {
		notempty();
		error("Nonzero address required@on this command");
	}
}

int
notable(int i)
{

	return (hush == 0 && !inglobal && i > value(REPORT));
}

void
notempty(void)
{

	if (dol == zero)
		error("No lines@in the buffer");
}


void
netchHAD(int cnt)
{

	netchange(lineDOL() - cnt);
}

void
netchange(int i)
{
	register char *cp;

	if (i > 0)
		notesgn = cp = "more ";
	else
		notesgn = cp = "fewer ", i = -i;
	if (inopen) {
		notecnt = i;
		notenam = "";
		return;
	}
	if (!notable(i))
		return;
	ex_printf(mesg("%d %slines@in file after %s"), i, cp, Command);
	putNFL();
}

void
putmark(line *addr)
{

	putmk1(addr, putline());
}

void
putmk1(line *addr, int n)
{
	register line *markp;
	int oldglobmk;

	oldglobmk = *addr & 1;
	*addr &= ~1;
	for (markp = (anymarks ? names : &names['z'-'a'+1]);
	  markp <= &names['z'-'a'+1]; markp++)
		if (*markp == *addr)
			*markp = n;
	*addr = n | oldglobmk;
}

char *
plural(long i)
{

	return (i == 1 ? "" : "s");
}

short	vcntcol;

int
qcolumn(char *lim, char *gp)
{
	int x = 0;
	void (*OO)();

	OO = Outchar;
	Outchar = qcount;
	vcntcol = 0;
	if (lim != NULL)
		x = lim[1], lim[1] = 0;
	pline(0);
	if (lim != NULL)
		lim[1] = x;
	if (gp)
		while (*gp)
			ex_putchar(*gp++);
	Outchar = OO;
	return (vcntcol);
}

static void
qcount(int c)
{

	if (c == '\t') {
		vcntcol += value(TABSTOP) - vcntcol % value(TABSTOP);
		return;
	}
	vcntcol++;
}

void
reverse(line *a1, line *a2)
{
	register line t;

	for (;;) {
		t = *--a2;
		if (a2 <= a1)
			return;
		*a2 = *a1;
		*a1++ = t;
	}
}

static void
save(line *a1, line *a2)
{
	register int more;
	size_t d1 = a1 - fendcore;
	size_t d2 = a2 - fendcore;

	if (!FIXUNDO)
		return;
#ifdef TRACE
	if (trace)
		vudump("before save");
#endif
	undkind = UNDNONE;
	undadot = dot;
	more = (ssize_t)(d2 - d1 + 1) - (unddol - dol);
	while (more > (endcore - truedol))
		if (morelines() < 0)
#ifdef UNIX_SBRK
			error("Out of memory@saving lines for undo - try using ed");
#else
			error("Out of memory@saving lines for undo - try increasing linelimit");
#endif
	if (more)
		(*(more > 0 ? copywR : copyw))(unddol + more + 1, unddol + 1,
		    (truedol - unddol));
	unddol += more;
	truedol += more;
	copyw(dol + 1, fendcore + d1, d2 - d1 + 1);
	undkind = UNDALL;
	unddel = fendcore + d1 - 1;
	undap1 = fendcore + d1;
	undap2 = fendcore + d2 + 1;
#ifdef TRACE
	if (trace)
		vudump("after save");
#endif
}

void
save12(void)
{

	save(addr1, addr2);
}

void
saveall(void)
{

	save(one, dol);
}

int
span(void)
{

	return (addr2 - addr1 + 1);
}

void
ex_sync(void)
{

	chng = 0;
	tchng = 0;
	xchng = 0;
}

int
skipwh(void)
{
	register int wh;

	wh = 0;
	while (iswhite(peekchar())) {
		wh++;
		ignchar();
	}
	return (wh);
}

/*VARARGS2*/
void
smerror(char *seekpt, char *cp)
{

	if (seekpt == 0)
		return;
	merror1(seekpt);
	if (inopen && CE)
		vclreol();
	if (SO && SE)
		putpad(SO);
	lprintf(mesg(linebuf), cp);
	if (SO && SE)
		putpad(SE);
}

char *
strend(char *cp)
{

	while (*cp)
		cp++;
	return (cp);
}

void
strcLIN(char *dp)
{

	CP(linebuf, dp);
}

void
syserror(void)
{
	char *strerror();

	dirtcnt = 0;
	ex_putchar(' ');
	error(strerror(errno));
}

/*
 * Return the column number that results from being in column col and
 * hitting a tab, where tabs are set every ts columns.  Work right for
 * the case where col > COLUMNS, even if ts does not divide COLUMNS.
 */
int
tabcol(int col, int ts)
{
	int offset, result;

	if (col >= COLUMNS) {
		offset = COLUMNS * (col/COLUMNS);
		col -= offset;
	} else
		offset = 0;
	result = col + ts - (col % ts) + offset;
	return (result);
}

char *
vfindcol(int i)
{
	register char *cp;
	void (*OO)() = Outchar;
	char *s;

	Outchar = qcount;
	s = linebuf;
	ignore(qcolumn(s - 1, NOSTR));
	for (cp = linebuf; *cp && vcntcol < i; cp++)
		ex_putchar(*cp);
	if (cp != linebuf)
		cp--;
	Outchar = OO;
	return (cp);
}

char *
vskipwh(char *cp)
{

	while (iswhite(*cp) && cp[1])
		cp++;
	return (cp);
}


char *
vpastwh(char *cp)
{

	while (iswhite(*cp))
		cp++;
	return (cp);
}

int
whitecnt(char *cp)
{
	register int i;

	i = 0;
	for (;;)
		switch (*cp++) {

		case '\t':
			i += value(TABSTOP) - i % value(TABSTOP);
			break;

		case ' ':
			i++;
			break;

		default:
			return (i);
		}
}

#ifdef lint
Ignore(a)
	char *a;
{

	a = a;
}

Ignorf(a)
	int (*a)();
{

	a = a;
}
#endif

void
markit(line *addr)
{

	if (addr != dot && addr >= one && addr <= dol)
		markDOT();
}

/*
 * The following code is defensive programming against a bug in the
 * pdp-11 overlay implementation.  Sometimes it goes nuts and asks
 * for an overlay with some garbage number, which generates an emt
 * trap.  This is a less than elegant solution, but it is somewhat
 * better than core dumping and losing your work, leaving your tty
 * in a weird state, etc.
 */
#ifdef SIGEMT
int _ovno;
void
onemt()
{
	signal(SIGEMT, onemt);
	/* 2 and 3 are valid on 11/40 type vi, so */
	if (_ovno < 0 || _ovno > 3)
		_ovno = 0;
	error("emt trap, _ovno is %d @ - try again");
}
#endif

/*
 * When a hangup occurs our actions are similar to a preserve
 * command.  If the buffer has not been [Modified], then we do
 * nothing but remove the temporary files and exit.
 * Otherwise, we sync the temp file and then attempt a preserve.
 * If the preserve succeeds, we unlink our temp files.
 * If the preserve fails, we leave the temp files as they are
 * as they are a backup even without preservation if they
 * are not removed.
 */
void
onhup(int i)
{

	(void)i;
	/*
	 * USG tty driver can send multiple HUP's!!
	 */
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	if (chng == 0) {
		cleanup(1);
		ex_exit(0);
	}
	if (setexit() == 0) {
		if (preserve()) {
			cleanup(1);
			ex_exit(0);
		}
	}
	ex_exit(1);
}

/*
 * An interrupt occurred.  Drain any output which
 * is still in the output buffering pipeline.
 * Catch interrupts again.  Unless we are in visual
 * reset the output state (out of -nl mode, e.g).
 * Then like a normal error (with the \n before Interrupt
 * suppressed in visual mode).
 */
void
onintr(int i)
{

	(void)i;
#ifndef CBREAK
	signal(SIGINT, onintr);
#else
	signal(SIGINT, inopen ? vintr : onintr);
#endif
	alarm(0);	/* in case we were called from map */
	draino();
	if (!inopen) {
		pstop();
		setlastchar('\n');
#ifdef CBREAK
	}
#else
	} else
		vraw();
#endif
	serror("%sInterrupt", inopen ? "" : "\n");
}

/*
 * If we are interruptible, enable interrupts again.
 * In some critical sections we turn interrupts off,
 * but not very often.
 */
void
setrupt(void)
{

	if (ruptible) {
#ifndef CBREAK
		signal(SIGINT, onintr);
#else
		signal(SIGINT, inopen ? vintr : onintr);
#endif
#ifdef SIGTSTP
		if (dosusp)
			signal(SIGTSTP, onsusp);
#endif
	}
}

int
preserve(void)
{

#ifdef VMUNIX
	tflush();
#endif
	synctmp();
	pid = fork();
	if (pid < 0)
		return (0);
	if (pid == 0) {
		close(0);
		dup(tfile);
		execl(_PATH_EXPRESERVE, "expreserve", (char *) 0);
		ex_exit(1);
	}
	waitfor();
	if (rpid == pid && status == 0)
		return (1);
	return (0);
}

#ifndef V6
void
ex_exit(int i)
{

# ifdef TRACE
	if (trace)
		fclose(trace);
# endif
	exit(i);
}
#endif

#ifdef SIGTSTP
/*
 * We have just gotten a susp.  Suspend and prepare to resume.
 */
void
onsusp(int i)
{
	ttymode f;
	struct winsize win;
	sigset_t sigmsk;

	(void)i;
	f = setty(normf);
	vnfl();
	putpad(TE);
	flush();

	sigemptyset(&sigmsk);
	sigprocmask(SIG_SETMASK, &sigmsk, NULL);
	signal(SIGTSTP, SIG_DFL);
	kill(0, SIGTSTP);

	/* the pc stops here */

	signal(SIGTSTP, onsusp);
	vcontin(0);
	ignore(setty(f));
	if (!inopen)
		error((char *) 0);
	else {
#ifdef	TIOCGWINSZ
		if (ioctl(0, TIOCGWINSZ, &win) >= 0)
			if (win.ws_row != winsz.ws_row ||
			    win.ws_col != winsz.ws_col)
				winch(0);
#endif
		if (vcnt < 0) {
			vcnt = -vcnt;
			if (state == VISUAL)
				vclear();
			else if (state == CRTOPEN)
				vcnt = 0;
		}
		vdirty(0, EX_LINES);
		vrepaint(cursor);
	}
}
#endif
