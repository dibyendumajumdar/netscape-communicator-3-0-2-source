/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char RCSid[] = 
"$Id: rdist.c,v 1.2 1996/04/24 00:12:42 dkarlton Exp $";

static char sccsid[] = "@(#)main.c	5.1 (Berkeley) 6/6/85";

static char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */


#include "defs.h"
#include <netdb.h>
#include <sys/ioctl.h>
#ifdef USE_SSL
#include <termios.h>
#include <stdlib.h>
#endif /* USE_SSL */
/*
 * Remote distribution program.
 */

char   	       *distfile = NULL;		/* Name of distfile to use */
int     	maxchildren = MAXCHILDREN;	/* Max no of concurrent PIDs */
int		nflag = 0;			/* Say without doing */
long		min_freespace = 0;		/* Min filesys free space */
long		min_freefiles = 0;		/* Min filesys free # files */
FILE   	       *fin = NULL;			/* Input file pointer */
struct group   *gr = NULL;			/* Static area for getgrent */
char		localmsglist[] = "stdout=all:notify=all:syslog=nerror,ferror";
char   	       *remotemsglist = NULL;
#ifdef USE_SSL
char		optchars[] = "A:a:bcd:DFf:hiK:k:l:L:M:m:NnOo:p:P:qRrS:st:Vvwxy";
#else /* USE_SSL */
char		optchars[] = "A:a:bcd:DFf:hil:L:M:m:NnOo:p:P:qRrst:Vvwxy";
#endif /* USE_SSL */
FILE   	       *opendist();
char	       *path_rdistd = _PATH_RDISTD;
char	       *path_remsh = _PATH_REMSH;

#ifdef USE_SSL
char	       *secargs = 0; 	/* default to not use ssl rsh */
int		keyflag = 0;	/* defaults to no client auth */
char	       *keydir = 0;	/* no key directory */
char 	       *password = 0;
char	       *nickname = 0;   /* name for key and cert, for client auth */
int             dongleflag = 0;

static void echoOff(int fd)
{
    if (isatty(fd)) {
        struct termios tio;
        tcgetattr(fd, &tio);
        tio.c_lflag &= ~ECHO;
        tcsetattr(fd, TCSAFLUSH, &tio);
    }
}

static void echoOn(int fd)
{
    if (isatty(fd)) {
        struct termios tio;
        tcgetattr(fd, &tio);
        tio.c_lflag |= ECHO;
        tcsetattr(fd, TCSAFLUSH, &tio);
    }
}

static char *getPassword(char *prompt)
{
    FILE *in, *out;
    char buf[2000];

    in = fopen("/dev/tty", "r");
    out = fopen("/dev/tty", "w");
    echoOff(fileno(out));
    for (;;) {
        fprintf(out, prompt);
        fflush(out);
        buf[0] = 0;
        fgets(buf, sizeof(buf), in);
        fprintf(out, "\n");
        if (buf[0]) break;
    }
    echoOn(fileno(out));
    fclose(out);
    fclose(in);
    return strdup(buf);
}

#endif /* USE_SSL */

/*
 * Add a hostname to the host list
 */
static void addhostlist(name, hostlist)
	char *name;
	struct namelist **hostlist;
{
	register struct namelist *ptr, *new;

	if (!name || !hostlist)
		return;

	new = (struct namelist *) xmalloc(sizeof(struct namelist));
	new->n_name = strdup(name);
	new->n_next = NULL;

	if (*hostlist) {
		for (ptr = *hostlist; ptr && ptr->n_next; ptr = ptr->n_next)
			;
		ptr->n_next = new;
	} else
		*hostlist = new;
}

