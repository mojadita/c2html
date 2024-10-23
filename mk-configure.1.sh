#!/bin/sh
# 

sed -e '/^[	 ]*\([A-Za-z_][A-Za-z0-9_]*\).*/s//\1/' \
| awk '{ printf("%s\"@%s@\n", $0, $0); }'
