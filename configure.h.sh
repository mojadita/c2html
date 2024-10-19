cat <<EOF
/* configure.h -- configuration options for package.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Fri Oct 11 08:50:06 EEST 2024
 * Copyright: (c) 2024 Luis Colorado.  All rights reserved.
 */
#ifndef _CONFIGURE_H
#define _CONFIGURE_H

EOF

# generate a list of sed(1) options to 
sed -e 's/\([A-Za-z_][A-Za-z0-9_]*\)[	 ]*?=[	 ]*\(.*\)/#define \1 @\1@/' \
	< make-options.mk


cat <<EOF

#endif /* _CONFIGURE_H */
EOF
