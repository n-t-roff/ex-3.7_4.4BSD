/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 *
 *	@(#)ex_argv.h	8.1 (Berkeley) 6/9/93
 */

/*
 * The current implementation of the argument list is poor,
 * using an argv even for internally done "next" commands.
 * It is not hard to see that this is restrictive and a waste of
 * space.  The statically allocated glob structure could be replaced
 * by a dynamically allocated argument area space.
 */
var char	**argv;
var char	**argv0;
var char	*args;
var char	*args0;
var int	argc;
var int	argc0;
var int	morargc;		/* Used with "More files to edit..." */

var int	firstln;		/* From +lineno */
var char	*firstpat;		/* From +/pat	*/

/* Yech... */
struct	glob {
	int	argc;			/* Index of current file in argv */
	int	argc0;			/* Number of arguments in argv */
	char	*argv[NARGS + 1];	/* WHAT A WASTE! */
	char	argspac[NCARGS + sizeof (int)];
};
var struct	glob frob;

void glob(struct glob *);