main(argc, argv, envp)
	int argc;
	char *argv[];
	char **envp;
{
	struct namelist *hostlist = NULL;
	register int x;
	register char *cp;
	int cmdargs = 0;
	int c;
	int path_set = 0;
	
	/*
	 * We initialize progname here instead of init() because
	 * things in msgparseopts() need progname set.
	 */
	setprogname(argv);

	if (cp = msgparseopts(localmsglist, TRUE)) {
		error("Bad builtin log option (%s): %s.", 
		      localmsglist, cp);
		usage();
	}

	if (init(argc, argv, envp) < 0)
		exit(1);

	/*
	 * Be backwards compatible.
	 */
	for (x = 1; x <= argc && argv[x]; x++) {
		if (strcmp(argv[x], "-Server") != 0)
			continue;
#if	defined(_PATH_OLDRDIST)
		message(MT_SYSLOG, 
			"Old rdist (-Server) requested; running %s", 
			_PATH_OLDRDIST);
		(void) execl(_PATH_OLDRDIST, basename(_PATH_OLDRDIST), 
			     "-Server", (char *)NULL);
		fatalerr("Exec old rdist failed: %s: %s.", 
			 _PATH_OLDRDIST, SYSERR);
#else	/* !_PATH_OLDRDIST */
		fatalerr("Old rdist not available.");
#endif	/* _PATH_OLDRDIST */
		exit(1);
	}

#if	defined(DIRECT_RCMD)
	if (becomeuser() != 0)
		exit(1);
#else	/* !DIRECT_RCMD */
	/*
	 * Perform check to make sure we are not incorrectly installed
	 * setuid to root or anybody else.
	 */
	if (getuid() != geteuid())
		fatalerr("This version of rdist should not be installed setuid.");
#endif	/* DIRECT_RCMD */

	while ((c = getopt(argc, argv, optchars)) != -1)
		switch (c) {
#ifdef USE_SSL
		        break;
		case 'k':
			keyflag = 1;
			nickname = strdup(optarg);
			break;
		case 'K':
			keydir = strdup(optarg);
			break;
		case 'S':
			secargs = strdup(optarg);
			if ( !path_set ) {
			    /* replace the path if it has not already been
			     * specified on the command line.
			     */
			    path_remsh = _PATH_SREMSH;
			}
			break;
		    
#endif /* USE_SSL */
		case 'l':
			if (cp = msgparseopts(optarg, TRUE)) {
				error("Bad log option \"%s\": %s.", optarg,cp);
				usage();
			}
			break;

		case 'L':
			remotemsglist = strdup(optarg);
			break;

		case 'A':
		case 'a':
		case 'M':
		case 't':
			if (!isdigit(*optarg)) {
				error("\"%s\" is not a number.", optarg);
				usage();
			}
			if (c == 'a')
				min_freespace = atoi(optarg);
			else if (c == 'A')
				min_freefiles = atoi(optarg);
			else if (c == 'M')
				maxchildren = atoi(optarg);
			else if (c == 't')
				rtimeout = atoi(optarg);
			break;

		case 'F':
			do_fork = FALSE;
			break;

		case 'f':
			distfile = strdup(optarg);
			if (distfile[0] == '-' && distfile[1] == CNULL)
				fin = stdin;
			break;

		case 'm':
			addhostlist(optarg, &hostlist);
			break;

		case 'd':
			define(optarg);
			break;

		case 'D':
			debug = DM_ALL;
			if (cp = msgparseopts("stdout=all,debug", TRUE)) {
				error("Enable debug messages failed: %s.", cp);
				usage();
			}
			break;

		case 'c':
			cmdargs++;
			break;

		case 'n':
			nflag++;
			break;

		case 'V':
			printf("%s\n", getversion());
			exit(0);

		case 'o':
			if (parsedistopts(optarg, &options, TRUE)) {
				error("Bad dist option string \"%s\".", 
				      optarg);
				usage();
			}
			break;

		case 'p':
			if (!optarg) {
				error("No path specified to \"-p\".");
				usage();
			}
			path_rdistd = strdup(optarg);
			break;

		case 'P':
			if (!optarg) {
				error("No path specified to \"-P\".");
				usage();
			}
			if (cp = searchpath(optarg)) {
				path_remsh = strdup(cp);
				path_set = 1;
			} else {
				error("No component of path \"%s\" exists.",
				      optarg);
				usage();
			}
			break;

			/*
			 * These options are obsoleted by -o.  They are
			 * provided only for backwards compatibility
			 */
		case 'v':	FLAG_ON(options, DO_VERIFY);		break;
		case 'N':	FLAG_ON(options, DO_CHKNFS);		break;
		case 'O':	FLAG_ON(options, DO_CHKREADONLY);	break;
		case 'q':	FLAG_ON(options, DO_QUIET);		break;
		case 'b':	FLAG_ON(options, DO_COMPARE);		break;
		case 'r':	FLAG_ON(options, DO_NODESCEND);		break;
		case 'R':	FLAG_ON(options, DO_REMOVE);		break;
		case 's':	FLAG_ON(options, DO_SAVETARGETS);	break;
		case 'w':	FLAG_ON(options, DO_WHOLE);		break;
		case 'y':	FLAG_ON(options, DO_YOUNGER);		break;
		case 'h':	FLAG_ON(options, DO_FOLLOW);		break;
		case 'i':	FLAG_ON(options, DO_IGNLNKS);		break;
		case 'x':	FLAG_ON(options, DO_NOEXEC);		break;

		case '?':
		default:
			usage();
		}

#ifdef USE_SSL
	if (keyflag) {
	    char donglefile[MAXPATHLEN];

	    if (!keydir) {
		keydir = getenv("SSL_DIR");
		if (!keydir) {
		    sprintf(donglefile, "/usr/etc/ssl/dongle");
		}
            } else {
	        sprintf(donglefile, "%s/dongle", keydir);
            }

	    if (access(donglefile, R_OK) == 0) {
		dongleflag = 1;
	    }

	    if (!dongleflag) {
		password = getPassword("Key Password: ");
	    }
	}
#endif /* USE_SSL */
	
	if (debug) {
		printf("%s\n", getversion());
		msgprconfig();
	}

	if (nflag && IS_ON(options, DO_VERIFY))
		fatalerr(
		 "The -n flag and \"verify\" mode may not both be used.");

	/*
	 * Don't fork children for nflag
	 */
	if (nflag)
		do_fork = 0;

	if (cmdargs)
		docmdargs(realargc - optind, &realargv[optind]);
	else {
		if (fin == NULL)
			fin = opendist(distfile);
		(void) yyparse();
		/*
		 * Need to keep stdin open for child processing later
		 */
		if (fin != stdin)
			(void) fclose(fin);
		if (nerrs == 0)
			docmds(hostlist, realargc-optind, &realargv[optind]);
	}

	exit(nerrs != 0);
}

