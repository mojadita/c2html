/* $Id: multifree.h,v 1.1 2000/07/16 21:51:06 luis Exp $
 * Author: Luis Colorado <Luis.Colorado@SLUG.CTV.ES>
 * Date: Fri Nov 20 22:23:54 MET 1998
 */

/* Do not include anything BEFORE the line below, as it would not be
 * protected against double inclusion from other files
 */
#ifndef MULTIFREE_H
#define MULTIFREE_H
/* constants */
#define SIZE 1023

/* types */

/* THESE ARE OPAQUE TYPES, DON'T USE INTERNAL STRUCTURE */
typedef struct _chunk {
  struct _chunk *next;
  void *list[SIZE];
} chunk;

typedef struct mf_data {
  chunk *mf_cl;
  int mf_n;
} mf_t;

/* prototypes */
void mf_init (mf_t *);
void *mf_register(mf_t *, void *);
void mf_apply(mf_t *, void (*)(void *));
void mf_free(mf_t *);

#endif /* MULTIFREE_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */

/* $Id: multifree.h,v 1.1 2000/07/16 21:51:06 luis Exp $ */
