#!/bin/sh
# sed-options.sh -- generate a list of sed(1) options to 
sed -e 's/\([A-Za-z_][A-Za-z0-9_]*\).*/ -e '\''s"@\1@"$(\1)"g'\''/' \
| tr '\n' ' '
echo