/*
 * Open a distfile
 */
FILE *opendist(distfile)
	char *distfile;
{
	char *file = NULL;
	FILE *fp;

	if (distfile == NULL) {
		if (access("distfile", R_OK) == 0)
			file = "distfile";
		else if (access("Distfile", R_OK) == 0)
			file = "Distfile";
	} else {
		/*
		 * Try to test to see if file is readable before running m4.
		 */
		if (access(distfile, R_OK) != 0)
			fatalerr("%s: Cannot access file: %s.", 
				 distfile, SYSERR);
		file = distfile;
	}

	if (file == NULL)
		fatalerr("No distfile found.");

	fp = fopen(file, "r");

	if (fp == NULL)
		fatalerr("%s: open failed: %s.", file, SYSERR);

	return(fp);
}

/*
 * Print usage message and exit.
 */
usage()
{
	char *sopts = "cDFnv";

	(void) fprintf(stderr,
		      "Usage: %s [-%s] [-A <num>] [-a <num>] [-d var=value]\n",
		       progname, sopts);
	(void) fprintf(stderr, 
       "\t[-f distfile] [-l <msgopt>] [-L <msgopt>] [-M <maxproc>]\n");
	(void) fprintf(stderr, 
       "\t[-m host] [-o <distopts>] [-p <rdistd-cmd>] [-P <rsh-path>]\n");
	(void) fprintf(stderr, 
       "\t[-t <timeout>] [target ...]\n");

	(void) fprintf(stderr,
		      "OR:    %s [-%s] -c source [...] machine[:dest]\n", 
		       progname, sopts);

	(void) fprintf(stderr, "OR:    %s -V\n", progname);

	(void) fprintf(stderr, "\nThe values for <distopts> are:\n\t%s\n",
		       getdistoptlist());

	msgprusage();

	exit(1);
}

/*
 * rcp like interface for distributing files.
 */
docmdargs(nargs, args)
	int nargs;
	char *args[];
{
	register struct namelist *nl, *prev;
	register char *cp;
	struct namelist *files, *hosts;
	struct subcmd *cmds;
	char *dest;
	static struct namelist tnl = { NULL, NULL };
	int i;

	if (nargs < 2)
		usage();

	prev = NULL;
	files = NULL;
	for (i = 0; i < nargs - 1; i++) {
		nl = makenl(args[i]);
		if (prev == NULL)
			files = prev = nl;
		else {
			prev->n_next = nl;
			prev = nl;
		}
	}

	cp = args[i];
	if ((dest = strchr(cp, ':')) != NULL)
		*dest++ = '\0';
	tnl.n_name = cp;
	hosts = expand(&tnl, E_ALL);
	if (nerrs)
		exit(1);

	if (dest == NULL || *dest == '\0')
		cmds = NULL;
	else {
		cmds = makesubcmd(INSTALL);
		cmds->sc_options = options;
		cmds->sc_name = dest;
	}

	debugmsg(DM_MISC, "docmdargs()\nfiles = %s", getnlstr(files));
	debugmsg(DM_MISC, "host = %s", getnlstr(hosts));

	insert((char *)NULL, files, hosts, cmds);
	docmds(0, (char **)NULL, 0, (char **)NULL);
}

/*
 * Get a list of NAME blocks (mostly for debugging).
 */
extern char *getnlstr(nl)
	register struct namelist *nl;
{
	static char buf[16384];
	register int count = 0, len = 0;

	(void) sprintf(buf, "(");

	while (nl != NULL) {
		if (nl->n_name == NULL)
			continue;
		len += strlen(nl->n_name) + 2;
		if (len >= sizeof(buf)) {
			(void) strcpy(buf,
				      "getnlstr() Buffer not large enough");
			return(buf);
		}
		++count;
		(void) strcat(buf, " ");
		(void) strcat(buf, nl->n_name);
		nl = nl->n_next;
	}

	(void) strcat(buf, " )");

	return(buf);
}
