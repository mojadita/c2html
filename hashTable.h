/* $Id: hashTable.h,v 1.1 2007/04/25 22:52:43 luis Exp $
 * Author: Luis Colorado <Luis.Colorado@SLUG.CTV.ES>
 * Date: Fri Mar 12 23:50:43 MET 1999
 */

/* Do not include anything BEFORE the line below, as it would not be
 * protected against double inclusion from other files
 */
#ifndef HASHTABLE_H
#define HASHTABLE_H
/* constants */

#define HASHSIZE	4096  /* 2 ** 12 */
#define ENTRIESPERCHUNK	16

/* types */
typedef struct _HashEntry {
  char *key;
  void *data;
} HashEntry;

typedef struct _ChunkOfEntries {
  struct _ChunkOfEntries *next;
  HashEntry theEntries [ENTRIESPERCHUNK];
} ChunkOfEntries;

typedef struct _HashTable {
  mf_t mf_data;
  ChunkOfEntries *hashEntries[HASHSIZE];
} HashTable;

/* prototypes */
HashTable *hashTableInit(HashTable *);
HashEntry *hashTableLookup(HashTable *, char *);
HashEntry *hashTablePermLookup(HashTable *, char *);
void hashTableFree(HashTable *);

/* variables */

/* functions */

#endif /* HASHTABLE_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */

/* $Id: hashTable.h,v 1.1 2007/04/25 22:52:43 luis Exp $ */
