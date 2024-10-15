/* lexical.h -- function prototypes for functions defined in lexical.l
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Thu Oct 10 09:11:00 EEST 2024
 * Copyright: (C) 2022-2024 Luis Colorado.  All rights reserved.
 * License: BSD.
 */
#ifndef _LEXICAL_H
#define _LEXICAL_H

#include "menu.h"

void scanfile(const node *);
void create_menu(tag_menu *m);
void newline(int do_reset);

#endif /* _LEXICAL_H */
