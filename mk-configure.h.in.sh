cat <<EOF
/* configure.h -- definitions for configuration options.
 * Author: @AUTHOR_NAME@ @AUTHOR_EMAIL@
 * Date: @BUILD_DATE@
 * Copyright: (c) @YEAR@ @AUTHOR_NAME@.  All rights reserved.
 */
#ifndef CONFIGURE_H
#define CONFIGURE_H

EOF

sed -e '/^[	 ]*\([A-Za-z_][A-Za-z0-9_]*\).*/s//\1/' \
| awk '
{ if ($0 ~ /^UQ_/)
	printf("#define %-25s @%s@\n", $0, $0);
  else
	printf("#define %-25s \"@%s@\"\n", $0, $0);
}
'

cat <<EOF

#endif /* CONFIGURE_H */
EOF
