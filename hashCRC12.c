/* $Id: hashCRC12.c,v 1.2 2002/11/09 18:59:06 luis Exp $
 * Author: Luis Colorado <Luis.Colorado@SLUG.CTV.ES>
 * Date: Fri Mar 12 20:27:22 MET 1999
 */

#define IN_HASHCRC12_C

static char rcsId [] = "\n$Id: hashCRC12.c,v 1.2 2002/11/09 18:59:06 luis Exp $\n";

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

/* constants */

/* types */

/* prototypes */

/* variables */
extern u_long crc12Table [];

/* functions */
u_long hashCRC12 (u_char *s)
{
  u_long result = 0U;
  static u_char aux [] = { 0, 0 };

  calcCRC (&result, s, strlen(s), crc12Table);
  calcCRC (&result, aux, 2, crc12Table);
  return result;
} /* hashCRC12 */

/* $Id: hashCRC12.c,v 1.2 2002/11/09 18:59:06 luis Exp $ */
