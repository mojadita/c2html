/* lexical.h -- function prototypes for functions defined in lexical.l
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date:
 * Copyright: (C) 2022 Luis Colorado.  All rights reserved.
 * License: BSD.
 */
#ifndef _LEXICAL_H
#define _LEXICAL_H

void scanfile(const node *);
void create_menu(tag_menu *m);
void newline(int do_reset);

#endif /* _LEXICAL_H */
