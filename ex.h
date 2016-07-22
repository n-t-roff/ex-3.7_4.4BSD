/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 *
 *	@(#)ex.h	8.1 (Berkeley) 6/9/93
 */

#include <stdio.h>
#include <stdarg.h>
#ifdef V6
#include <retrofit.h>
#endif

/*
 * Ex version 3 (see exact version in ex_cmds.c, search for /Version/)
 *
 * Mark Horton, UC Berkeley
 * Bill Joy, UC Berkeley
 * November 1979
 *
 * This file contains most of the declarations common to a large number
 * of routines.  The file ex_vis.h contains declarations
 * which are used only inside the screen editor.
 * The file ex_tune.h contains parameters which can be diddled per installation.
 *
 * The declarations relating to the argument list, regular expressions,
 * the temporary file data structure used by the editor
 * and the data describing terminals are each fairly substantial and
 * are kept in the files ex_{argv,re,temp,tty}.h which
 * we #include separately.
 *
 * If you are going to dig into ex, you should look at the outline of the
 * distribution of the code into files at the beginning of ex.c and ex_v.c.
 * Code which is similar to that of ed is lightly or undocumented in spots
 * (e.g. the regular expression code).  Newer code (e.g. open and visual)
 * is much more carefully documented, and still rough in spots.
 *
 * Please forward bug reports to
 *
 *	Mark Horton
 *	Computer Science Division, EECS
 *	EVANS HALL
 *	U.C. Berkeley 94704
 *	(415) 642-4948
 *	(415) 642-1024 (dept. office)
 *
 * or to csvax.mark@berkeley on the ARPA-net.  I would particularly like to hear
 * of additional terminal descriptions you add to the termcap data base.
 */

#include <sys/ioctl.h>
#ifndef	vms
#include <sys/param.h>
#else
#include <types.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#ifndef	vms
#include <sys/stat.h>
#else
#include <stat.h>
#endif
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

#ifndef var
#define var	extern
#endif
/*
 *	The following little dance copes with the new USG tty handling.
 *	This stuff has the advantage of considerable flexibility, and
 *	the disadvantage of being incompatible with anything else.
 *	The presence of the symbol USG3TTY will indicate the new code:
 *	in this case, we define CBREAK (because we can simulate it exactly),
 *	but we won't actually use it, so we set it to a value that will
 *	probably blow the compilation if we goof up.
 */
#ifdef USG3TTY
#include <termios.h>
#define CBREAK xxxxx
#else
#ifndef	vms
#include <sgtty.h>
#else
#include "vmstty.h"
#endif
#endif

extern	int errno;

#ifndef VMUNIX
typedef	short	line;
#else
typedef	int	line;
#endif
typedef	short	bool;

int tgetent(char *, const char *);
int tgetflag(char *);
int tgetnum(char *);
char *tgetstr(char *id, char **area);
char *tgoto(const char *, int, int);
int tputs(const char *, int, int (*)(int));

#include "ex_tune.h"
#include "ex_vars.h"
/*
 * Options in the editor are referred to usually by "value(name)" where
 * name is all uppercase, i.e. "value(PROMPT)".  This is actually a macro
 * which expands to a fixed field in a static structure and so generates
 * very little code.  The offsets for the option names in the structure
 * are generated automagically from the structure initializing them in
 * ex_data.c... see the shell script "makeoptions".
 */
struct	option {
	char	*oname;
	char	*oabbrev;
	int	otype;		/* Types -- see below */
	int	odefault;	/* Default value */
	int	ovalue;		/* Current value */
	char	*osvalue;
};

#define	ONOFF	0
#define	NUMERIC	1
#define	STRING	2		/* SHELL or DIRECTORY */
#define	OTERM	3

#define	value(a)	options[a].ovalue
#define	svalue(a)	options[a].osvalue

extern	 struct	option options[NOPTS + 1];

#ifdef vms
#define	st_blksize	st_fab_mrs
#define	_exit(n)	vms_exit(n)
#define	fork()		vfork()
#endif

/*
 * The editor does not normally use the standard i/o library.  Because
 * we expect the editor to be a heavily used program and because it
 * does a substantial amount of input/output processing it is appropriate
 * for it to call low level read/write primitives directly.  In fact,
 * when debugging the editor we use the standard i/o library.  In any
 * case the editor needs a printf which prints through "putchar" ala the
 * old version 6 printf.  Thus we normally steal a copy of the "printf.c"
 * and "strout" code from the standard i/o library and mung it for our
 * purposes to avoid dragging in the stdio library headers, etc if we
 * are not debugging.  Such a modified printf exists in "printf.c" here.
 */
#ifdef TRACE
	var	FILE	*trace;
	var	bool	trubble;
	var	bool	techoin;
	var	char	tracbuf[BUFSIZ];
#endif

/*
 * Character constants and bits
 *
 * The editor uses the QUOTE bit as a flag to pass on with characters
 * e.g. to the putchar routine.  The editor never uses a simple char variable.
 * Only arrays of and pointers to characters are used and parameters and
 * registers are never declared character.
 */
#ifdef BIT8
# define	QUOTE	0400
#else
# define	QUOTE	0200
#endif
#define	RE_QUOTE	0200
#define	TRIM	0177
#ifndef vms
#undef CTRL
#endif
#define	CTRL(c)	(c & 037)
#define	NL	CTRL('j')
#define	CR	CTRL('m')
#define	DELETE	0177		/* See also ATTN, QUIT in ex_tune.h */
#define	ESCAPE	033

/*
 * Miscellaneous random variables used in more than one place
 */
var	bool	aiflag;		/* Append/change/insert with autoindent */
var	bool	anymarks;	/* We have used '[a-z] */
var	int	chng;		/* Warn "No write" */
var	char	*Command;
var	short	defwind;	/* -w# change default window size */
var	int	dirtcnt;	/* When >= MAXDIRT, should sync temporary */
#ifdef SIGTSTP
var	bool	dosusp;		/* Do SIGTSTP in visual when ^Z typed */
#endif
var	bool	edited;		/* Current file is [Edited] */
var	line	*endcore;	/* Last available core location */
extern	 bool	endline;	/* Last cmd mode command ended with \n */
#ifdef EXSTRINGS
var	short	erfile;		/* Error message file unit */
#endif
var	line	*fendcore;	/* First address in line pointer space */
var	char	file[FNSIZE];	/* Working file name */
var	char	genbuf[LBSIZE]; /* Working buffer when manipulating linebuf */
var	bool	hush;		/* Command line option - was given, hush up! */
var	char	*globp;		/* (Untyped) input string to command mode */
var	bool	holdcm;		/* Don't cursor address */
var	bool	inappend;	/* in ex command append mode */
var	bool	inglobal;	/* Inside g//... or v//... */
var	char	*initev;	/* Initial : escape for visual */
var	bool	inopen;		/* Inside open or visual */
var	char	*input;		/* Current position in cmd line input buffer */
var	bool	intty;		/* Input is a tty */
var	short	io;		/* General i/o unit (auto-closed on error!) */
extern	 short	lastc;		/* Last character ret'd from cmd input */
var	bool	laste;		/* Last command was an "e" (or "rec") */
var	char	lastmac;	/* Last macro called for ** */
var	char	lasttag[TAGSIZE];	/* Last argument to a tag command */
var	char	*linebp;	/* Used in substituting in \n */
var	char	linebuf[LBSIZE];	/* The primary line buffer */
var	bool	listf;		/* Command should run in list mode */
var	char	*loc1;		/* Where re began to match (in linebuf) */
var	char	*loc2;		/* First char after re match (") */
var	line	names['z'-'a'+2];	/* Mark registers a-z,' */
var	int	notecnt;	/* Count for notify (to visual from cmd) */
var	bool	numberf;	/* Command should run in number mode */
var	char	obuf[BUFSIZ];	/* Buffer for tty output */
var	short	oprompt;	/* Saved during source */
var	speed_t	ex_ospeed;		/* Output speed (from gtty) */
var	int	otchng;		/* Backup tchng to find changes in macros */
var	short	peekc;		/* Peek ahead character (cmd mode input) */
var	char	*pkill[2];	/* Trim for put with ragged (LISP) delete */
var	bool	pfast;		/* Have stty -nl'ed to go faster */
var	int	pid;		/* Process id of child */
var	int	ppid;		/* Process id of parent (e.g. main ex proc) */
var	jmp_buf	resetlab;	/* For error throws to top level (cmd mode) */
var	int	rpid;		/* Pid returned from wait() */
var	bool	ruptible;	/* Interruptible is normal state */
var	bool	seenprompt;	/* 1 if have gotten user input */
var	bool	shudclob;	/* Have a prompt to clobber (e.g. on ^D) */
var	int	status;		/* Status returned from wait() */
var	int	tchng;		/* If nonzero, then [Modified] */
extern	int	tfile;		/* Temporary file unit */
var	bool	vcatch;		/* Want to catch an error (open/visual) */
var	jmp_buf	vreslab;	/* For error throws to a visual catch */
var	bool	writing;	/* 1 if in middle of a file write */
var	int	xchng;		/* Suppresses multiple "No writes" in !cmd */
var	long	bsize;		/* Block size for disk i/o */

/*
 * Macros
 */
#define	CP(a, b)	memmove(a, b, strlen(b) + 1)
			/*
			 * FIXUNDO: do we want to mung undo vars?
			 * Usually yes unless in a macro or global.
			 */
#define FIXUNDO		(inopen >= 0 && (inopen || !inglobal))
#define ckaw()		{if (chng && value(AUTOWRITE)) wop(0);}
#define	copy(a,b,c)	Copy((char *) a, (char *) b, c)
#define	eq(a, b)	((void *)(a) != NULL && (void *)(b) != NULL && strcmp(a, b) == 0)
#define	getexit(a)	copy(a, resetlab, sizeof (jmp_buf))
#define	lastchar()	lastc
#define	outchar(c)	(*Outchar)(c)
#define	pastwh()	(ignore(skipwh()))
#define	pline(no)	(*Pline)(no)
#define	reset()		longjmp(resetlab,1)
#define	resexit(a)	copy(resetlab, a, sizeof (jmp_buf))
#define	setexit()	setjmp(resetlab)
#define	setlastchar(c)	lastc = c
#define	ungetchar(c)	peekc = c

#define	CATCH		vcatch = 1; if (setjmp(vreslab) == 0) {
#define	ONERR		} else { vcatch = 0;
#define	ENDCATCH	} vcatch = 0;

/*
 * Environment like memory
 */
var	char	altfile[FNSIZE];	/* Alternate file name */
extern	 char	direct[ONMSZ];		/* Temp file goes here */
extern	 char	shell[ONMSZ];		/* Copied to be settable */
extern	 char	ex_ttytype[ONMSZ];		/* A long and pretty name */
var	char	uxb[UXBSIZE + 2];	/* Last !command for !! */

/*
 * The editor data structure for accessing the current file consists
 * of an incore array of pointers into the temporary file tfile.
 * Each pointer is 15 bits (the low bit is used by global) and is
 * padded with zeroes to make an index into the temp file where the
 * actual text of the line is stored.
 *
 * To effect undo, copies of affected lines are saved after the last
 * line considered to be in the buffer, between dol and unddol.
 * During an open or visual, which uses the command mode undo between
 * dol and unddol, a copy of the entire, pre-command buffer state
 * is saved between unddol and truedol.
 */
var	line	*addr1;			/* First addressed line in a command */
var	line	*addr2;			/* Second addressed line */
var	line	*dol;			/* Last line in buffer */
var	line	*dot;			/* Current line */
var	line	*one;			/* First line */
var	line	*truedol;		/* End of all lines, including saves */
var	line	*unddol;		/* End of undo saved lines */
var	line	*zero;			/* Points to empty slot before one */

/*
 * Undo information
 *
 * For most commands we save lines changed by salting them away between
 * dol and unddol before they are changed (i.e. we save the descriptors
 * into the temp file tfile which is never garbage collected).  The
 * lines put here go back after unddel, and to complete the undo
 * we delete the lines [undap1,undap2).
 *
 * Undoing a move is much easier and we treat this as a special case.
 * Similarly undoing a "put" is a special case for although there
 * are lines saved between dol and unddol we don't stick these back
 * into the buffer.
 */
var	short	undkind;

var	line	*unddel;	/* Saved deleted lines go after here */
var	line	*undap1;	/* Beginning of new lines */
var	line	*undap2;	/* New lines end before undap2 */
var	line	*undadot;	/* If we saved all lines, dot reverts here */

#define	UNDCHANGE	0
#define	UNDMOVE		1
#define	UNDALL		2
#define	UNDNONE		3
#define	UNDPUT		4

#ifdef CRYPT
/*
 * Various miscellaneous flags and buffers needed by the encryption routines.
 */
#define	KSIZE   9       /* key size for encryption */
#define	KEYPROMPT       "Key: "
var	int	xflag;		/* True if we are in encryption mode */
var	int	xtflag;		/* True if the temp file is being encrypted */
var	int	kflag;		/* True if the key has been accepted */
var	char	perm[768];
var	char	tperm[768];
var	char	*key;
var	char	crbuf[CRSIZE];
char	*getpass();
#endif

/*
 * Function type definitions
 */
#define	NOSTR	(char *) 0
#define	NOLINE	(line *) 0

extern	void	(*Outchar)();
extern	void	(*Pline)();
extern	void	(*Put_char)();
struct	sigaction	oldhup;
void	(*setlist(bool))();
int	(*setnorm())();
int	(*setnorm())();
void	(*setnumb(bool))();
line	*address(char *);
char	*genindent(int);
char	*getblock(line, int);
#ifdef	vms
char	*getlog();
#endif
line	*getmark(int);
char	*mesg(char *);
char	*place(char *, char *, char *);
char	*plural(long);
void	setin(line *);
char	*strend(char *);
line	*vback(line *, int);
char	*vfindcol(int);
char	*vgetline(int, char *, bool *, int);
char	*vpastwh(char *);
char	*vskipwh(char *);
void	put(void);
void	putreg(int);
void	YANKreg(int);
void	ex_delete(bool);
void	filter(int);
int	getfile(void);
int	getsub(void);
int	gettty(void);
void	join(int);
void	listchar(int);
void	normline(void);
void	numbline(int);
var	void (*oldquit)();
void	onhup(int);
void	onintr(int);
void	onsusp(int);
int	putch(int);
void	shift(int, int);
void	termchar(int);
void	vfilter(void);
#ifdef CBREAK
void	vintr(int);
#endif
int	vputch(int);
void	vshftop(void);
void	yank(void);
void	setall(void);
void	setcount(void);
void	commands(bool, bool);
void	vcontin(bool);
void	resetflav(void);
void	nomore(void);
void	ex_newline(void);
void	mapcmd(int, int);
void	zop2(int, int);
void	tagfind(bool);
void	squish(void);
void	source(char *, bool);
void	putfile(int);
void	rop(int);
void	filename(int);
void	pstop(void);
void	pstart(void);
void	set(void);
void	merror(char *);
void	imerror(char *, int);
void	smerror(char *, char *);
void	error(char *);
void	ierror(char *, int);
void	serror(char *, char *);
void	reverse(line *, line *);
void	netchange(int);
void	killcnt(int);
void	synctmp(void);
void	fileinit(void);
void	gettmode(void);
void	vsetsiz(int);
void	savevis(void);
void	vop(void);
void	vdirty(int, int);
void	sethard(void);
void	vreplace(int, int, int);
void	vsync1(int);
void	vredraw(int);
void	vrepaint(char *);
void	vscrap(void);
void	vmoveitup(int, bool);
void	macpush(char *, int);
void	addtext(char *);
void	setLAST(void);
void	vsave(void);
void	vmain(void);
void	operate(int, int);
void	vrep(int);
void	vmacchng(bool);
void	ex_vUndo(void);
void	vappend(int, int, int);
void	takeout(char *);
void	physdc(int, int);
void	vgoto(int, int);
void	vclrech(bool);
void	vclreol(void);
void	vshow(line *, line *);
void	vdown(int, int, bool);
void	vup(int, int, bool);
void	ex_printf(char *, ...);
void	ex_vprintf(char *, va_list);
void	lprintf(char *, ...);
void	eend(void (*)(void));
void	wrerror(void);
void	clrstats(void);
void	ex_getline(line);
void	syserror(void);
void	flush(void);
void	flush1(void);
void	fgoto(void);
void	ex_tab(int);
void	noteinp(void);
void	termreset(void);
void	draino(void);
void	flusho(void);
void	putnl(void);
void	putpad(char *);
void	setoutt(void);
void	putNFL(void);
void	tostart(void);
void	tostop(void);
void	ex_gTTY(int);
void	noonl(void);
void	ex_putchar(int);
int	any(int, char *);
int	backtab(int);
void	change(void);
int	column(char *);
void	comment(void);
void	Copy(char *, char *, ssize_t);
void	copyw(line *, line *, int);
void	copywR(line *, line *, int);
int	ctlof(int);
void	dingdong(void);
int	fixindent(int);
void	filioerr(char *);
void	getDOT(void);
int	getn(char *);
void	ignnEOF(void);
int	iswhite(int);
int	junk(int);
void	killed(void);
int	lineno(line *);
int	lineDOL(void);
int	lineDOT(void);
void	markpr(line *);
int	markreg(int);
int	morelines(void);
void	nonzero(void);
int	notable(int);
void	notempty(void);
void	netchHAD(int);
void	putmark(line *);
void	putmk1(line *, int);
int	qcolumn(char *, char *);
void	save12(void);
void	saveall(void);
int	span(void);
void	ex_sync(void);
int	skipwh(void);
void	strcLIN(char *);
int	tabcol(int, int);
int	whitecnt(char *);
void	markit(line *);
void	setrupt(void);
int	preserve(void);
void	ex_exit(int);
int	append(int (*)(), line *);
void	appendnone(void);
void	pargs(void);
void	deletenone(void);
void	move(void);
void	pragged(bool);
void	zop(int);
void	plines(line *, line *, bool);
void	pofix(void);
void	undo(bool);
void	cmdmac(char);
void	oop(void);
void	winch(int);
void	cleanup(bool);
line	putline(void);
void	tlaste(void);
void	tflush(void);
int	partreg(int);
void	notpart(int);
void	regbuf(int, char *, int);
void	unix0(bool);
void	recover(void);
void	waitfor(void);
void	revocer(void);
void	markDOT(void);
int	cmdreg(void);
int	endcmd(int);
void	eol(void);
void	erewind(void);
void	fixol(void);
int	exclam(void);
void	makargs(void);
void	next(void);
int	quickly(void);
int	skipend(void);
void	tailspec(int);
void	tail(char *);
void	tail2of(char *);
void	tailprim(char *, int, bool);
void	vnfl(void);
int	getargs(void);
void	getone(void);
void	rop2(void);
void	rop3(int);
void	wop(bool);
void	TSYNC(void);
void	setdot(void);
void	setdot1(void);
int	getnum(void);
void	setnoaddr(void);
void	setCNL(void);
void	setNAEOL(void);
void	ignchar(void);
int	ex_getchar(void);
int	getcd(void);
int	peekchar(void);
int	peekcd(void);
int	topen(char *, char *);
int	tseek(int, long);
int	tgets(char *, int, int);
void	tclose(int);
void	init(void);

/*
 * C doesn't have a (void) cast, so we have to fake it for lint's sake.
 */
#ifdef lint
#	define	ignore(a)	Ignore((char *) (a))
#	define	ignorf(a)	Ignorf((int (*) ()) (a))
#else
#	define	ignore(a)	a
#	define	ignorf(a)	a
#endif
