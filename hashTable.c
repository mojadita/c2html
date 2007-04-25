/* $Id: hashTable.c,v 1.1 2007/04/25 22:52:43 luis Exp $
 * Author: Luis Colorado <Luis.Colorado@SLUG.CTV.ES>
 * Date: Fri Mar 12 19:47:53 MET 1999
 */

#define IN_HASHTABLE_C

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "hashCRC12.h"
#include "multifree.h"
#include "hashTable.h" /* multifree.h must be included before this one */

/* constants */

/* types */

/* prototypes */

/* variables */

/* functions */

HashTable *hashTableInit(HashTable *p)
{
  int i;

  if (!p)
    p = malloc(sizeof (HashTable));
  if (!p) {
    perror ("hashTableInit: malloc");
    return NULL;
  }
  mf_init (&(p->mf_data));
  /* Initialize the entries */
  for (i = 0; i < HASHSIZE; i++)
    p->hashEntries[i] = NULL;
  return p;
} /* hashTableInit */

/* This function obtains an entry for the key in HashTable t.
 * It gets an empty entry, if the key is not there yet, and the
 * proper one, if it exists in the table, so we can use this
 * function in the Perm and the no-Perm versions of the lookup function.
 */
static HashEntry *_hashTableLookup (HashTable *t, char *key)
{
  ChunkOfEntries **chunkPtrRef;
  HashEntry *lastEmpty = NULL;

  /* Normally, we'll have only one chunk of entries */
  for (chunkPtrRef = t->hashEntries + hashCRC12(key);
       *chunkPtrRef;
       chunkPtrRef = &((*chunkPtrRef)->next))
  {
    HashEntry *p;
    register int i;

    /* look every entry in this chunk */
    for (i = 0, p = (*chunkPtrRef)->theEntries;
         i < ENTRIESPERCHUNK;
	 i++, p++)
    {
      if (p->key && !strcmp(p->key, key)) {
        /* found */
	return p;
      }
      if (!lastEmpty && !p->key)
        lastEmpty = p;
    } /* for (i) */
    /* go for the next chunk */
  } /* for */
  if (!lastEmpty) {
    ChunkOfEntries *newChunk;
    int i;

    /* there was not an empty entry, so we must allocate one */
    newChunk = mf_register(
      &(t->mf_data),
      malloc(sizeof(ChunkOfEntries)));
    if (!newChunk) {
      perror ("hashTableLookup: malloc");
      return NULL;
    }
    /* insert into the list */
    newChunk->next = *chunkPtrRef;
    *chunkPtrRef = newChunk;

    /* initialize the entries */
    for (i = 0; i < ENTRIESPERCHUNK; i++) {
      newChunk->theEntries[i].key = NULL;
      newChunk->theEntries[i].data = NULL;
    }
    lastEmpty = newChunk->theEntries;
  } /* if (!lastEmpty) */
  return lastEmpty;
} /* _hashTableLookup */

HashEntry *hashTablePermLookup (HashTable *t, char *key)
{
  HashEntry *result;
  result = _hashTableLookup(t, key);
  if (!(result->key))
    result->key = key;
  return result;
} /* hashTablePermLookup */

HashEntry *hashTableLookup (HashTable *t, char *key)
{
  HashEntry *result;
  result = _hashTableLookup(t, key);
  if (!(result->key)) {
    result->key = mf_register(
      &(t->mf_data),
      (void *) strdup(key));
  }
  return result;
} /* hashTableLookup */

void hashTableFree (HashTable *t)
{
  extern void free(void *);

  mf_apply (&(t->mf_data), free);
  mf_free (&(t->mf_data));
} /* hashTableFree */

/* $Id: hashTable.c,v 1.1 2007/04/25 22:52:43 luis Exp $ */
