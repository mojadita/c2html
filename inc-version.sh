#!/bin/sh
# inc-version.sh -- version incrementer
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Fri Oct 18 13:46:06 EEST 2024
# Copyright: (c) 2024 Luis Colorado.  All rights reserved.

# get the date.
DATE=$(date "+%Y.%m.%d")

export OLD_VERSION=$(make version)
export NEW_VERSION=$(echo "$OLD_VERSION" \
| sed -e 's/[.-]/ /g' \
| awk '{ printf("%d.%d.%d-%s\n", $1, $2, $3 + 1, "'"${DATE}"'"); }')
ed Makefile <<EOF
/^\(VERSION[	 ][	 ]*?=\).*/s//\1 ${NEW_VERSION} # PREV: ${OLD_VERSION}/
w
q
EOF
