
#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H

typedef unsigned long sigset_t;

/* Values for the HOW argument to `sigprocmask'.  */
#define	SIG_BLOCK     0		 /* Block signals.  */
#define	SIG_UNBLOCK   1		 /* Unblock signals.  */
#define	SIG_SETMASK   2		 /* Set the set of blocked signals.  */

extern int sigprocmask (int a, const sigset_t *b, sigset_t *c);

#endif