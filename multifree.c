/* $Id: multifree.c,v 1.1 2000/07/16 21:51:06 luis Exp $
 * Author: Luis Colorado <Luis.Colorado@SLUG.CTV.ES>
 * Date: Fri Nov 20 21:54:30 MET 1998
 */

#define IN_MULTIFREE_C

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "multifree.h"

/* constants */

/* types */
/* prototypes */

/* variables */

/* functions */

void mf_init (mf_t *p)
{
  p->mf_cl = NULL;
  p->mf_n = 0;
} /* mf_init */

void *mf_register(mf_t *s, void *p)
{
  if (!p)
    return NULL;  /* Nothing to register */

  /* if we don't have anything or have a full chunk */
  if ((s->mf_n % SIZE) == 0) {
    register chunk *q;

    q = malloc(sizeof (chunk));
    if (!q) {
      perror ("mf_register: malloc");
      return NULL;  /* malloc(3) failed, no more memory */
    }
    q->next = s->mf_cl;
    s->mf_cl = q;
    s->mf_n = 0;  /* an empty one */
  }
  s->mf_cl->list[s->mf_n++] = p;
  return p;  /* return the registered value */
} /* mf_register */

void mf_apply (mf_t *s, void (*f)(void *))
{
  register chunk *q;

  for (q = s->mf_cl; q; q = q->next) { /* once on every chunk */
    register int i, n;
    n = (q == s->mf_cl) ? s->mf_n : SIZE;

    for (i = 0; i < n; i++)
      f(q->list[i]);

  } /* for(q) */
} /* mf_apply */

void mf_free(mf_t *s)
{
  while (s->mf_cl) {  /* once on every chunk */
    register chunk *q = s->mf_cl;

    s->mf_cl = s->mf_cl->next;
    free(q);
  }
  /* mf_cl == NULL
   * no more chunks, set mf_n */
  s->mf_n = 0;
} /* mf_free */

/* $Id: multifree.c,v 1.1 2000/07/16 21:51:06 luis Exp $ */
