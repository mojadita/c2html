/* $Id: calcCRC.c,v 1.1 2000/07/16 21:53:04 luis Exp $
 * Author: Luis Colorado <Luis.Colorado@SLUG.CTV.ES>
 * Date: Mon Mar  8 20:39:19 MET 1999
 */

#define IN_CALCCRC_C

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

/* constants */

/* types */

/* prototypes */

/* variables */

/* functions */

void calcCRC (u_long *crc, u_char *b, size_t n, u_long *t)
{
  while (n--) {
    size_t i = (*crc) & 0xff;
    (*crc) >>= 8;
    (*crc) ^= t[i];
    (*crc) ^= (u_long)(*b++) << 8;
  } /* while */
} /* calcCRC */

/* $Id: calcCRC.c,v 1.1 2000/07/16 21:53:04 luis Exp $ */
