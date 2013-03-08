#!/bin/sh

set -x

if test "`uname`" = "Darwin"; then
LIBTOOLIZE=glibtoolize
else
LIBTOOLIZE=libtoolize
fi

aclocal -I m4 \
&& ${LIBTOOLIZE} --automake --force --copy \
&& autoheader \
&& automake --gnu --add-missing --force --copy \
&& autoconf

