#!/bin/bash
srcdir=$(dirname $(readlink -f $0))
if [[ "$srcdir" != "$PWD" ]]; then
	rm -rf *
else
	echo "Do not run this script in source tree!" >&2
	exit 1
fi
cmake "$srcdir" \
	-DCMAKE_INSTALL_PREFIX=/usr \
	-DCMAKE_BUILD_TYPE=DEBUG
